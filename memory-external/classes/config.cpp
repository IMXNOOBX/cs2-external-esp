#include "config.hpp"

namespace config {
    bool read() {
        if (!updater::file_good(file_path)) {
            save();
            return false;
        }

        std::ifstream f(file_path);
        if (!f.is_open() || f.fail()) {
            std::cerr << "[config] Failed to open config file for reading: " << file_path << std::endl;
            return false;
        }

        json data;
        try {
            data = json::parse(f);
        }
        catch (const std::exception& e) {
            std::cerr << "[config] JSON parse error: " << e.what() << " — saving defaults and aborting read." << std::endl;
            save();
            return false; // stop here, don't continue with empty data
        }

        if (data.empty())
            return false;

        if (data.contains("show_box_esp") && data["show_box_esp"].is_boolean())
            show_box_esp = data["show_box_esp"];
        if (data.contains("show_skeleton_esp") && data["show_skeleton_esp"].is_boolean())
            show_skeleton_esp = data["show_skeleton_esp"];
        if (data.contains("show_head_tracker") && data["show_head_tracker"].is_boolean())
            show_head_tracker = data["show_head_tracker"];
        if (data.contains("team_esp") && data["team_esp"].is_boolean())
            team_esp = data["team_esp"];
        if (data.contains("automatic_update") && data["automatic_update"].is_boolean())
            automatic_update = data["automatic_update"];
        if (data.contains("render_distance") && data["render_distance"].is_number())
            render_distance = data["render_distance"];
        if (data.contains("flag_render_distance") && data["flag_render_distance"].is_number())
            flag_render_distance = data["flag_render_distance"];
        if (data.contains("show_health_bars") && data["show_health_bars"].is_boolean())
            show_health_bars = data["show_health_bars"]; // fixed: previously used wrong var
        if (data.contains("show_health_flags") && data["show_health_flags"].is_boolean())
            show_health_flags = data["show_health_flags"];
        if (data.contains("show_money_flag") && data["show_money_flag"].is_boolean())
            show_money_flag = data["show_money_flag"];
        if (data.contains("show_distance_flag") && data["show_distance_flag"].is_boolean())
            show_distance_flag = data["show_distance_flag"];
        if (data.contains("show_name_flag") && data["show_name_flag"].is_boolean())
            show_name_flag = data["show_name_flag"];
        if (data.contains("show_weapon_flag") && data["show_weapon_flag"].is_boolean())
            show_weapon_flag = data["show_weapon_flag"];
        if (data.contains("show_defusing_flag") && data["show_defusing_flag"].is_boolean())
            show_defusing_flag = data["show_defusing_flag"];
        if (data.contains("show_flashed_flag") && data["show_flashed_flag"].is_boolean())
            show_flashed_flag = data["show_flashed_flag"];

        if (data.find("esp_box_color_team") != data.end() && data["esp_box_color_team"].is_array()) {
            esp_box_color_team = {
                data["esp_box_color_team"][0].get<int>(),
                data["esp_box_color_team"][1].get<int>(),
                data["esp_box_color_team"][2].get<int>()
            };
        }

        if (data.find("esp_box_color_enemy") != data.end() && data["esp_box_color_enemy"].is_array()) {
            esp_box_color_enemy = {
                data["esp_box_color_enemy"][0].get<int>(),
                data["esp_box_color_enemy"][1].get<int>(),
                data["esp_box_color_enemy"][2].get<int>()
            };
        }

        // Fixed: assign to skeleton colors, not box colors
        if (data.find("esp_skeleton_color_team") != data.end() && data["esp_skeleton_color_team"].is_array()) {
            esp_skeleton_color_team = {
                data["esp_skeleton_color_team"][0].get<int>(),
                data["esp_skeleton_color_team"][1].get<int>(),
                data["esp_skeleton_color_team"][2].get<int>()
            };
        }

        if (data.find("esp_skeleton_color_enemy") != data.end() && data["esp_skeleton_color_enemy"].is_array()) {
            esp_skeleton_color_enemy = {
                data["esp_skeleton_color_enemy"][0].get<int>(),
                data["esp_skeleton_color_enemy"][1].get<int>(),
                data["esp_skeleton_color_enemy"][2].get<int>()
            };
        }

        if (data.find("esp_name_color") != data.end() && data["esp_name_color"].is_array()) {
            esp_name_color = {
                data["esp_name_color"][0].get<int>(),
                data["esp_name_color"][1].get<int>(),
                data["esp_name_color"][2].get<int>()
            };
        }

        if (data.find("esp_distance_color") != data.end() && data["esp_distance_color"].is_array()) {
            esp_distance_color = {
                data["esp_distance_color"][0].get<int>(),
                data["esp_distance_color"][1].get<int>(),
                data["esp_distance_color"][2].get<int>()
            };
        }

        return true;
    }

	void save() {
		json data;
		
		data["show_box_esp"] = show_box_esp;
		data["show_skeleton_esp"] = show_skeleton_esp;
		data["show_head_tracker"] = show_head_tracker;
		data["team_esp"] = team_esp;
		data["automatic_update"] = automatic_update;
		data["render_distance"] = render_distance;
		data["flag_render_distance"] = flag_render_distance;
		data["show_health_bars"] = show_health_bars;
		data["show_health_flags"] = show_health_flags;
		data["show_money_flag"] = show_money_flag;
		data["show_distance_flag"] = show_distance_flag;
		data["show_name_flag"] = show_name_flag;
		data["show_weapon_flag"] = show_weapon_flag;
		data["show_defusing_flag"] = show_defusing_flag;
		data["show_flashed_flag"] = show_flashed_flag;

		data["esp_box_color_team"] = { esp_box_color_team.r, esp_box_color_team.g, esp_box_color_team.b };
		data["esp_box_color_enemy"] = { esp_box_color_enemy.r, esp_box_color_enemy.g, esp_box_color_enemy.b };
		data["esp_skeleton_color_team"] = { esp_box_color_team.r, esp_box_color_team.g, esp_box_color_team.b };
		data["esp_skeleton_color_enemy"] = { esp_box_color_enemy.r, esp_box_color_enemy.g, esp_box_color_enemy.b };
		data["esp_name_color"] = { esp_name_color.r, esp_name_color.g, esp_name_color.b };
		data["esp_distance_color"] = { esp_distance_color.r, esp_distance_color.g, esp_distance_color.b };

		//data["rainbow"] = rainbow;
		//data["rainbow_speed"] = rainbow_speed;

		std::ofstream output(file_path);
		output << std::setw(4) << data << std::endl;
		output.close();

		utils.update_console_title();
	}
}
