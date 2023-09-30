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

	bool check_and_update(bool prompt_update);
	bool get_last_commit_date(json& commit);
	bool download_file(const char* url, const char* localPath);

	inline std::string github_repo_api = "https://api.github.com/repos/IMXNOOBX/cs2-external-esp/commits";
	inline std::string raw_updated_offets = "https://github.com/IMXNOOBX/cs2-external-esp/raw/main/offsets/offsets.json";
}