#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>

#include <string>
#include <format>
#include <array>

enum class LogColor
{
    RESET,
    WHITE = 97,
    CYAN = 36,
    MAGENTA = 35,
    BLUE = 34,
    GREEN = 32,
    YELLOW = 33,
    RED = 31,
    BLACK = 30
};

class LogHelper {
public:
    ~LogHelper()                           = default;
    LogHelper(const LogHelper&)            = delete;
    LogHelper(LogHelper&&)                 = delete;
    LogHelper& operator=(const LogHelper&) = delete;
    LogHelper& operator=(LogHelper&&)      = delete;

    static void Destroy();
    static bool Init();
private:
    LogHelper(){};

    static LogHelper& GetInstance()
    {
        static LogHelper i{};
        return i;
    }

    bool InitImpl();

private:
    static LogColor GetColor(const eLogLevel level);
    static const char* GetLevelStr(const eLogLevel level);
    std::string FormatConsole(const LogMessagePtr msg);
private:
	std::ofstream m_ConsoleOut;
};