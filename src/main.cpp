#include <iostream>

#include "core/engine/Engine.hpp"

int main()
{
    LogHelper::Init();

    if (!Engine::Init())
        LOGF(FATAL, "Engine failed to initialize, cannot continue execution");

    LogHelper::Destroy();
}