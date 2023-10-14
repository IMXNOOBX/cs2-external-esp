#include <thread>
#include <cmath>
#include "../memory/memory.hpp"
#include "../classes/globals.hpp"
#include "../classes/render.hpp"
#include "../classes/config.hpp"



namespace hack {
	std::shared_ptr<pProcess> process;
	ProcessModule base_module;

	//const float pi = 3.14159265358979323846f;

	void loop() {
		const uintptr_t localPlayer = process->read<uintptr_t>(base_module.base + updater::offsets::dwLocalPlayer);
		if (!localPlayer)
			return;

		const int localTeam = process->read<int>(localPlayer + updater::offsets::m_iTeamNum);
		const view_matrix_t view_matrix = process->read<view_matrix_t>(base_module.base + updater::offsets::dwViewMatrix);
		const Vector3 localOrigin = process->read<Vector3>(localPlayer + updater::offsets::m_vecOrigin);
		const uintptr_t entity_list = process->read<uintptr_t>(base_module.base + updater::offsets::dwEntityList);

		int playerIndex = 0;
		uintptr_t list_entry;

		/**
		* Loop through all the players in the entity list
		*
		* (This could have been done by getting a entity list count from the engine, but I'm too lazy to do that)
		**/
		while (true) {
			playerIndex++;
			list_entry = process->read<uintptr_t>(entity_list + (8 * (playerIndex & 0x7FFF) >> 9) + 16);
			if (!list_entry)
				break;

			const uintptr_t player = process->read<uintptr_t>(list_entry + 120 * (playerIndex & 0x1FF));
			if (!player)
				continue;

			/**
			* Skip rendering your own character and teammates
			*
			* If you really want you can exclude your own character from the check but
			* since you are in the same team as yourself it will be excluded anyway
			**/
			const int playerTeam = process->read<int>(player + updater::offsets::m_iTeamNum);
			if (config::team_esp && (playerTeam == localTeam))
				continue;


			const std::uint32_t playerPawn = process->read<std::uint32_t>(player + updater::offsets::dwPlayerPawn);

			const uintptr_t list_entry2 = process->read<uintptr_t>(entity_list + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);
			if (!list_entry2)
				continue;


			const uintptr_t pCSPlayerPawn = process->read<uintptr_t>(list_entry2 + 120 * (playerPawn & 0x1FF));
			if (!pCSPlayerPawn)
				continue;

			const int playerHealth = process->read<int>(pCSPlayerPawn + updater::offsets::m_iHealth);
			if (playerHealth <= 0 || playerHealth > 100)
				continue;

			if (config::team_esp && (pCSPlayerPawn == localPlayer))
				continue;


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

			if (config::render_distance != -1 && (localOrigin - origin).length2d() > config::render_distance)
				continue;


			if ((localOrigin - origin).length2d() < 5)
				continue;


			const Vector3 screenPos = origin.world_to_screen(view_matrix);
			const Vector3 screenHead = head.world_to_screen(view_matrix);

			const float height = screenPos.y - screenHead.y;
			const float width = height / 2.4f;

			const float head_height = (screenPos.y - screenHead.y) / 8;
			const float head_width = (height / 2.4f) / 4;

			if (screenPos.z >= 0.01f) {
				float distance = localOrigin.calculate_distance(origin);
				int roundedDistance = std::round(distance / 10);

				/**
				* I know is not the best way but a simple way to not saturate the screen with a ton of information
				* From IfBars - I decided to move this up here so that it will not render anything unless the player is within the flag_render_distance
				* I did this so people aren't confused on why only half the players hp values are showing, however feel free to change this however you see fit
				* Personally I prefer to see everyone no matter the difference, but everyone has their preferences
				*/
				if (roundedDistance > config::flag_render_distance)
					continue;

				render::DrawBorderBox(
					g::hdcBuffer,
					screenHead.x - width / 2,
					screenHead.y,
					width,
					height,
					(localTeam == playerTeam ? config::esp_box_color_team : config::esp_box_color_enemy)
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
					config::esp_name_color,
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

				if (config::show_extra_flags)
				{
					render::RenderText(
						g::hdcBuffer,
						screenHead.x + (width / 2 + 5),
						screenHead.y + 22,
						(std::to_string(roundedDistance) + "m away").c_str(),
						config::esp_distance_color,
						10
					);
				}

				/**
				* Great idea by @ifBars that will be implemented later on
				*/
				/*
					if (config::head_tracker)
					{
						if (roundedDistance > 35)
						{
							render::DrawFilledBox(
								g::hdcBuffer,
								screenHead.x - (head_width / 2),
								(screenHead.y + 10 - (roundedDistance / 80)) - (head_height / 2),
								head_width,
								head_height,
								(localTeam == playerTeam ? RGB(75, 175, 75) : RGB(175, 75, 75))
							);
						}
						else if (roundedDistance > 5)
						{
							render::DrawFilledBox(
								g::hdcBuffer,
								screenHead.x - (head_width / 2),
								screenHead.y + 25 - (head_height / 2),
								head_width,
								head_height,
								(localTeam == playerTeam ? RGB(75, 175, 75) : RGB(175, 75, 75))
							);
						}
					}
				*/
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
