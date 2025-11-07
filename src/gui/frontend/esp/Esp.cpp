#include "Esp.hpp"

#include "core/engine/cache/Cache.hpp"

void Esp::Render() {
    return GetInstance().RenderImpl();
}

void Esp::RenderImpl() {
	auto d = ImGui::GetBackgroundDrawList();

	auto game = Cache::CopyGame();
	auto globals = Cache::CopyGlobals();
	auto players = Cache::CopyPlayers();

	auto io = ImGui::GetIO();

	std::string player_list;
	for (auto& player : players) {
		player_list += std::format("{} ({})\n", player.name, player.steam_id);

		if (!player.alive)
			continue;

		if (player.localplayer)
			continue;

		std::pair<Vec2_t, Vec2_t> bounds;
		// Out of view
		if (!player.GetBounds(game.view_matrix, io.DisplaySize, bounds))
			continue;

		d->AddRect(
			bounds.first,
			bounds.second,
			IM_COL32(255, 255, 255, 255)
		);
	}

	d->AddText(
		ImVec2(io.DisplaySize.x - 100, 10),
		IM_COL32(255, 255, 255, 255),
		globals.map_name
	);


	d->AddText(
		ImVec2(10, 10),
		IM_COL32(255, 255, 255, 255),
		player_list.c_str()
	);

}