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
		//player_list += std::format("{} ({})\n", player.name, player.steam_id);

		if (!player.alive)
			continue;

		if (player.localplayer)
			continue;

		std::pair<Vec2_t, Vec2_t> bounds;
		// Out of view
		if (!player.GetBounds(game.view_matrix, io.DisplaySize, bounds))
			continue;

		auto name_size = ImGui::CalcTextSize(player.name);

		d->AddText(
			bounds.second - Vec2_t((bounds.second.x - bounds.first.x) / 2 + name_size.x / 2, 20),
			IM_COL32(255, 255, 255, 255),
			player.name
		);

		d->AddRect(
			bounds.first,
			bounds.second,
			IM_COL32(255, 255, 255, 255)
		);

		auto bone_count = player.bone_list.size();
		for (const auto& bone : connections) {
			int first = bone[0], second = bone[1];

			if (bone_count <= first || bone_count <= second)
				continue;

			const auto& bone1 = player.bone_list[first];
			const auto& bone2 = player.bone_list[second];

			Vec2_t scb1;
			if (!game.view_matrix.wts(bone1.pos, io.DisplaySize, scb1))
				continue;

			Vec2_t scb2;
			if (!game.view_matrix.wts(bone2.pos, io.DisplaySize, scb2))
				continue;

			d->AddLine(
				scb1,
				scb2,
				IM_COL32(255, 255, 255, 255),
				1.5f
			);
		}

		auto head = player.bone_list[bone_index::head];

		Vec2_t scHead;
		if (!game.view_matrix.wts(head.pos, io.DisplaySize, scHead))
			continue;

		d->AddCircle(
			scHead,
			(bounds.first.x - bounds.second.x) / 6,
			IM_COL32(255, 0, 0, 255),
			10
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