#include "auto_updater.hpp"


namespace updater {
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
		std::tm commit_date = {};
		std::istringstream(last_commit_date) >> std::get_time(&commit_date, "%Y-%m-%dT%H:%M:%SZ");

		auto currentTime = std::chrono::system_clock::now();

		std::chrono::system_clock::time_point commitTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&commit_date));

		if (file_good("offsets.json")) {
			fs::file_time_type lastModifiedTime = fs::last_write_time("offsets.json");
			auto lastModifiedClockTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
				lastModifiedTime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());

			// Check if the local file is older than the last GitHub commit
			if (lastModifiedClockTime < commitTimePoint) {
				std::cout << "[updater] Local file is older than the last GitHub commit." << std::endl;
				
				char response;
				if (!automatic_update) {
					std::cout << "[updater] Do you want to download the latest offsets? (y/n): ";
					std::cin >> response;
				}

				if (automatic_update || (response == 'Y' || response == 'y')) {
					if (download_file(raw_updated_offets.c_str(), "offsets.json")) {
						std::cout << "[updater] Successfully downloaded latest offsets.json file\n" << std::endl;
						return true;
					}
					else {
						std::cout << "[updater] Error: Failed to download file, try downloading manually from " << raw_updated_offets << "\n" << std::endl;
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
				if (download_file(raw_updated_offets.c_str(), "offsets.json")) {
					std::cout << "[updater] Successfully downloaded latest offsets.json file\n" << std::endl;
					return true;
				}
				else {
					std::cout << "[updater] Error: Failed to download file, try downloading manually from " << raw_updated_offets << "\n" << std::endl;
				}
			}
		}

		return false;
	}

	bool file_good(const std::string& name) {
		std::ifstream f(name.c_str());
		return f.good();
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

		if (data.is_array()) {
			json last_commit = data[0];
			json last_commit_author = last_commit["commit"]["author"];

			commit = last_commit_author;
		}

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

		if (data["dwLocalPlayer"].is_number())
			offsets::dwLocalPlayer = data["dwLocalPlayer"];
		if (data["dwEntityList"].is_number())
			offsets::dwEntityList = data["dwEntityList"];
		if (data["dwViewMatrix"].is_number())
			offsets::dwViewMatrix = data["dwViewMatrix"];

		if (data["dwPawnHealth"].is_number())
			offsets::dwPawnHealth = data["dwPawnHealth"];
		if (data["dwPlayerPawn"].is_number())
			offsets::dwPlayerPawn = data["dwPlayerPawn"];
		if (data["dwSanitizedName"].is_number())
			offsets::dwSanitizedName = data["dwSanitizedName"];
		if (data["m_iTeamNum"].is_number())
			offsets::m_iTeamNum = data["m_iTeamNum"];
		if (data["m_vecOrigin"].is_number())
			offsets::m_vecOrigin = data["m_vecOrigin"];

		return true;
	}

	void save() {
		json data;

		data["dwLocalPlayer"] = offsets::dwLocalPlayer;
		data["dwEntityList"] = offsets::dwEntityList;
		data["dwViewMatrix"] = offsets::dwViewMatrix;

		data["dwPawnHealth"] = offsets::dwPawnHealth;
		data["dwPlayerPawn"] = offsets::dwPlayerPawn;
		data["dwSanitizedName"] = offsets::dwSanitizedName;
		data["m_iTeamNum"] = offsets::m_iTeamNum;
		data["m_vecOrigin"] = offsets::m_vecOrigin;

		std::ofstream output(file_path);
		output << std::setw(4) << data << std::endl;
		output.close();
	}
}