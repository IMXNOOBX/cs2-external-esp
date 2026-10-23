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

    struct Macro {
        bool gui_accessible = true;
        int hold_key = 0;
    };

    struct Keybind {
        int key;
        std::string macro;
    };

    class Scripting {
    public:
        static constexpr const char* kScriptPath = "scripts.sl";

        static Scripting& Get() {
            static Scripting instance;
            return instance;
        }

        void Init();
        void Update();
        void LoadScripts();
        void ExecuteMacro(const std::string& name);

        const std::unordered_map<std::string, Macro>& GetMacros() const { return macros; }

        ~Scripting();

    private:
        Scripting() = default;

        struct PendingMacro {
            std::string name;
            int hold_key = 0;
        };

        void RegisterConfigVars();
        void RegisterNatives(solis::Vm& vm);
        void WritePrelude();
        void RunInit();
        void RunMacro(const std::string& name);
        void RunImage(const std::vector<uint8_t>& image, const std::string& label);
        void QueueMacro(const std::string& name, int hold_key = 0);
        void WorkerThread();

        void SetBoolValue(const std::string& name, bool value);
        void SetIntValue(const std::string& name, int value);
        void SetFloatValue(const std::string& name, float value);
        void SetColorValue(const std::string& name, float r, float g, float b, float a);
        bool GetBoolValue(const std::string& name) const;
        int GetIntValue(const std::string& name) const;
        float GetFloatValue(const std::string& name) const;
        std::string GetColorValue(const std::string& name) const;

        void RegisterBind(const std::string& key, const std::string& macro);
        void RegisterHoldKey(const std::string& key, const std::string& macro);
        void InitKeyNameMap();
        int ParseKeyCode(const std::string& key_str);
        void SendInput(int vk_code, int duration_ms, int repeat_count);

        std::unordered_map<std::string, ConfigVar> config_map;
        std::unordered_map<std::string, Macro> macros;
        std::vector<Keybind> keybinds;
        std::unordered_map<int, bool> key_states;
        std::unordered_map<std::string, int> key_name_map;

        std::filesystem::file_time_type last_load_time;
        std::vector<uint8_t> init_image;
        std::unordered_map<std::string, std::vector<uint8_t>> macro_images;
        std::mutex image_mutex;

        std::thread worker_thread;
        std::queue<PendingMacro> macro_queue;
        std::mutex queue_mutex;
        std::atomic<bool> shutdown_thread{false};
        std::atomic<unsigned int> active_macro_count{0};
        std::atomic<int> active_hold_key{0};
        std::atomic<bool> initializing{false};
    };
}
