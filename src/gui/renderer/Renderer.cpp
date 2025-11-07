#include "Renderer.hpp"
#include "window/Window.hpp"

#include "core/engine/Engine.hpp"
#include "gui/frontend/esp/Esp.hpp"
#include "gui/frontend/menu/Menu.hpp"

bool Renderer::Init() {
    return GetInstance().InitImpl();
}

void Renderer::Thread() {
    return GetInstance().ThreadImpl();
}

void Renderer::Destroy() {
    return GetInstance().DestroyImpl();
}

bool Renderer::InitImpl() {
    Window::SpawnWindow();
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

    //if (!HandleWindowOrder()) {
    //    LOGF(WARNING, "Failed to set window order, overlay will not control any states");
    //}

    Menu::Init();

    //std::thread(Thread).detach();

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

        HandleState();
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
    
    if (isOpen) Menu::Render();

    Window::EndRender();
}

void Renderer::HandleState() {
    isRunning = Window::shouldRun; // From the window event handler

    static bool was_holding = false;

    bool pressed_insert = (GetAsyncKeyState(VK_INSERT) & 0x8000);
    bool pressed_rshift = (GetAsyncKeyState(VK_RSHIFT) & 0x8000);

    if (
        !was_holding
        && (pressed_insert || pressed_rshift)
        ) {
        this->isOpen = !isOpen;

        // Release cursor when opening the menu
        if (this->isOpen)
            SetForegroundWindow(Window::hwnd);
        else
            SetForegroundWindow(Engine::GetProcess()->hwnd_);

        Window::SetClickthrough(Window::hwnd, !this->isOpen);
        LOGF(VERBOSE, "Captured global VK_INSERT or VK_RSHIFT, toggling menu state to {}", this->isOpen);
    }

    was_holding = pressed_insert || pressed_rshift;
}

bool Renderer::HandleWindowOrder() {
    auto p = Engine::GetProcess();

    if (!p || (!p->hwnd_ && !p->UpdateHWND()))
        return false;

    // Makes our overlay dissapepar as it clears the transparency or something :1
    //SetParent(Window::hwnd, p->hwnd_);
    //LOGF(WARNING, "Setting {} as our parent window ({}), to follow their movements", (int)p->hwnd_, (int)Window::hwnd);
    
    static bool overlay_visible = true;
    bool game_focused = (GetForegroundWindow() == Window::hwnd || GetForegroundWindow() == p->hwnd_);

    if (!game_focused && overlay_visible) {
        ShowWindow(Window::hwnd, SW_HIDE);
        overlay_visible = false;
    }
    else if (!overlay_visible && game_focused) {
        ShowWindow(Window::hwnd, SW_SHOW);
        overlay_visible = true;
    }

    static RECT last_rect = { 0, 0, 0, 0 };

    RECT game_rect;
    if (!GetWindowRect(p->hwnd_, &game_rect))
        return false;

    // Only update if it actually changed
    if (memcmp(&game_rect, &last_rect, sizeof(RECT)) != 0) {
        SetWindowPos(
            Window::hwnd,
            HWND_TOPMOST,
            game_rect.left,
            game_rect.top,
            game_rect.right - game_rect.left,
            game_rect.bottom - game_rect.top,
            SWP_NOACTIVATE | SWP_SHOWWINDOW
        );

        last_rect = game_rect;
    }

    return true;
}