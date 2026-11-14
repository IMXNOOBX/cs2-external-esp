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
#include <unordered_set>
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

        void FindInDirectory(std::vector<std::filesystem::path>& files, const std::filesystem::path& dir) {
            std::error_code ec;
            if (!std::filesystem::is_directory(dir, ec)) {
                return;
            }

            for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
                if (ec) {
                    return;
                }

                if (!entry.is_regular_file() || entry.path().extension() != ".sl") {
                    continue;
                }

                const std::string name = entry.path().filename().string();
                if (name == "prelude.sl" || name.rfind("solis_tmp_", 0) == 0) {
                    continue;
                }

                files.push_back(entry.path());
            }
        }
    }

    void Scripting::Init() {
        try {
            InitKeyNameMap();
            RegisterConfigVars();
            LoadScripts();

            // Might cause random issues with the key input, it has to be handled properly
        #if 0
            if (!input::InstallPhysicalInputHooks()) {
                LOGF(WARNING, "Could not install physical input hooks; hold bindings are unavailable");
            }
        #endif
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

    Scripting::ScriptRuntime::~ScriptRuntime() = default;

    void Scripting::RegisterNatives(solis::Vm& vm, const std::string& script_path, ScriptRuntime* runtime) {
        vm.define("set_bool", "(str,bool)->()", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "set_bool");
            SetBoolValue(ArgString(args, 0), args[1].asBool());
            return solis::Value();
        });

        vm.define("set_int", "(str,int)->()", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "set_int");
            SetIntValue(ArgString(args, 0), static_cast<int>(args[1].asInt()));
            return solis::Value();
        });

        vm.define("set_float", "(str,float)->()", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "set_float");
            SetFloatValue(ArgString(args, 0), static_cast<float>(args[1].asFloat()));
            return solis::Value();
        });

        vm.define("set_color", "(str,float,float,float,float)->()", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 5, "set_color");
            SetColorValue(ArgString(args, 0),
                static_cast<float>(args[1].asFloat()),
                static_cast<float>(args[2].asFloat()),
                static_cast<float>(args[3].asFloat()),
                static_cast<float>(args[4].asFloat()));
            return solis::Value();
        });

        vm.define("get_bool", "(str)->bool", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "get_bool");
            return solis::Value::boolean(GetBoolValue(ArgString(args, 0)));
        });

        vm.define("get_int", "(str)->int", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "get_int");
            return solis::Value::integer(GetIntValue(ArgString(args, 0)));
        });

        vm.define("get_float", "(str)->float", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "get_float");
            return solis::Value::number(GetFloatValue(ArgString(args, 0)));
        });

        vm.define("get_color", "(str)->str", [this](solis::Vm& vm, solis::Args args) {
            RequireArgs(args, 1, "get_color");
            return vm.string(GetColorValue(ArgString(args, 0)));
        });

        vm.define("reset_theme", "()->()", [](solis::Vm&, solis::Args) {
            theme::ThemeManager::Get().RequestReset();
            return solis::Value();
        });

        vm.define("echo", "(str)->()", [](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "echo");
            LOGF(INFO, "{}", args[0].toString());
            return solis::Value();
        });

        vm.define("print", "(str)->()", [](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "print");
            LOGF(INFO, "{}", args[0].toString());
            return solis::Value();
        });

        vm.define("bind", "(str,str)->()", [this, script_path, runtime](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "bind");
            if (!runtime || !runtime->binding_allowed) {
                LOGF(WARNING, "bind() can only be called from main()");
                return solis::Value();
            }
            RegisterBind(script_path, ArgString(args, 0), ArgString(args, 1));
            return solis::Value();
        });

        vm.define("hold", "(str,str)->()", [this, script_path, runtime](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "hold");
            if (!runtime || !runtime->binding_allowed) {
                LOGF(WARNING, "hold() can only be called from main()");
                return solis::Value();
            }
            RegisterHoldKey(script_path, ArgString(args, 0), ArgString(args, 1));
            return solis::Value();
        });

        vm.define("client_base", "()->int", [](solis::Vm&, solis::Args) {
            return solis::Value::integer(static_cast<int64_t>(Engine::GetClient().base));
        });

        vm.define("engine_base", "()->int", [](solis::Vm&, solis::Args) {
            return solis::Value::integer(static_cast<int64_t>(Engine::GetEngine().base));
        });

        vm.define("menu_open", "()->bool", [](solis::Vm&, solis::Args) {
            return solis::Value::boolean(Renderer::IsOpen());
        });

        vm.define("menu_x", "()->int", [](solis::Vm&, solis::Args) {
            return solis::Value::integer(static_cast<int64_t>(Menu::GetPos().x));
        });

        vm.define("menu_y", "()->int", [](solis::Vm&, solis::Args) {
            return solis::Value::integer(static_cast<int64_t>(Menu::GetPos().y));
        });

#ifdef _DEBUG
        vm.define("send", "(str,int,int)->()", [this](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "send");
            const int key = ParseKeyCode(ArgString(args, 0));
            const int duration = args.size() > 1 ? static_cast<int>(args[1].asInt()) : 50;
            const int repeat = args.size() > 2 ? static_cast<int>(args[2].asInt()) : 1;
            SendInput(key, duration, repeat);
            return solis::Value();
        });

        vm.define("read_int", "(int)->int", [](solis::Vm&, solis::Args args) {
            RequireArgs(args, 1, "read_int");
            const auto address = static_cast<uintptr_t>(args[0].asInt());
            return solis::Value::integer(Engine::GetProcess()->read<int>(address));
        });

        vm.define("write_int", "(int,int)->()", [](solis::Vm&, solis::Args args) {
            RequireArgs(args, 2, "write_int");
            const auto address = static_cast<uintptr_t>(args[0].asInt());
            Engine::GetProcess()->write<int>(address, static_cast<int>(args[1].asInt()));
            return solis::Value();
        });
#endif
    }

    std::string Scripting::HostDeclarations() {

        solis::Vm vm;
        RegisterNatives(vm, "", nullptr);
        return vm.declarations();
    }

    std::vector<std::filesystem::path> Scripting::FindScripts() const {
        std::vector<std::filesystem::path> files;
        const std::filesystem::path cwd = std::filesystem::current_path();

        FindInDirectory(files, cwd);
        FindInDirectory(files, cwd / "scripts");

        std::sort(files.begin(), files.end());
        return files;
    }

    std::string Scripting::DisplayName(const std::filesystem::path& file) const {
        std::error_code ec;
        auto path = std::filesystem::relative(file, std::filesystem::current_path(), ec);
        if (ec) {
            path = file;
        }

        path.replace_extension();
        return path.generic_string();
    }

    bool Scripting::CompileScript(const std::string& path, ScriptInfo& info, std::shared_ptr<ScriptRuntime>& runtime) {
        const std::string host = HostDeclarations();

        SolisScript::CompiledScript compiled;
        if (!SolisScript::Compile(path, host, compiled)) {
            LOGF(WARNING, "Failed to compile {}", path);
            return false;
        }

        // One script, one image, one Vm. Functions are called by name on that Vm, so there is no
        // per-function wrapper to compile and the script's globals keep their values between calls.
        auto script = std::make_shared<ScriptRuntime>();
        try {
            script->vm = std::make_unique<solis::Vm>();
            script->vm->openStd();
            RegisterNatives(*script->vm, path, script.get());
            script->vm->load(compiled.image);
        }
        catch (const std::exception& e) {
            LOGF(WARNING, "Could not load {}: {}", path, e.what());
            return false;
        }

        info.path = path;
        info.name = DisplayName(path);

        const std::string module = std::filesystem::path(path).stem().string();
        for (const auto& function : compiled.functions) {
            // Prefer module.function; fall back to the bare name, which Solis accepts when it is unambiguous.
            const std::string qualified = module + "." + function;
            script->functions.emplace(function, script->vm->hasFunction(qualified) ? qualified : function);

            if (function.rfind("btn_", 0) == 0 && function.size() > 4) {
                info.buttons.push_back({ function.substr(4), function });
            }
        }

        runtime = std::move(script);
        return true;
    }

    std::shared_ptr<Scripting::ScriptRuntime> Scripting::GetRuntime(const std::string& path) const {
        std::lock_guard<std::mutex> lock(runtime_mutex);
        const auto it = runtimes.find(path);
        return it != runtimes.end() ? it->second : nullptr;
    }

    void Scripting::UnloadRuntime(const std::string& path) {
        {
            std::lock_guard<std::mutex> lock(runtime_mutex);
            runtimes.erase(path);
        }

        keybinds.erase(std::remove_if(keybinds.begin(), keybinds.end(), [&](const Keybind& bind) {
            return bind.path == path;
        }), keybinds.end());

        holds.erase(std::remove_if(holds.begin(), holds.end(), [&](const HoldKey& hold) {
            return hold.path == path;
        }), holds.end());
    }

    void Scripting::UnloadScript(const std::string& path) {
        const auto it = std::find_if(scripts.begin(), scripts.end(), [&](const ScriptInfo& script) {
            return script.path == path;
        });

        if (it == scripts.end() || !it->loaded) {
            return;
        }

        UnloadRuntime(path);
        it->loaded = false;
        it->buttons.clear();
        LOGF(INFO, "Unloaded {}", it->name);
    }

    void Scripting::LoadScript(const std::string& path) {
        const auto it = std::find_if(scripts.begin(), scripts.end(), [&](const ScriptInfo& script) {
            return script.path == path;
        });

        if (it == scripts.end()) {
            return;
        }

        if (it->loaded) {
            UnloadScript(path);
        }

        ScriptInfo info;
        std::shared_ptr<ScriptRuntime> runtime;
        if (!CompileScript(path, info, runtime)) {
            it->loaded = false;
            it->buttons.clear();
            return;
        }

        {
            std::lock_guard<std::mutex> lock(runtime_mutex);
            runtimes[path] = std::move(runtime);
        }

        info.loaded = true;
        *it = std::move(info);

        RunInit(path);
        LOGF(INFO, "Loaded {}", it->name);
    }

    void Scripting::RefreshScripts() {
        const auto files = FindScripts();

        std::unordered_set<std::string> present;
        for (const auto& file : files) {
            present.insert(file.string());
        }

        for (const auto& script : scripts) {
            if (present.find(script.path) == present.end()) {
                UnloadRuntime(script.path);
            }
        }

        std::vector<ScriptInfo> updated;
        updated.reserve(files.size());

        for (const auto& file : files) {
            const std::string path = file.string();
            const auto existing = std::find_if(scripts.begin(), scripts.end(), [&](const ScriptInfo& script) {
                return script.path == path;
            });

            if (existing != scripts.end()) {
                updated.push_back(std::move(*existing));
            } else {
                ScriptInfo info;
                info.path = path;
                info.name = DisplayName(file);
                updated.push_back(std::move(info));
            }
        }

        scripts = std::move(updated);
        LOGF(INFO, "Refreshed script list: {} script(s)", scripts.size());
    }

    void Scripting::LoadScripts() {
        const auto files = FindScripts();

        scripts.clear();
        keybinds.clear();
        holds.clear();
        key_states.clear();
        {
            std::lock_guard<std::mutex> lock(runtime_mutex);
            runtimes.clear();
        }

        for (const auto& file : files) {
            ScriptInfo info;
            info.path = file.string();
            info.name = DisplayName(file);
            scripts.push_back(std::move(info));
        }

        for (const auto& script : scripts) {
            LoadScript(script.path);
        }

        const auto loaded = std::count_if(scripts.begin(), scripts.end(), [](const ScriptInfo& script) {
            return script.loaded;
        });

        if (scripts.empty()) {
            LOGF(INFO, "No scripts found in {} or {}", std::filesystem::current_path().string(), (std::filesystem::current_path() / "scripts").string());
        } else {
            LOGF(INFO, "Loaded {}/{} script(s)", loaded, scripts.size());
        }
    }

    void Scripting::RunInit(const std::string& path) {
        const auto runtime = GetRuntime(path);
        if (!runtime) {
            return;
        }

        const std::string label = DisplayName(path) + ".main";

        // Held for the whole run: a Vm is not thread safe, and an unload must wait for it to finish.
        std::lock_guard<std::mutex> lock(runtime->vm_mutex);
        runtime->binding_allowed = true;
        try {
            runtime->vm->run();
        }
        catch (const solis::Error& e) {
            LOGF(WARNING, "Solis error in '{}': {}", label, e.what());
        }
        catch (const std::exception& e) {
            LOGF(WARNING, "Error in '{}': {}", label, e.what());
        }
        runtime->binding_allowed = false;
    }

    void Scripting::RunFunction(const std::string& path, const std::string& function) {
        const auto runtime = GetRuntime(path);

        std::string target;
        if (runtime) {
            const auto it = runtime->functions.find(function);
            if (it != runtime->functions.end()) {
                target = it->second;
            }
        }

        if (target.empty()) {
            LOGF(WARNING, "Unknown function: {}.{}", DisplayName(path), function);
            return;
        }

        const std::string label = DisplayName(path) + "." + function;

        std::lock_guard<std::mutex> lock(runtime->vm_mutex);
        try {
            runtime->vm->call(target);
        }
        catch (const solis::Error& e) {
            LOGF(WARNING, "Solis error in '{}': {}", label, e.what());
        }
        catch (const std::exception& e) {
            LOGF(WARNING, "Error in '{}': {}", label, e.what());
        }
    }

    void Scripting::Update() {
        for (const auto& bind : keybinds) {
            const bool pressed = (GetAsyncKeyState(bind.key) & 0x8000) != 0;
            if (pressed && !key_states[bind.key] && active_script_count == 0) {
                QueueFunction(bind.path, bind.function);
            }
            key_states[bind.key] = pressed;
        }

        for (const auto& hold : holds) {
            const bool pressed = input::IsPhysicalInputDown(hold.key);
            if (pressed && !key_states[hold.key] && active_script_count == 0) {
                QueueFunction(hold.path, hold.function, hold.key);
            }
            key_states[hold.key] = pressed;
        }
    }

    void Scripting::ExecuteFunction(const std::string& path, const std::string& function) {
        QueueFunction(path, function);
    }

    void Scripting::QueueFunction(const std::string& path, const std::string& function, int hold_key) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        function_queue.push(PendingFunction{ path, function, hold_key });
    }

    void Scripting::WorkerThread() {
        while (!shutdown_thread) {
            PendingFunction pending;
            bool has_function = false;

            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                if (!function_queue.empty()) {
                    pending = std::move(function_queue.front());
                    function_queue.pop();
                    has_function = true;
                }
            }

            if (!has_function) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            active_script_count.fetch_add(1);
            active_hold_key = pending.hold_key;
            RunFunction(pending.path, pending.function);
            active_hold_key = 0;
            active_script_count.fetch_sub(1);
        }
    }

    bool Scripting::FunctionExists(const std::string& path, const std::string& function) const {
        const auto runtime = GetRuntime(path);
        return runtime && runtime->functions.find(function) != runtime->functions.end();
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

    void Scripting::RegisterBind(const std::string& path, const std::string& key, const std::string& function) {
        if (!FunctionExists(path, function)) {
            LOGF(WARNING, "No function '{}' in {}", function, DisplayName(path));
            return;
        }

        const int code = ParseKeyCode(key);
        if (code == 0) {
            LOGF(WARNING, "Unknown key name: {}", key);
            return;
        }

        keybinds.push_back({ code, path, function });
        LOGF(INFO, "Bound {} to {}.{}", key, DisplayName(path), function);
    }

    void Scripting::RegisterHoldKey(const std::string& path, const std::string& key, const std::string& function) {
        if (!FunctionExists(path, function)) {
            LOGF(WARNING, "No function '{}' in {}", function, DisplayName(path));
            return;
        }

        const int code = ParseKeyCode(key);
        if (code == 0) {
            LOGF(WARNING, "Unknown hold key name: {}", key);
            return;
        }

        holds.push_back({ code, path, function });
        LOGF(INFO, "Holding {} runs {}.{}", key, DisplayName(path), function);
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

        for (int i = 0; i <= 9; i++) {
            key_name_map["NUMPAD" + std::to_string(i)] = VK_NUMPAD0 + i;
        }
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
