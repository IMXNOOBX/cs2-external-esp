#include "Theme.hpp"
#include "core/logger/LogHelper.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace theme {

    void ThemeManager::Init() {
        // name mapping
        color_name_map["text"] = ImGuiCol_Text;
        color_name_map["textdisabled"] = ImGuiCol_TextDisabled;
        color_name_map["windowbg"] = ImGuiCol_WindowBg;
        color_name_map["childbg"] = ImGuiCol_ChildBg;
        color_name_map["popupbg"] = ImGuiCol_PopupBg;
        color_name_map["border"] = ImGuiCol_Border;
        color_name_map["bordershadow"] = ImGuiCol_BorderShadow;
        color_name_map["framebg"] = ImGuiCol_FrameBg;
        color_name_map["framebghovered"] = ImGuiCol_FrameBgHovered;
        color_name_map["framebgactive"] = ImGuiCol_FrameBgActive;
        color_name_map["titlebg"] = ImGuiCol_TitleBg;
        color_name_map["titlebgactive"] = ImGuiCol_TitleBgActive;
        color_name_map["titlebgcollapsed"] = ImGuiCol_TitleBgCollapsed;
        color_name_map["menubarbg"] = ImGuiCol_MenuBarBg;
        color_name_map["scrollbarbg"] = ImGuiCol_ScrollbarBg;
        color_name_map["scrollbargrab"] = ImGuiCol_ScrollbarGrab;
        color_name_map["scrollbargrabhovered"] = ImGuiCol_ScrollbarGrabHovered;
        color_name_map["scrollbargrabactive"] = ImGuiCol_ScrollbarGrabActive;
        color_name_map["checkmark"] = ImGuiCol_CheckMark;
        color_name_map["slidergrab"] = ImGuiCol_SliderGrab;
        color_name_map["slidergrabactive"] = ImGuiCol_SliderGrabActive;
        color_name_map["button"] = ImGuiCol_Button;
        color_name_map["buttonhovered"] = ImGuiCol_ButtonHovered;
        color_name_map["buttonactive"] = ImGuiCol_ButtonActive;
        color_name_map["header"] = ImGuiCol_Header;
        color_name_map["headerhovered"] = ImGuiCol_HeaderHovered;
        color_name_map["headeractive"] = ImGuiCol_HeaderActive;
        color_name_map["separator"] = ImGuiCol_Separator;
        color_name_map["separatorhovered"] = ImGuiCol_SeparatorHovered;
        color_name_map["separatoractive"] = ImGuiCol_SeparatorActive;
        color_name_map["resizegrip"] = ImGuiCol_ResizeGrip;
        color_name_map["resizegriphovered"] = ImGuiCol_ResizeGripHovered;
        color_name_map["resizegripactive"] = ImGuiCol_ResizeGripActive;
        color_name_map["tab"] = ImGuiCol_Tab;
        color_name_map["tabhovered"] = ImGuiCol_TabHovered;
        color_name_map["tabactive"] = ImGuiCol_TabActive;
        color_name_map["tabunfocused"] = ImGuiCol_TabUnfocused;
        color_name_map["tabunfocusedactive"] = ImGuiCol_TabUnfocusedActive;
        color_name_map["dockingpreview"] = ImGuiCol_DockingPreview;
        color_name_map["dockingemptybg"] = ImGuiCol_DockingEmptyBg;
        color_name_map["plotlines"] = ImGuiCol_PlotLines;
        color_name_map["plotlineshovered"] = ImGuiCol_PlotLinesHovered;
        color_name_map["plothistogram"] = ImGuiCol_PlotHistogram;
        color_name_map["plothistogramhovered"] = ImGuiCol_PlotHistogramHovered;
        color_name_map["tableheaderbg"] = ImGuiCol_TableHeaderBg;
        color_name_map["tablebordersstrong"] = ImGuiCol_TableBorderStrong;
        color_name_map["tableborderslight"] = ImGuiCol_TableBorderLight;
        color_name_map["tablerowbg"] = ImGuiCol_TableRowBg;
        color_name_map["tablerowbgalt"] = ImGuiCol_TableRowBgAlt;
        color_name_map["textselectedbg"] = ImGuiCol_TextSelectedBg;
        color_name_map["dragdroptarget"] = ImGuiCol_DragDropTarget;
        color_name_map["navhighlight"] = ImGuiCol_NavHighlight;
        color_name_map["navwindowinghighlight"] = ImGuiCol_NavWindowingHighlight;
        color_name_map["navwindowingdimbg"] = ImGuiCol_NavWindowingDimBg;
        color_name_map["modalwindowdimbg"] = ImGuiCol_ModalWindowDimBg;

        // Create "Default" theme from base colors
        CaptureCurrentColors();
        ThemeInfo default_theme = themes["current"];
        default_theme.name = "Default";
        default_theme.author = "Built-in";
        default_theme.description = "Default theme";
        themes["Default"] = default_theme;

        LoadThemesFromDirectory();
        LOGF(INFO, "Theme system initialized with {} themes", themes.size());
    }

    void ThemeManager::LoadThemesFromDirectory() {
        std::string theme_dir = "themes";
        
        if (!std::filesystem::exists(theme_dir)) {
            std::filesystem::create_directories(theme_dir);
            LOGF(INFO, "Created themes directory at {}", theme_dir);
        }

        for (const auto& entry : std::filesystem::directory_iterator(theme_dir)) {
            if (entry.path().extension() == ".txt") {
                LoadTheme(entry.path().string());
            }
        }
    }

    bool ThemeManager::LoadTheme(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            LOGF(WARNING, "Failed to open theme file: {}", filename);
            return false;
        }

        ThemeInfo theme;
        
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            theme.colors[i] = style.Colors[i];
        }

        std::string line;
        int line_num = 0;
        bool has_name = false;

        while (std::getline(file, line)) {
            line_num++;
            
            size_t first = line.find_first_not_of(" \t");
            if (first == std::string::npos) continue;
            line = line.substr(first);
            
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            std::string key;
            ss >> key;

            std::transform(key.begin(), key.end(), key.begin(), ::tolower);

            // metadata
            if (key == "name") {
                std::getline(ss, theme.name);
                size_t start = theme.name.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    theme.name = theme.name.substr(start);
                }
                has_name = true;
                continue;
            }
            else if (key == "author") {
                std::getline(ss, theme.author);
                size_t start = theme.author.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    theme.author = theme.author.substr(start);
                }
                continue;
            }
            else if (key == "description") {
                std::getline(ss, theme.description);
                size_t start = theme.description.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    theme.description = theme.description.substr(start);
                }
                continue;
            }

            auto it = color_name_map.find(key);
            if (it != color_name_map.end()) {
                float r, g, b, a;
                if (ss >> r >> g >> b >> a) {
                    r = std::clamp(r, 0.0f, 1.0f);
                    g = std::clamp(g, 0.0f, 1.0f);
                    b = std::clamp(b, 0.0f, 1.0f);
                    a = std::clamp(a, 0.0f, 1.0f);
                    
                    theme.colors[it->second] = ImVec4(r, g, b, a);
                } else {
                    LOGF(WARNING, "Invalid color values in {} at line {}", filename, line_num);
                }
            } else {
                LOGF(WARNING, "Unknown color name '{}' in {} at line {}", key, filename, line_num);
            }
        }

        if (!has_name) {
            std::filesystem::path p(filename);
            theme.name = p.stem().string();
        }

        themes[theme.name] = theme;
        LOGF(INFO, "Loaded theme: {} from {}", theme.name, filename);
        
        return true;
    }

    void ThemeManager::ApplyTheme(const std::string& theme_name) {
        auto it = themes.find(theme_name);
        if (it == themes.end()) {
            LOGF(WARNING, "Theme '{}' not found", theme_name);
            return;
        }

        const ThemeInfo& theme = it->second;
        ImGuiStyle& style = ImGui::GetStyle();

        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            style.Colors[i] = theme.colors[i];
        }

        current_theme_name = theme_name;
        LOGF(INFO, "Applied theme: {}", theme_name);
    }

    void ThemeManager::SaveCurrentTheme(const std::string& filename) {
        CaptureCurrentColors();
        
        std::string filepath = "assets/themes/" + filename;
        if (filepath.find(".txt") == std::string::npos) {
            filepath += ".txt";
        }

        std::ofstream file(filepath);
        if (!file.is_open()) {
            LOGF(FATAL, "Failed to save theme to: {}", filepath);
            return;
        }

        std::filesystem::path p(filename);
        std::string name = p.stem().string();

        file << "# Custom Theme: " << name << "\n";
        file << "# Generated by CS2 External ESP\n\n";
        file << "name " << name << "\n";
        file << "author YourName\n";
        file << "description Custom theme\n\n";

        ImGuiStyle& style = ImGui::GetStyle();
        for (const auto& [color_name, idx] : color_name_map) {
            const ImVec4& color = style.Colors[idx];
            file << color_name << " " 
                 << color.x << " " 
                 << color.y << " " 
                 << color.z << " " 
                 << color.w << "\n";
        }

        file.close();
        LOGF(INFO, "Saved current theme to: {}", filepath);
    }

    void ThemeManager::ExportDefaultTheme() {
        SaveCurrentTheme("default.txt");
    }

    std::vector<std::string> ThemeManager::GetThemeNames() const {
        std::vector<std::string> names;
        names.reserve(themes.size());
        
        // Add "Default" first if it exists
        if (themes.count("Default")) {
            names.push_back("Default");
        }
        
        // Add all other themes
        for (const auto& [name, theme] : themes) {
            if (name != "Default" && name != "current") {
                names.push_back(name);
            }
        }
        
        // Sort all themes except "Default" (keep it at position 0)
        if (names.size() > 1) {
            std::sort(names.begin() + 1, names.end());
        }
        
        return names;
    }

    const ThemeInfo* ThemeManager::GetThemeInfo(const std::string& name) const {
        auto it = themes.find(name);
        if (it != themes.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void ThemeManager::CaptureCurrentColors() {
        ThemeInfo current;
        current.name = "current";
        
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            current.colors[i] = style.Colors[i];
        }
        
        themes["current"] = current;
    }

    std::string ThemeManager::GetColorName(ImGuiCol idx) const {
        for (const auto& [name, color_idx] : color_name_map) {
            if (color_idx == idx) {
                return name;
            }
        }
        return "unknown";
    }

    ImGuiCol ThemeManager::GetColorIndexFromName(const std::string& name) const {
        std::string lower_name = name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        
        auto it = color_name_map.find(lower_name);
        if (it != color_name_map.end()) {
            return it->second;
        }
        
        return ImGuiCol_Text; // fall
    }

}
