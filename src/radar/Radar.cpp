#include "Radar.hpp"
#include "Page.hpp"
#include "core/engine/cache/Cache.hpp"
#include "assets/fonts/WeaponIcons.h"
#include <sstream>

using json = nlohmann::json;

#pragma comment(lib, "ws2_32.lib")

bool Radar::Init() {
    return GetInstance().InitImpl();
}

void Radar::Destroy() {
    GetInstance().DestroyImpl();
}

bool Radar::Share() {
    auto& radar = GetInstance();
    if (IsSharing())
        return true;
    StopSharing();
    auto fail = [&](const char* message) {
        std::lock_guard lock(radar.sharingMutex);
        radar.sharing.error = message;
        return false;
    };
    if (!radar.running)
        return fail("Enable Browser radar before sharing.");

    std::array<wchar_t, 32768> module{};
    DWORD length = GetModuleFileNameW(nullptr, module.data(), static_cast<DWORD>(module.size()));
    if (!length || length == module.size())
        return fail("Could not locate the application folder.");
    auto directory = std::filesystem::path(module.data()).parent_path();
    auto script = directory / L"cloudflare" / L"share-radar.ps1";
    std::error_code error;
    if (!std::filesystem::is_regular_file(script, error)) {
        return fail("Sharing script missing from the cloudflare folder. Rebuild the project.");
    }

    std::array<wchar_t, MAX_PATH> system{};
    length = GetSystemDirectoryW(system.data(), static_cast<UINT>(system.size()));
    if (!length || length >= system.size())
        return fail("Could not locate Windows PowerShell.");
    auto powershell = std::filesystem::path(system.data()) / L"WindowsPowerShell" / L"v1.0" / L"powershell.exe";
    std::wstring command = L"\"" + powershell.wstring() +
        L"\" -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" + script.wstring() + L"\"";

    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    if (!job)
        return fail("Could not prepare radar sharing.");
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    // Keep the tunnel and its launcher in the same job.
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits))) {
        CloseHandle(job);
        return fail("Could not prepare radar sharing.");
    }

    SECURITY_ATTRIBUTES security{ sizeof(security), nullptr, TRUE };
    HANDLE output = nullptr, writer = nullptr;
    HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (!CreatePipe(&output, &writer, &security, 0) ||
        !SetHandleInformation(output, HANDLE_FLAG_INHERIT, 0) || input == INVALID_HANDLE_VALUE) {
        if (output) CloseHandle(output);
        if (writer) CloseHandle(writer);
        if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
        CloseHandle(job);
        return fail("Could not capture sharing output.");
    }
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = input;
    startup.hStdOutput = startup.hStdError = writer;
    PROCESS_INFORMATION process{};
    bool created = CreateProcessW(powershell.c_str(), command.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr, directory.c_str(), &startup, &process);
    CloseHandle(writer);
    CloseHandle(input);
    if (!created) {
        CloseHandle(output);
        CloseHandle(job);
        return fail("Could not start Windows PowerShell.");
    }
    if (!AssignProcessToJobObject(job, process.hProcess) || ResumeThread(process.hThread) == static_cast<DWORD>(-1)) {
        TerminateProcess(process.hProcess, 1);
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        CloseHandle(output);
        CloseHandle(job);
        return fail("Could not start radar sharing.");
    }
    CloseHandle(process.hThread);
    radar.sharingProcess = process.hProcess;
    radar.sharingJob = job;
    radar.sharingOutput = output;
    {
        std::lock_guard lock(radar.sharingMutex);
        radar.sharing = { true, "Starting sharing...", {}, {} };
    }
    try {
        radar.sharingThread = std::thread(&Radar::ReadSharingOutput, &radar);
    } catch (const std::exception&) {
        StopSharing();
        return fail("Could not start the sharing monitor.");
    }
    return true;
}

bool Radar::IsSharing() {
    return GetSharingState().active;
}

Radar::SharingState Radar::GetSharingState() {
    auto& radar = GetInstance();
    if (radar.sharingProcess && WaitForSingleObject(radar.sharingProcess, 0) != WAIT_TIMEOUT)
        radar.CloseSharing(false);
    std::lock_guard lock(radar.sharingMutex);
    return radar.sharing;
}

void Radar::StopSharing() {
    GetInstance().CloseSharing(true);
}

void Radar::CloseSharing(bool clear) {
    if (sharingJob)
        CloseHandle(sharingJob);
    if (sharingThread.joinable())
        sharingThread.join();
    if (sharingOutput)
        CloseHandle(sharingOutput);
    if (sharingProcess)
        CloseHandle(sharingProcess);
    sharingJob = sharingProcess = sharingOutput = nullptr;
    std::lock_guard lock(sharingMutex);
    if (clear)
        sharing = {};
}

void Radar::ReadSharingOutput() {
    std::string pending, url, lastError;
    bool connected = false, startupBanner = false;
    auto line = [&](std::string text) {
        if (!text.empty() && text.back() == '\r') text.pop_back();
        auto log = json::parse(text, nullptr, false);
        if (log.is_object() && log.contains("message") && log["message"].is_string())
            text = log["message"].get<std::string>();
        if (text.starts_with("ERROR: ")) {
            auto message = text.substr(7, 500);
            if (lastError.empty() || !message.starts_with("Cloudflare tunnel exited"))
                lastError = message;
        } else if (text.find(" ERR ") != std::string::npos ||
            (log.is_object() && log.contains("level") && log["level"] == "error")) {
            lastError = text.substr(0, 500);
        }

        if (log.is_object() && log.contains("level") && log["level"] == "info") {
            std::istringstream lines(text);
            std::string part;
            while (std::getline(lines, part)) {
                if (part.find("Your quick Tunnel has been created!") != std::string::npos) {
                    startupBanner = true;
                    continue;
                }
                if (!startupBanner || !url.empty()) continue;
                auto first = part.find_first_not_of(" \t\r|");
                if (first == std::string::npos) continue;
                startupBanner = false;
                auto candidate = part.substr(first, part.find_last_not_of(" \t\r|") - first + 1);
                const std::string prefix = "https://", suffix = ".trycloudflare.com";
                if (!candidate.starts_with(prefix) || !candidate.ends_with(suffix)) continue;
                auto label = candidate.substr(prefix.size(), candidate.size() - prefix.size() - suffix.size());
                if (!label.empty() && label.size() <= 63 && label.front() != '-' && label.back() != '-' &&
                    label.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-") == std::string::npos)
                    url = candidate;
            }
            if (text == "Registered tunnel connection")
                connected = true;
        }
        std::lock_guard lock(sharingMutex);
        if (text == "Downloading cloudflared..." || text == "Verifying download..." || text == "Connecting to Cloudflare...")
            sharing.status = text;
        if (connected && !url.empty()) {
            sharing.url = url;
            sharing.status = "Sharing radar";
        } else if (connected) {
            sharing.error = "Cloudflare connected, but its share link could not be read. Stop sharing and try again.";
        }
    };
    char buffer[2048];
    DWORD count = 0;
    while (ReadFile(sharingOutput, buffer, sizeof(buffer), &count, nullptr) && count) {
        pending.append(buffer, count);
        size_t end;
        while ((end = pending.find('\n')) != std::string::npos) {
            line(pending.substr(0, end));
            pending.erase(0, end + 1);
        }
        if (pending.size() > 16384) pending.clear();
    }
    if (!pending.empty()) line(pending);
    std::lock_guard lock(sharingMutex);
    sharing.active = false;
    sharing.url.clear();
    sharing.status.clear();
    sharing.error = lastError.empty() ? "Sharing stopped. Check your connection and try again." : lastError;
}

Radar::~Radar() {
    DestroyImpl();
}

bool Radar::InitImpl() {
    if (running)
        return true;

    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
        return false;

    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(27080);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (listener == INVALID_SOCKET ||
        bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR ||
        listen(listener, SOMAXCONN) == SOCKET_ERROR) {
        LOGF(WARNING, "Could not start browser radar on port 27080 ({})", WSAGetLastError());
        if (listener != INVALID_SOCKET)
            closesocket(listener);
        listener = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    updated = {};
    running = true;
    try {
        thread = std::thread(&Radar::Thread, this);
    }
    catch (const std::exception& e) {
        LOGF(WARNING, "Could not start browser radar thread: {}", e.what());
        running = false;
        closesocket(listener);
        listener = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    LOGF(INFO, "Browser radar: http://127.0.0.1:27080");
    return true;
}

void Radar::DestroyImpl() {
    StopSharing();
    if (!running.exchange(false))
        return;

    if (thread.joinable())
        thread.join();
    closesocket(listener);
    listener = INVALID_SOCKET;
    WSACleanup();
}

const std::string& Radar::GetState() {
    auto now = std::chrono::steady_clock::now();
    if (now - updated < 50ms)
        return state;

    auto snapshot = Cache::CopySnapshot();
    const auto& globals = snapshot.globals;
    bool in_match = cfg::enabled && globals.in_match && snapshot.game.entity_list && snapshot.game.list_entry;

    json data = {
        { "in_match", in_match },
        { "local_team", 0 },
        { "map", in_match ? std::string(globals.map_name, strnlen(globals.map_name, sizeof(globals.map_name))) : "" },
        { "players", json::array() }
    };

    if (in_match) {
        for (const auto& player : snapshot.players) {
            if (player.localplayer)
                data["local_team"] = player.team;
            if (!player.alive || !std::isfinite(player.pos.x) ||
                !std::isfinite(player.pos.y) || !std::isfinite(player.pos.z))
                continue;

            data["players"].push_back({
                { "id", player.index },
                { "name", std::string(player.name, strnlen(player.name, sizeof(player.name))) },
                { "team", player.team },
                { "health", player.health },
                { "weapon", player.weapon.name == "Invalid" ? "" : player.weapon.name },
                { "weapon_icon", player.weapon.name == "Invalid" || !player.weapon.icon ? "" : player.weapon.icon },
                { "local", player.localplayer },
                { "x", player.pos.x }, { "y", player.pos.y }, { "z", player.pos.z }
            });
        }
    }

    state = data.dump(-1, ' ', false, json::error_handler_t::replace);
    updated = now;
    return state;
}

void Radar::Thread() {
    while (running) {
        fd_set sockets;
        FD_ZERO(&sockets);
        FD_SET(listener, &sockets);
        timeval timeout{ 0, 100000 };
        if (select(0, &sockets, nullptr, nullptr, &timeout) <= 0)
            continue;

        SOCKET client = accept(listener, nullptr, nullptr);
        if (client == INVALID_SOCKET)
            continue;

        DWORD wait = 250;
        setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&wait), sizeof(wait));
        setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&wait), sizeof(wait));

        std::string request;
        auto deadline = std::chrono::steady_clock::now() + 500ms;
        while (running && request.size() < 4096 && request.find("\r\n\r\n") == std::string::npos &&
            std::chrono::steady_clock::now() < deadline) {
            char buffer[1024];
            int size = recv(client, buffer, sizeof(buffer), 0);
            if (size <= 0)
                break;
            request.append(buffer, size);
        }

        std::string body;
        std::string status = "404 Not Found";
        std::string type = "text/plain; charset=utf-8";
        std::string headers = request;
        for (char& c : headers)
            if (c >= 'A' && c <= 'Z') c += 'a' - 'A';

        // Only serve requests addressed to this listener.
        bool valid = headers.find("\r\nhost: 127.0.0.1:27080\r\n") != std::string::npos &&
            request.find("\r\n\r\n") != std::string::npos;
        if (!valid) {
            status = "400 Bad Request";
        }
        else if (request.starts_with("GET / HTTP/1.")) {
            body = radar_page;
            status = "200 OK";
            type = "text/html; charset=utf-8";
        }
        else if (request.starts_with("GET /weapon-font.ttf HTTP/1.")) {
            body.assign(reinterpret_cast<const char*>(weapon_icon_font), weapon_icon_font_len);
            status = "200 OK";
            type = "font/ttf";
        }
        else if (request.starts_with("GET /state HTTP/1.")) {
            body = GetState();
            status = "200 OK";
            type = "application/json; charset=utf-8";
        }

        std::string response = "HTTP/1.1 " + status + "\r\nContent-Type: " + type +
            "\r\nCache-Control: no-store\r\nConnection: close\r\nX-Content-Type-Options: nosniff\r\n" +
            "Referrer-Policy: no-referrer\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
        size_t sent = 0;
        while (running && sent < response.size()) {
            int size = send(client, response.data() + sent, static_cast<int>(response.size() - sent), 0);
            if (size <= 0)
                break;
            sent += size;
        }
        closesocket(client);
    }
}
