#include "Esp.hpp"

#include "core/vischeck/VisCheckManager.h"
#include "gui/renderer/Renderer.hpp"
#include "assets/fonts/WeaponIcons.h"
#include "assets/fonts/Icons.h"

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

	this->font_merged_icons = io.Fonts->AddFontFromMemoryTTF(
		weapon_icon_font,
		weapon_icon_font_len,
		16.0f,
		&cfg
	);

	cfg.MergeMode = true;

	static const ImWchar general_ranges[] = { 0xE100, 0xE108, 0 };
	io.Fonts->AddFontFromMemoryTTF(
		icons_font,
		icons_font_len,
		16.0f,
		&cfg,
		general_ranges
	);

	return true;
}

void Esp::RenderImpl() {
	if (!cfg::enabled)
		return;

	auto snapshot = Cache::CopySnapshot();
	auto& game = snapshot.game;
	auto& bomb = snapshot.bomb;
	auto& local = snapshot.local;
	auto& globals = snapshot.globals;
	auto& players = snapshot.players;
	
	ImGui::PushFont(this->font);

	this->io = ImGui::GetIO();
	this->d = ImGui::GetBackgroundDrawList();

	this->matrix = game.view_matrix;

	auto now = std::chrono::steady_clock::now();

	for (auto& player : players) {
		if (!player.alive)
			continue;
		if (player.localplayer)
			continue;

		bool mate = player.team == local.team;

		auto& track = tracks[player.index];
		track.last_seen = now;

		if (cfg::esp::sound) {
			if (!cfg::esp::team && mate)
				goto skip_sound;

			if (cfg::esp::sound_footsteps) {
				float speed = player.vel.length();
				constexpr float step_threshold = 5.0f;
				if (speed > step_threshold) {
					auto elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(
						now - track.last_footstep
					).count();
					if (elapsed > 0.35f) {
						track.last_footstep = now;
						sound_markers.push_back({ player.pos, now, mate, 0 });
					}
				}
			}

			if (cfg::esp::sound_gunfire && player.ammo != -1) {
				if (track.last_ammo >= 0 && player.ammo < track.last_ammo) {
					sound_markers.push_back({ player.pos, now, mate, 1 });
				}
			}

			if (cfg::esp::sound_gunfire) {
				if (player.is_reloading && !track.was_reloading) {
					sound_markers.push_back({ player.pos, now, mate, 2 });
				}
				track.was_reloading = player.is_reloading;
			}
		}
		skip_sound:

		if (cfg::esp::hit_markers) {
			if (track.last_health > 0 && player.health < track.last_health) {
				int damage = track.last_health - player.health;
				Vec3_t hit_pos = player.pos;
				if ((int)player.bone_list.size() > bone_index::chest)
					hit_pos = player.bone_list[bone_index::chest].pos;
				hit_markers.push_back({ hit_pos, now, mate, damage });
			}
		}

		if (cfg::esp::hit_markers)
			track.last_health = player.health;
		if (cfg::esp::sound_gunfire)
			track.last_ammo = static_cast<float>(player.ammo);
	}

	if (cfg::esp::sound) {
		auto fade_duration = std::chrono::duration<float>(cfg::esp::sound_fade);
		for (auto it = sound_markers.begin(); it != sound_markers.end(); ) {
			auto age = std::chrono::duration_cast<std::chrono::duration<float>>(now - it->birth);
			if (age >= fade_duration) {
				it = sound_markers.erase(it);
				continue;
			}

			Vec2_t screen;
			if (matrix.wts(it->pos, io.DisplaySize, screen)) {
				float alpha_ratio = 1.0f - (age.count() / cfg::esp::sound_fade);
				color_t base_color;
				switch (it->kind) {
					case 0: base_color = it->mate ? cfg::esp::colors::sound::footstep_team : cfg::esp::colors::sound::footstep_enemy; break;
					case 1: base_color = it->mate ? cfg::esp::colors::sound::gunfire_team : cfg::esp::colors::sound::gunfire_enemy; break;
					case 2: base_color = it->mate ? cfg::esp::colors::sound::reload_team : cfg::esp::colors::sound::reload_enemy; break;
					default: base_color = it->mate ? cfg::esp::colors::sound::footstep_team : cfg::esp::colors::sound::footstep_enemy; break;
				}
				ImColor col(base_color.r, base_color.g, base_color.b, base_color.a * alpha_ratio);

				float radius = 10.0f + (1.0f - alpha_ratio) * 15.0f;
				int segments = 12;
				d->AddCircle(screen, radius, col, segments);
				d->AddCircle(screen, radius * 0.65f, col, segments);

				const char* label;
				switch (it->kind) {
					case 0: label = "step"; break;
					case 1: label = "fire"; break;
					case 2: label = "reload"; break;
					default: label = "?"; break;
				}
				auto txt_sz = ImGui::CalcTextSize(label);
				d->AddText(
					Vec2_t(screen.x - txt_sz.x * 0.5f, screen.y - radius - txt_sz.y - 2),
					ImColor(1.0f, 1.0f, 1.0f, alpha_ratio),
					label
				);
			}
			++it;
		}
	}

	if (cfg::esp::hit_markers) {
		auto hm_fade = std::chrono::duration<float>(cfg::esp::hit_marker_fade);
		for (auto it = hit_markers.begin(); it != hit_markers.end(); ) {
			auto age = std::chrono::duration_cast<std::chrono::duration<float>>(now - it->birth);
			if (age >= hm_fade) {
				it = hit_markers.erase(it);
				continue;
			}

			Vec2_t screen;
			if (matrix.wts(it->pos, io.DisplaySize, screen)) {
				float alpha = 1.0f - (age.count() / cfg::esp::hit_marker_fade);
				auto col = cfg::esp::colors::hit_marker;
				ImColor c(col.r, col.g, col.b, col.a * alpha);
				constexpr float size = 5.f;

				d->AddLine(
					Vec2_t(screen.x - size, screen.y - size),
					Vec2_t(screen.x + size, screen.y + size),
					c, 1.5f
				);
				d->AddLine(
					Vec2_t(screen.x + size, screen.y - size),
					Vec2_t(screen.x - size, screen.y + size),
					c, 1.5f
				);

				auto txt = std::to_string(it->damage);
				auto txt_sz = ImGui::CalcTextSize(txt.c_str());
				d->AddText(
					Vec2_t(screen.x - txt_sz.x * 0.5f, screen.y - size - txt_sz.y - 2),
					ImColor(1.f, 1.f, 1.f, alpha),
					txt.c_str()
				);
			}
			++it;
		}
	}

	const Vec3_t eye_pos = local.bone_list.size() > bone_index::head
		? local.bone_list[bone_index::head].pos
		: local.pos + Vec3_t(0, 0, 64.f);
	const bool vis_ready = (cfg::esp::spotted || cfg::esp::los_spotted) && VisCheckManager::IsReady();
	static uint8_t vis_hold[64]{};

	for (auto& player : players) {
		if (!player.alive)
			continue;

		if (player.localplayer)
			continue;

		bool mate = player.team == local.team;

		if (!cfg::esp::team && mate)
			continue;

		if (
			local.observer_services.target == player.pawn_controller_addr
			&& local.observer_services.mode == ObserverMode::First
		)
			continue;

		bool visible = false;
		if (vis_ready && player.bone_list.size() > bone_index::pelvis) {
			const auto& bones = player.bone_list;
			bool has_los = VisCheckManager::IsVisible(eye_pos, bones[bone_index::head].pos)
				|| VisCheckManager::IsVisible(eye_pos, bones[bone_index::chest].pos);

			if (cfg::esp::los_extra_bones) {
				has_los = has_los
					|| VisCheckManager::IsVisible(eye_pos, bones[bone_index::shoulder_L].pos)
					|| VisCheckManager::IsVisible(eye_pos, bones[bone_index::shoulder_R].pos)
					|| VisCheckManager::IsVisible(eye_pos, bones[bone_index::pelvis].pos);
			}

			auto& h = vis_hold[player.index & 63];
			if (has_los)
				h = 6;
			else if (h)
				--h;

			visible = h > 0;
		}

		if (cfg::esp::spotted && !visible)
			continue;

		RenderPlayerTracers(local, player, mate);
		RenderPlayer(player, mate, visible);
	}

	for (auto it = tracks.begin(); it != tracks.end(); ) {
		if (now - it->second.last_seen > std::chrono::seconds(3))
			it = tracks.erase(it);
		else
			++it;
	}

	RenderCrosshair(local);
	RenderBombBox(bomb);
	ImGui::PopFont();
}

void Esp::RenderPlayer(Player player, bool mate, bool visible) {
	std::pair<Vec2_t, Vec2_t> bounds;
	if (!player.GetBounds(matrix, io.DisplaySize, bounds))
		return;
	if (!player.alive)
		return;

	const bool use_vis = visible && cfg::esp::los_spotted && cfg::esp::los_use_visible_colors;

	if (cfg::esp::box) {
		auto color = mate ? cfg::esp::colors::box_team : cfg::esp::colors::box_enemy;
		if (use_vis)
			color = mate ? cfg::esp::colors::los_visible_team : cfg::esp::colors::los_visible_enemy;
		d->AddRect(bounds.first, bounds.second, ImColor(color));
	}

	if (cfg::esp::box && cfg::esp::box_3d)
		RenderPlayerBox3D(player, bounds, mate, use_vis);

	if (cfg::esp::skeleton)
		RenderPlayerBones(player, mate, use_vis);

	if (cfg::esp::head_tracker)
		RenderPlayerTracker(player, bounds, mate, use_vis);

	RenderPlayerBars(player, bounds);
	RenderPlayerFalgs(player, bounds, mate);
}

void Esp::RenderPlayerBones(Player player, bool mate, bool use_vis) {
	auto color = mate ? cfg::esp::colors::skeleton_team : cfg::esp::colors::skeleton_enemy;
	if (use_vis)
		color = mate ? cfg::esp::colors::los_visible_team : cfg::esp::colors::los_visible_enemy;

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

void Esp::RenderPlayerTracker(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate, bool use_vis) {
	if (player.bone_list.empty())
		return;

	auto head_bone = player.bone_list[bone_index::head];

	Vec2_t head;
	if (!matrix.wts(head_bone.pos, io.DisplaySize, head))
		return;

	auto width = bounds.second.x - bounds.first.x;
	auto color = mate ? cfg::esp::colors::tracker_team : cfg::esp::colors::tracker_enemy;
	if (use_vis)
		color = mate ? cfg::esp::colors::los_visible_team : cfg::esp::colors::los_visible_enemy;

	d->AddCircle(
		head,
		width / 6,
		ImColor(color),
		15
	);
}

void Esp::RenderPlayerBars(Player player, std::pair<Vec2_t, Vec2_t> bounds) {
	if (cfg::esp::health) {
		auto x_start = bounds.first.x - 4;
		auto x_end = x_start - 2;

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
		auto y_start = bounds.second.y + 4;
		auto y_end = y_start + 2;

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

	if (cfg::esp::flags::ammo && player.ammo != -1) {
		auto txt = std::to_string(player.ammo);
		auto ammo_size = ImGui::CalcTextSize(txt.c_str());

		d->AddText(
			Vec2_t(
				(bounds.first.x + bounds.second.x) / 2 - ammo_size.x / 2,
				bounds.second.y + 20
			),
			IM_COL32(255, 255, 255, 255),
			txt.data()
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

		offset -= offset_mult;
	}

	if (cfg::esp::flags::ping && player.ping) {
		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			IM_COL32(255, 255, 255, 255),
			std::format("{}ms", player.ping).c_str()
		);

		offset -= offset_mult;
	}

	ImGui::PushFont(this->font_merged_icons);

	if (cfg::esp::flags::flashed && player.flashed || cfg::dev::force_show_flags) {
		auto color = mate ? cfg::esp::colors::flags::flashed_team : cfg::esp::colors::flags::flashed_enemy;

		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			ImColor(color),
			Icons::BLIND
		);

		offset -= offset_mult;
	}

	if (cfg::esp::flags::reloading && player.is_reloading || cfg::dev::force_show_flags) {
		auto color = mate ? cfg::esp::colors::flags::reloading_team : cfg::esp::colors::flags::reloading_enemy;

		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			ImColor(color),
			Icons::RELOAD
		);

		offset -= offset_mult;
	}

	if (cfg::esp::flags::defusing && player.defusing || cfg::dev::force_show_flags) {
		auto color = mate ? cfg::esp::colors::flags::defusing_team : cfg::esp::colors::flags::defusing_enemy;

		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			ImColor(color),
			WeaponIcons::CUTTERS
		);

		offset -= offset_mult;
	}

	if (cfg::esp::flags::scoped && player.scoped || cfg::dev::force_show_flags) {
		auto color = mate ? cfg::esp::colors::flags::scoped_team : cfg::esp::colors::flags::scoped_enemy;

		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			ImColor(color),
			WeaponIcons::SCOPE
		);

		offset -= offset_mult;
	}

	if (cfg::esp::flags::weapon) {
		auto weapon_size = ImGui::CalcTextSize(player.weapon.icon);

		d->AddText(
			Vec2_t(
				(bounds.first.x + bounds.second.x) / 2 - weapon_size.x / 2,
				bounds.second.y + 6
			),
			IM_COL32(255, 255, 255, 255),
			player.weapon.icon
		);
	}

	if (cfg::esp::flags::has_c4 && player.has_c4 || cfg::dev::force_show_flags) {
		auto color = mate ? cfg::esp::colors::flags::c4_team : cfg::esp::colors::flags::c4_enemy;

		ImGui::PushFont(this->font_merged_icons);
		auto icon_size = ImGui::CalcTextSize(WeaponIcons::C4);
		ImGui::PopFont();

		// Flash the icon if they're holding the C4, presumably planting since nobody just holds it really
		ImColor draw_color = ImColor(color);
		if (player.weapon.item_index == weapon_c4) {
			float alpha = 0.5f + 0.5f * sinf((float)ImGui::GetTime() * 8.0f);
			draw_color = ImColor(color.r, color.g, color.b, alpha);
		}

		d->AddText(
			this->font_merged_icons,
			16.0f,
			Vec2_t(
				(bounds.first.x + bounds.second.x) / 2 - icon_size.x / 2,
				bounds.first.y - 20 - icon_size.y - 2
			),
			draw_color,
			WeaponIcons::C4
		);

		offset -= offset_mult;
	}

	ImGui::PopFont();
}

void Esp::RenderBombBox(Bomb bomb) {
	if (!cfg::esp::bomb)
		return;

	if (!bomb.is_planted)
		return;

	float w = 10.f, l = 5.f, h = 10.f;
	Vec3_t half_size = { w / 2.f, h / 2.f, l / 2.f };

	Vec3_t corners[8] = {
		{ bomb.pos.x - half_size.x, bomb.pos.y - half_size.y, bomb.pos.z - half_size.z },
		{ bomb.pos.x + half_size.x, bomb.pos.y - half_size.y, bomb.pos.z - half_size.z },
		{ bomb.pos.x + half_size.x, bomb.pos.y - half_size.y, bomb.pos.z + half_size.z },
		{ bomb.pos.x - half_size.x, bomb.pos.y - half_size.y, bomb.pos.z + half_size.z },
		{ bomb.pos.x - half_size.x, bomb.pos.y + half_size.y, bomb.pos.z - half_size.z },
		{ bomb.pos.x + half_size.x, bomb.pos.y + half_size.y, bomb.pos.z - half_size.z },
		{ bomb.pos.x + half_size.x, bomb.pos.y + half_size.y, bomb.pos.z + half_size.z },
		{ bomb.pos.x - half_size.x, bomb.pos.y + half_size.y, bomb.pos.z + half_size.z },
	};

	Vec2_t projected[8];
	bool visible[8] = { false };
	int visible_count = 0;

	for (int i = 0; i < 8; ++i) {
		if (matrix.wts(corners[i], io.DisplaySize, projected[i])) {
			visible[i] = true;
			visible_count++;
		}
	}

	if (visible_count == 0)
		return;

	auto color = cfg::esp::colors::bomb;

	int edges[12][2] = {
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, // Bottom
		{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, // Top
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }  // Verticals
	};

	for (auto& edge : edges) {
		int i = edge[0];
		int j = edge[1];
		if (visible[i] && visible[j]) {
			d->AddLine(projected[i], projected[j], ImColor(color), 1.0f);
		}
	}

	Vec2_t screen;
	if (!matrix.wts(bomb.pos + Vec3_t(0, 0, 8), io.DisplaySize, screen))
		return;

	ImGui::PushFont(this->font_merged_icons);
	d->AddText(
		this->font_merged_icons,
		16.0f,
		Vec2_t(
			screen.x - 8, // lazy
			screen.y
		),
		ImColor(255, 255, 255),
		WeaponIcons::C4
	);
	ImGui::PopFont();
}

void Esp::RenderCrosshair(Player local)
{
	if (!cfg::world::crosshair::enabled)
		return;

	if (local.scoped)
		return;

	auto weapon = local.weapon;

	if (weapon.item_index == -1)
		return;

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

void Esp::RenderPlayerBox3D(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate, bool use_vis) {
	float height = 72.0f;
	float width = 32.0f;
	float depth = 32.0f;

	Vec3_t base = player.pos;
	Vec3_t top = base + Vec3_t(0, 0, height);

	Vec3_t corners[8] = {
		{ base.x - width / 2, base.y - depth / 2, base.z },
		{ base.x + width / 2, base.y - depth / 2, base.z },
		{ base.x + width / 2, base.y + depth / 2, base.z },
		{ base.x - width / 2, base.y + depth / 2, base.z },
		{ top.x - width / 2, top.y - depth / 2, top.z },
		{ top.x + width / 2, top.y - depth / 2, top.z },
		{ top.x + width / 2, top.y + depth / 2, top.z },
		{ top.x - width / 2, top.y + depth / 2, top.z },
	};

	Vec2_t projected[8];
	for (int i = 0; i < 8; ++i) {
		if (!matrix.wts(corners[i], io.DisplaySize, projected[i]))
			return;
	}

	auto color = mate ? cfg::esp::colors::box_team : cfg::esp::colors::box_enemy;
	if (use_vis)
		color = mate ? cfg::esp::colors::los_visible_team : cfg::esp::colors::los_visible_enemy;
	ImColor col(color);
	float thickness = 1.5f;

	d->AddLine(projected[0], projected[1], col, thickness);
	d->AddLine(projected[1], projected[2], col, thickness);
	d->AddLine(projected[2], projected[3], col, thickness);
	d->AddLine(projected[3], projected[0], col, thickness);

	d->AddLine(projected[4], projected[5], col, thickness);
	d->AddLine(projected[5], projected[6], col, thickness);
	d->AddLine(projected[6], projected[7], col, thickness);
	d->AddLine(projected[7], projected[4], col, thickness);

	d->AddLine(projected[0], projected[4], col, thickness);
	d->AddLine(projected[1], projected[5], col, thickness);
	d->AddLine(projected[2], projected[6], col, thickness);
	d->AddLine(projected[3], projected[7], col, thickness);
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