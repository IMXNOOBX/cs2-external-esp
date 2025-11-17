#pragma once

#include <curl/curl.h>

using json = nlohmann::json;

class HttpHelper {
public:
    ~HttpHelper() = default;
    HttpHelper(const HttpHelper&) = delete;
    HttpHelper(HttpHelper&&) = delete;
    HttpHelper& operator=(const HttpHelper&) = delete;
    HttpHelper& operator=(HttpHelper&&) = delete;

    static int Get(std::string url, json& response);
    static int Post(std::string url, json body, json& response);
private:
    HttpHelper() {};

    static HttpHelper& GetInstance()
    {
        static HttpHelper i{};
        return i;
    }

    int GetImpl(std::string url, json& response);
    int PostImpl(std::string url, json body, json& response);

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};