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
static HWND gui_hwnd = nullptr;

// DirectX 11 variables for ImGui
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

void CleanupImGui()
{
    if (gui_initialized)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        gui_initialized = false;
    }

    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }

    if (gui_hwnd)
    {
        DestroyWindow(gui_hwnd);
        gui_hwnd = nullptr;
    }
}

// GUI Window processor
LRESULT CALLBACK GuiWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            ID3D11Texture2D* pBackBuffer;
            g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
            pBackBuffer->Release();
        }
        return 0;
    case WM_SETFOCUS:
        gui_has_focus = true;
        return 0;
    case WM_KILLFOCUS:
        gui_has_focus = false;
        return 0;
    case WM_CLOSE:
        show_gui = false;
        ShowWindow(hWnd, SW_HIDE);
        SetForegroundWindow(g_game.process->hwnd_);
        return 0;
    case WM_DESTROY:
        return 0;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

bool CreateGuiWindow()
{
    if (gui_hwnd)
        return true;

    // Register GUI window class
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA) };
    wc.lpfnWndProc = GuiWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "CS2ConfigGUI";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_CLASSDC;
    RegisterClassExA(&wc);

    // Create GUI window
    gui_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "CS2ConfigGUI",
        "CS2 ESP Config",
        WS_POPUP, // Borderless
        200, 200, 450, 500,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (!gui_hwnd)
        return false;

    // Create DirectX device
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 450;
    sd.BufferDesc.Height = 500;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = gui_hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
        &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext)))
    {
        std::cout << "[gui] Failed to create DirectX device for GUI" << std::endl;
        DestroyWindow(gui_hwnd);
        gui_hwnd = nullptr;
        return false;
    }

    // Create render target
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();

    return true;
}

bool InitImGui()
{
    if (gui_initialized)
        return true;

    if (!CreateGuiWindow())
        return false;

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.WindowPadding = ImVec2(15, 15);

    // Setup Renderer backends
    ImGui_ImplWin32_Init(gui_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    gui_initialized = true;
    return true;
}

void RenderImGui()
{
    if (!show_gui || !gui_initialized || !g_pd3dDevice)
        return;

    // Start the ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Full window ImGui
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("CS2 ESP Configuration", nullptr, window_flags);

    // Header
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 200, 255, 255));
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("CS2 External ESP Configuration").x) * 0.5f);
    ImGui::Text("CS2 External ESP Configuration");
    ImGui::PopStyleColor();

    ImGui::Separator();
    ImGui::Spacing();

    // Focus status
    ImGui::Text("Focus: %s", gui_has_focus ? "GUI (Press INSERT to focus CS2)" : "CS2 (Press INSERT to focus GUI)");
    ImGui::Separator();
    ImGui::Spacing();

    // ESP Settings
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 100, 255));
    ImGui::Text("ESP Settings");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // ESP Toggles
    ImGui::PushStyleColor(ImGuiCol_Text, config::show_box_esp ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
    if (ImGui::Checkbox("Box ESP [F4]", &config::show_box_esp))
        config::save();
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, config::show_skeleton_esp ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
    if (ImGui::Checkbox("Skeleton ESP [F8]", &config::show_skeleton_esp))
        config::save();
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, config::show_head_tracker ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
    if (ImGui::Checkbox("Head Tracker [F9]", &config::show_head_tracker))
        config::save();
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, config::team_esp ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
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
        ImGui::PushStyleColor(ImGuiCol_Text, config::automatic_update ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
        if (ImGui::Checkbox("Automatic Updates [F6]", &config::automatic_update))
            config::save();
        ImGui::PopStyleColor();
#endif

        ImGui::PushStyleColor(ImGuiCol_Text, config::show_health_bars ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
        if (ImGui::Checkbox("Health Bars", &config::show_health_bars))
            config::save();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, config::show_health_flags ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
        if (ImGui::Checkbox("Health Flags", &config::show_health_flags))
            config::save();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, config::show_money_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
        if (ImGui::Checkbox("Money Flag", &config::show_money_flag))
            config::save();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, config::show_distance_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
        if (ImGui::Checkbox("Distance Flag", &config::show_distance_flag))
            config::save();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, config::show_name_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
        if (ImGui::Checkbox("Name Flag", &config::show_name_flag))
            config::save();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, config::show_weapon_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
        if (ImGui::Checkbox("Weapon Flag", &config::show_weapon_flag))
            config::save();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, config::show_defusing_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
        if (ImGui::Checkbox("Defusing Flag", &config::show_defusing_flag))
            config::save();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, config::show_flashed_flag ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 100, 100, 255));
        if (ImGui::Checkbox("Flashed Flag", &config::show_flashed_flag))
            config::save();
        ImGui::PopStyleColor();

        // COLOR SELECTORS
        ImGui::Text("Colors");
        if (ImGui::ColorEdit3("Box (Team)", (float*)&config::esp_box_color_team))
            config::save();

        if (ImGui::ColorEdit3("Box (Enemy)", (float*)&config::esp_box_color_enemy))
            config::save();

        if (ImGui::ColorEdit3("Skeleton (Team)", (float*)&config::esp_skeleton_color_team))
            config::save();

        if (ImGui::ColorEdit3("Skeleton (Enemy)", (float*)&config::esp_skeleton_color_enemy))
            config::save();

        if (ImGui::ColorEdit3("Name", (float*)&config::esp_name_color))
            config::save();

        if (ImGui::ColorEdit3("Distance", (float*)&config::esp_distance_color))
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
    if (ImGui::Button("Save Config", ImVec2(150, 35)))
    {
        config::save();
        ImGui::OpenPopup("Config Saved");
    }

    ImGui::SameLine();
    if (ImGui::Button("Focus CS2 [INSERT]", ImVec2(150, 35)))
    {
        SetForegroundWindow(g_game.process->hwnd_);
        gui_has_focus = false;
    }

    ImGui::SameLine();
    if (ImGui::Button("Close GUI", ImVec2(150, 35)))
    {
        show_gui = false;
    }

    // Save popup
    if (ImGui::BeginPopupModal("Config Saved", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Configuration saved successfully!");
        if (ImGui::Button("OK", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Status and keybinds
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, 255));
    ImGui::Text("Status: %s", gui_finish ? "Stopping..." : "Running");
    ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 100, 255));
    ImGui::Text("Keybinds:");
    ImGui::PopStyleColor();

    ImGui::BulletText("INSERT - Toggle Focus (CS2 <-> GUI)");
    ImGui::BulletText("F4 - Toggle Box ESP");
    ImGui::BulletText("F5 - Toggle Team ESP");
#ifndef _UC
    ImGui::BulletText("F6 - Toggle Auto Updates");
#endif
    ImGui::BulletText("F7 - Toggle Extra Flags");
    ImGui::BulletText("F8 - Toggle Skeleton ESP");
    ImGui::BulletText("F9 - Toggle Head Tracker");
    ImGui::BulletText("END - Exit ESP");

    ImGui::End();

    // Render
    ImGui::Render();
    const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 0.95f };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0);
}

void GuiThread()
{
    while (!gui_finish)
    {
        if (show_gui)
        {
            if (!gui_initialized)
            {
                if (!InitImGui())
                {
                    std::cout << "[gui] Failed to initialize ImGui" << std::endl;
                    show_gui = false;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }
                else
                {
                    std::cout << "[gui] ImGui initialized successfully" << std::endl;
                }
            }

            ShowWindow(gui_hwnd, SW_SHOW);
            UpdateWindow(gui_hwnd);

            // Process GUI messages
            MSG msg;
            while (PeekMessage(&msg, gui_hwnd, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            RenderImGui();
        }
        else
        {
            if (gui_hwnd)
                ShowWindow(gui_hwnd, SW_HIDE);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    CleanupImGui();
}