#include "Theme.hpp"
#include "core/logger/LogHelper.hpp"
#include <algorithm>
#include <utility>

namespace theme {

    void ThemeManager::Init() {
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

        // Load default theme
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            default_colors[i] = style.Colors[i];
        }

        LOGF(VERBOSE, "UI theme ready, loaded {} colors", color_name_map.size());
    }

    bool ThemeManager::SetColor(const std::string& name, float r, float g, float b, float a) {
        if (color_name_map.empty()) {
            LOGF(WARNING, "UI theme not initialized, cannot set color '{}'", name);
            return false;
        }

        std::string lower_name = name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

        auto it = color_name_map.find(lower_name);
        if (it == color_name_map.end()) {
            return false; // unknown name, let the caller warn
        }

        // Stage the change, FlushPending() applies it on the render thread.
        {
            std::lock_guard<std::mutex> lock(style_mutex);
            pending_colors[it->second] = ImVec4(
                std::clamp(r, 0.0f, 1.0f),
                std::clamp(g, 0.0f, 1.0f),
                std::clamp(b, 0.0f, 1.0f),
                std::clamp(a, 0.0f, 1.0f)
            );
        }

        return true;
    }

    void ThemeManager::RequestReset() {
        std::lock_guard<std::mutex> lock(style_mutex);
        pending_reset = true;
        pending_colors.clear();
    }

    void ThemeManager::FlushPending() {
        bool do_reset = false;
        std::unordered_map<int, ImVec4> colors;
        {
            std::lock_guard<std::mutex> lock(style_mutex);
            do_reset = pending_reset;
            pending_reset = false;
            colors = std::move(pending_colors);
            pending_colors.clear();
        }

        ImGuiStyle& style = ImGui::GetStyle();

        if (do_reset) {
            for (int i = 0; i < ImGuiCol_COUNT; i++) {
                style.Colors[i] = default_colors[i];
            }
        }

        if (!colors.empty()) {
            for (const auto& [idx, color] : colors) {
                style.Colors[idx] = color;
            }
        }
    }

}
