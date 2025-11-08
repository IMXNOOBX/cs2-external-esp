#include "Esp.hpp"

void Esp::Render() {
    return GetInstance().RenderImpl();
}

void Esp::RenderImpl() {
	if (!cfg::enabled)
		return;

	auto d = ImGui::GetBackgroundDrawList();

	auto game = Cache::CopyGame();
	auto globals = Cache::CopyGlobals();
	auto players = Cache::CopyPlayers();

	auto io = ImGui::GetIO();

	std::string player_list;
	static int local_team = 0;
	for (auto& player : players) {
		player_list += std::format("{} ({})\n", player.name, player.steam_id);

		if (!player.alive)
			continue;

		if (player.localplayer) {
			local_team = player.team;
			continue;
		}

		if (!cfg::esp::team && player.team == local_team)
			continue;

		RenderPlayer(player, game.view_matrix);
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

void Esp::RenderPlayer(Player player, view_matrix_t matrix) {
	auto d = ImGui::GetBackgroundDrawList();
	auto io = ImGui::GetIO();

	// Needed for flags & item sizing, so even if the box is not enabled
	// Should be calculated
	std::pair<Vec2_t, Vec2_t> bounds;
	if (!player.GetBounds(matrix, io.DisplaySize, bounds))
		return;

	if (cfg::esp::box)
		d->AddRect(
			bounds.first,
			bounds.second,
			IM_COL32(255, 255, 255, 255)
		);

	if (cfg::esp::health) {
		auto x_start = bounds.first.x -4; // -4 is padding
		auto x_end = x_start -2; // -2 is the inner space of the rect

		auto y_start = bounds.first.y;
		auto y_end = bounds.second.y;

		float height = y_end - y_start;
		float filled_height = height * (player.health / 100.0f);

		d->AddRectFilled(
			ImVec2(x_start, y_end - filled_height),
			ImVec2(x_end, y_end),
			IM_COL32(100, 255, 100, 255)
		);

		d->AddRect(
			ImVec2(x_start, y_start),
			ImVec2(x_end, y_end),
			IM_COL32(0, 0, 0, 50)
		);

		//d->AddText(
		//	ImVec2(x_start - 10, y_end - filled_height - 4),
		//	IM_COL32(0, 0, 0, 255),
		//	std::format("{}hp", player.health).c_str()
		//);
	}

	if (cfg::esp::skeleton)
		RenderPlayerBones(player, matrix);

	if (cfg::esp::head_tracker)
		RenderPlayerTracker(player, matrix, bounds);

	RenderPlayerFalgs(player, matrix, bounds);
}

void Esp::RenderPlayerTracker(Player player, view_matrix_t matrix, std::pair<Vec2_t, Vec2_t> bounds) {
	auto d = ImGui::GetBackgroundDrawList();
	auto io = ImGui::GetIO();

	auto head_bone = player.bone_list[bone_index::head];

	Vec2_t head;
	if (!matrix.wts(head_bone.pos, io.DisplaySize, head))
		return;

	auto width = bounds.first.x - bounds.second.x;

	d->AddCircle(
		head,
		width / 6,
		IM_COL32(255, 0, 0, 255),
		15
	);
}

void Esp::RenderPlayerBones(Player player, view_matrix_t matrix) {
	auto d = ImGui::GetBackgroundDrawList();
	auto io = ImGui::GetIO();

	auto bone_count = player.bone_list.size();
	for (const auto& bone : connections) {
		int first = bone[0], second = bone[1];

		if (bone_count <= first || bone_count <= second)
			continue;

		const auto& bone1 = player.bone_list[first];
		const auto& bone2 = player.bone_list[second];

		Vec2_t scb1;
		if (!matrix.wts(bone1.pos, io.DisplaySize, scb1))
			continue;

		Vec2_t scb2;
		if (!matrix.wts(bone2.pos, io.DisplaySize, scb2))
			continue;

		d->AddLine(
			scb1,
			scb2,
			IM_COL32(255, 255, 255, 255),
			1.5f
		);
	}
}

void Esp::RenderPlayerFalgs(Player player, view_matrix_t matrix, std::pair<Vec2_t, Vec2_t> bounds) {
	auto d = ImGui::GetBackgroundDrawList();
	auto io = ImGui::GetIO();

	if (cfg::esp::flags::name) {
		auto name_size = ImGui::CalcTextSize(player.name);

		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) / 2 + name_size.x / 2, 20),
			IM_COL32(255, 255, 255, 255),
			player.name
		);
	}
}