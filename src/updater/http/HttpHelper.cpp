#include "HttpHelper.hpp"

int HttpHelper::Get(std::string url, json& response) {
	return GetInstance().GetImpl(url, response);
}

int HttpHelper::Post(std::string url, json body, json& response) {
	return GetInstance().PostImpl(url, body, response);
}

int HttpHelper::GetImpl(std::string url, json& response) {
    CURL* curl = curl_easy_init();
    if (!curl) return -1;

    std::string response_string;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        LOGF(WARNING, "Curl returned a non successfull status code {}", (int)res);
        return -1;
    }

    try {
        response = json::parse(response_string);
    }
    catch (...) {
        return -2; // parse error
    }

    return static_cast<int>(http_code);
}

int HttpHelper::PostImpl(std::string url, json body, json& response) {
    CURL* curl = curl_easy_init();
    if (!curl) return -1;

    std::string response_string;
    std::string body_string = body.dump();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_string.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body_string.size());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    // Set JSON header
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        return -1;

    try {
        response = json::parse(response_string);
    }
    catch (...) {
        return -2; // parse error
    }

    return static_cast<int>(http_code);
}

size_t HttpHelper::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t total_size = size * nmemb;
	std::string* str = static_cast<std::string*>(userp);
	str->append(static_cast<char*>(contents), total_size);
	return total_size;
}