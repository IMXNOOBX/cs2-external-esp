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

	inline int build_number = 13963;

	namespace offsets {
		inline std::ptrdiff_t dwLocalPlayer = 0x17E27C8;
		inline std::ptrdiff_t dwEntityList = 0x1793DC8;
		inline std::ptrdiff_t dwViewMatrix = 0x1881D70;
		inline std::ptrdiff_t dwBuildNumber = 0x487514;
		inline std::ptrdiff_t dwPlantedC4 = 0x1887FC0;

		inline std::ptrdiff_t m_flC4Blow = 0xEB0;
		inline std::ptrdiff_t m_flNextBeep = 0xEAC;
		inline std::ptrdiff_t m_flTimerLength = 0xEB8;

		inline std::ptrdiff_t m_pInGameMoneyServices = 0x6D0;
		inline std::ptrdiff_t m_iAccount = 0x40;
		inline std::ptrdiff_t m_vecAbsOrigin = 0xC8;
		inline std::ptrdiff_t m_pGameSceneNode = 0x310;
		inline std::ptrdiff_t m_bIsDefusing = 0x1390;
		inline std::ptrdiff_t m_szName = 0xC18;
		inline std::ptrdiff_t m_pClippingWeapon = 0x1290;
		inline std::ptrdiff_t m_ArmorValue = 0x14F0;
		inline std::ptrdiff_t m_iHealth = 0x32C;
		inline std::ptrdiff_t dwPlayerPawn = 0x7BC;
		inline std::ptrdiff_t dwSanitizedName = 0x720;
		inline std::ptrdiff_t m_iTeamNum = 0x3BF;
		inline std::ptrdiff_t m_vecOrigin = 0x1214;
	}
}
