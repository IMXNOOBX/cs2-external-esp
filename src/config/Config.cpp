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
		cfg::esp::skeleton = data["esp"].value("skeleton", true);
		cfg::esp::health = data["esp"]["flags"].value("health", true);
		cfg::esp::head_tracker = data["esp"].value("head_tracker", true);

		cfg::esp::flags::name = data["esp"]["flags"].value("name", true);
		cfg::esp::flags::armor = data["esp"]["flags"].value("armor", true);
		cfg::esp::flags::defusing = data["esp"]["flags"].value("defusing", false);
		cfg::esp::flags::money = data["esp"]["flags"].value("money", false);
		cfg::esp::flags::flashed = data["esp"]["flags"].value("flashed", false);

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
	data["esp"]["skeleton"] = cfg::esp::skeleton;
	data["esp"]["flags"]["health"] = cfg::esp::health;
	data["esp"]["head_tracker"] = cfg::esp::head_tracker;

	data["esp"]["flags"]["name"] = cfg::esp::flags::name;
	data["esp"]["flags"]["armor"] = cfg::esp::flags::armor;
	data["esp"]["flags"]["defusing"] = cfg::esp::flags::defusing;
	data["esp"]["flags"]["money"] = cfg::esp::flags::money;
	data["esp"]["flags"]["flashed"] = cfg::esp::flags::flashed;

	// utils
	data["utils"]["console"] = cfg::settings::console;
	data["utils"]["streamproof"] = cfg::settings::streamproof;
	data["utils"]["open_menu_key"] = cfg::settings::open_menu_key;

	f << std::setw(4) << data << std::endl;
	f.close();

	LOGF(VERBOSE, "Writting configuration to file");

	return true;
}