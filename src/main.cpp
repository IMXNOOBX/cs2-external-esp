#include <iostream>

#include "core/engine/Engine.hpp"
#include "gui/renderer/Renderer.hpp"

#include <external/exception.hpp>

int main()
{
    c_exception_handler::setup();

    LogHelper::Init();

    LOGF(INFO, "Compiled {}, Welcome to cs2-external-esp-recode!", __TIMESTAMP__);

    if (!Engine::Init()) {
        LOGF(FATAL, "Engine failed to initialize, cannot continue execution");
        std::exit(1);
    }

    if (!Renderer::Init()) {
        LOGF(FATAL, "Renderer failed to initialize, cannot continue execution");
        std::exit(1);
    }

    // Locking
    Renderer::Thread();

    LogHelper::Destroy();
}