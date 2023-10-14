#pragma once
#include <Windows.h>
#include <WinINet.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include "json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;


namespace updater {
	const std::string file_path = "offsets.json";

#ifndef _UC
	bool check_and_update(bool prompt_update);
	bool get_last_commit_date(json& commit);
	bool download_file(const char* url, const char* localPath);
#endif
	bool file_good(const std::string& name);

	extern bool read();
	extern void save();

	const inline std::string github_repo_api = "https://api.github.com/repos/IMXNOOBX/cs2-external-esp/commits";
	const inline std::string raw_updated_offets = "https://github.com/IMXNOOBX/cs2-external-esp/raw/main/offsets/offsets.json";

	namespace offsets {
		inline std::ptrdiff_t dwLocalPlayer = 0x17DE508;
		inline std::ptrdiff_t dwEntityList = 0x178FC88;
		inline std::ptrdiff_t dwViewMatrix = 0x187DAB0;

		// inline std::ptrdiff_t m_iPawnArmor = 0x80c;
		inline std::ptrdiff_t m_iHealth = 0x32C;
		inline std::ptrdiff_t dwPlayerPawn = 0x7BC;
		inline std::ptrdiff_t dwSanitizedName = 0x720;
		inline std::ptrdiff_t m_iTeamNum = 0x3BF;
		inline std::ptrdiff_t m_vecOrigin = 0x1214;
	}
}
