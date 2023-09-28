#include "auto_updater.hpp"


namespace updater {
	bool check_and_update(bool prompt_update) {
		json commit;
		if (!get_last_commit_date(commit)) {
			std::cout << "[updater] error getting last commit information from github" << std::endl;
			return false;
		}

		std::string last_commit_date = commit["date"];
		std::string last_commit_author_name = commit["name"];

		std::cout << "[updater] Last github repository update was made by " << last_commit_author_name << " the " << last_commit_date << std::endl;

		// Parse the GitHub date string and convert it to a std::tm structure
		std::tm commit_date = {};
		std::istringstream(last_commit_date) >> std::get_time(&commit_date, "%Y-%m-%dT%H:%M:%SZ");

		auto currentTime = std::chrono::system_clock::now();

		std::chrono::seconds diff = std::chrono::duration_cast<std::chrono::seconds>(currentTime - std::chrono::system_clock::from_time_t(std::mktime(&commit_date)));

		// Prompt the user if he wants to update the offsets.json if there was a commit in the last 5 days
		if (diff.count() <= (5 * 24 * 3600)) {
			std::cout << "[updater] There has been an update in the repository in the last 5 days!" << std::endl;
			if (prompt_update)
				std::cout << "[updater] Do you want to download the latest offsets? (y/n): ";

			char response;
			std::cin >> response;

			if (!prompt_update || (response == 'Y' || response == 'y')) {
				if (download_file(raw_updated_offets.c_str(), "offsets.json")) {
					std::cout << "[updater] Successfully downloaded latest offsets.json file\n" << std::endl;
					return true;
				}
				else {
					std::cout << "[updater] Error failed to download file, try downloading manually from " << raw_updated_offets << "\n" << std::endl;
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
}