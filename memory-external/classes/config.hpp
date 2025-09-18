#pragma once
#include <fstream>
#include "utils.h"
#include "json.hpp"
#include "auto_updater.hpp"

using json = nlohmann::json;

struct RGB {
	int r;
	int g;
	int b;

	// Conversion function from RGB to COLORREF
	operator COLORREF() const {
		return RGB(r, g, b);
	}
};

namespace config {
	const std::string file_path = "config.json";

	extern bool read();
	extern void save();

	inline bool automatic_update = false;
	inline bool team_esp = false;
	inline float render_distance = -1.f;
	inline int flag_render_distance = 200;
	inline bool show_box_esp = true;
	inline bool show_skeleton_esp = false;
	inline bool show_head_tracker = false;
	inline bool show_health_bars = true;
	inline bool show_health_flags = true;
	inline bool show_money_flag = false;
	inline bool show_distance_flag = false;
	inline bool show_name_flag = true;
	inline bool show_weapon_flag = false;
	inline bool show_defusing_flag = false;
	inline bool show_flashed_flag = false;

	inline RGB esp_box_color_team = { 75, 175, 75 };
	inline RGB esp_box_color_enemy = { 225, 75, 75 };
	inline RGB esp_skeleton_color_team = { 75, 175, 75 };
	inline RGB esp_skeleton_color_enemy = { 225, 75, 75 };
	inline RGB esp_name_color = { 250, 250, 250 };
	inline RGB esp_distance_color = { 75, 75, 175 };
}
