#include "Radar.hpp"
#include "Page.hpp"
#include "core/engine/cache/Cache.hpp"
#include "assets/fonts/WeaponIcons.h"

using json = nlohmann::json;

#pragma comment(lib, "ws2_32.lib")

bool Radar::Init() {
    return GetInstance().InitImpl();
}

void Radar::Destroy() {
    GetInstance().DestroyImpl();
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
