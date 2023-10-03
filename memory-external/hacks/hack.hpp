#include <thread>
#include "../memory/memory.hpp"
#include "classes/globals.hpp"
#include "../classes/render.hpp"
#include "../classes/config.hpp"

namespace hack {
	std::shared_ptr<pProcess> process;
	ProcessModule base_module;

    void loop() {
        const uintptr_t localPlayer = process->read<uintptr_t>(base_module.base + config::dwLocalPlayer);
        if (!localPlayer)
            return;

        const int localTeam = process->read<int>(localPlayer + config::m_iTeamNum);
        const view_matrix_t view_matrix = process->read<view_matrix_t>(base_module.base + config::dwViewMatrix);
        const Vector3 localOrigin = process->read<Vector3>(localPlayer + config::m_vecOrigin);
        const uintptr_t entity_list = process->read<uintptr_t>(base_module.base + config::dwEntityList);

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

            const int playerHealth = process->read<int>(player + config::dwPawnHealth);
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
            const int playerTeam = process->read<int>(player + config::m_iTeamNum);
            if (playerTeam == localTeam) {
                playerIndex++;
                continue;
            }

            const std::uint32_t playerPawn = process->read<std::uint32_t>(player + config::dwPlayerPawn);

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

            /**
            * Skip rendering players that are too far away (1000 units)
            * (Hopefully in the future this will be a setting inside the config, for now uncomment the code below)
            * 
            * Implemented from the old code, now it works
            **/
            const Vector3 origin = process->read<Vector3>(pCSPlayerPawn + config::m_vecOrigin);
            /*if ((localOrigin - origin).length2d() > 1000.f) {
				playerIndex++;
				continue;
			}*/

            std::string playerName = "Invalid Name";
            const DWORD64 playerNameAddress = process->read<DWORD64>(player + config::dwSanitizedName);

            if (playerNameAddress) {
                char buf[256];
                process->read_raw(playerNameAddress, buf, sizeof(buf));
                playerName = std::string(buf);
            }

            const Vector3 head = { origin.x, origin.y, origin.z + 75.f };

            const Vector3 screenPos = origin.world_to_screen(view_matrix);
            const Vector3 screenHead = head.world_to_screen(view_matrix);

            const float height = screenPos.y - screenHead.y;
            const float width = height / 2.4f;

            if (screenPos.z >= 0.01f) {
                const COLORREF boxColor = RGB(175, 75, 75);
                const COLORREF healthBarColor = RGB(255 - playerHealth, 55 + playerHealth * 2, 75);

                render::DrawBorderBox(g::hdcBuffer, screenHead.x - width / 2, screenHead.y, width, height, boxColor);
                render::DrawBorderBox(g::hdcBuffer, screenHead.x - (width / 2 + 5), screenHead.y + (height * (100 - playerHealth) / 100), 2, height - (height * (100 - playerHealth) / 100), healthBarColor);
                render::RenderText(g::hdcBuffer, screenHead.x + (width / 2 + 5), screenHead.y, playerName.c_str(), RGB(75, 75, 175), 10);
                render::RenderText(g::hdcBuffer, screenHead.x + (width / 2 + 5), screenHead.y + 10, (std::to_string(playerHealth) + "hp").c_str(), healthBarColor, 10);
            }
            playerIndex++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
