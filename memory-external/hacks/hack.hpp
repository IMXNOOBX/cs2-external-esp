#include <thread>
#include "../memory/memory.hpp"
#include "../classes/globals.hpp"
#include "../classes/render.hpp"
#include "../classes/config.hpp"

namespace hack {
	std::shared_ptr<pProcess> process;
	ProcessModule base_module;

    void loop() {
        const uintptr_t localPlayer = process->read<uintptr_t>(base_module.base + updater::offsets::dwLocalPlayer);
        if (!localPlayer)
            return;

        const int localTeam = process->read<int>(localPlayer + updater::offsets::m_iTeamNum);
        const view_matrix_t view_matrix = process->read<view_matrix_t>(base_module.base + updater::offsets::dwViewMatrix);
        const Vector3 localOrigin = process->read<Vector3>(localPlayer + updater::offsets::m_vecOrigin);
        const uintptr_t entity_list = process->read<uintptr_t>(base_module.base + updater::offsets::dwEntityList);

        int playerIndex = 1;
        uintptr_t list_entry;

        /**
        * Loop through all the players in the entity list
        * 
        * (This could have been done by getting a entity list count from the engine, but I'm too lazy to do that)
        **/
        while (true) {
            list_entry = process->read<uintptr_t>(entity_list + (8 * (playerIndex & 0x7FFF) >> 9) + 16);
            if (!list_entry)
                break;

            const uintptr_t player = process->read<uintptr_t>(list_entry + 120 * (playerIndex & 0x1FF));
            if (!player) {
                playerIndex++;
                continue;
            }

            const int playerHealth = process->read<int>(player + updater::offsets::dwPawnHealth);
            if (playerHealth <= 0 || playerHealth > 100) {
                playerIndex++;
                continue;
            }

            /**
            * Skip rendering your own character and teammates
            * 
            * If you really want you can exclude your own character from the check but
            * since you are in the same team as yourself it will be excluded anyway
            **/
            const int playerTeam = process->read<int>(player + updater::offsets::m_iTeamNum);
            if (config::team_esp && (playerTeam == localTeam)) {
                playerIndex++;
                continue;
            }

            const std::uint32_t playerPawn = process->read<std::uint32_t>(player + updater::offsets::dwPlayerPawn);

            const uintptr_t list_entry2 = process->read<uintptr_t>(entity_list + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);
            if (!list_entry2) {
                playerIndex++;
                continue;
            }

            const uintptr_t pCSPlayerPawn = process->read<uintptr_t>(list_entry2 + 120 * (playerPawn & 0x1FF));
            if (!pCSPlayerPawn) {
                playerIndex++;
                continue;
            }

            if (config::team_esp && (pCSPlayerPawn == localPlayer)) {
                playerIndex++;
                continue;
            }

            std::string playerName = "Invalid Name";
            const DWORD64 playerNameAddress = process->read<DWORD64>(player + updater::offsets::dwSanitizedName);

            if (playerNameAddress) {
                char buf[256];
                process->read_raw(playerNameAddress, buf, sizeof(buf));
                playerName = std::string(buf);
            }

            const Vector3 origin = process->read<Vector3>(pCSPlayerPawn + updater::offsets::m_vecOrigin);
            const Vector3 head = { origin.x, origin.y, origin.z + 75.f };

            if (config::render_distance != -1 && (localOrigin - origin).length2d() > config::render_distance) {
                playerIndex++;
                continue;
            }

            const Vector3 screenPos = origin.world_to_screen(view_matrix);
            const Vector3 screenHead = head.world_to_screen(view_matrix);

            const float height = screenPos.y - screenHead.y;
            const float width = height / 2.4f;

            if (screenPos.z >= 0.01f) {
                const COLORREF boxColor = RGB(175, 75, 75);
                const COLORREF healthBarColor = RGB(255 - playerHealth, 55 + playerHealth * 2, 75);

                render::DrawBorderBox(
                    g::hdcBuffer,
                    screenHead.x - width / 2,
                    screenHead.y,
                    width,
                    height,
                    (localTeam == playerTeam ? RGB(75, 175, 75) : RGB(175, 75, 75))
                );

                render::DrawBorderBox(
                    g::hdcBuffer,
                    screenHead.x - (width / 2 + 5),
                    screenHead.y + (height * (100 - playerHealth) / 100),
                    2,
                    height - (height * (100 - playerHealth) / 100),
                    RGB(
                        (255 - playerHealth),
                        (55 + playerHealth * 2),
                        75
                    )
                );

                render::RenderText(
                    g::hdcBuffer,
                    screenHead.x + (width / 2 + 5),
                    screenHead.y,
                    playerName.c_str(),
                    RGB(75, 75, 175),
                    10
                );

                render::RenderText(
                    g::hdcBuffer,
                    screenHead.x + (width / 2 + 5),
                    screenHead.y + 10,
                    (std::to_string(playerHealth) + "hp").c_str(),
                    RGB(
                        (255 - playerHealth),
                        (55 + playerHealth * 2),
                        75
                    ),
                    10
                );
            }
            playerIndex++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
