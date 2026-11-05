// Most of this file was written with AI assistance

#include "Radar.hpp"
#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif
#include <curl/curl.h>
#include <sstream>
#include <bcrypt.h>
#include <stdexcept>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "crypt32.lib")

using json = nlohmann::json;

namespace {
    namespace fs = std::filesystem;

    constexpr wchar_t cacheFolder[] = L"cloudflare";
    constexpr wchar_t executableName[] = L"cloudflared.exe";
    constexpr char assetName[] = "cloudflared-windows-amd64.exe";
    constexpr char releaseApi[] = "https://api.github.com/repos/cloudflare/cloudflared/releases/latest";
    constexpr auto pollInterval = 50ms;
    constexpr long metadataTimeout = 20, downloadTimeout = 120;

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

    size_t WriteDownload(char* data, size_t size, size_t count, void* context) noexcept {
        auto& output = *static_cast<std::ostream*>(context);
        try {
            output.write(data, size * count);
            return output ? size * count : 0;
        } catch (...) {
            return 0;
        }
    }

    int CancelDownload(void* context, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
        return static_cast<const std::atomic<bool>*>(context)->load() ? 1 : 0;
    }

    void Download(const std::string& url, std::ostream& output, const std::atomic<bool>& stopped, long timeout) {
        CheckCancelled(stopped);
        static const auto initialized = curl_global_init(CURL_GLOBAL_DEFAULT);
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> request(curl_easy_init(), curl_easy_cleanup);
        if (initialized != CURLE_OK || !request) throw std::runtime_error("Could not prepare the download.");
        auto* curl = request.get();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "cs2-external-esp");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteDownload);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, CancelDownload);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &stopped);
        auto result = curl_easy_perform(curl);
        CheckCancelled(stopped);
        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        if (result != CURLE_OK || status != 200)
            throw std::runtime_error(status == 403
                ? "GitHub denied the download (HTTP 403). Try again later or install cloudflared manually; see docs/radar-sharing.md."
                : "Could not download cloudflared. See docs/radar-sharing.md for manual installation.");
    }

    std::string Sha256(const fs::path& path, const std::atomic<bool>& stopped) {
        BCRYPT_ALG_HANDLE rawAlgorithm = nullptr;
        if (BCryptOpenAlgorithmProvider(&rawAlgorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0)
            throw std::runtime_error("Could not prepare checksum verification.");
        auto closeAlgorithm = [](void* handle) { BCryptCloseAlgorithmProvider(handle, 0); };
        std::unique_ptr<void, decltype(closeAlgorithm)> algorithm(rawAlgorithm, closeAlgorithm);
        BCRYPT_HASH_HANDLE rawHash = nullptr;
        if (BCryptCreateHash(rawAlgorithm, &rawHash, nullptr, 0, nullptr, 0, 0) != 0)
            throw std::runtime_error("Could not prepare checksum verification.");
        std::unique_ptr<void, decltype(&BCryptDestroyHash)> hash(rawHash, BCryptDestroyHash);
        std::ifstream input(path, std::ios::binary);
        if (!input) throw std::runtime_error("Could not read the downloaded file.");
        std::array<unsigned char, 64 * 1024> buffer{};
        while (input) {
            CheckCancelled(stopped);
            input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
            if (BCryptHashData(rawHash, buffer.data(), static_cast<ULONG>(input.gcount()), 0) != 0)
                throw std::runtime_error("Could not verify the downloaded file.");
        }
        if (!input.eof()) throw std::runtime_error("Could not read the downloaded file.");
        std::array<unsigned char, 32> digest{};
        if (BCryptFinishHash(rawHash, digest.data(), static_cast<ULONG>(digest.size()), 0) != 0)
            throw std::runtime_error("Could not verify the downloaded file.");
        constexpr char hex[] = "0123456789abcdef";
        std::string result;
        for (auto byte : digest) {
            result += hex[byte >> 4];
            result += hex[byte & 15];
        }
        return result;
    }

    fs::path PrepareExecutable(const fs::path& directory, const std::atomic<bool>& stopped,
        const std::function<void(const char*)>& status) {
        auto executable = directory / cacheFolder / executableName;
        if (fs::is_regular_file(executable)) return executable;
        status("Downloading cloudflared...");
        std::ostringstream metadata;
        Download(releaseApi, metadata, stopped, metadataTimeout);
        auto release = json::parse(metadata.str(), nullptr, false);
        if (!release.is_object() || !release.contains("assets") || !release["assets"].is_array())
            throw std::runtime_error("GitHub returned invalid release information.");
        std::string url, digest;
        for (const auto& asset : release["assets"]) {
            if (asset.is_object() && asset.value("name", "") == assetName) {
                url = asset.value("browser_download_url", "");
                digest = asset.value("digest", "");
                break;
            }
        }
        if (url.empty() || !digest.starts_with("sha256:") || digest.size() != 71 ||
            digest.find_first_not_of("0123456789abcdefABCDEF", 7) != std::string::npos)
            throw std::runtime_error("The release has no verifiable Windows download.");
        digest.erase(0, 7);
        for (char& c : digest)
            if (c >= 'A' && c <= 'F') c += 'a' - 'A';

        fs::create_directories(executable.parent_path());
        auto temporary = executable;
        temporary += L".download";
        try {
            std::ofstream file(temporary, std::ios::binary);
            if (!file) throw std::runtime_error("Could not save the download. See docs/radar-sharing.md for manual installation.");
            Download(url, file, stopped, downloadTimeout);
            file.close();
            if (!file) throw std::runtime_error("Could not save the download.");
            status("Verifying download...");
            if (Sha256(temporary, stopped) != digest)
                throw std::runtime_error("The downloaded file failed its SHA-256 check.");
            CheckCancelled(stopped);
            fs::rename(temporary, executable);
        } catch (...) {
            std::error_code ignored;
            fs::remove(temporary, ignored);
            throw;
        }
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
        auto executable = PrepareExecutable(directory, sharingStopped, [this](const char* status) {
            std::lock_guard lock(sharingMutex);
            sharing.status = status;
        });
        CheckCancelled(sharingStopped);
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
            throw std::runtime_error("Could not start cloudflared. Install it manually; see docs/radar-sharing.md.");
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
    }
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
