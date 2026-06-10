#include "Cache.hpp"

#include "core/engine/Engine.hpp" // Circular dep
#include "core/offsets/Dumper.hpp"

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

    // Resolve the C4 carrier via the global weaponC4 pointer
    // client.base + weaponC4 -> ptr to C4 entity -> m_hOwnerEntity (0x520) = pawn handle
    uintptr_t c4_carrier_pawn = 0;
    {
        auto c4_ptr = p->read<uintptr_t>(client.base + offsets::weaponC4);
        if (c4_ptr) {
            auto c4_entity = p->read<uintptr_t>(c4_ptr);
            if (c4_entity)
                c4_carrier_pawn = (uintptr_t)p->read<int>(c4_entity + 0x520);
        }
    }

    std::vector<Player> scan;
    scan.reserve(globals.max_clients);
    for (int i = 0; i < globals.max_clients; i++) {
        auto player = Player(i, game.entity_list, game.list_entry);

        if (!player.Update())
            continue;

        if (player.localplayer)
            this->local = player;

        player.has_c4 = (c4_carrier_pawn != 0 && (uintptr_t)player.pawn_controller_addr == c4_carrier_pawn);

#ifdef _DEBUG
        static int p_tick = 0;
        if (c4_carrier_pawn != 0 && ++p_tick % 60 == 0) {
            LOGF(VERBOSE, "[C4] player#{} pawn_ctrl=0x{:X} has_c4={}", 
                (int)player.index, (uintptr_t)player.pawn_controller_addr, player.has_c4);
        }
#endif

        // TODO: Handle or at least alert, in case of multiple lp
        //if (player.localplayer && (this->local.index == -1 || this->local.index == player.index))
        //    this->local = player;
        //else if (player.localplayer)
        //    LOGF(FATAL, "Offset missmatch, initial({}) current({}) there are more than one local players, update needed", this->local.index, player.index);
    
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
