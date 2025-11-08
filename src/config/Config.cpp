#include "Config.hpp"

bool Config::Read() {
	return GetInstance().Read();
}

bool Config::Write() {
	return GetInstance().Write();
}

bool Config::ReadImpl() {
	std::ifstream f("config.json");

	if (!f.good()) {
		LOGF(FATAL, "Failed to read configuration file");
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
		cfg::esp::team = data["esp"].value("team", false);
		cfg::esp::box = data["esp"].value("box", true);
		cfg::esp::skeleton = data["esp"].value("skeleton", true);
		cfg::esp::head_tracker = data["esp"].value("head_tracker", true);

		cfg::esp::flags::name = data["esp"]["flags"].value("name", true);
		cfg::esp::flags::health = data["esp"]["flags"].value("health", true);
		cfg::esp::flags::armor = data["esp"]["flags"].value("armor", true);
		cfg::esp::flags::defusing = data["esp"]["flags"].value("defusing", false);
		cfg::esp::flags::money = data["esp"]["flags"].value("money", false);
		cfg::esp::flags::flashed = data["esp"]["flags"].value("flashed", false);

		// utils
		cfg::utils::console = data["utils"].value("console", true);
		cfg::utils::streamproof = data["utils"].value("streamproof", false);
		cfg::utils::open_menu_key = data["utils"].value("open_menu_key", 0);
	} catch (const std::exception& e) {
		LOGF(FATAL, "Failed to parse configuration");
		WriteImpl();
		return false;
	}

	return true;
}

bool Config::WriteImpl() {
	json data;

	data["enabled"] = cfg::enabled;

	// esp
	data["esp"]["team"] = cfg::esp::team;
	data["esp"]["box"] = cfg::esp::box;
	data["esp"]["skeleton"] = cfg::esp::skeleton;
	data["esp"]["head_tracker"] = cfg::esp::head_tracker;

	data["esp"]["flags"]["name"] = cfg::esp::flags::name;
	data["esp"]["flags"]["health"] = cfg::esp::flags::health;
	data["esp"]["flags"]["armor"] = cfg::esp::flags::armor;
	data["esp"]["flags"]["defusing"] = cfg::esp::flags::defusing;
	data["esp"]["flags"]["money"] = cfg::esp::flags::money;
	data["esp"]["flags"]["flashed"] = cfg::esp::flags::flashed;

	// utils
	data["utils"]["console"] = cfg::utils::console;
	data["utils"]["streamproof"] = cfg::utils::streamproof;
	data["utils"]["open_menu_key"] = cfg::utils::open_menu_key;

	std::ofstream output("config.json");
	output << std::setw(4) << data << std::endl;
	output.close();

	return true;
}