#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include "imgui.h"

namespace theme {

    struct ThemeInfo {
        std::string name;
        std::string author;
        std::string description;
        ImVec4 colors[ImGuiCol_COUNT];
    };

    class ThemeManager {
    public:
        static ThemeManager& Get() {
            static ThemeManager instance;
            return instance;
        }

        void Init();
        void LoadThemesFromDirectory();
        bool LoadTheme(const std::string& filename);
        void ApplyTheme(const std::string& theme_name);
        void SaveCurrentTheme(const std::string& filename);
        void ExportDefaultTheme();
        
        std::vector<std::string> GetThemeNames() const;
        const ThemeInfo* GetThemeInfo(const std::string& name) const;
        std::string GetCurrentThemeName() const { return current_theme_name; }

    private:
        ThemeManager() = default;

        void CaptureCurrentColors();
        std::string GetColorName(ImGuiCol idx) const;
        ImGuiCol GetColorIndexFromName(const std::string& name) const;
        
        std::unordered_map<std::string, ThemeInfo> themes;
        std::string current_theme_name = "Default";
        
        // Map color names to ImGuiCol indices
        std::unordered_map<std::string, ImGuiCol> color_name_map;
    };

} // namespace theme
