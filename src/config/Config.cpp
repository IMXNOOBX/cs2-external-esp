#include "Config.hpp"

bool Config::Read() {
	return GetInstance().ReadImpl();
}

bool Config::Write() {
	return GetInstance().WriteImpl();
}

bool Config::ReadImpl() {
	std::ifstream f("config.json");

	if (!f.good()) {
		LOGF(FATAL, "Configuration file does not exist, creating a new one");
		WriteImpl();
		return false;
	}

	json data;
	try {
		data = json::parse(f);
	}
	catch (const std::exception& e) {
		LOGF(FATAL, "Failed to parse configuration file");
		WriteImpl();
		return false;
	}

	if (data.empty())
		return false;

	try {
		// general
		cfg::enabled = data.value("enabled", true);

		// esp
		cfg::esp::box = data["esp"].value("box", true);
		cfg::esp::team = data["esp"].value("team", false);
		cfg::esp::armor = data["esp"].value("armor", true);
		cfg::esp::health = data["esp"].value("health", true);
		cfg::esp::skeleton = data["esp"].value("skeleton", true);
		cfg::esp::head_tracker = data["esp"].value("head_tracker", true);

		// flags
		cfg::esp::flags::name = data["esp"]["flags"].value("name", true);
		cfg::esp::flags::defusing = data["esp"]["flags"].value("defusing", false);
		cfg::esp::flags::money = data["esp"]["flags"].value("money", false);
		cfg::esp::flags::flashed = data["esp"]["flags"].value("flashed", false);

		// colors
		const auto& col = data["esp"]["colors"];
		cfg::esp::colors::box_team = JsonToColor(col, "box_team", { 0.29f, 0.69f, 0.29f, 1.f });
		cfg::esp::colors::box_enemy = JsonToColor(col, "box_enemy", { 0.88f, 0.29f, 0.29f, 1.f });
		cfg::esp::colors::skeleton_team = JsonToColor(col, "skeleton_team", { 0.29f, 0.69f, 0.29f, 1.f });
		cfg::esp::colors::skeleton_enemy = JsonToColor(col, "skeleton_enemy", { 0.88f, 0.29f, 0.29f, 1.f });
		cfg::esp::colors::tracker_team = JsonToColor(col, "tracker_team", { 0.98f, 0.98f, 0.98f, 1.f });
		cfg::esp::colors::tracker_enemy = JsonToColor(col, "tracker_enemy", { 0.98f, 0.98f, 0.98f, 1.f });

		// utils
		cfg::settings::console = data["utils"].value("console", true);
		cfg::settings::streamproof = data["utils"].value("streamproof", false);
		cfg::settings::open_menu_key = data["utils"].value("open_menu_key", 0);
	} catch (const std::exception& e) {
		LOGF(FATAL, "Failed to parse configuration");
		WriteImpl();
		return false;
	}

	LOGF(INFO, "Successfully parsed configuration");
	return true;
}

bool Config::WriteImpl() {
	std::ofstream f("config.json");

	json data;

	data["enabled"] = cfg::enabled;

	// esp
	data["esp"]["box"] = cfg::esp::box;
	data["esp"]["team"] = cfg::esp::team;
	data["esp"]["armor"] = cfg::esp::armor;
	data["esp"]["health"] = cfg::esp::health;
	data["esp"]["skeleton"] = cfg::esp::skeleton;
	data["esp"]["head_tracker"] = cfg::esp::head_tracker;

	// falgs
	data["esp"]["flags"]["name"] = cfg::esp::flags::name;
	data["esp"]["flags"]["defusing"] = cfg::esp::flags::defusing;
	data["esp"]["flags"]["money"] = cfg::esp::flags::money;
	data["esp"]["flags"]["flashed"] = cfg::esp::flags::flashed;

	// colors
	auto& col = data["esp"]["colors"];
	ColorToJson(col, "box_team", cfg::esp::colors::box_team);
	ColorToJson(col, "box_enemy", cfg::esp::colors::box_enemy);
	ColorToJson(col, "skeleton_team", cfg::esp::colors::skeleton_team);
	ColorToJson(col, "skeleton_enemy", cfg::esp::colors::skeleton_enemy);
	ColorToJson(col, "tracker_team", cfg::esp::colors::tracker_team);
	ColorToJson(col, "tracker_enemy", cfg::esp::colors::tracker_enemy);

	// utils
	data["utils"]["console"] = cfg::settings::console;
	data["utils"]["streamproof"] = cfg::settings::streamproof;
	data["utils"]["open_menu_key"] = cfg::settings::open_menu_key;

	f << std::setw(4) << data << std::endl;
	f.close();

	LOGF(VERBOSE, "Writting configuration to file");

	return true;
}

color_t Config::JsonToColor(const json& parent, const std::string& key, const color_t& def) {
	if (!parent.contains(key) || !parent[key].is_array() || parent[key].size() != 4)
		return def;
	return color_t(
		parent[key][0].get<float>(),
		parent[key][1].get<float>(),
		parent[key][2].get<float>(),
		parent[key][3].get<float>()
	);
}

void Config::ColorToJson(json& parent, const std::string& key, const color_t& color) {
	parent[key] = { color.r, color.g, color.b, color.a };
}