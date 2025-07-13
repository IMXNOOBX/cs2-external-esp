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
	const inline std::string raw_updated_offsets = "https://github.com/IMXNOOBX/cs2-external-esp/raw/main/offsets/offsets.json";

	inline int build_number = 0;

	namespace offsets {
		inline std::ptrdiff_t dwLocalPlayerController = 0x0;
		inline std::ptrdiff_t dwEntityList = 0x0;
		inline std::ptrdiff_t dwViewMatrix = 0x0;
		inline std::ptrdiff_t dwBuildNumber = 0x0;
		inline std::ptrdiff_t dwPlantedC4 = 0x0;

		inline std::ptrdiff_t m_flC4Blow = 0x0;
		inline std::ptrdiff_t m_flNextBeep = 0x0;
		inline std::ptrdiff_t m_flTimerLength = 0x0;

		inline std::ptrdiff_t m_pInGameMoneyServices = 0x0;
		inline std::ptrdiff_t m_iAccount = 0x0;
		inline std::ptrdiff_t m_vecAbsOrigin = 0x0;
		inline std::ptrdiff_t m_vOldOrigin = 0x0;
		inline std::ptrdiff_t m_pGameSceneNode = 0x0;
		inline std::ptrdiff_t m_flFlashOverlayAlpha = 0x0;
		inline std::ptrdiff_t m_bIsDefusing = 0x0;
		inline std::ptrdiff_t m_szName = 0x0;
		inline std::ptrdiff_t m_pClippingWeapon = 0x0;
		inline std::ptrdiff_t m_ArmorValue = 0x0;
		inline std::ptrdiff_t m_iHealth = 0x0;
		inline std::ptrdiff_t m_hPlayerPawn = 0x0;
		inline std::ptrdiff_t m_iTeamNum = 0x0;
		inline std::ptrdiff_t m_hController = 0x0;
		inline std::ptrdiff_t m_iszPlayerName = 0x0;
	}
}
