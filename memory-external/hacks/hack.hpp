#include <thread>
#include <cmath>
#include "reader.hpp"
#include "../classes/render.hpp"
#include "../classes/config.hpp"
#include "../classes/globals.hpp"

namespace hack {
	std::vector<std::pair<std::string, std::string>> boneConnections = {
						{"neck_0", "spine_1"},
						{"spine_1", "spine_2"},
						{"spine_2", "pelvis"},
						{"spine_1", "arm_upper_L"},
						{"arm_upper_L", "arm_lower_L"},
						{"arm_lower_L", "hand_L"},
						{"spine_1", "arm_upper_R"},
						{"arm_upper_R", "arm_lower_R"},
						{"arm_lower_R", "hand_R"},
						{"pelvis", "leg_upper_L"},
						{"leg_upper_L", "leg_lower_L"},
						{"leg_lower_L", "ankle_L"},
						{"pelvis", "leg_upper_R"},
						{"leg_upper_R", "leg_lower_R"},
						{"leg_lower_R", "ankle_R"}
	};

	void loop(render::DX11Renderer renderer) {
		if (config::panic) return;

		std::lock_guard<std::mutex> lock(reader_mutex);

		if (g_game.isC4Planted)
		{
			Vector3 c4Origin = g_game.c4.get_origin();
			const Vector3 c4ScreenPos = g_game.world_to_screen(&c4Origin);

			if (c4ScreenPos.z >= 0.01f) {
				float c4Distance = g_game.localOrigin.calculate_distance(c4Origin);
				float c4RoundedDistance = std::round(c4Distance / 500.f);

				float height = 10 - c4RoundedDistance;
				float width = height * 1.4f;

				render::DrawFilledBox(
					c4ScreenPos.x - (width / 2),
					c4ScreenPos.y - (height / 2),
					width,
					height,
					config::esp_box_color_enemy
				);

				renderer.RenderText(
					c4ScreenPos.x + (width / 2 + 5),
					c4ScreenPos.y,
					"C4",
					config::esp_name_color,
					10
				);
			}
		}

		int playerIndex = 0;
		uintptr_t list_entry;

		/**
		* Loop through all the players in the entity list
		*
		* (This could have been done by getting a entity list count from the engine, but I'm too lazy to do that)
		**/
		for (auto player = g_game.players.begin(); player < g_game.players.end(); player++) {
			const Vector3 screenPos = g_game.world_to_screen(&player->origin);
			const Vector3 screenHead = g_game.world_to_screen(&player->head);

			if (
				screenPos.z < 0.01f || 
				!utils.is_in_bounds(screenPos, g_game.game_bounds.right, g_game.game_bounds.bottom)
				)
				continue;

			const float height = screenPos.y - screenHead.y;
			const float width = height / 2.4f;

			float distance = g_game.localOrigin.calculate_distance(player->origin);
			int roundedDistance = std::round(distance / 10.f);

			int flagBaseY = screenHead.y;  // for dynamic flag pos
			int flagYOffset = 0;           // incremented for each flag
			int flagLineSpacing = 10;      // space between the flags

			if (config::show_head_tracker) {
				render::DrawCircle(
					player->bones.bonePositions["head"].x,
					player->bones.bonePositions["head"].y - width / 12,
					width / 5,
					(g_game.localTeam == player->team ? config::esp_skeleton_color_team : config::esp_skeleton_color_enemy)
				);
			}

			if (config::show_skeleton_esp) {
				for (const auto& connection : boneConnections) {
					const std::string& boneFrom = connection.first;
					const std::string& boneTo = connection.second;

					render::DrawLine(
						player->bones.bonePositions[boneFrom].x, player->bones.bonePositions[boneFrom].y,
						player->bones.bonePositions[boneTo].x, player->bones.bonePositions[boneTo].y,
						g_game.localTeam == player->team ? config::esp_skeleton_color_team : config::esp_skeleton_color_enemy
					);
				}
			}

			if (config::show_box_esp)
			{
				render::DrawBorderBox(
					screenHead.x - width / 2,
					screenHead.y,
					width,
					height,
					(g_game.localTeam == player->team ? config::esp_box_color_team : config::esp_box_color_enemy)
				);
			}

			if (config::show_health_bars)
			{
				render::DrawBorderBox(
					screenHead.x - (width / 2 + 10),
					screenHead.y + (height * (100 - player->armor) / 100),
					2,
					height - (height * (100 - player->armor) / 100),
					RGB(0, 185, 255)
				);

				render::DrawBorderBox(
					screenHead.x - (width / 2 + 5),
					screenHead.y + (height * (100 - player->health) / 100),
					2,
					height - (height * (100 - player->health) / 100),
					RGB(
						(255 - player->health),
						(55 + player->health * 2),
						75
					)
				);
			}

			if (config::show_name_flag)
			{
				renderer.RenderText(
					screenHead.x + (width / 2 + 5),
					flagBaseY + flagYOffset,
					player->name.c_str(),
					config::esp_name_color,
					10
				);
				flagYOffset += flagLineSpacing;
			}

			if (config::show_health_flags)
			{
				/**
				* I know is not the best way but a simple way to not saturate the screen with a ton of information
				*/
				if (roundedDistance > config::flag_render_distance)
					continue;

				renderer.RenderText(
					screenHead.x + (width / 2 + 5),
					flagBaseY + flagYOffset,
					(std::to_string(player->health) + "hp").c_str(),
					RGB(
						(255 - player->health),
						(55 + player->health * 2),
						75
					),
					10
				);
				flagYOffset += flagLineSpacing;

				renderer.RenderText(
					screenHead.x + (width / 2 + 5),
					flagBaseY + flagYOffset,
					(std::to_string(player->armor) + "armor").c_str(),
					RGB(
						(255 - player->armor),
						(55 + player->armor * 2),
						75
					),
					10
				);
				flagYOffset += flagLineSpacing;
			}

			if (config::show_weapon_flag)
			{
				renderer.RenderText(
					screenHead.x + (width / 2 + 5),
					flagBaseY + flagYOffset,
					player->weapon.c_str(),
					config::esp_distance_color,
					10
				);
				flagYOffset += flagLineSpacing;
			}

			if (config::show_distance_flag)
			{
				renderer.RenderText(
					screenHead.x + (width / 2 + 5),
					flagBaseY + flagYOffset,
					(std::to_string(roundedDistance) + "m away").c_str(),
					config::esp_distance_color,
					10
				);
				flagYOffset += flagLineSpacing;				
			}

			if (config::show_money_flag)
			{
				renderer.RenderText(
					screenHead.x + (width / 2 + 5),
					flagBaseY + flagYOffset,
					("$" + std::to_string(player->money)).c_str(),
					RGB(0, 125, 0),
					10
				);	
				flagYOffset += flagLineSpacing;			
			}

			if (config::show_flashed_flag)
			{
				if (player->flashAlpha > 100)
				{
					renderer.RenderText(
						screenHead.x + (width / 2 + 5),
						flagBaseY + flagYOffset,
						"Player is flashed",
						config::esp_distance_color,
						10
					);
					flagYOffset += flagLineSpacing;
				}
			}

			if (config::show_defusing_flag)
			{
				if (player->is_defusing)
				{
					const std::string defuText = "Player is defusing";
					renderer.RenderText(
						screenHead.x + (width / 2 + 5),
						flagBaseY + flagYOffset,
						defuText.c_str(),
						config::esp_distance_color,
						10
					);
				}
				flagYOffset += flagLineSpacing;
			}
		}
		
		// std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}