#include "Cache.hpp"

#include "core/engine/Engine.hpp" // Circular dep
#include "core/offsets/Dumper.hpp"

#define MAX_CLIENTS 64

bool Cache::Refresh() {
    return Get().RefreshImpl();
}

Snapshot Cache::CopySnapshot() {
    std::lock_guard<std::mutex> lock(Get().mtx);
    return {
        Get().game,
        Get().bomb,
        Get().local,
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
    scan.reserve(MAX_CLIENTS);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        auto player = Player(i, game.entity_list, game.list_entry);

        if (!player.Update())
            continue;

        if (player.localplayer)
            this->local = player;

        player.has_c4 = (bomb.carrier != 0 && (uintptr_t)player.pawn_controller_addr == bomb.carrier);

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
