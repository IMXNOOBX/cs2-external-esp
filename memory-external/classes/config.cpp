#include "config.hpp"

namespace config {
	bool read() {
		if (!updater::file_good(file_path)) {
			save();
			return false;
		}

		std::ifstream f(file_path);

		json data;
		try {
			data = json::parse(f);
		}
		catch (const std::exception& e) {
			save();
		}

		if (data.empty())
			return false;

		if (data["team_esp"].is_boolean())
			team_esp = data["team_esp"];
		if (data["automatic_update"].is_boolean())
			automatic_update = data["automatic_update"];
		if (data["render_distance"].is_number())
			render_distance = data["render_distance"];
		if (data["show_distance"].is_boolean())
			show_distance = data["show_distance"];
		if (data["show_armor"].is_boolean())
			show_armor = data["show_armor"];
		if (data["rainbow"].is_boolean())
			rainbow = data["rainbow"];
		if (data["rainbow_speed"].is_number())
			rainbow_speed = data["rainbow_speed"];
		if (data["head_tracker"].is_boolean())
			head_tracker = data["head_tracker"];

		return true;
	}

	void save() {
		json data;

		data["team_esp"] = team_esp;
		data["automatic_update"] = automatic_update;
		data["render_distance"] = render_distance;
		data["show_distance"] = show_distance;
		data["show_armor"] = show_armor;
		data["rainbow"] = rainbow;
		data["rainbow_speed"] = rainbow_speed;
		data["head_tracker"] = head_tracker;
		
		std::ofstream output(file_path);
		output << std::setw(4) << data << std::endl;
		output.close();
	}
}
