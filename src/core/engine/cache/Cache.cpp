#include "Cache.hpp"

#include "core/engine/Engine.hpp" // Circular dep
#include "core/offsets/Dumper.hpp"

bool Cache::Refresh() {
    return Get().RefreshImpl();
}

Game Cache::CopyGame() {
    std::lock_guard<std::mutex> lock(Get().mtx);
    return Get().game;
}

Globals Cache::CopyGlobals() {
    std::lock_guard<std::mutex> lock(Get().mtx);
    return Get().globals;
}

std::vector<Player> Cache::CopyPlayers() {
    std::lock_guard<std::mutex> lock(Get().mtx);
    return Get().players;
}

bool Cache::RefreshImpl() {
    auto p = Engine::GetProcess();
    auto client = Engine::GetClient();

    if (!p)
        return false;

    auto now = steady_clock::now();

    // Without this, we are pointless :c
    // And needs to be updated as fast as possible
    if (!game.Update()) 
        return false;

    // Just refresh every 5ms
    if (now - last < 5ms)
        return true; // All good

    globals.Update();
    //LOGF(VERBOSE, "Playing in map {} with {} clients & time is {}", globals.map_name, globals.max_clients, globals.current_time);

    std::vector<Player> scan;
    scan.reserve(globals.max_clients);
    for (int i = 0; i < globals.max_clients; i++) {
        auto player = Player(i, game.list_entry);

        if (!player.Update())
            continue;

        //LOGF(VERBOSE, "Player {} ({}) [{}] has {}hp in team {} at pos ({}x, {}y, {}z)", player.name, player.steam_id, i, player.health, player.team, player.pos.x, player.pos.y, player.pos.z);
    
        scan.push_back(player);
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        players = std::move(scan);
        last = steady_clock::now();
        duration = duration_cast<std::chrono::milliseconds>(last - now);
    }

    return true;
}
