#include "Esp.hpp"

#include "gui/renderer/Renderer.hpp"

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
	// i don't see this reset anywhere else
	this->localplayer = nullptr;
	if (!cfg::enabled)
		return;

	auto snapshot = Cache::CopySnapshot();
	auto& game = snapshot.game;
	auto& bomb = snapshot.bomb;
	auto& local = snapshot.local;
	auto& globals = snapshot.globals;
	auto& players = snapshot.players;
	
	RenderSpectatorList(players);

	ImGui::PushFont(this->font);

	this->io = ImGui::GetIO();
	this->d = ImGui::GetBackgroundDrawList();

	this->matrix = game.view_matrix;

	RenderBomb(local, bomb);

	for (auto& player : players) {
		if (!player.alive)
			continue;

		if (player.localplayer)
			continue;

		bool mate = player.team == local.team;

		if (!cfg::esp::team && mate)
			continue;

		if (cfg::esp::spotted && !player.spotted)
			continue;

		// Are we spectating the player in first person? then dont render
		// TODO: Exception here when spectating someone
		if (
			local.observer_services.target == player.pawn_controller_addr
			&& local.observer_services.mode == ObserverMode::First
		)
			continue;

		RenderPlayerTracers(local, player, mate);
		RenderPlayer(player, mate);
	}

	RenderCrosshair(local);

	ImGui::PopFont();
}

void Esp::RenderPlayer(Player player, bool mate) {
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
	if (player.bone_list.empty())
		return;

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

		if (cfg::esp::health_number && player.health < 100) {
			auto txt = std::to_string(player.health);
			auto sz = ImGui::CalcTextSize(txt.c_str());

			d->AddText(
				Vec2_t(
					(x_start + x_end) * 0.5f - sz.x * 0.5f,
					y_end - filled_height - sz.y * 0.5f
				),
				IM_COL32(255, 255, 255, 255),
				txt.c_str()
			);
		}
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
		auto weapon_size = ImGui::CalcTextSize(player.weapon.name.data());

		d->AddText(
			Vec2_t(
				(bounds.first.x + bounds.second.x) / 2 - weapon_size.x / 2,
				bounds.second.y + 5
			), 
			IM_COL32(255, 255, 255, 255),
			player.weapon.name.data()
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
			IM_COL32(100, 255, 100, 255),
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

	if (cfg::esp::flags::scoped && player.scoped)
	{
		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			IM_COL32(100, 100, 255, 255),
			"scoped");

		offset += offset_mult;
	}
}

void Esp::RenderCrosshair(Player local)
{
	if (!cfg::settings::crosshair)
		return;

	//if (!localplayer)
	//	return;

	if (local.scoped)
		return;

	auto weapon = local.weapon;

	if (weapon.item_index == -1)
		return;

	//static std::vector<std::string> valid_weapons = { "ssg08", "awp", "g3sg1", "scar20" };
	static std::vector<WeaponIds> valid_weapons = { weapon_ssg08, weapon_awp, weapon_g3sg1, weapon_scar20 };

	if (std::find(valid_weapons.begin(), valid_weapons.end(), weapon.item_index) == valid_weapons.end())
		return;

	ImVec2 center(
		floorf(io.DisplaySize.x * 0.5f),
		floorf(io.DisplaySize.y * 0.5f));

	constexpr float size = 6.f;
	constexpr float thickness = 1.0f;

	d->AddLine(
		ImVec2(center.x - size, center.y),
		ImVec2(center.x + size + 1, center.y),
		IM_COL32(255, 255, 255, 255),

		thickness);
	d->AddLine(
		ImVec2(center.x, center.y - size),
		ImVec2(center.x, center.y + size + 1),
		IM_COL32(255, 255, 255, 255),
		thickness);
}

void Esp::RenderPlayerTracers(Player source, Player player, bool mate) {
	if (!cfg::esp::tracers)
		return;

	Vec2_t screenPos;
	bool projected = matrix.wts(player.pos, io.DisplaySize, screenPos, false);

	if (!projected)
	{
		Vec3_t camPos = source.pos;
		Vec3_t dir = player.pos - camPos;

		// projection for off screen players
		Vec3_t viewDir;
		viewDir.x = matrix[0][0] * dir.x + matrix[0][1] * dir.y + matrix[0][2] * dir.z;
		viewDir.y = matrix[1][0] * dir.x + matrix[1][1] * dir.y + matrix[1][2] * dir.z;
		viewDir.z = matrix[2][0] * dir.x + matrix[2][1] * dir.y + matrix[2][2] * dir.z;

		if (viewDir.z > 0.0f)
		{
			viewDir.x = -viewDir.x;
			viewDir.y = -viewDir.y;
		}

		// normalize
		float len = sqrt(viewDir.x * viewDir.x + viewDir.y * viewDir.y);
		if (len > 0.001f)
		{
			viewDir.x /= len;
			viewDir.y /= len;
		}

		screenPos.x = io.DisplaySize.x * 0.5f + viewDir.x * io.DisplaySize.x * 0.5f;
		screenPos.y = io.DisplaySize.y * 0.5f - viewDir.y * io.DisplaySize.y * 0.5f;

		float margin = 10.f;
		screenPos.x = std::clamp(screenPos.x, margin, io.DisplaySize.x - margin);
		screenPos.y = std::clamp(screenPos.y, margin, io.DisplaySize.y - margin);
	}

	auto color = mate ? cfg::esp::colors::tracer_team : cfg::esp::colors::tracer_enemy;

	d->AddLine(
		Vec2_t(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
		screenPos,
		ImColor(color),
		1.0f
	);
}

void Esp::RenderBomb(Player local, Bomb bomb) {
	if (!cfg::esp::bomb_location && !cfg::esp::bomb_timer)
		return;

	if (!bomb.is_planted)
		return;

	if (!bomb.pos.length())
		return;

	auto marker = bomb.pos + Vec3_t(0, 0, 20);

	Vec2_t pos;
	if (!matrix.wts(bomb.pos, io.DisplaySize, pos))
		return;

	if (!local.alive)
		return;

	auto distance = bomb.pos.dist_to(local.pos);

	float width = 20.f;
	float height = 20.f;
	float rounding = 10.f;

	static int margin = 10;
	static int padding = 10;

	//auto distance_str = std::format("{}pt", (int)distance);
	auto duration_str = std::format("{}s", bomb.time_left);
	auto bombsite_str = std::string(bomb.site == BombSite::A ? "A" : "B");

	std::string bomb_string = "";

	if (cfg::esp::bomb_location)
	{
		bomb_string += bombsite_str;
	}

	if (cfg::esp::bomb_timer)
	{
		if (cfg::esp::bomb_location)
			bomb_string += " - ";
		else
			bomb_string += " ";

		bomb_string += duration_str;
	}

	auto text_size = ImGui::CalcTextSize(bomb_string.data());
	width = text_size.x; height = text_size.y;

	d->AddRectFilled(
		ImVec2(pos.x + margin - padding, pos.y - height - 20 - margin - padding),
		ImVec2(pos.x + width + margin + padding, pos.y - 20 - margin + padding),
		IM_COL32(0, 0, 0, 200),
		rounding
	);

	d->AddRect(
		ImVec2(pos.x + margin - padding, pos.y - height - 20 - margin - padding),
		ImVec2(pos.x + width + margin + padding, pos.y - 20 - margin + padding),
		IM_COL32(100, 100, 100, 200),
		rounding
	);

	d->AddText(
		ImVec2(pos.x + margin, pos.y - height - 20 - margin),
		IM_COL32(255, 255, 255, 255),
		bomb_string.data()
	);
}

Player* FindPlayerByPawnIndex(std::vector<Player>& players, int index) {
	Player* found = nullptr;

	for (auto& p : players) {
		if (p.pawn_controller_addr == index) {
			found = &p;
			break;
		}
	}
	return found;
}

// TODO: Move this to Overlays.cpp
void Esp::RenderSpectatorList(std::vector<Player>& players) {
	if (!cfg::spectators::enabled) 
		return;

	static auto io = ImGui::GetIO();
	static auto screen = io.DisplaySize;
	const bool detailed = cfg::spectators::detailed;
	const bool self_only = cfg::spectators::self_only;
	const bool is_menu_open = Renderer::IsOpen();
	
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;
	ImGuiTableFlags flags_table = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersV;

	bool should_render = false;
	for (Player& p : players) {
		if (auto i = p.observer_services.target) {
			Player* target = FindPlayerByPawnIndex(players, i);

			if (self_only && (!target || !target->localplayer))
				continue;
		
			should_render = true;
			break;
		}
	}

	if (!should_render && !is_menu_open)
		return;

	// Window
	ImGui::SetNextWindowPos(ImVec2(cfg::spectators::pos.x, cfg::spectators::pos.y), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(150.f, 50.f), ImVec2(FLT_MAX, FLT_MAX));

	if (!ImGui::Begin("Spectator list", nullptr, flags)) {
		ImGui::End();
		return;
	}

	if (is_menu_open)
		cfg::spectators::pos = {
			ImGui::GetWindowPos().x,
			ImGui::GetWindowPos().y
		};

	if (!should_render && is_menu_open) {
		ImGui::TextDisabled("No spectators");
		return ImGui::End();
	}

	//const int columns = detailed ? 3 : 1;

	if (detailed) {
		if (ImGui::BeginTable("##detailed", 3, flags_table)) {
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Mode");
			ImGui::TableSetupColumn("Target");
			ImGui::TableHeadersRow();

			for (Player& player : players) {
				if (player.alive) continue;

				int targetIndex = player.observer_services.target;
				if (targetIndex == 0) continue;

				Player* target = FindPlayerByPawnIndex(players, targetIndex);

				if (self_only && (!target || !target->localplayer)) 
					continue;

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", player.name);

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", player.observer_services.ToString());

				ImGui::TableSetColumnIndex(2);
				if (self_only) ImGui::Text("You");
				else if (player.observer_services.mode == ObserverMode::Free) ImGui::Text("No One");
				else ImGui::Text("%s", target ? target->name : "Invalid/bomb");
			}

			ImGui::EndTable();
		}
	}
	else {
		for (Player& player : players) {
			if (player.alive) continue;
			int targetIndex = player.observer_services.target;
			if (targetIndex == 0) continue;
			Player* target = FindPlayerByPawnIndex(players, targetIndex);

			if (self_only && (!target || !target->localplayer)) continue;

			ImGui::Text("%s", player.name);
		}
	}

	ImGui::End();
}