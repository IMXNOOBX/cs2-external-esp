#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <variant>
#include <filesystem>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include "config/Current.hpp"

namespace scripting {
    using ConfigVar = std::variant<bool*, int*, float*, color_t*>;
    using CommandFunc = std::function<void(const std::vector<std::string>&)>;

    struct Macro {
        std::string name;
        std::vector<std::string> commands;
        bool gui_accessible = false;
        bool contains_dangerous_commands = false;
        int stop_key = 0; // Key code to stop this macro (0 = no stop key)
    };

    struct Keybind {
        int key;
        std::string action;
    };

    class Scripting {
    public:
        static Scripting& Get() {
            static Scripting instance;
            return instance;
        }

        void Init();
        void Update();
        void ExecuteCommand(const std::string& command_line);
        void LoadScripts();
        const std::unordered_map<std::string, Macro>& GetMacros() const { return macros; }
        
        ~Scripting() {
            shutdown_thread = true;
            if (command_thread.joinable()) {
                command_thread.join();
            }
        }

    private:
        Scripting() = default;

        void RegisterConfigVars();
        void RegisterCommands();
        int ParseKeyCode(const std::string& key_str);
        void CommandWorkerThread();
        void ExecuteCommandInternal(const std::string& command_line);

        std::unordered_map<std::string, ConfigVar> config_map;
        std::unordered_map<std::string, CommandFunc> command_map;
        std::unordered_map<std::string, Macro> macros;
        std::vector<Keybind> keybinds;
        std::unordered_map<int, bool> key_states;
        std::unordered_map<std::string, std::string> variables;
        std::unordered_map<std::string, int> key_name_map;

        std::filesystem::file_time_type last_load_time;
        bool has_dangerous_scripts = false;

        // Async
        std::thread command_thread;
        std::queue<std::string> command_queue;
        std::mutex queue_mutex;
        std::atomic<bool> shutdown_thread{false};
        std::atomic<bool> stop_current_macro{false};
        std::string current_running_macro;

        void ProcessCommand(const std::string& cmd, const std::vector<std::string>& args);
        std::string EvaluateExpression(const std::string& expr);
        float EvaluateArithmetic(const std::string& expr);
        std::string GetConfigValue(const std::string& var_name);
        void SetColorValue(const std::string& var_name, float r, float g, float b, float a);
        void InitKeyNameMap();
        void SendInput(int vk_code, int duration_ms, int repeat_count);
        void ReadMemory(const std::string& var_name, uintptr_t address);
        void WriteMemory(const std::string& var_name, uintptr_t address);
        bool IsDangerousCommand(const std::string& cmd);
    };
}
