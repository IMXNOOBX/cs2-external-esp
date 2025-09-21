#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <d3d11.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"

#include "gui.hpp"
#include "classes/config.hpp"
#include "hacks/reader.hpp"

#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Global GUI state variables
bool gui_finish = false;
bool show_gui = false;
bool gui_has_focus = false;
static bool gui_initialized = false;

void CleanupImGui()
{
    if (gui_initialized)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        gui_initialized = false;
        std::cout << "[gui] ImGui cleaned up" << std::endl;
    }
}

bool InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
    if (gui_initialized)
        return true;

    if (!hwnd || !device || !deviceContext) {
        std::cout << "[gui] Invalid parameters for ImGui initialization" << std::endl;
        return false;
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowPadding = ImVec2(15, 15);
    style.Alpha = 0.95f;

    // Setup renderer backends
    if (!ImGui_ImplWin32_Init(hwnd)) {
        std::cout << "[gui] Failed to initialize ImGui Win32" << std::endl;
        return false;
    }

    if (!ImGui_ImplDX11_Init(device, deviceContext)) {
        std::cout << "[gui] Failed to initialize ImGui DX11" << std::endl;
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    gui_initialized = true;
    std::cout << "[gui] ImGui initialized successfully" << std::endl;
    return true;
}

bool IsImGuiInitialized()
{
    return gui_initialized;
}

void OnWindowResize()
{
    if (gui_initialized) {

    }
}

bool ColorEdit3RGB(const char* label, RGB& color)
{
    float col[3] = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f
    };

    bool changed = ImGui::ColorEdit3(label, col);

    if (changed) {
        color.r = static_cast<int>(col[0] * 255.0f);
        color.g = static_cast<int>(col[1] * 255.0f);
        color.b = static_cast<int>(col[2] * 255.0f);
    }

    return changed;
}

void RenderImGui()
{
    if (!show_gui || !gui_initialized)
        return;

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Main configuration window
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("CS2 External ESP Configuration", &show_gui, window_flags))
    {
        // Header
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 200, 255, 255));
        ImGui::Text("CS2 External ESP Configuration");
        ImGui::PopStyleColor();

        ImGui::Separator();
        ImGui::Spacing();

        // Focus status
        ImGui::Text("Status: %s", gui_has_focus ? "GUI Active" : "ESP Active");
        ImGui::Text("Press INSERT to toggle focus");
        ImGui::Separator();
        ImGui::Spacing();

        // ESP Settings
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 100, 255));
        ImGui::Text("ESP Settings");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // ESP Toggles with colored text based on state
        ImGui::PushStyleColor(ImGuiCol_Text, config::show_box_esp ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
        if (ImGui::Checkbox("Box ESP [F4]", &config::show_box_esp))
            config::save();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, config::show_skeleton_esp ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
        if (ImGui::Checkbox("Skeleton ESP [F8]", &config::show_skeleton_esp))
            config::save();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, config::show_head_tracker ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
        if (ImGui::Checkbox("Head Tracker [F9]", &config::show_head_tracker))
            config::save();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, config::team_esp ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
        if (ImGui::Checkbox("Team ESP [F5]", &config::team_esp))
            config::save();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Advanced settings
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 100, 255));
        if (ImGui::CollapsingHeader("Advanced Settings"))
        {
            ImGui::PopStyleColor();
            ImGui::Spacing();

#ifndef _UC
            ImGui::PushStyleColor(ImGuiCol_Text, config::automatic_update ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
            if (ImGui::Checkbox("Automatic Updates [F6]", &config::automatic_update))
                config::save();
            ImGui::PopStyleColor();
#endif

            ImGui::PushStyleColor(ImGuiCol_Text, config::show_health_bars ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
            if (ImGui::Checkbox("Health Bars", &config::show_health_bars))
                config::save();
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, config::show_health_flags ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
            if (ImGui::Checkbox("Health Flags", &config::show_health_flags))
                config::save();
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, config::show_money_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
            if (ImGui::Checkbox("Money Flag", &config::show_money_flag))
                config::save();
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, config::show_distance_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
            if (ImGui::Checkbox("Distance Flag", &config::show_distance_flag))
                config::save();
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, config::show_name_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
            if (ImGui::Checkbox("Name Flag", &config::show_name_flag))
                config::save();
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, config::show_weapon_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
            if (ImGui::Checkbox("Weapon Flag", &config::show_weapon_flag))
                config::save();
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, config::show_defusing_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
            if (ImGui::Checkbox("Defusing Flag", &config::show_defusing_flag))
                config::save();
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, config::show_flashed_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 255, 255, 255));
            if (ImGui::Checkbox("Flashed Flag", &config::show_flashed_flag))
                config::save();
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // COLOR SELECTORS
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 100, 255));
            ImGui::Text("Colors");
            ImGui::PopStyleColor();

            if (ColorEdit3RGB("Box (Team)", config::esp_box_color_team))
                config::save();

            if (ColorEdit3RGB("Box (Enemy)", config::esp_box_color_enemy))
                config::save();

            if (ColorEdit3RGB("Skeleton (Team)", config::esp_skeleton_color_team))
                config::save();

            if (ColorEdit3RGB("Skeleton (Enemy)", config::esp_skeleton_color_enemy))
                config::save();

            if (ColorEdit3RGB("Name", config::esp_name_color))
                config::save();

            if (ColorEdit3RGB("Distance", config::esp_distance_color))
                config::save();
        }
        else
        {
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Control buttons
        if (ImGui::Button("Save Config", ImVec2(120, 30)))
        {
            config::save();
        }

        ImGui::SameLine();
        if (ImGui::Button("Hide GUI [INSERT]", ImVec2(140, 30)))
        {
            show_gui = false;
            gui_has_focus = false;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Keybind reference
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 100, 255));
        ImGui::Text("Keybinds:");
        ImGui::PopStyleColor();

        ImGui::BulletText("INSERT - Toggle GUI");
        ImGui::BulletText("F4 - Toggle Box ESP");
        ImGui::BulletText("F5 - Toggle Team ESP");
#ifndef _UC
        ImGui::BulletText("F6 - Toggle Auto Updates");
#endif
        ImGui::BulletText("F7 - Panic Mode");
        ImGui::BulletText("F8 - Toggle Skeleton ESP");
        ImGui::BulletText("F9 - Toggle Head Tracker");
        ImGui::BulletText("END - Exit ESP");
    }
    ImGui::End();

    // Render everything
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}