/*
    This project is licensed under CC BY-NC 4.0
    https://creativecommons.org/licenses/by-nc/4.0*

    You are free to:
    • Share: Copy and redistribute the material in any medium or format.
    • Adapt: Remix, transform, and build upon the material*

    Under the following terms:
    • Attribution: You must give appropriate credit, provide a link to the original source repository, and indicate if changes were made.
    • Non-Commercial: You may not use the material for commercial purposes*

    You are not allowed to:
    • Sell: This license forbids selling original or modified material for commercial purposes.
    • Sublicense: This license forbids sublicensing original or modified material.

    © Copyright by IMXNOOBX (https://github.com/IMXNOOBX) and contributors.
    See https://github.com/IMXNOOBX/cs2-external-esp/-/blob/main/LICENSE for full details.
*/

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

    // Needs to be ran as ADMINISTRATOR
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
        LOGF(WARNING, "Could not set application process priority to HIGH");

    if (!Updater::Init() || !Updater::Process()) {
        LOGF(FATAL, "Updater failed to run, the application has not verified its status, execution its not recommended");
        LOGF(INFO, "Click any key to continue...");
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