#include "config.hpp"

namespace config {
	using json = nlohmann::json;
	const std::string file_path = "offsets.json";

	bool file_exsits(const std::string& name) {
		std::ifstream f(name.c_str());
		return f.good();
	}

	bool read() {
		if (!file_exsits(file_path)) {
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

		if (data["dwLocalPlayer"].is_number())
			dwLocalPlayer = data["dwLocalPlayer"];
		if (data["dwEntityList"].is_number())
			dwEntityList = data["dwEntityList"];
		if (data["dwViewMatrix"].is_number())
			dwViewMatrix = data["dwViewMatrix"];

		if (data["dwPawnHealth"].is_number())
			dwPawnHealth = data["dwPawnHealth"];
		if (data["dwPlayerPawn"].is_number())
			dwPlayerPawn = data["dwPlayerPawn"];
		if (data["dwSanitizedName"].is_number())
			dwSanitizedName = data["dwSanitizedName"];
		if (data["m_iTeamNum"].is_number())
			m_iTeamNum = data["m_iTeamNum"];
		if (data["m_vecOrigin"].is_number())
			m_vecOrigin = data["m_vecOrigin"];

		return true;
	}

	void save() {
		json data;

		data["dwLocalPlayer"] = dwLocalPlayer;
		data["dwEntityList"] = dwEntityList;
		data["dwViewMatrix"] = dwViewMatrix;

		data["dwPawnHealth"] = dwPawnHealth;
		data["dwPlayerPawn"] = dwPlayerPawn;
		data["dwSanitizedName"] = dwSanitizedName;
		data["m_iTeamNum"] = m_iTeamNum;
		data["m_vecOrigin"] = m_vecOrigin;

		std::ofstream output(file_path);
		output << std::setw(4) << data << std::endl;
		output.close();
	}
}