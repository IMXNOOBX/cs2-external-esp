#include "auto_updater.hpp"


namespace updater {
#ifndef _UC
	bool check_and_update(bool automatic_update) {
		json commit;
		if (!get_last_commit_date(commit)) {
			std::cout << "[updater] error getting last commit information from GitHub" << std::endl;
			return false;
		}

		std::string last_commit_date = commit["date"];
		std::string last_commit_author_name = commit["name"];

		std::cout << "[updater] Last GitHub repository update was made by " << last_commit_author_name << " on " << last_commit_date.substr(0, 10) << std::endl;

		// Parse the GitHub date string and convert it to a std::tm structure
		std::tm commit_tm_date = {};
		std::istringstream(last_commit_date) >> std::get_time(&commit_tm_date, "%Y-%m-%dT%H:%M:%SZ");

		std::chrono::system_clock::time_point commit_date = std::chrono::system_clock::from_time_t(std::mktime(&commit_tm_date));

		if (file_good("offsets.json")) {
			fs::file_time_type last_modified_file_time = fs::last_write_time("offsets.json");
			auto last_modified = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
				last_modified_file_time - fs::file_time_type::clock::now() + std::chrono::system_clock::now());

			// Check if the local file is older than the last GitHub commit
			if (last_modified < commit_date) {
				std::cout << "[updater] Local file is older than the last GitHub commit." << std::endl;

				char response;
				if (!automatic_update) {
					std::cout << "[updater] Do you want to download the latest offsets? (y/n): ";
					std::cin >> response;
				}

				if (automatic_update || (response == 'Y' || response == 'y')) {
					if (download_file(raw_updated_offsets.c_str(), "offsets.json")) {
						std::cout << "[updater] Successfully downloaded latest offsets.json file\n" << std::endl;
						return true;
					}
					else {
						std::cout << "[updater] Error: Failed to download file, try downloading manually from " << raw_updated_offsets << "\n" << std::endl;
					}
				}

			}
			else {
				std::cout << "[updater] Local file is up to date.\n" << std::endl;
			}
		}
		else {
			char response;
			if (!automatic_update) {
				std::cout << "[updater] Do you want to download the latest offsets? (y/n): ";
				std::cin >> response;
			}

			if (automatic_update || (response == 'Y' || response == 'y')) {
				if (download_file(raw_updated_offsets.c_str(), "offsets.json")) {
					std::cout << "[updater] Successfully downloaded latest offsets.json file\n" << std::endl;
					return true;
				}
				else {
					std::cout << "[updater] Error: Failed to download file, try downloading manually from " << raw_updated_offsets << "\n" << std::endl;
				}
			}
		}

		return false;
	}

	bool get_last_commit_date(json& commit) {
		HINTERNET hInternet, hConnect;

		hInternet = InternetOpen("AutoUpdater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		if (!hInternet) {
			return false;
		}

		hConnect = InternetOpenUrlA(hInternet, github_repo_api.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
		if (!hConnect) {
			InternetCloseHandle(hInternet);
			return false;
		}

		char buffer[4096];
		DWORD bytesRead;
		std::string commitData;

		while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
			commitData.append(buffer, bytesRead);
		}

		json data;
		try {
			data = json::parse(commitData);
		}
		catch (const std::exception& e) {
			std::cout << "[updater] exception while parsing json response from github" << std::endl;
			return false;
		}

		if (data.empty())
			return false;

		if (!data.is_array())
			return false;

		json last_commit = data[0];
		json last_commit_author = last_commit["commit"]["author"];

		commit = last_commit_author;

		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);

		return true;
	}

	bool download_file(const char* url, const char* localPath) {
		HINTERNET hInternet, hConnect;

		hInternet = InternetOpen("AutoUpdater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		if (!hInternet) {
			std::cerr << "InternetOpen failed." << std::endl;
			return false;
		}

		hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
		if (!hConnect) {
			std::cerr << "InternetOpenUrlA failed." << std::endl;
			InternetCloseHandle(hInternet);
			return false;
		}

		std::ofstream outFile(localPath, std::ios::binary);
		if (!outFile) {
			std::cerr << "Failed to create local file." << std::endl;
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return false;
		}

		char buffer[4096];
		DWORD bytesRead;

		while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
			outFile.write(buffer, bytesRead);
		}

		outFile.close();
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);

		return true;
	}
#endif

	bool file_good(const std::string& name) {
		std::ifstream f(name.c_str());
		return f.good();
	}

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

		if (data["build_number"].is_number())
			build_number = data["build_number"];

		/* little cross compatibility with older builds */
		if (data["dwLocalPlayer"].is_number())
			offsets::dwLocalPlayerController = data["dwLocalPlayer"];
		if (data["dwLocalPlayerController"].is_number())
			offsets::dwLocalPlayerController = data["dwLocalPlayerController"];
		if (data["dwEntityList"].is_number())
			offsets::dwEntityList = data["dwEntityList"];
		if (data["dwViewMatrix"].is_number())
			offsets::dwViewMatrix = data["dwViewMatrix"];
		if (data["dwBuildNumber"].is_number())
			offsets::dwBuildNumber = data["dwBuildNumber"];
		if (data["dwPlantedC4"].is_number())
			offsets::dwPlantedC4 = data["dwPlantedC4"];

		if (data["m_flC4Blow"].is_number())
			offsets::m_flC4Blow = data["m_flC4Blow"];
		if (data["m_flNextBeep"].is_number())
			offsets::m_flNextBeep = data["m_flNextBeep"];
		if (data["m_flTimerLength"].is_number())
			offsets::m_flTimerLength = data["m_flTimerLength"];

		if (data["m_pInGameMoneyServices"].is_number())
			offsets::m_pInGameMoneyServices = data["m_pInGameMoneyServices"];
		if (data["m_iAccount"].is_number())
			offsets::m_iAccount = data["m_iAccount"];
		if (data["m_vecAbsOrigin"].is_number())
			offsets::m_vecAbsOrigin = data["m_vecAbsOrigin"];
		if (data["m_vOldOrigin"].is_number())
			offsets::m_vOldOrigin = data["m_vOldOrigin"];
		if (data["m_pGameSceneNode"].is_number())
			offsets::m_pGameSceneNode = data["m_pGameSceneNode"];
		if (data["m_flFlashOverlayAlpha"].is_number())
			offsets::m_flFlashOverlayAlpha = data["m_flFlashOverlayAlpha"];
		if (data["m_bIsDefusing"].is_number())
			offsets::m_bIsDefusing = data["m_bIsDefusing"];
		if (data["m_szName"].is_number())
			offsets::m_szName = data["m_szName"];
		if (data["m_pClippingWeapon"].is_number())
			offsets::m_pClippingWeapon = data["m_pClippingWeapon"];
		if (data["m_ArmorValue"].is_number())
			offsets::m_ArmorValue = data["m_ArmorValue"];
		if (data["m_iHealth"].is_number())
			offsets::m_iHealth = data["m_iHealth"];
		if (data["m_hPlayerPawn"].is_number())
			offsets::m_hPlayerPawn = data["m_hPlayerPawn"];
		if (data["m_iTeamNum"].is_number())
			offsets::m_iTeamNum = data["m_iTeamNum"];
		if (data["m_hController"].is_number())
			offsets::m_hController = data["m_hController"];
		if (data["m_iszPlayerName"].is_number())
			offsets::m_iszPlayerName = data["m_iszPlayerName"];

		return true;
	}

	void save() {
		json data;

		data["build_number"] = build_number;

		data["dwLocalPlayerController"] = offsets::dwLocalPlayerController;
		data["dwEntityList"] = offsets::dwEntityList;
		data["dwViewMatrix"] = offsets::dwViewMatrix;
		data["dwBuildNumber"] = offsets::dwBuildNumber;
		data["dwPlantedC4"] = offsets::dwPlantedC4;

		data["m_flNextBeep"] = offsets::m_flNextBeep;
		data["m_flC4Blow"] = offsets::m_flC4Blow;
		data["m_flTimerLength"] = offsets::m_flTimerLength;

		data["m_pInGameMoneyServices"] = offsets::m_pInGameMoneyServices;
		data["m_iAccount"] = offsets::m_iAccount;
		data["m_vecAbsOrigin"] = offsets::m_vecAbsOrigin;
		data["m_vOldOrigin"] = offsets::m_vOldOrigin;
		data["m_pGameSceneNode"] = offsets::m_pGameSceneNode;
		data["m_flFlashOverlayAlpha"] = offsets::m_flFlashOverlayAlpha;
		data["m_bIsDefusing"] = offsets::m_bIsDefusing;
		data["m_szName"] = offsets::m_szName;
		data["m_pClippingWeapon"] = offsets::m_pClippingWeapon;
		data["m_ArmorValue"] = offsets::m_ArmorValue;
		data["m_iHealth"] = offsets::m_iHealth;
		data["m_hPlayerPawn"] = offsets::m_hPlayerPawn;
		data["m_iTeamNum"] = offsets::m_iTeamNum;
		data["m_hController"] = offsets::m_hController;
		data["m_iszPlayerName"] = offsets::m_iszPlayerName;

		std::ofstream output(file_path);
		output << std::setw(4) << data << std::endl;
		output.close();
	}
}
