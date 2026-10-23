#include "Scripting.hpp"
#include "SolisScript.hpp"
#include "core\engine\types\Types.hpp"
#include "config/Current.hpp"
#include "core/scripting/hooks/PhysicalInput.hpp"
#include "core/logger/LogHelper.hpp"
#include "core\engine\Engine.hpp"
#include "gui\frontend\menu\Menu.hpp"
#include "gui\renderer\Renderer.hpp"
#include "gui/frontend/theme/Theme.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
#include "solis.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <Windows.h>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <utility>

namespace scripting {
    namespace {
        std::string ArgString(solis::Args args, int index) {
            return std::string(args[index].asString());
        }

        void RequireArgs(solis::Args args, int count, const char* name) {
            if (args.size() < count) {
                throw solis::Error(std::string(name) + " expects " + std::to_string(count) + " argument(s)");
            }
        }
    }

    void Scripting::Init() {
        try {
            InitKeyNameMap();
            RegisterConfigVars();
            LoadScripts();

            if (!input::InstallPhysicalInputHooks()) {
                LOGF(WARNING, "Could not install physical input hooks; hold macros are unavailable");
            }
        }
        catch (std::exception& e) {
            LOGF(FATAL, "Caught exception while initializing: {}", e.what());
        }

        worker_thread = std::thread(&Scripting::WorkerThread, this);

        LOGF(INFO, "Scripting system initialized");
    }

    Scripting::~Scripting() {
        shutdown_thread = true;
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
        input::UninstallPhysicalInputHooks();
    }

    void Scripting::RegisterConfigVars() {
        config_map["enabled"] = &cfg::enabled;
        config_map["esp.team"] = &cfg::esp::team;
        config_map["esp.box"] = &cfg::esp::box;
        config_map["esp.armor"] = &cfg::esp::armor;
        config_map["esp.health"] = &cfg::esp::health;
        config_map["esp.skeleton"] = &cfg::esp::skeleton;
        config_map["esp.head_tracker"] = &cfg::esp::head_tracker;
        config_map["esp.health_number"] = &cfg::esp::health_number;
        config_map["esp.spotted"] = &cfg::esp::spotted;
        config_map["esp.bomb"] = &cfg::esp::bomb;
        config_map["esp.tracers"] = &cfg::esp::tracers;

        config_map["esp.flags.name"] = &cfg::esp::flags::name;
        config_map["esp.flags.ping"] = &cfg::esp::flags::ping;
        config_map["esp.flags.money"] = &cfg::esp::flags::money;
        config_map["esp.flags.weapon"] = &cfg::esp::flags::weapon;
        config_map["esp.flags.ammo"] = &cfg::esp::flags::ammo;
        config_map["esp.flags.reloading"] = &cfg::esp::flags::reloading;
        config_map["esp.flags.scoped"] = &cfg::esp::flags::scoped;
        config_map["esp.flags.defusing"] = &cfg::esp::flags::defusing;
        config_map["esp.flags.flashed"] = &cfg::esp::flags::flashed;
        config_map["esp.flags.has_c4"] = &cfg::esp::flags::has_c4;

        config_map["world.spectators.enabled"] = &cfg::world::spectators::enabled;
        config_map["world.spectators.detailed"] = &cfg::world::spectators::detailed;
        config_map["world.spectators.self_only"] = &cfg::world::spectators::self_only;

        config_map["world.bomb.location"] = &cfg::world::bomb::location;
        config_map["world.bomb.timer"] = &cfg::world::bomb::timer;
        config_map["world.bomb.hud"] = &cfg::world::bomb::hud;

        config_map["world.crosshair.enabled"] = &cfg::world::crosshair::enabled;
        config_map["world.radar.enabled"] = &cfg::world::radar::enabled;
        config_map["world.radar.no_rotate"] = &cfg::world::radar::no_rotate;

        config_map["settings.watermark"] = &cfg::settings::watermark;
        config_map["settings.streamproof"] = &cfg::settings::streamproof;
        config_map["settings.vsync"] = &cfg::settings::vsync;
        config_map["settings.free_cpu"] = &cfg::settings::free_cpu;

        config_map["esp.color.box_team"] = &cfg::esp::colors::box_team;
        config_map["esp.color.box_enemy"] = &cfg::esp::colors::box_enemy;
        config_map["esp.color.skeleton_team"] = &cfg::esp::colors::skeleton_team;
        config_map["esp.color.skeleton_enemy"] = &cfg::esp::colors::skeleton_enemy;
        config_map["esp.color.tracker_team"] = &cfg::esp::colors::tracker_team;
        config_map["esp.color.tracker_enemy"] = &cfg::esp::colors::tracker_enemy;
        config_map["esp.color.tracer_team"] = &cfg::esp::colors::tracer_team;
        config_map["esp.color.tracer_enemy"] = &cfg::esp::colors::tracer_enemy;

        config_map["esp.color.flags.flashed_team"] = &cfg::esp::colors::flags::flashed_team;
        config_map["esp.color.flags.flashed_enemy"] = &cfg::esp::colors::flags::flashed_enemy;
        config_map["esp.color.flags.reloading_team"] = &cfg::esp::colors::flags::reloading_team;
        config_map["esp.color.flags.reloading_enemy"] = &cfg::esp::colors::flags::reloading_enemy;
        config_map["esp.color.flags.defusing_team"] = &cfg::esp::colors::flags::defusing_team;
        config_map["esp.color.flags.defusing_enemy"] = &cfg::esp::colors::flags::defusing_enemy;
        config_map["esp.color.flags.scoped_team"] = &cfg::esp::colors::flags::scoped_team;
        config_map["esp.color.flags.scoped_enemy"] = &cfg::esp::colors::flags::scoped_enemy;
        config_map["esp.color.flags.c4_team"] = &cfg::esp::colors::flags::c4_team;
        config_map["esp.color.flags.c4_enemy"] = &cfg::esp::colors::flags::c4_enemy;

        config_map["esp.bomb_color"] = &cfg::esp::bomb_color;
    }

    void Scripting::RegisterNatives(solis::Vm& vm) {
        vm.define("set_bool", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "set_bool");
            SetBoolValue(ArgString(args, 0), args[1].asBool());
            return solis::Value();
        });

        vm.define("set_int", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "set_int");
            SetIntValue(ArgString(args, 0), static_cast<int>(args[1].asInt()));
            return solis::Value();
        });

        vm.define("set_float", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "set_float");
            SetFloatValue(ArgString(args, 0), static_cast<float>(args[1].asFloat()));
            return solis::Value();
        });

        vm.define("set_color", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 5, "set_color");
            SetColorValue(ArgString(args, 0),
                static_cast<float>(args[1].asFloat()),
                static_cast<float>(args[2].asFloat()),
                static_cast<float>(args[3].asFloat()),
                static_cast<float>(args[4].asFloat()));
            return solis::Value();
        });

        vm.define("get_bool", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "get_bool");
            return solis::Value::boolean(GetBoolValue(ArgString(args, 0)));
        });

        vm.define("get_int", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "get_int");
            return solis::Value::integer(GetIntValue(ArgString(args, 0)));
        });

        vm.define("get_float", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "get_float");
            return solis::Value::number(GetFloatValue(ArgString(args, 0)));
        });

        vm.define("get_color", [this](solis::Vm& vm, solis::Args args) {
            RequireArgs(args, 1, "get_color");
            return vm.string(GetColorValue(ArgString(args, 0)));
        });

        vm.define("reset_theme", [](solis::Vm&, solis::Args) {
            theme::ThemeManager::Get().RequestReset();
            return solis::Value();
        });

        vm.define("echo", [](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "echo");
            LOGF(INFO, "{}", args[0].toString());
            return solis::Value();
        });

        vm.define("print", [](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "print");
            LOGF(INFO, "{}", args[0].toString());
            return solis::Value();
        });

        vm.define("bind", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "bind");
            if (!initializing) {
                LOGF(WARNING, "bind() can only be called from main()");
                return solis::Value();
            }
            RegisterBind(ArgString(args, 0), ArgString(args, 1));
            return solis::Value();
        });

        vm.define("hold", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "hold");
            if (!initializing) {
                LOGF(WARNING, "hold() can only be called from main()");
                return solis::Value();
            }
            RegisterHoldKey(ArgString(args, 0), ArgString(args, 1));
            return solis::Value();
        });

        vm.define("client_base", [](solis::Vm&, solis::Args) {
            return solis::Value::integer(static_cast<int64_t>(Engine::GetClient().base));
        });

        vm.define("engine_base", [](solis::Vm&, solis::Args) {
            return solis::Value::integer(static_cast<int64_t>(Engine::GetEngine().base));
        });

        vm.define("menu_open", [](solis::Vm&, solis::Args) {
            return solis::Value::boolean(Renderer::IsOpen());
        });

        vm.define("menu_x", [](solis::Vm&, solis::Args) {
            return solis::Value::integer(static_cast<int64_t>(Menu::GetPos().x));
        });

        vm.define("menu_y", [](solis::Vm&, solis::Args) {
            return solis::Value::integer(static_cast<int64_t>(Menu::GetPos().y));
        });

#ifdef _DEBUG
        vm.define("send", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "send");
            const int key = ParseKeyCode(ArgString(args, 0));
            const int duration = args.size() > 1 ? static_cast<int>(args[1].asInt()) : 50;
            const int repeat = args.size() > 2 ? static_cast<int>(args[2].asInt()) : 1;
            SendInput(key, duration, repeat);
            return solis::Value();
        });

        vm.define("read_int", [](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "read_int");
            const auto address = static_cast<uintptr_t>(args[0].asInt());
            return solis::Value::integer(Engine::GetProcess()->read<int>(address));
        });

        vm.define("write_int", [](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "write_int");
            const auto address = static_cast<uintptr_t>(args[0].asInt());
            Engine::GetProcess()->write<int>(address, static_cast<int>(args[1].asInt()));
            return solis::Value();
        });
#endif
    }


    // Prelude, this exports the methods that can be used by the scripts from the esp
    void Scripting::WritePrelude() {
        std::string text = R"(extern func set_bool(name: str, value: bool);
extern func set_int(name: str, value: int);
extern func set_float(name: str, value: float);
extern func set_color(name: str, r: float, g: float, b: float, a: float);
extern func get_bool(name: str): bool;
extern func get_int(name: str): int;
extern func get_float(name: str): float;
extern func get_color(name: str): str;

extern func reset_theme();
extern func echo(message: str);
extern func print(message: str);

extern func bind(key: str, macro: str);
extern func hold(key: str, macro: str);

extern func client_base(): int;
extern func engine_base(): int;
extern func menu_open(): bool;
extern func menu_x(): int;
extern func menu_y(): int;
)";

#ifdef _DEBUG
        text += R"(
extern func send(key: str, duration_ms: int, repeat_count: int);
extern func read_int(address: int): int;
extern func write_int(address: int, value: int);
)";
#endif

        const std::filesystem::path prelude = std::filesystem::path(kScriptPath).parent_path() / "prelude.sl";

        std::ofstream out(prelude, std::ios::trunc);
        if (!out.is_open()) {
            LOGF(WARNING, "Could not write {}", prelude.string());
            return;
        }

        out << text;
    }

    void Scripting::LoadScripts() {
        std::error_code ec;
        if (!std::filesystem::exists(kScriptPath, ec)) {
            LOGF(WARNING, "No {} found", kScriptPath);
            return;
        }

        last_load_time = std::filesystem::last_write_time(kScriptPath, ec);

        macros.clear();
        keybinds.clear();
        key_states.clear();

        WritePrelude();

        SolisScript::CompiledScript script;
        if (!SolisScript::Compile(kScriptPath, script)) {
            LOGF(WARNING, "Failed to compile {}", kScriptPath);
            std::lock_guard<std::mutex> lock(image_mutex);
            init_image.clear();
            macro_images.clear();
            return;
        }

        std::unordered_map<std::string, std::vector<uint8_t>> images;
        for (const auto& function : script.functions) {
            std::vector<uint8_t> image;
            if (!SolisScript::CompileMacro(kScriptPath, function, image)) {
                LOGF(WARNING, "Failed to compile macro '{}'", function);
                continue;
            }
            images.emplace(function, std::move(image));
        }

        {
            std::lock_guard<std::mutex> lock(image_mutex);
            init_image = std::move(script.image);
            macro_images = std::move(images);
        }

        for (const auto& [name, image] : macro_images) {
            (void)image;
            macros[name].gui_accessible = name.front() != '_';
        }

        initializing = true;
        RunInit();
        initializing = false;

        LOGF(INFO, "Loaded {} macro(s) from {}", macros.size(), kScriptPath);
    }

    void Scripting::RunInit() {
        std::vector<uint8_t> image;
        {
            std::lock_guard<std::mutex> lock(image_mutex);
            image = init_image;
        }

        RunImage(image, "main");
    }

    void Scripting::RunMacro(const std::string& name) {
        std::vector<uint8_t> image;
        {
            std::lock_guard<std::mutex> lock(image_mutex);
            const auto it = macro_images.find(name);
            if (it != macro_images.end()) {
                image = it->second;
            }
        }

        if (image.empty()) {
            LOGF(WARNING, "Unknown macro: {}", name);
            return;
        }

        RunImage(image, name);
    }

    void Scripting::RunImage(const std::vector<uint8_t>& image, const std::string& label) {
        if (image.empty()) {
            return;
        }

        try {
            solis::Vm vm;
            vm.openStd();
            RegisterNatives(vm);
            vm.load(image);
            vm.run();
        }
        catch (const solis::Error& e) {
            LOGF(WARNING, "Solis error in '{}': {}", label, e.what());
        }
        catch (const std::exception& e) {
            LOGF(WARNING, "Error in '{}': {}", label, e.what());
        }
    }

    void Scripting::Update() {
        std::error_code ec;
        if (std::filesystem::exists(kScriptPath, ec)) {
            const auto write_time = std::filesystem::last_write_time(kScriptPath, ec);
            if (!ec && write_time != last_load_time) {
                LOGF(INFO, "{} changed, reloading", kScriptPath);
                LoadScripts();
            }
        }

        for (const auto& bind : keybinds) {
            const bool pressed = (GetAsyncKeyState(bind.key) & 0x8000) != 0;
            if (pressed && !key_states[bind.key] && active_macro_count == 0) {
                QueueMacro(bind.macro);
            }
            key_states[bind.key] = pressed;
        }

        for (const auto& [name, macro] : macros) {
            if (macro.hold_key == 0) {
                continue;
            }

            const bool pressed = input::IsPhysicalInputDown(macro.hold_key);
            if (pressed && !key_states[macro.hold_key] && active_macro_count == 0) {
                QueueMacro(name, macro.hold_key);
            }
            key_states[macro.hold_key] = pressed;
        }
    }

    void Scripting::ExecuteMacro(const std::string& name) {
        QueueMacro(name);
    }

    void Scripting::QueueMacro(const std::string& name, int hold_key) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        macro_queue.push(PendingMacro{ name, hold_key });
    }

    void Scripting::WorkerThread() {
        while (!shutdown_thread) {
            PendingMacro pending;
            bool has_macro = false;

            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                if (!macro_queue.empty()) {
                    pending = std::move(macro_queue.front());
                    macro_queue.pop();
                    has_macro = true;
                }
            }

            if (!has_macro) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            active_macro_count.fetch_add(1);
            active_hold_key = pending.hold_key;
            RunMacro(pending.name);
            active_hold_key = 0;
            active_macro_count.fetch_sub(1);
        }
    }

    void Scripting::SetBoolValue(const std::string& name, bool value) {
        const auto it = config_map.find(name);
        if (it == config_map.end()) {
            LOGF(WARNING, "Unknown config variable: {}", name);
            return;
        }

        if (auto* var = std::get_if<bool*>(&it->second)) {
            **var = value;
        } else if (auto* var = std::get_if<int*>(&it->second)) {
            **var = value ? 1 : 0;
        } else if (auto* var = std::get_if<float*>(&it->second)) {
            **var = value ? 1.0f : 0.0f;
        } else {
            LOGF(WARNING, "{} is a color, use set_color", name);
        }
    }

    void Scripting::SetIntValue(const std::string& name, int value) {
        const auto it = config_map.find(name);
        if (it == config_map.end()) {
            LOGF(WARNING, "Unknown config variable: {}", name);
            return;
        }

        if (auto* var = std::get_if<int*>(&it->second)) {
            **var = value;
        } else if (auto* var = std::get_if<float*>(&it->second)) {
            **var = static_cast<float>(value);
        } else if (auto* var = std::get_if<bool*>(&it->second)) {
            **var = value != 0;
        } else {
            LOGF(WARNING, "{} is a color, use set_color", name);
        }
    }

    void Scripting::SetFloatValue(const std::string& name, float value) {
        const auto it = config_map.find(name);
        if (it == config_map.end()) {
            LOGF(WARNING, "Unknown config variable: {}", name);
            return;
        }

        if (auto* var = std::get_if<float*>(&it->second)) {
            **var = value;
        } else if (auto* var = std::get_if<int*>(&it->second)) {
            **var = static_cast<int>(value);
        } else if (auto* var = std::get_if<bool*>(&it->second)) {
            **var = value != 0.0f;
        } else {
            LOGF(WARNING, "{} is a color, use set_color", name);
        }
    }

    void Scripting::SetColorValue(const std::string& name, float r, float g, float b, float a) {
        const auto it = config_map.find(name);
        if (it != config_map.end()) {
            if (auto* var = std::get_if<color_t*>(&it->second)) {
                color_t* color = *var;
                color->r = std::clamp(r, 0.0f, 1.0f);
                color->g = std::clamp(g, 0.0f, 1.0f);
                color->b = std::clamp(b, 0.0f, 1.0f);
                color->a = std::clamp(a, 0.0f, 1.0f);
            } else {
                LOGF(WARNING, "{} is not a color variable", name);
            }
            return;
        }

        if (!theme::ThemeManager::Get().SetColor(name, r, g, b, a)) {
            LOGF(WARNING, "Unknown color variable: {}", name);
        }
    }

    bool Scripting::GetBoolValue(const std::string& name) const {
        const auto it = config_map.find(name);
        if (it == config_map.end()) {
            LOGF(WARNING, "Unknown config variable: {}", name);
            return false;
        }

        if (const auto* var = std::get_if<bool*>(&it->second)) return **var;
        if (const auto* var = std::get_if<int*>(&it->second)) return **var != 0;
        if (const auto* var = std::get_if<float*>(&it->second)) return **var != 0.0f;
        return false;
    }

    int Scripting::GetIntValue(const std::string& name) const {
        const auto it = config_map.find(name);
        if (it == config_map.end()) {
            LOGF(WARNING, "Unknown config variable: {}", name);
            return 0;
        }

        if (const auto* var = std::get_if<int*>(&it->second)) return **var;
        if (const auto* var = std::get_if<float*>(&it->second)) return static_cast<int>(**var);
        if (const auto* var = std::get_if<bool*>(&it->second)) return **var ? 1 : 0;
        return 0;
    }

    float Scripting::GetFloatValue(const std::string& name) const {
        const auto it = config_map.find(name);
        if (it == config_map.end()) {
            LOGF(WARNING, "Unknown config variable: {}", name);
            return 0.0f;
        }

        if (const auto* var = std::get_if<float*>(&it->second)) return **var;
        if (const auto* var = std::get_if<int*>(&it->second)) return static_cast<float>(**var);
        if (const auto* var = std::get_if<bool*>(&it->second)) return **var ? 1.0f : 0.0f;
        return 0.0f;
    }

    std::string Scripting::GetColorValue(const std::string& name) const {
        const auto it = config_map.find(name);
        if (it == config_map.end()) {
            LOGF(WARNING, "Unknown config variable: {}", name);
            return "0,0,0,0";
        }

        if (const auto* var = std::get_if<color_t*>(&it->second)) {
            const color_t* color = *var;
            return std::to_string(color->r) + "," +
                   std::to_string(color->g) + "," +
                   std::to_string(color->b) + "," +
                   std::to_string(color->a);
        }

        return "0,0,0,0";
    }

    void Scripting::RegisterBind(const std::string& key, const std::string& macro) {
        const int code = ParseKeyCode(key);
        if (code == 0) {
            LOGF(WARNING, "Unknown key name: {}", key);
            return;
        }

        keybinds.push_back({ code, macro });
        LOGF(INFO, "Bound {} to {}", key, macro);
    }

    void Scripting::RegisterHoldKey(const std::string& key, const std::string& macro) {
        const int code = ParseKeyCode(key);
        if (code == 0) {
            LOGF(WARNING, "Unknown hold key name: {}", key);
            return;
        }

        macros[macro].hold_key = code;
        LOGF(INFO, "Holding {} runs {}", key, macro);
    }

    void Scripting::InitKeyNameMap() {
        for (char c = 'A'; c <= 'Z'; c++) {
            key_name_map[std::string(1, c)] = c;
        }

        for (char c = '0'; c <= '9'; c++) {
            key_name_map[std::string(1, c)] = c;
        }

        key_name_map["F1"] = VK_F1;
        key_name_map["F2"] = VK_F2;
        key_name_map["F3"] = VK_F3;
        key_name_map["F4"] = VK_F4;
        key_name_map["F5"] = VK_F5;
        key_name_map["F6"] = VK_F6;
        key_name_map["F7"] = VK_F7;
        key_name_map["F8"] = VK_F8;
        key_name_map["F9"] = VK_F9;
        key_name_map["F10"] = VK_F10;
        key_name_map["F11"] = VK_F11;
        key_name_map["F12"] = VK_F12;

        key_name_map["SPACE"] = VK_SPACE;
        key_name_map["ENTER"] = VK_RETURN;
        key_name_map["RETURN"] = VK_RETURN;
        key_name_map["TAB"] = VK_TAB;
        key_name_map["BACKSPACE"] = VK_BACK;
        key_name_map["ESCAPE"] = VK_ESCAPE;
        key_name_map["ESC"] = VK_ESCAPE;
        key_name_map["DELETE"] = VK_DELETE;
        key_name_map["DEL"] = VK_DELETE;
        key_name_map["INSERT"] = VK_INSERT;
        key_name_map["INS"] = VK_INSERT;
        key_name_map["HOME"] = VK_HOME;
        key_name_map["END"] = VK_END;
        key_name_map["PAGEUP"] = VK_PRIOR;
        key_name_map["PGUP"] = VK_PRIOR;
        key_name_map["PAGEDOWN"] = VK_NEXT;
        key_name_map["PGDN"] = VK_NEXT;

        key_name_map["LEFT"] = VK_LEFT;
        key_name_map["RIGHT"] = VK_RIGHT;
        key_name_map["UP"] = VK_UP;
        key_name_map["DOWN"] = VK_DOWN;

        key_name_map["SHIFT"] = VK_SHIFT;
        key_name_map["LSHIFT"] = VK_LSHIFT;
        key_name_map["RSHIFT"] = VK_RSHIFT;
        key_name_map["CTRL"] = VK_CONTROL;
        key_name_map["CONTROL"] = VK_CONTROL;
        key_name_map["LCTRL"] = VK_LCONTROL;
        key_name_map["RCTRL"] = VK_RCONTROL;
        key_name_map["ALT"] = VK_MENU;
        key_name_map["LALT"] = VK_LMENU;
        key_name_map["RALT"] = VK_RMENU;

        key_name_map["NUM0"] = VK_NUMPAD0;
        key_name_map["NUM1"] = VK_NUMPAD1;
        key_name_map["NUM2"] = VK_NUMPAD2;
        key_name_map["NUM3"] = VK_NUMPAD3;
        key_name_map["NUM4"] = VK_NUMPAD4;
        key_name_map["NUM5"] = VK_NUMPAD5;
        key_name_map["NUM6"] = VK_NUMPAD6;
        key_name_map["NUM7"] = VK_NUMPAD7;
        key_name_map["NUM8"] = VK_NUMPAD8;
        key_name_map["NUM9"] = VK_NUMPAD9;
        key_name_map["NUMLOCK"] = VK_NUMLOCK;

        key_name_map["MINUS"] = VK_OEM_MINUS;
        key_name_map["PLUS"] = VK_OEM_PLUS;
        key_name_map["COMMA"] = VK_OEM_COMMA;
        key_name_map["PERIOD"] = VK_OEM_PERIOD;
        key_name_map["SEMICOLON"] = VK_OEM_1;
        key_name_map["SLASH"] = VK_OEM_2;
        key_name_map["TILDE"] = VK_OEM_3;
        key_name_map["LBRACKET"] = VK_OEM_4;
        key_name_map["BACKSLASH"] = VK_OEM_5;
        key_name_map["RBRACKET"] = VK_OEM_6;
        key_name_map["QUOTE"] = VK_OEM_7;

        key_name_map["MOUSE1"] = VK_LBUTTON;
        key_name_map["MOUSE2"] = VK_RBUTTON;
        key_name_map["MOUSE3"] = VK_MBUTTON;
        key_name_map["MOUSE4"] = VK_XBUTTON1;
        key_name_map["MOUSE5"] = VK_XBUTTON2;
    }

    int Scripting::ParseKeyCode(const std::string& key_str) {
        try {
            const int code = std::stoi(key_str);
            if (code > 0 && code < 256) {
                return code;
            }
        }
        catch (...) {
        }

        std::string key = key_str;
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);

        const auto it = key_name_map.find(key);
        if (it != key_name_map.end()) {
            return it->second;
        }

        return 0;
    }

#ifdef _DEBUG
    void Scripting::SendInput(int vk_code, int duration_ms, int repeat_count) {
        if (vk_code == 0) {
            LOGF(WARNING, "Invalid key code for send");
            return;
        }

        for (int i = 0; i < repeat_count; i++) {
            if (shutdown_thread || !Renderer::IsFocused()) {
                break;
            }

            const int hold_key = active_hold_key.load();
            if (hold_key != 0 && !input::IsPhysicalInputDown(hold_key)) {
                break;
            }

            input::SendVirtualInput(vk_code, true);
            Sleep(duration_ms);
            input::SendVirtualInput(vk_code, false);

            if (i < repeat_count - 1) {
                Sleep(10);
            }
        }
    }
#endif
}
