#include "Updater.hpp"

bool Updater::Init() {
	return GetInstance().InitImpl();
}

Status Updater::GetStatus() {
	return GetInstance().status;
}

bool Updater::InitImpl() {
	json response;
	auto http_status = HttpHelper::Get(this->status_url, response);

	if (http_status == -1)
		LOGF(FATAL, "Failed to fetch status json from the repository");

	if (http_status == -2)
		LOGF(FATAL, "Failed to parse json, from status file in the repository");

	if (http_status != 200)
		return false;

	try {
		status.unsafe = response.value("unsafe", false);
		status.notice = response.value("notice", "");

		status.version_current = response["version"].value("current", 0);
		status.version_minimum = response["version"].value("minimum", 0);
	}
	catch (std::exception& e) {
		LOGF(FATAL, "Failed to parse status json, most likely changed structure, and we need updating");
		return false;
	}

	return true;
}