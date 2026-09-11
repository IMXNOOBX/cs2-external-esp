#pragma once

namespace scripting::input {
    bool InstallPhysicalKeyboardHook();
    void UninstallPhysicalKeyboardHook();
    bool IsPhysicalKeyDown(int virtual_key);
}
