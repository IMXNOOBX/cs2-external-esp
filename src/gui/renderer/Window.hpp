#pragma once

#include <d3d11.h>
//#include <D3DX11tex.h>
#include "imgui/backends/imgui_impl_dx11.h"
#include "imgui/backends/imgui_impl_win32.h"

class Window {
public:
	static bool CreateDevice();
	static void DestroyDevice();

	static void SpawnWindow();
	static void DespawnWindow();

	static bool CreateImGui();
	static void DestroyImGui();

	static void StartRender();
	static void EndRender();

	static HWND window_;
	static HWND viewport_;
	static WNDCLASSEX wc;

	static bool IsWindowInForeground(HWND window) { return GetForegroundWindow() == window; }
	static bool BringToForeground(HWND window) { return SetForegroundWindow(window); }

	static void SetTopMost(HWND window, bool up_down = true);

	static void SetForeground(HWND window);

	static ID3D11Device* device;
	static ID3D11DeviceContext* device_context;
	static IDXGISwapChain* swap_chain;
	static ID3D11RenderTargetView* render_targetview;

	static HINSTANCE parent_instance;

	inline static bool RenderMenu;
	inline static bool shouldRun;
};

 inline Window window;