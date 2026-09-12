#pragma once

namespace scripting::input {
    bool InstallPhysicalInputHooks();
    void UninstallPhysicalInputHooks();
    bool IsPhysicalInputDown(int virtual_key) noexcept;
    void SendVirtualInput(int virtual_key, bool pressed) noexcept;
}
