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

Bomb Cache::CopyBomb() {
    std::lock_guard<std::mutex> lock(Get().mtx);
    return Get().bomb;
}

Globals Cache::CopyGlobals() {
    std::lock_guard<std::mutex> lock(Get().mtx);
    return Get().globals;
}

std::vector<Player> Cache::CopyPlayers() {
    std::lock_guard<std::mutex> lock(Get().mtx);
    return Get().players;
}

Snapshot Cache::CopySnapshot() {
    std::lock_guard<std::mutex> lock(Get().mtx);
    return {
        Get().game,
        Get().bomb,
        Get().globals,
        Get().players
    };
}

bool Cache::RefreshImpl() {
    auto p = Engine::GetProcess();
    auto client = Engine::GetClient();

    if (!p)
        return false;

    auto now = steady_clock::now();

    // Without this, we are pointless :c
    // Its just calling game.UpdateMatrix() which has to bee updated as fast as possible
    if (!game.Update())
        return false;

#ifdef _DEBUG
    // Testing performance
    if (now - last < (cfg::dev::cache_refresh_rate * 1ms)) 
        return true;
#else
    // Just refresh every 5ms good for most people
    if (now - last < 5ms) 
        return true; // All good
#endif

    game.UpdateEntityList();
    globals.Update();
    bomb.Update();

    std::vector<Player> scan;
    scan.reserve(globals.max_clients);
    for (int i = 0; i < globals.max_clients; i++) {
        auto player = Player(i, game.list_entry);

        if (!player.Update())
            continue;
    
        scan.push_back(player);
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        players = std::move(scan);

        duration = duration_cast<std::chrono::milliseconds>(last - now);
        last = now;
    }

    return true;
}
