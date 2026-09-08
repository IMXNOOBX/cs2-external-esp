#pragma once

#include <string>
#include <array>
#include <unordered_map>
#include <mutex>
#include "imgui.h"

namespace theme {
    class ThemeManager {
    public:
        static ThemeManager& Get() {
            static ThemeManager instance;
            return instance;
        }

        void Init();

        bool SetColor(const std::string& name, float r, float g, float b, float a);

        void RequestReset();

        void FlushPending();

    private:
        ThemeManager() = default;

        std::unordered_map<std::string, ImGuiCol> color_name_map;
        std::array<ImVec4, ImGuiCol_COUNT> default_colors{};

        std::mutex style_mutex;
        std::unordered_map<int, ImVec4> pending_colors;
        bool pending_reset = false;
    };

}
