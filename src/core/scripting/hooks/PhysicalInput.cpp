#include "PhysicalInput.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <ranges>
#include <thread>
#include <Windows.h>

namespace {
    constexpr std::array<int, 5> kMouseButtons {
        VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2
    };

    std::array<std::atomic_bool, 256> g_physical_key_states{};
    std::array<std::atomic_bool, kMouseButtons.size()> g_physical_mouse_states{};

    std::jthread g_physical_input_thread;
    std::atomic<DWORD> g_physical_input_thread_id{ 0 };

    std::mutex g_init_mutex;
    std::condition_variable g_init_cv;
    bool g_init_finished = false;
    std::atomic_bool g_hooks_installed{ false };

    constexpr int GetMouseButtonIndex(int virtual_key) {
        const auto it = std::ranges::find(kMouseButtons, virtual_key);

        return (it != kMouseButtons.end())
            ? static_cast<int>(std::distance(kMouseButtons.begin(), it))
            : -1;
    }

    int GetMouseButtonVirtualKey(WPARAM message, const MSLLHOOKSTRUCT& mouse) {
        switch (message) {
        case WM_LBUTTONDOWN: case WM_LBUTTONUP: return VK_LBUTTON;
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: return VK_RBUTTON;
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: return VK_MBUTTON;
        case WM_XBUTTONDOWN: case WM_XBUTTONUP:
            return (HIWORD(mouse.mouseData) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
        default:
            return 0;
        }
    }

    constexpr bool IsMouseButtonDownMessage(WPARAM message) {
        return message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
            message == WM_MBUTTONDOWN || message == WM_XBUTTONDOWN;
    }

    LRESULT CALLBACK PhysicalKeyboardHook(int code, WPARAM message, LPARAM data) {
        if (code == HC_ACTION) {
            const auto* key = reinterpret_cast<const KBDLLHOOKSTRUCT*>(data);

            // Ignore emulted input events, our own basically
            if ((key->flags & LLKHF_INJECTED) == 0 && key->vkCode < g_physical_key_states.size()) {
                if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
                    g_physical_key_states[key->vkCode].store(true, std::memory_order_relaxed);
                }
                else if (message == WM_KEYUP || message == WM_SYSKEYUP) {
                    g_physical_key_states[key->vkCode].store(false, std::memory_order_relaxed);
                }
            }
        }

        return CallNextHookEx(nullptr, code, message, data);
    }

    LRESULT CALLBACK PhysicalMouseHook(int code, WPARAM message, LPARAM data) {
        if (code == HC_ACTION) {
            const auto* mouse = reinterpret_cast<const MSLLHOOKSTRUCT*>(data);

            // Same as with the keyboard, ignore our own injected events
            if ((mouse->flags & LLMHF_INJECTED) == 0) {
                const int button_index = GetMouseButtonIndex(GetMouseButtonVirtualKey(message, *mouse));
                if (button_index >= 0) {
                    g_physical_mouse_states[button_index].store(
                        IsMouseButtonDownMessage(message), std::memory_order_relaxed
                    );
                }
            }
        }

        return CallNextHookEx(nullptr, code, message, data);
    }

    void RunPhysicalInputHooks() {
        MSG message{};
        PeekMessageW(&message, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
        g_physical_input_thread_id.store(GetCurrentThreadId(), std::memory_order_relaxed);

        const HHOOK kb_hook = SetWindowsHookExW(
            WH_KEYBOARD_LL, PhysicalKeyboardHook, GetModuleHandleW(nullptr), 0);
        const HHOOK mouse_hook = SetWindowsHookExW(
            WH_MOUSE_LL, PhysicalMouseHook, GetModuleHandleW(nullptr), 0);

        {
            std::lock_guard lock(g_init_mutex);
            g_hooks_installed.store(kb_hook != nullptr && mouse_hook != nullptr, std::memory_order_relaxed);
            g_init_finished = true;
        }

        g_init_cv.notify_one();

        if (g_hooks_installed.load(std::memory_order_relaxed)) {
            while (GetMessageW(&message, nullptr, 0, 0) > 0) {
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
        }

        if (kb_hook != nullptr) UnhookWindowsHookEx(kb_hook);
        if (mouse_hook != nullptr) UnhookWindowsHookEx(mouse_hook);

        g_physical_input_thread_id.store(0, std::memory_order_relaxed);
    }

    void SendMouseButtonInput(int virtual_key, bool pressed) {
        INPUT input{};
        input.type = INPUT_MOUSE;

        switch (virtual_key) {
        case VK_LBUTTON:
            input.mi.dwFlags = pressed ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
            break;
        case VK_RBUTTON:
            input.mi.dwFlags = pressed ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
            break;
        case VK_MBUTTON:
            input.mi.dwFlags = pressed ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
            break;
        case VK_XBUTTON1:
        case VK_XBUTTON2:
            input.mi.dwFlags = pressed ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
            input.mi.mouseData = (virtual_key == VK_XBUTTON1) ? XBUTTON1 : XBUTTON2;
            break;
        default:
            return;
        }

        SendInput(1, &input, sizeof(INPUT));
    }

    void SendKeyInput(int virtual_key, bool pressed) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(virtual_key);
        input.ki.dwFlags = pressed ? 0 : KEYEVENTF_KEYUP;

        SendInput(1, &input, sizeof(INPUT));
    }
}

namespace scripting::input {

    bool InstallPhysicalInputHooks() {
        if (g_physical_input_thread.joinable())
            return g_hooks_installed.load(std::memory_order_relaxed);


        {
            std::lock_guard lock(g_init_mutex);
            g_init_finished = false;
            g_hooks_installed.store(false, std::memory_order_relaxed);
        }

        g_physical_input_thread = std::jthread(RunPhysicalInputHooks);

        std::unique_lock lock(g_init_mutex);
        g_init_cv.wait(lock, [] { return g_init_finished; });
        return g_hooks_installed.load(std::memory_order_relaxed);
    }

    void UninstallPhysicalInputHooks() {
        const DWORD thread_id = g_physical_input_thread_id.load(std::memory_order_relaxed);
        if (thread_id != 0) 
            PostThreadMessageW(thread_id, WM_QUIT, 0, 0);

        if (g_physical_input_thread.joinable()) 
            g_physical_input_thread.join();
    }

    bool IsPhysicalInputDown(int virtual_key) {
        const int mouse_button_index = GetMouseButtonIndex(virtual_key);

        if (mouse_button_index >= 0)
            return g_physical_mouse_states[mouse_button_index].load(std::memory_order_relaxed);


        if (virtual_key >= 0 && virtual_key < static_cast<int>(g_physical_key_states.size()))
            return g_physical_key_states[virtual_key].load(std::memory_order_relaxed);

        return false;
    }

    void SendVirtualInput(int virtual_key, bool pressed) {
        if (GetMouseButtonIndex(virtual_key) >= 0)
            return SendMouseButtonInput(virtual_key, pressed);

         SendKeyInput(virtual_key, pressed);
    }

}