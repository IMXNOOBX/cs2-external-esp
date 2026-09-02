#include "Scripting.hpp"
#include "core/logger/LogHelper.hpp"
#include "core\engine\types\Types.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <Windows.h>

#ifdef _DEBUG
#include "core/engine/Engine.hpp"
#endif

namespace scripting {

    void Scripting::Init() {
        try {
            InitKeyNameMap();
            RegisterConfigVars();
            RegisterCommands();
            LoadScripts();
        }
        catch (std::exception& e) {
            LOGF(FATAL, "Caught exception while initializing: {}", e.what());
        }
        
        command_thread = std::thread(&Scripting::CommandWorkerThread, this);
        
        LOGF(INFO, "Scripting system initialized");
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

        // Colors
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

    void Scripting::RegisterCommands() {
        command_map["set"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 2) return;
            const std::string& var_name = args[0];
            std::string value = args[1];

            // Evaluate if it contains operators
            if (value.find_first_of("+-*/") != std::string::npos || value[0] == '$') {
                value = EvaluateExpression(value);
            }

            if (config_map.find(var_name) == config_map.end()) {
                LOGF(WARNING, "Unknown config variable: {}", var_name);
                return;
            }

            auto& var = config_map[var_name];
            if (std::holds_alternative<bool*>(var)) {
                *std::get<bool*>(var) = (value == "true" || value == "1");
            } else if (std::holds_alternative<int*>(var)) {
                *std::get<int*>(var) = static_cast<int>(EvaluateArithmetic(value));
            } else if (std::holds_alternative<float*>(var)) {
                *std::get<float*>(var) = EvaluateArithmetic(value);
            }
        };

        command_map["setcolor"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 5) {
                LOGF(WARNING, "setcolor requires 5 arguments: variable r g b a");
                return;
            }
            
            const std::string& var_name = args[0];
            try {
                float r = EvaluateArithmetic(args[1]);
                float g = EvaluateArithmetic(args[2]);
                float b = EvaluateArithmetic(args[3]);
                float a = EvaluateArithmetic(args[4]);
                SetColorValue(var_name, r, g, b, a);
            } catch (const std::exception& e) {
                LOGF(WARNING, "Error setting color {}: {}", var_name, e.what());
            }
        };

        command_map["get"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                LOGF(WARNING, "get requires 2 arguments: variable destination");
                return;
            }
            
            const std::string& var_name = args[0];
            const std::string& dest_var = args[1];
            
            std::string value = GetConfigValue(var_name);
            variables[dest_var] = value;
            LOGF(INFO, "get {} -> ${} = {}", var_name, dest_var, value);
        };

        command_map["var"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 2) return;
            const std::string& var_name = args[0];
            std::string value = args[1];
            
            // Evaluate if needed
            if (value.find_first_of("+-*/") != std::string::npos || value[0] == '$') {
                value = EvaluateExpression(value);
            }
            
            variables[var_name] = value;
            LOGF(INFO, "var ${} = {}", var_name, value);
        };

        command_map["echo"] = [this](const std::vector<std::string>& args) {
            if (args.empty()) return;
            std::string msg;
            for (const auto& arg : args) {
                if (!msg.empty()) msg += " ";
                msg += (arg[0] == '$') ? EvaluateExpression(arg) : arg;
            }
            LOGF(INFO, "echo: {}", msg);
        };

        command_map["run"] = [this](const std::vector<std::string>& args) {
            if (args.empty()) {
                LOGF(WARNING, "run requires macro name");
                return;
            }
            
            const std::string& macro_name = args[0];
            
            if (macros.count(macro_name)) {
                LOGF(INFO, "Running macro: {}", macro_name);
                for (const auto& macro_cmd : macros[macro_name].commands) {
                    ExecuteCommand(macro_cmd);
                }
            } else {
                LOGF(WARNING, "Macro '{}' not found", macro_name);
            }
        };

#ifdef _DEBUG
        // Dangerous stuff
        command_map["send"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 1) {
                LOGF(WARNING, "send requires: key [duration_ms] [repeat_count]");
                return;
            }
            
            int vk_code = ParseKeyCode(args[0]);
            int duration_ms = args.size() > 1 ? std::stoi(EvaluateExpression(args[1])) : 50;
            int repeat_count = args.size() > 2 ? std::stoi(EvaluateExpression(args[2])) : 1;
            
            SendInput(vk_code, duration_ms, repeat_count);
        };

        command_map["!read"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                LOGF(WARNING, "!read requires: variable_name address");
                return;
            }
            
            const std::string& var_name = args[0];
            uintptr_t address = std::stoull(EvaluateExpression(args[1]), nullptr, 16);
            ReadMemory(var_name, address);
        };

        command_map["!write"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                LOGF(WARNING, "!write requires: variable_name address");
                return;
            }
            
            const std::string& var_name = args[0];
            uintptr_t address = std::stoull(EvaluateExpression(args[1]), nullptr, 16);
            WriteMemory(var_name, address);
        };
#endif
    }

    void Scripting::Update() {
        if (std::filesystem::exists("scripts.mcr")) {
            auto current_write_time = std::filesystem::last_write_time("scripts.mcr");
            if (current_write_time != last_load_time) {
                LOGF(INFO, "scripts.mcr changed, reloading...");
                LoadScripts();
            }
        }

        // Check for stop key down
        if (!current_running_macro.empty() && macros.count(current_running_macro)) {
            int stop_key = macros[current_running_macro].stop_key;
            if (stop_key != 0 && (GetAsyncKeyState(stop_key) & 0x8000)) {
                stop_current_macro = true;
                LOGF(INFO, "Stop key pressed - terminating macro '{}'", current_running_macro);
            }
        }

        for (const auto& bind : keybinds) {
            bool is_pressed = (GetAsyncKeyState(bind.key) & 0x8000);
            if (is_pressed && !key_states[bind.key]) {
                ExecuteCommand(bind.action);
            }
            key_states[bind.key] = is_pressed;
        }
    }

    void Scripting::ExecuteCommand(const std::string& command_line) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        command_queue.push(command_line);
    }

    void Scripting::CommandWorkerThread() {
        while (!shutdown_thread) {
            std::string command_to_execute;
            
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                if (!command_queue.empty()) {
                    command_to_execute = command_queue.front();
                    command_queue.pop();
                }
            }
            
            if (!command_to_execute.empty()) {
                ExecuteCommandInternal(command_to_execute);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    void Scripting::ExecuteCommandInternal(const std::string& command_line) {
        std::stringstream ss(command_line);
        std::string cmd;
        ss >> cmd;

        std::vector<std::string> args;
        std::string arg;
        while (ss >> arg) {
            args.push_back(arg);
        }

        try {
            ProcessCommand(cmd, args);
        }
        catch (std::exception& e) {
            LOGF(FATAL, "Caught exception on command execution: {}", e.what());
        }
    }

    void Scripting::ProcessCommand(const std::string& cmd, const std::vector<std::string>& args) {
        if (command_map.count(cmd)) {
            command_map[cmd](args);
        } else if (macros.count(cmd)) {
            current_running_macro = cmd;
            stop_current_macro = false;
            
            LOGF(INFO, "Executing macro: {}", cmd);
            
            for (const auto& macro_cmd : macros[cmd].commands) {
                if (stop_current_macro) {
                    LOGF(INFO, "Macro '{}' stopped by user", cmd);
                    break;
                }
                ExecuteCommandInternal(macro_cmd);
            }
            
            current_running_macro = "";
        } else {
            LOGF(WARNING, "Unknown command or macro: {}", cmd);
        }
    }

    void Scripting::LoadScripts() {
        std::ifstream f("scripts.mcr");
        if (!f.is_open()) {
            LOGF(INFO, "No scripts.mcr found, skipping");
            return;
        }

        keybinds.clear();
        macros.clear();

        std::string line;
        std::string current_macro = "";
        bool in_braces = false;
        int brace_depth = 0;
        
        while (std::getline(f, line)) {
            // Trim
            size_t first = line.find_first_not_of(" \t");
            if (first == std::string::npos) continue;
            line = line.substr(first);
            
            if (line.empty() || line[0] == '#') continue;

            // opening brace
            if (!current_macro.empty() && line.find("{") != std::string::npos) {
                in_braces = true;
                brace_depth++;
                continue;
            }

            // closing brace
            if (in_braces && line.find("}") != std::string::npos) {
                brace_depth--;
                if (brace_depth == 0) {
                    in_braces = false;
                    
                    // Check for stop key after closing brace
                    size_t brace_pos = line.find("}");
                    size_t stop_key_start = line.find_first_not_of(" \t", brace_pos + 1);
                    if (stop_key_start != std::string::npos && !current_macro.empty()) {
                        std::string stop_key_str = line.substr(stop_key_start);
                        // Remove trailing semicolon or whitespace
                        size_t end = stop_key_str.find_first_of(" \t;");
                        if (end != std::string::npos) {
                            stop_key_str = stop_key_str.substr(0, end);
                        }
                        
                        int stop_key = ParseKeyCode(stop_key_str);
                        if (stop_key != 0) {
                            macros[current_macro].stop_key = stop_key;
                            LOGF(INFO, "Macro '{}' has stop key: {} ({})", current_macro, stop_key_str, stop_key);
                        }
                    }
                    
                    current_macro = "";
                }
                continue;
            }

            if (line.find("bind ") == 0) {
                current_macro = "";
                in_braces = false;
                std::stringstream ss(line.substr(5));
                std::string key_str;
                std::string action;
                if (ss >> key_str >> action) {
                    int key = ParseKeyCode(key_str);
                    if (key != 0) {
                        keybinds.push_back({ key, action });
                        LOGF(INFO, "Bound key '{}' ({}) to action '{}'", key_str, key, action);
                    } else {
                        LOGF(WARNING, "Unknown key name: {}", key_str);
                    }
                }
            } else if (line.find("macro ") == 0) {
                in_braces = false;
                std::stringstream ss(line.substr(6));
                std::string name;
                if (ss >> name) {
                    bool gui_accessible = false;
                    if (name[0] == '@') {
                        gui_accessible = true;
                        name = name.substr(1);
                    }
                    
                    current_macro = name;
                    macros[name].name = name;
                    macros[name].gui_accessible = gui_accessible;
                    macros[name].stop_key = 0; // Default no stop key
                    
                    std::string rest;
                    std::getline(ss, rest);
                    
                    // Check for stop key after closing brace
                    size_t brace_pos = rest.find("{");
                    if (brace_pos != std::string::npos) {
                        in_braces = true;
                        brace_depth++;
                        
                        // Look for stop key after }
                        size_t close_brace = rest.find("}", brace_pos);
                        if (close_brace != std::string::npos) {
                            // Single-line macro with stop key
                            size_t stop_key_start = rest.find_first_not_of(" \t", close_brace + 1);
                            if (stop_key_start != std::string::npos) {
                                std::string stop_key_str = rest.substr(stop_key_start);
                                // Remove trailing semicolon or whitespace
                                size_t end = stop_key_str.find_first_of(" \t;");
                                if (end != std::string::npos) {
                                    stop_key_str = stop_key_str.substr(0, end);
                                }
                                
                                int stop_key = ParseKeyCode(stop_key_str);
                                if (stop_key != 0) {
                                    macros[name].stop_key = stop_key;
                                    LOGF(INFO, "Macro '{}' has stop key: {} ({})", name, stop_key_str, stop_key);
                                }
                            }
                        }
                    }
                }
            } else if (in_braces && !current_macro.empty()) {
                macros[current_macro].commands.push_back(line);
                // Check for dangerous commands
                std::stringstream cmd_ss(line);
                std::string cmd;
                cmd_ss >> cmd;
                if (IsDangerousCommand(cmd)) {
                    macros[current_macro].contains_dangerous_commands = true;
                    has_dangerous_scripts = true;
                }
            } else if (!in_braces && !current_macro.empty() && (line[0] == ' ' || line[0] == '\t')) {
                size_t cmd_start = line.find_first_not_of(" \t");
                if (cmd_start != std::string::npos) {
                    std::string cmd_line = line.substr(cmd_start);
                    macros[current_macro].commands.push_back(cmd_line);
                    // Check for dangerous commands
                    std::stringstream cmd_ss(cmd_line);
                    std::string cmd;
                    cmd_ss >> cmd;
                    if (IsDangerousCommand(cmd)) {
                        macros[current_macro].contains_dangerous_commands = true;
                        has_dangerous_scripts = true;
                    }
                }
            } else if (!in_braces) {
                // Top-level command - execute immediately during script load (not async)
                current_macro = "";
                ExecuteCommandInternal(line);
            }
        }

        if (std::filesystem::exists("scripts.mcr")) {
            last_load_time = std::filesystem::last_write_time("scripts.mcr");
        }

        // Warn about dangerous scripts
        if (has_dangerous_scripts) {
            LOGF(WARNING, "=======================================================");
            LOGF(WARNING, "WARNING: Script contains DANGEROUS commands!");
            LOGF(WARNING, "Commands detected: send, !read, or !write");
            LOGF(WARNING, "These can:");
            LOGF(WARNING, "  - Modify game memory (crash/instability)");
            LOGF(WARNING, "  - Send inputs (automation detection)");
            LOGF(WARNING, "  - Trigger anti-cheat systems");
            LOGF(WARNING, "USE AT YOUR OWN RISK!");
            LOGF(WARNING, "=======================================================");
        }

        LOGF(INFO, "Scripts loaded successfully - {} macros, {} keybinds", macros.size(), keybinds.size());
    }

    std::string Scripting::EvaluateExpression(const std::string& expr) {
        if (expr[0] == '$') {
            std::string var_name = expr.substr(1);
            if (variables.find(var_name) != variables.end()) {
                return variables[var_name];
            }
            return "0";
        }

        if (expr.find_first_of("+-*/") != std::string::npos) {
            return std::to_string(EvaluateArithmetic(expr));
        }

        return expr;
    }

    float Scripting::EvaluateArithmetic(const std::string& expr) {
        std::string eval_expr = expr;
        
        size_t pos = 0;
        while ((pos = eval_expr.find('$', pos)) != std::string::npos) {
            size_t end = eval_expr.find_first_of(" +-*/()", pos + 1);
            if (end == std::string::npos) end = eval_expr.length();
            
            std::string var_name = eval_expr.substr(pos + 1, end - pos - 1);
            std::string value = "0";
            if (variables.find(var_name) != variables.end()) {
                value = variables[var_name];
            }
            
            eval_expr.replace(pos, end - pos, value);
            pos += value.length();
        }

        try {
            eval_expr.erase(std::remove(eval_expr.begin(), eval_expr.end(), ' '), eval_expr.end());

            size_t mul_pos;
            while ((mul_pos = eval_expr.find('*')) != std::string::npos || 
                   (mul_pos = eval_expr.find('/')) != std::string::npos) {
                char op = eval_expr[mul_pos];
                
                size_t left_start = mul_pos;
                while (left_start > 0 && (std::isdigit(eval_expr[left_start - 1]) || 
                                          eval_expr[left_start - 1] == '.' ||
                                          eval_expr[left_start - 1] == '-')) {
                    left_start--;
                }

                size_t right_end = mul_pos + 1;
                if (right_end < eval_expr.length() && eval_expr[right_end] == '-') {
                    right_end++; // Handle neg numbers
                }
                while (right_end < eval_expr.length() && 
                       (std::isdigit(eval_expr[right_end]) || eval_expr[right_end] == '.')) {
                    right_end++;
                }
                
                float left = std::stof(eval_expr.substr(left_start, mul_pos - left_start));
                float right = std::stof(eval_expr.substr(mul_pos + 1, right_end - mul_pos - 1));
                float result = (op == '*') ? (left * right) : (left / right);
                
                eval_expr.replace(left_start, right_end - left_start, std::to_string(result));
            }
            
            float result = 0;
            size_t i = 0;
            bool is_negative = false;
            
            if (!eval_expr.empty() && eval_expr[0] == '-') {
                is_negative = true;
                i = 1;
            }
            
            size_t start = i;
            while (i <= eval_expr.length()) {
                if (i == eval_expr.length() || eval_expr[i] == '+' || 
                    (eval_expr[i] == '-' && i > 0)) {
                    if (i > start) {
                        float num = std::stof(eval_expr.substr(start, i - start));
                        result += is_negative ? -num : num;
                    }
                    
                    if (i < eval_expr.length()) {
                        is_negative = (eval_expr[i] == '-');
                        start = i + 1;
                    }
                }
                i++;
            }
            
            return result;
        } catch (const std::exception& e) {
            LOGF(WARNING, "Error evaluating arithmetic '{}': {}", expr, e.what());
            return 0.0f;
        }
    }

    std::string Scripting::GetConfigValue(const std::string& var_name) {
        if (config_map.find(var_name) == config_map.end()) {
            LOGF(WARNING, "Unknown config variable: {}", var_name);
            return "0";
        }

        auto& var = config_map[var_name];
        if (std::holds_alternative<bool*>(var)) {
            return *std::get<bool*>(var) ? "true" : "false";
        } else if (std::holds_alternative<int*>(var)) {
            return std::to_string(*std::get<int*>(var));
        } else if (std::holds_alternative<float*>(var)) {
            return std::to_string(*std::get<float*>(var));
        } else if (std::holds_alternative<color_t*>(var)) {
            auto* color = std::get<color_t*>(var);
            return std::to_string(color->data()[0]) + "," + 
                   std::to_string(color->data()[1]) + "," + 
                   std::to_string(color->data()[2]) + "," + 
                   std::to_string(color->data()[3]);
        }
        
        return "0";
    }

    void Scripting::SetColorValue(const std::string& var_name, float r, float g, float b, float a) {
        if (config_map.find(var_name) == config_map.end()) {
            LOGF(WARNING, "Unknown config variable: {}", var_name);
            return;
        }

        auto& var = config_map[var_name];
        if (std::holds_alternative<color_t*>(var)) {
            auto* color = std::get<color_t*>(var);
            color->r = std::clamp(r, 0.0f, 1.0f);
            color->g = std::clamp(g, 0.0f, 1.0f);
            color->b = std::clamp(b, 0.0f, 1.0f);
            color->a = std::clamp(a, 0.0f, 1.0f);
            LOGF(INFO, "Set color {}: ({}, {}, {}, {})", var_name, r, g, b, a);
        } else {
            LOGF(WARNING, "{} is not a color variable", var_name);
        }
    }

    void Scripting::InitKeyNameMap() {
        // Letters A-Z
        for (char c = 'A'; c <= 'Z'; c++) {
            key_name_map[std::string(1, c)] = c;
        }
        
        // Numbers 0-9
        for (char c = '0'; c <= '9'; c++) {
            key_name_map[std::string(1, c)] = c;
        }
        
        // fn keys
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
        
        // special keys
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
        
        // dir keys
        key_name_map["LEFT"] = VK_LEFT;
        key_name_map["RIGHT"] = VK_RIGHT;
        key_name_map["UP"] = VK_UP;
        key_name_map["DOWN"] = VK_DOWN;
        
        // mod keys
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
        
        // nump
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
        
        // symbol
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
        
        // mouse
        key_name_map["MOUSE1"] = VK_LBUTTON;
        key_name_map["MOUSE2"] = VK_RBUTTON;
        key_name_map["MOUSE3"] = VK_MBUTTON;
        key_name_map["MOUSE4"] = VK_XBUTTON1;
        key_name_map["MOUSE5"] = VK_XBUTTON2;
    }

    int Scripting::ParseKeyCode(const std::string& key_str) {
        // parse numeric key code first
        try {
            int key_code = std::stoi(key_str);
            if (key_code > 0 && key_code < 256) {
                return key_code;
            }
        } catch (...) {
            // Not a number
        }
        
        // Convert to uppercase
        std::string upper_key = key_str;
        std::transform(upper_key.begin(), upper_key.end(), upper_key.begin(), ::toupper);
        
        // Look up key
        auto it = key_name_map.find(upper_key);
        if (it != key_name_map.end()) {
            return it->second;
        }
        
        return 0; // Unknown
    }

    bool Scripting::IsDangerousCommand(const std::string& cmd) {
        return (cmd == "send" || cmd == "!read" || cmd == "!write");
    }

#ifdef _DEBUG
    void Scripting::SendInput(int vk_code, int duration_ms, int repeat_count) {
        if (vk_code == 0) {
            LOGF(WARNING, "Invalid key code for send");
            return;
        }

        LOGF(INFO, "Sending input: key={}, duration={}ms, repeat={}", vk_code, duration_ms, repeat_count);

        for (int i = 0; i < repeat_count; i++) {
            // Key down
            keybd_event(static_cast<BYTE>(vk_code), 0, 0, 0);
            Sleep(duration_ms);
            // Key up
            keybd_event(static_cast<BYTE>(vk_code), 0, KEYEVENTF_KEYUP, 0);
            
            if (i < repeat_count - 1) {
                Sleep(10); // Small delay between repeats
            }
        }
    }

    void Scripting::ReadMemory(const std::string& var_name, uintptr_t address) {
        LOGF(WARNING, "!read command: Reading from address 0x{:X} into variable ${}", address, var_name);
        variables[var_name] = Engine::GetProcess()->read<int>(address);

    }

    void Scripting::WriteMemory(const std::string& var_name, uintptr_t address) {
        if (variables.find(var_name) == variables.end()) {
            LOGF(WARNING, "Variable ${} not found for !write", var_name);
            return;
        }
        int value = std::stoi(variables[var_name]);
        Engine::GetProcess()->write<int>(address, value);
        LOGF(WARNING, "!write command: Writing ${} to address 0x{:X}", var_name, address);
    }
#endif

}
