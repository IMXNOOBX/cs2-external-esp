#include <iostream>

#include "updater/Updater.hpp"
#include "core/engine/Engine.hpp"
#include "gui/renderer/Renderer.hpp"

#include <external/exception.hpp>

int main()
{
    c_exception_handler::setup();

    LogHelper::Init();

    LOGF(INFO, "Compiled {}, Welcome to cs2-external-esp-recode!", __TIMESTAMP__);

    if (!Updater::Init() || !Updater::Process()) {
        LOGF(FATAL, "Updater failed to run, the application has not verified its status, execution its not recommended");
        LOGF(INFO, "Click any key to continue... [NOT RECOMMENDED]");
        std::cin.get();
        //goto exit;
    }

    if (!Engine::Init()) {
        LOGF(FATAL, "Engine failed to initialize, cannot continue execution");
        goto exit;
    }


    if (!Renderer::Init()) {
        LOGF(FATAL, "Renderer failed to initialize, cannot continue execution");
        goto exit;
    }

    // Locking
    Renderer::Thread();

exit:
    LOGF(INFO, "Thats it, im done, hope you had a great time!");
    LogHelper::Destroy();
    std::cin.get();
}