#pragma once
#include <fstream>
#include "json.hpp"
#include "auto_updater.hpp"

using json = nlohmann::json;

namespace config {
	const std::string file_path = "config.json";

	extern bool read();
	extern void save();

	inline bool automatic_update = false;
	inline bool team_esp = false;
	inline float render_distance = -1.f; 
}