#include "Renderer.hpp"
#include "window/Window.hpp"

#include "core/engine/Engine.hpp"
#include "gui/frontend/esp/Esp.hpp"
#include "gui/frontend/menu/Menu.hpp"
#include "gui/frontend/watermark/Watermark.hpp"

bool Renderer::Init() {
    return GetInstance().InitImpl();
}

void Renderer::Thread() {
    return GetInstance().ThreadImpl();
}

void Renderer::Destroy() {
    return GetInstance().DestroyImpl();
}   

bool Renderer::IsOpen() {
    return GetInstance().isOpen;
}

bool Renderer::InitImpl() {
    if (!Window::SpawnWindow()) {
        LOGF(FATAL, "Failed to create window");
        return false;
    }

    if (!Window::CreateDevice()) {
        LOGF(FATAL, "Failed to create device");
        return false;
    }

    if (!Window::CreateImGui()) {
        LOGF(FATAL, "Failed to create ImGui");
        return false;
    }

    Menu::Init();
    Esp::Init();
    Watermark::Init();

    // Focus the game
    SetForegroundWindow(Engine::GetProcess()->hwnd_);

    if (cfg::settings::streamproof)
        Window::SetAffinity(Window::hwnd, WindowAffinity::Invisible);

    if (!cfg::dev::console)
        LogHelper::Free();

    // We want the main thread to call render
    // std::thread(Thread).detach();

    LOGF(INFO, "Successfully initialized renderer...");
    return true;
}

void Renderer::DestroyImpl() {
    isRunning = false; // Prepare to stop thread loop
    LOGF(VERBOSE, "Successfully programed renderer destruction...");
}

void Renderer::ThreadImpl() {
    while (isRunning) {
        Render();

        // If the game is not focused dont do states, 
        // or will start focusing game & overlay
        if (this->isFocused && HandleState())
            continue; // It will cause flickering if we handle window order after window closes

        HandleWindowOrder();
    }

    // Once exited, destroy everything
    Window::DestroyImGui();
    Window::DestroyDevice();
    Window::DespawnWindow();
}

void Renderer::Render() {
    Window::StartRender();

    Esp::Render();
    Watermark::Render();

    Menu::RenderStartupHelp();
    if (isOpen) Menu::Render();

    Window::EndRender();
}

bool Renderer::HandleState() {
    isRunning = Window::shouldRun; // From the window event handler

    static bool was_holding = false;

    bool pressed_insert = (GetAsyncKeyState(VK_INSERT) & 0x8000);
    bool pressed_rshift = (GetAsyncKeyState(VK_RSHIFT) & 0x8000);

    bool should_toggle = !was_holding && (pressed_insert || pressed_rshift);

    if (should_toggle) {
        this->isOpen = !isOpen;

        // Release cursor when opening the menu
        // Sometimes flashes the render as its handling the window order
        if (this->isOpen)
            SetForegroundWindow(Window::hwnd);
        else
            SetForegroundWindow(Engine::GetProcess()->hwnd_);

        Window::SetClickthrough(Window::hwnd, !this->isOpen);
        LOGF(VERBOSE, "Captured global VK_INSERT or VK_RSHIFT, toggling menu state to {}", this->isOpen);

        // Not the best way, but wont bother the user
        std::thread(Config::Write).detach(); // Not needed, but just in case
    }

    was_holding = pressed_insert || pressed_rshift;
    return should_toggle;
}

bool Renderer::HandleWindowOrder() {
    auto p = Engine::GetProcess();

    if (!p || (!p->hwnd_ && !p->UpdateHWND()))
        return false;

    // Check if game window is still valid, if not, most likely game closed
    if (!IsWindow(p->hwnd_))
        this->isRunning = false;

    static bool overlay_visible = true;
    auto foreground = GetForegroundWindow();
    this->isFocused = (foreground == Window::hwnd || foreground == p->hwnd_);

    if (!this->isFocused && overlay_visible) {
        ShowWindow(Window::hwnd, SW_HIDE);
        overlay_visible = false;
        return true;
    }

    if (!overlay_visible && this->isFocused) {  
        ShowWindow(Window::hwnd, SW_SHOW);
        overlay_visible = true;
        return true;
    }

    static RECT last_rect = { 0, 0, 0, 0 };

    RECT window_rect;
    if (!GetWindowRect(p->hwnd_, &window_rect))
        return false;

    // All good, no movements from the client
    if (memcmp(&window_rect, &last_rect, sizeof(RECT)) == 0)
        return true;

    RECT client_rect;
    if (!GetClientRect(p->hwnd_, &client_rect))
        return false;

    POINT top_left = { client_rect.left, client_rect.top };
    POINT bottom_right = { client_rect.right, client_rect.bottom };

    ClientToScreen(p->hwnd_, &top_left);
    ClientToScreen(p->hwnd_, &bottom_right);

    RECT screen_rect = { top_left.x, top_left.y, bottom_right.x, bottom_right.y };

    SetWindowPos(
        Window::hwnd,
        HWND_TOPMOST,
        screen_rect.left,
        screen_rect.top,
        screen_rect.right - screen_rect.left,
        screen_rect.bottom - screen_rect.top,
        SWP_NOACTIVATE | SWP_SHOWWINDOW
    );

    last_rect = window_rect;

    return true;
}