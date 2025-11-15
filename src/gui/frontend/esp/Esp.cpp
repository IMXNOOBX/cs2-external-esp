#include "Esp.hpp"

bool Esp::Init() {
	return GetInstance().InitImpl();
}

void Esp::Render() {
    return GetInstance().RenderImpl();
}

bool Esp::InitImpl() {
	auto& io = ImGui::GetIO();

	ImFontConfig cfg{};
	cfg.FontDataOwnedByAtlas = false;

	this->font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 12.0f, &cfg);

	return true;
}

void Esp::RenderImpl() {
	if (!cfg::enabled)
		return;

	auto d = ImGui::GetBackgroundDrawList();

	auto game = Cache::CopyGame();
	auto globals = Cache::CopyGlobals();
	auto players = Cache::CopyPlayers();

	auto io = ImGui::GetIO();

	ImGui::PushFont(this->font);

	static int local_team = 0;
	for (auto& player : players) {
		if (!player.alive)
			continue;

		if (player.localplayer) {
			local_team = player.team;
			continue;
		}

		bool mate = player.team == local_team;

		if (!cfg::esp::team && mate)
			continue;

		if (cfg::esp::spotted && !player.spotted)
			continue;

		RenderPlayer(player, mate);
	}

#ifdef _DEBUG
	static int margin = 10;
	static int padding = 10;
	std::string debug_string = "> Debug Window\n";

	debug_string += std::format("Map: {}\n", globals.map_name);
	debug_string += std::format("Cache Refresh: {}ms\n", cfg::dev::cache_refresh_rate);

	if (!players.empty())
		debug_string += std::format("Players ({}):\n", players.size());

	for (auto& player : players) 
		debug_string += std::format("- {} ({}) {}hp {}ms\n", player.name, player.steam_id, player.health, player.ping);

	auto size = ImGui::CalcTextSize(debug_string.data());

	d->AddRectFilled(
		ImVec2(10 + margin - padding, io.DisplaySize.y - size.y - 20 - margin - padding),
		ImVec2(10 + size.x + margin + padding, io.DisplaySize.y - 20 - margin + padding),
		IM_COL32(0, 0, 0, 200),
		10.f
	);

	d->AddRect(
		ImVec2(10 + margin - padding, io.DisplaySize.y - size.y - 20 - margin - padding),
		ImVec2(10 + size.x + margin + padding, io.DisplaySize.y - 20 - margin + padding),
		IM_COL32(100, 100, 100, 200),
		10.f
	);

	d->AddText(
		ImVec2(10 + margin, io.DisplaySize.y - size.y - 20 - margin),
		IM_COL32(255, 255, 255, 255),
		debug_string.data()
	);
#endif

	ImGui::PopFont();
}

void Esp::RenderPlayer(Player player, bool mate) {
	auto io = ImGui::GetIO();
	auto d = ImGui::GetBackgroundDrawList();
	this->matrix = Cache::CopyGame().view_matrix;

	// Needed for flags & item sizing, so even if the box is not enabled
	// Should be calculated
	std::pair<Vec2_t, Vec2_t> bounds;
	if (!player.GetBounds(matrix, io.DisplaySize, bounds))
		return;

	// Causes hp bars across the screen when they respawn
	if (!player.alive)
		return;

	if (cfg::esp::box) {
		auto color = mate ? cfg::esp::colors::box_team : cfg::esp::colors::box_enemy;

		d->AddRect(
			bounds.first,
			bounds.second,
			ImColor(color)
		);
	}

	if (cfg::esp::skeleton)
		RenderPlayerBones(player, mate);

	if (cfg::esp::head_tracker)
		RenderPlayerTracker(player, bounds, mate);

	RenderPlayerBars(player, bounds);
	RenderPlayerFalgs(player, bounds, mate);
}

void Esp::RenderPlayerBones(Player player, bool mate) {
	auto io = ImGui::GetIO();
	auto d = ImGui::GetBackgroundDrawList();

	auto color = mate ? cfg::esp::colors::skeleton_team : cfg::esp::colors::skeleton_enemy;

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
			ImColor(color),
			1.5f
		);
	}
}

void Esp::RenderPlayerTracker(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate) {
	auto io = ImGui::GetIO();
	auto d = ImGui::GetBackgroundDrawList();

	auto head_bone = player.bone_list[bone_index::head];

	Vec2_t head;
	if (!matrix.wts(head_bone.pos, io.DisplaySize, head))
		return;

	auto width = bounds.second.x - bounds.first.x;
	auto color = mate ? cfg::esp::colors::tracker_team : cfg::esp::colors::tracker_enemy;

	d->AddCircle(
		head,
		width / 6,
		ImColor(color),
		15
	);
}

void Esp::RenderPlayerBars(Player player, std::pair<Vec2_t, Vec2_t> bounds) {
	auto io = ImGui::GetIO();
	auto d = ImGui::GetBackgroundDrawList();

	if (cfg::esp::health) {
		auto x_start = bounds.first.x - 4; // -4 is padding
		auto x_end = x_start - 2; // -2 is the inner space of the rect

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
	}

	if (cfg::esp::armor) {
		auto y_start = bounds.second.y + 4; // 4 is padding
		auto y_end = y_start + 2; // 2 is the inner space of the rect

		auto x_start = bounds.first.x;
		auto x_end = bounds.second.x;

		float width = x_end - x_start;
		float filled_width = width * (player.armor / 100.0f);

		d->AddRectFilled(
			ImVec2(x_start, y_start),
			ImVec2(x_start + filled_width, y_end),
			IM_COL32(150, 150, 255, 255)
		);

		d->AddRect(
			ImVec2(x_start, y_start),
			ImVec2(x_end, y_end),
			IM_COL32(0, 0, 0, 50)
		);
	}
}

void Esp::RenderPlayerFalgs(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate) {
	auto io = ImGui::GetIO();
	auto d = ImGui::GetBackgroundDrawList();

	if (cfg::esp::flags::name) {
		auto sanitized_name = std::format("{}{}", player.name, (player.bot ? " (Bot)" : ""));
		auto name_size = ImGui::CalcTextSize(sanitized_name.data());

		d->AddText(
			Vec2_t(
				(bounds.first.x + bounds.second.x) / 2 - name_size.x / 2,
				bounds.first.y - 20
			), 
			IM_COL32(255, 255, 255, 255),
			sanitized_name.data()
		);
	}

	if (cfg::esp::flags::weapon) {
		auto weapon_size = ImGui::CalcTextSize(player.clean_weapon.data());

		d->AddText(
			Vec2_t(
				(bounds.first.x + bounds.second.x) / 2 - weapon_size.x / 2,
				bounds.second.y + 5
			), 
			IM_COL32(255, 255, 255, 255),
			player.clean_weapon.data()
		);
	}

	int offset = 0;
	static int offset_mult = 15;

	if (cfg::esp::flags::money && player.money) {
		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			IM_COL32(255, 255, 255, 255),
			std::format("{}$", player.money).c_str()
		);

		offset += offset_mult;
	}

	if (cfg::esp::flags::ping && player.ping) {
		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			IM_COL32(255, 255, 255, 255),
			std::format("{}ms", player.ping).c_str()
		);

		offset += offset_mult;
	}

	if (cfg::esp::flags::flashed && player.flashed) {
		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			IM_COL32(100, 100, 255, 255),
			"flashed"
		);

		offset += offset_mult;
	}

	if (cfg::esp::flags::defusing && player.defusing) {
		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			IM_COL32(255, 100, 100, 255),
			"defusing"
		);

		offset += offset_mult;
	}
}