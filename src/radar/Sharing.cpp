#include "Radar.hpp"
#include "Cloudflared.hpp"
#include <stdexcept>

using json = nlohmann::json;

namespace {
    namespace fs = std::filesystem;

    constexpr auto pollInterval = 50ms;

    void CheckCancelled(const std::atomic<bool>& stopped) {
        if (stopped) throw std::runtime_error("Sharing cancelled.");
    }

    struct CloseHandleDeleter {
        void operator()(HANDLE handle) const {
            if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
        }
    };
    using Handle = std::unique_ptr<void, CloseHandleDeleter>;

    fs::path ApplicationDirectory() {
        std::array<wchar_t, 32768> module{};
        DWORD length = GetModuleFileNameW(nullptr, module.data(), static_cast<DWORD>(module.size()));
        if (!length || length >= module.size())
            throw std::runtime_error("Could not locate the application folder.");
        return fs::path(module.data()).parent_path();
    }


    fs::path FindExecutable(const fs::path& directory) {
        auto executable = radar::FindCloudflared(directory);
        if (executable.empty())
            throw std::runtime_error("cloudflared.exe was not found, next to the program, in Program Files or on PATH. "
                "See docs/radar-sharing.md.");
        return executable;
    }
}

bool Radar::Share() {
    auto& radar = GetInstance();
    if (IsSharing()) return true;
    StopSharing();
    {
        std::lock_guard lock(radar.sharingMutex);
        if (!radar.running) {
            radar.sharing.error = "Enable Browser radar before sharing.";
            return false;
        }
        try {
            FindExecutable(ApplicationDirectory());
        } catch (const std::exception& exception) {
            radar.sharing = { false, {}, {}, exception.what() };
            return false;
        }
        radar.sharing = { true, "Starting sharing...", {}, {} };
    }
    radar.sharingStopped = false;
    try {
        radar.sharingThread = std::thread(&Radar::RunSharing, &radar);
    } catch (const std::exception&) {
        std::lock_guard lock(radar.sharingMutex);
        radar.sharing = { false, {}, {}, "Could not start radar sharing." };
        return false;
    }
    return true;
}

bool Radar::IsSharing() {
    return GetSharingState().active;
}

Radar::SharingState Radar::GetSharingState() {
    auto& radar = GetInstance();
    std::lock_guard lock(radar.sharingMutex);
    return radar.sharing;
}

void Radar::StopSharing() {
    auto& radar = GetInstance();
    radar.sharingStopped = true;
    if (radar.sharingThread.joinable()) radar.sharingThread.join();
    std::lock_guard lock(radar.sharingMutex);
    radar.sharing = {};
}

void Radar::RunSharing() {
    std::string error;
    try {
        auto directory = ApplicationDirectory();
        auto executable = FindExecutable(directory);
        CheckCancelled(sharingStopped);
        {
            std::lock_guard lock(sharingMutex);
            sharing.status = "Starting cloudflared...";
        }
        std::wstring command = L"\"" + executable.wstring() +
            L"\" tunnel --output json --loglevel info --url http://127.0.0.1:27080"
            L" --http-host-header 127.0.0.1:27080";

        Handle job(CreateJobObjectW(nullptr, nullptr));
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if (!job || !SetInformationJobObject(job.get(), JobObjectExtendedLimitInformation, &limits, sizeof(limits)))
            throw std::runtime_error("Could not prepare radar sharing.");

        SECURITY_ATTRIBUTES security{ sizeof(security), nullptr, TRUE };
        HANDLE reader = nullptr, writer = nullptr;
        if (!CreatePipe(&reader, &writer, &security, 0))
            throw std::runtime_error("Could not capture sharing output.");
        Handle output(reader), writeEnd(writer);
        Handle input(CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
            &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
        if (!SetHandleInformation(output.get(), HANDLE_FLAG_INHERIT, 0) || input.get() == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Could not capture sharing output.");
        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdInput = input.get();
        startup.hStdOutput = startup.hStdError = writeEnd.get();
        PROCESS_INFORMATION info{};
        if (!CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, TRUE,
            CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr, directory.c_str(), &startup, &info))
            throw std::runtime_error("Could not start cloudflared.exe. Check your antivirus and see docs/radar-sharing.md.");
        Handle process(info.hProcess), processThread(info.hThread);
        writeEnd.reset();
        input.reset();
        // Assign it before it gets a chance to start anything
        if (!AssignProcessToJobObject(job.get(), process.get()) || ResumeThread(processThread.get()) == static_cast<DWORD>(-1)) {
            TerminateProcess(process.get(), 1);
            throw std::runtime_error("Could not start radar sharing.");
        }
        processThread.reset();
        {
            std::lock_guard lock(sharingMutex);
            sharing.status = "Connecting to Cloudflare...";
        }
        error = ReadSharingOutput(output.get(), process.get());
        // Closing the job also stops any children cloudflared started
        job.reset();
        WaitForSingleObject(process.get(), INFINITE);
    } catch (const std::exception& exception) {
        error = exception.what();
    } catch (...) {
        error = "Could not share the radar.";
    }
    if (error.empty() && !sharingStopped) error = "Sharing stopped. Check your connection and try again.";
    std::lock_guard lock(sharingMutex);
    sharing = { false, {}, {}, sharingStopped ? "" : error };
}

std::string Radar::ReadSharingOutput(HANDLE output, HANDLE process) {
    std::string pending, url, lastError;
    bool connected = false, startupBanner = false;
    auto line = [&](const std::string& entry) {
        auto log = json::parse(entry, nullptr, false);
        if (!log.is_object() || !log.contains("message") || !log["message"].is_string()) return;
        auto text = log["message"].get<std::string>();
        auto level = log.value("level", "");
        if (level == "error") lastError = text.substr(0, 500);
        if (level != "info") return;
        if (text == "Registered tunnel connection") connected = true;
        if (text.find("Your quick Tunnel has been created!") != std::string::npos) startupBanner = true;
        auto first = text.find("https://");
        if (startupBanner && url.empty() && first != std::string::npos) {
            auto end = text.find_first_of(" \t\r\n|", first);
            auto candidate = text.substr(first, end == std::string::npos ? end : end - first);
            const std::string prefix = "https://", suffix = ".trycloudflare.com";
            if (candidate.ends_with(suffix)) {
                auto label = candidate.substr(prefix.size(), candidate.size() - prefix.size() - suffix.size());
                if (!label.empty() && label.size() <= 63 && label.front() != '-' && label.back() != '-' &&
                    label.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-") == std::string::npos)
                    url = candidate;
            }
        }
        std::lock_guard lock(sharingMutex);
        if (connected && !url.empty()) {
            sharing.url = url + "/" + token + "/";
            sharing.status = "Sharing radar";
            sharing.error.clear();
        } else if (connected) {
            sharing.error = "Cloudflare connected, but its share link could not be read. Stop sharing and try again.";
        }
    };
    std::array<char, 2048> buffer{};
    while (!sharingStopped) {
        DWORD available = 0;
        if (!PeekNamedPipe(output, nullptr, 0, nullptr, &available, nullptr)) break;
        if (!available) {
            if (WaitForSingleObject(process, static_cast<DWORD>(pollInterval.count())) != WAIT_TIMEOUT &&
                (!PeekNamedPipe(output, nullptr, 0, nullptr, &available, nullptr) || !available)) break;
            continue;
        }
        DWORD count = 0;
        if (!ReadFile(output, buffer.data(), static_cast<DWORD>(std::min<size_t>(available, buffer.size())), &count, nullptr) || !count)
            break;
        pending.append(buffer.data(), count);
        size_t end;
        while ((end = pending.find('\n')) != std::string::npos) {
            line(pending.substr(0, end));
            pending.erase(0, end + 1);
        }
        if (pending.size() > 16384) pending.clear();
    }
    if (!pending.empty()) line(pending);
    return lastError.empty() ? "Sharing stopped. Check your connection and try again." : lastError;
}
