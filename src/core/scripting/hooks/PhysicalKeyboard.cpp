#include "PhysicalKeyboard.hpp"

#include <array>
#include <atomic>
#include <Windows.h>

namespace {
    HHOOK physical_keyboard_hook = nullptr;
    std::array<std::atomic_bool, 256> physical_key_states{};

    LRESULT CALLBACK PhysicalKeyboardHook(int code, WPARAM message, LPARAM data) {
        if (code == HC_ACTION) {
            const auto* key = reinterpret_cast<const KBDLLHOOKSTRUCT*>(data);
            if ((key->flags & LLKHF_INJECTED) == 0 && key->vkCode < physical_key_states.size()) {
                if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
                    physical_key_states[key->vkCode].store(true);
                } else if (message == WM_KEYUP || message == WM_SYSKEYUP) {
                    physical_key_states[key->vkCode].store(false);
                }
            }
        }

        return CallNextHookEx(physical_keyboard_hook, code, message, data);
    }
}

namespace scripting::input {
    bool InstallPhysicalKeyboardHook() {
        physical_keyboard_hook = SetWindowsHookExW(
            WH_KEYBOARD_LL,
            PhysicalKeyboardHook,
            GetModuleHandleW(nullptr),
            0
        );

        return physical_keyboard_hook != nullptr;
    }

    void UninstallPhysicalKeyboardHook() {
        if (physical_keyboard_hook == nullptr)
            return;

        UnhookWindowsHookEx(physical_keyboard_hook);
        physical_keyboard_hook = nullptr;
    }

    bool IsPhysicalKeyDown(int virtual_key) {
        return virtual_key >= 0 
                && virtual_key <static_cast<int>(physical_key_states.size()) 
                && physical_key_states[virtual_key].load();
    }
}
