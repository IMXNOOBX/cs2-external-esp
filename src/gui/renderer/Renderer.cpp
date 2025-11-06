#include "Renderer.hpp"
#include "window/Window.hpp"

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

    if (!Window::CreateDevice()) {
        LOGF(FATAL, "Failed to create device");
        return false;
    }

    if (!Window::CreateImGui()) {
        LOGF(FATAL, "Failed to create ImGui");
        return false;
    }

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
        Window::StartRender();

        Menu::Render();

        Window::EndRender();
    }

    // Once exited, destroy everything
    Window::DestroyImGui();
    Window::DestroyDevice();
    Window::DespawnWindow();
}