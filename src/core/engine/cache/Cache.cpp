#include "Cache.hpp"

#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"

bool Cache::Refresh() {
    return Get().RefreshImpl();
}

bool Cache::RefreshImpl() {
    auto p = Engine::GetProcess();
    auto client = Engine::GetClient();

    if (!p)
        return false;

    view_matrix_t view_matrix = p->read<view_matrix_t>(client.base + offsets::viewMatrix);

    globals.Update();

    LOGF(VERBOSE, "Playing in map {} with {} clients & time is {}", globals.map_name, globals.max_clients, globals.current_time);

    auto entity_list_entry = p->read<DWORD64>(client.base + offsets::entityList);
         entity_list_entry = p->read<DWORD64>(entity_list_entry + 0x10);

    players.clear();
    for (int i = 0; i < globals.max_clients; i++) {
        auto player = Player(i, entity_list_entry);

        if (!player.Update())
            continue;

        LOGF(VERBOSE, "Player {} ({}) [{}] has {}hp in team {} at pos ({}x, {}y, {}z)", player.name, player.steam_id, i, player.health, player.team, player.pos.x, player.pos.y, player.pos.z);
    
        players.push_back(player);
    }
}
