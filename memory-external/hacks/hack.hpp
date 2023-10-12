#include <thread>
#include <cmath>
#include "../memory/memory.hpp"
#include "../classes/globals.hpp"
#include "../classes/render.hpp"
#include "../classes/config.hpp"

namespace hack {
	std::shared_ptr<pProcess> process;
	ProcessModule base_module;

    float CalculateDistance(const Vector3& point1, const Vector3& point2) {
        float dx = point2.x - point1.x;
        float dy = point2.y - point1.y;
        float dz = point2.z - point1.z;

        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

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
            const int playerArmor = process->read<int>(player + updater::offsets::m_iPawnArmor);
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
            const Vector3 head2 = { origin.x + 10, origin.y + 10, origin.z + 75.f };
            const Vector3 localOrigin = process->read<Vector3>(localPlayer + updater::offsets::m_vecOrigin);

            if (config::render_distance != -1 && (localOrigin - origin).length2d() > config::render_distance) {
                playerIndex++;
                continue;
            }

            const Vector3 screenPos = origin.world_to_screen(view_matrix);
            const Vector3 screenHead = head.world_to_screen(view_matrix);

            const float height = screenPos.y - screenHead.y;
            const float width = height / 2.4f;

            const float height2 = (screenPos.y - screenHead.y) / 8;
            const float width2 = (height / 2.4f) / 4;

            if (screenPos.z >= 0.01f) {
                const COLORREF boxColor = RGB(175, 75, 75);
                const COLORREF healthBarColor = RGB(255 - playerHealth, 55 + playerHealth * 2, 75);

                const float pi = 3.14159265358979323846f;

                // Get the current time
                DWORD currentTime = GetTickCount();

                // Calculate the sinusoidal values for RGB colors
                float red = (sin(config::rainbow_speed * currentTime + 0) * 127.5f + 127.5f);  // 0 phase shift
                float green = (sin(config::rainbow_speed * currentTime + 2 * pi / 3) * 127.5f + 127.5f);  // 2*pi/3 phase shift
                float blue = (sin(config::rainbow_speed * currentTime + 4 * pi / 3) * 127.5f + 127.5f);  // 4*pi/3 phase shift

                // Convert the floating point values to integers for RGB color creation
                COLORREF rainbowColor = RGB(static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue));

                float distance = CalculateDistance(localOrigin, origin);
                // Round the distance to the nearest integer
                int roundedDistance = std::round(distance / 10);

                if (config::rainbow) {
                    render::DrawBorderBox(
                        g::hdcBuffer,
                        screenHead.x - width / 2,
                        screenHead.y,
                        width,
                        height,
                        (localTeam == playerTeam ? rainbowColor : rainbowColor)
                    );
                }
                else
                {
                    render::DrawBorderBox(
                        g::hdcBuffer,
                        screenHead.x - width / 2,
                        screenHead.y,
                        width,
                        height,
                        (localTeam == playerTeam ? RGB(75, 175, 75) : RGB(175, 75, 75))
                    );
                }

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

                if (config::head_tracker)
                {
                    if (roundedDistance > 35)
                    {
                        render::DrawFilledBox(
                            g::hdcBuffer,
                            screenHead.x - (width2 / 2),
                            (screenHead.y + 10 - (roundedDistance / 80)) - (height2 / 2),
                            width2,
                            height2,
                            rainbowColor
                        );
                    }
                    else if (roundedDistance > 5)
                    {
                        render::DrawFilledBox(
                            g::hdcBuffer,
                            screenHead.x - (width2 / 2),
                            screenHead.y + 25 - (height2 / 2),
                            width2,
                            height2,
                            rainbowColor
                        );
                    }
                }

                if (config::rainbow)
                {
                    render::RenderText(
                        g::hdcBuffer,
                        screenHead.x + (width / 2 + 5),
                        screenHead.y,
                        playerName.c_str(),
                        rainbowColor,
                        10
                    );
                }
                else
                {
                    render::RenderText(
                        g::hdcBuffer,
                        screenHead.x + (width / 2 + 5),
                        screenHead.y,
                        playerName.c_str(),
                        RGB(75, 75, 175),
                        10
                    );
                }

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

                if (config::show_armor)
                {
                    render::RenderText(
                        g::hdcBuffer,
                        screenHead.x + (width / 2 + 5),
                        screenHead.y + 21,
                        (std::to_string(playerArmor) + "armor").c_str(),
                        RGB(
                            (255 - playerArmor),
                            (55 + playerArmor * 2),
                            75
                        ),
                        10
                    );
                }

                if (config::show_distance)
                {
                    if (config::rainbow)
                    {
                        render::RenderText(
                            g::hdcBuffer,
                            screenHead.x + (width / 2 + 5),
                            screenHead.y + 32,
                            (std::to_string(roundedDistance) + "m away").c_str(),
                            rainbowColor,
                            10
                        );
                    }
                    else
                    {
                        render::RenderText(
                            g::hdcBuffer,
                            screenHead.x + (width / 2 + 5),
                            screenHead.y + 32,
                            (std::to_string(roundedDistance) + "m away").c_str(),
                            RGB(75, 75, 175),
                            10
                        );
                    }
                } 
            }
            playerIndex++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
