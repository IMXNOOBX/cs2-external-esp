#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

struct color_t;

namespace solis {
    class Vm;
}

namespace scripting {
    using ConfigVar = std::variant<bool*, int*, float*, color_t*>;

    struct ScriptButton {
        std::string label;
        std::string function;
    };

    struct ScriptInfo {
        std::string name;
        std::string path;
        bool loaded = false;
        std::vector<ScriptButton> buttons;
    };

    struct Keybind {
        int key;
        std::string path;
        std::string function;
    };

    struct HoldKey {
        int key;
        std::string path;
        std::string function;
    };

    class Scripting {
    public:
        static Scripting& Get() {
            static Scripting instance;
            return instance;
        }

        void Init();
        void Update();
        void LoadScripts();
        void RefreshScripts();
        void LoadScript(const std::string& path);
        void UnloadScript(const std::string& path);
        void ExecuteFunction(const std::string& path, const std::string& function);

        const std::vector<ScriptInfo>& GetScripts() const { return scripts; }

        ~Scripting();

    private:
        Scripting() = default;

        struct ScriptRuntime {
            std::vector<uint8_t> init;
            std::unordered_map<std::string, std::vector<uint8_t>> functions;
        };

        struct PendingFunction {
            std::string path;
            std::string function;
            int hold_key = 0;
        };

        void RegisterConfigVars();
        void RegisterNatives(solis::Vm& vm, const std::string& script_path, bool allow_bind);
        void WritePrelude(const std::filesystem::path& dir);

        std::vector<std::filesystem::path> FindScripts() const;
        bool CompileScript(const std::string& path, ScriptInfo& info, ScriptRuntime& runtime);
        void UnloadRuntime(const std::string& path);
        std::string DisplayName(const std::filesystem::path& file) const;

        void RunInit(const std::string& path);
        void RunFunction(const std::string& path, const std::string& function);
        void RunImage(const std::vector<uint8_t>& image, const std::string& label, const std::string& script_path, bool allow_bind);
        void QueueFunction(const std::string& path, const std::string& function, int hold_key = 0);
        void WorkerThread();

        bool FunctionExists(const std::string& path, const std::string& function) const;

        void SetBoolValue(const std::string& name, bool value);
        void SetIntValue(const std::string& name, int value);
        void SetFloatValue(const std::string& name, float value);
        void SetColorValue(const std::string& name, float r, float g, float b, float a);
        bool GetBoolValue(const std::string& name) const;
        int GetIntValue(const std::string& name) const;
        float GetFloatValue(const std::string& name) const;
        std::string GetColorValue(const std::string& name) const;

        void RegisterBind(const std::string& path, const std::string& key, const std::string& function);
        void RegisterHoldKey(const std::string& path, const std::string& key, const std::string& function);
        void InitKeyNameMap();
        int ParseKeyCode(const std::string& key_str);
        void SendInput(int vk_code, int duration_ms, int repeat_count);

        std::unordered_map<std::string, ConfigVar> config_map;
        std::vector<ScriptInfo> scripts;
        std::unordered_map<std::string, ScriptRuntime> runtimes;

        std::vector<Keybind> keybinds;
        std::vector<HoldKey> holds;
        std::unordered_map<int, bool> key_states;
        std::unordered_map<std::string, int> key_name_map;

        std::mutex runtime_mutex;
        std::thread worker_thread;
        std::queue<PendingFunction> function_queue;
        std::mutex queue_mutex;
        std::atomic<bool> shutdown_thread{false};
        std::atomic<unsigned int> active_script_count{0};
        std::atomic<int> active_hold_key{0};
    };
}
