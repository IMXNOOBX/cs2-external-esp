#include "Engine.hpp"

#include "core/offsets/Dumper.hpp"
#include "core/engine/types/Player.hpp"
#include "core/engine/types/Globals.hpp"

bool Engine::Init() {
    return GetInstance().InitImpl();
}

ProcessModule Engine::GetClient() {
    return GetInstance().base_client;
}

ProcessModule Engine::GetEngine() {
    return GetInstance().base_engine;
}

std::shared_ptr<pProcess> Engine::GetProcess() {
    return GetInstance().process;
}

bool Engine::InitImpl() {
    process = std::make_shared<pProcess>();

    if (!this->AwaitProcess()) {
        LOGF(FATAL, "Could not find process, please make sure the game is open");
        return false;
    }

    if (!this->AwaitModules()) {
        LOGF(FATAL, "Game took too long to load, please open me again once its fully loaded");
        return false;
    }

    if (!Dumper::Init()) {
        LOGF(FATAL, "Failed to dump game offsets");
        return false;
    }
    
    std::thread(&Engine::Thread, this).detach();

    LOGF(INFO, "Succesfully initialized engine...");
}

struct view_matrix_t {
    float* operator[ ](int index) {
        return matrix[index];
    }

    float matrix[4][4];
};

void Engine::Thread() {
    //uintptr_t number = process->read<uintptr_t>(base_engine.base + offsets::buildNumber);
    //LOGF(VERBOSE, "Build number is {}", number);

    while (true) {
        view_matrix_t view_matrix = process->read<view_matrix_t>(base_client.base + offsets::viewMatrix);

        Globals::Update();

        LOGF(VERBOSE, "Playing in map {} with {} clients & time is {}", Globals::map_name, Globals::max_clients, Globals::current_time);

        auto entity_list_entry = process->read<DWORD64>(base_client.base + offsets::entityList);
             entity_list_entry = process->read<DWORD64>(entity_list_entry + 0x10);

        for (int i = 0; i < Globals::max_clients; i++) {
            auto player = Player(i, entity_list_entry);

            if (!player.Update())
                continue;

            LOGF(VERBOSE, "Player {} ({}) [{}] has {}hp in team {} at pos ({}x, {}y, {}z)", player.name, player.steam_id, i, player.health, player.team, player.pos.x, player.pos.y, player.pos.z);
        }

        std::this_thread::sleep_for(1s);
    }
}

bool Engine::AwaitProcess() {
    if (!process || process->handle_) // Process not initialized, or already attached
        return false;

    do {
        if (process->AttachProcess("cs2.exe"))
            break;

        static int attempts = 0;
        if (attempts > 10)
            return false;
        attempts++;

        std::this_thread::sleep_for(5s);
    } while (true);

    return true;
}

bool Engine::AwaitModules() {
    if (!process || !process->handle_) // Process not initialized, or not attached
        return false;

    do {
        this->base_client = process->GetModule("client.dll");
        this->base_engine = process->GetModule("engine2.dll");

        if (this->base_client.base && this->base_engine.base)
            break;

        static int attempts = 0;
        if (attempts > 10)
            return false;
        attempts++;

        std::this_thread::sleep_for(5s);
    } while (true);


    return true;
}