#pragma once

#include "Current.hpp"

using json = nlohmann::json;

class Config {
public:
    ~Config() = default;
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(const Config&) = delete;
    Config& operator=(Config&&) = delete;

    static bool Read();
    static bool Write();
private:
    Config() {};

    static Config& GetInstance()
    {
        static Config i{};
        return i;
    }

    bool ReadImpl();
    bool WriteImpl();
};