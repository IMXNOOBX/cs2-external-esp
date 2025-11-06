#include "Window.hpp"

ID3D11Device* Window::device = nullptr;
ID3D11DeviceContext* Window::device_context = nullptr;
IDXGISwapChain* Window::swap_chain = nullptr;
ID3D11RenderTargetView* Window::render_targetview = nullptr;

HWND Window::hwnd = nullptr;
HWND Window::viewport = nullptr;
WNDCLASSEX Window::wc = { };

extern LRESULT CALLBACK window_procedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

bool Window::CreateDevice()
{
	// First we setup our swap chain, this basically just holds a bunch of descriptors for the swap chain.
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

	// create device and swap chain
	HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		featureLevelArray,
		2,
		D3D11_SDK_VERSION,
		&sd,
		&swap_chain,
		&device,
		&featureLevel,
		&device_context);

	// if the hardware isn't supported create with WARP (basically just a different renderer)
	if (result == DXGI_ERROR_UNSUPPORTED) {
		result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_WARP,
			nullptr,
			0U,
			featureLevelArray,
			2, D3D11_SDK_VERSION,
			&sd,
			&swap_chain,
			&device,
			&featureLevel,
			&device_context);

		LOGF(FATAL, "DXGI_ERROR | Created with D3D_DRIVER_TYPE_WARP");
	}

	if (result != S_OK) {
		LOGF(FATAL, "Device Not Okay");
		return false;
	}

	ID3D11Texture2D* back_buffer{ nullptr };
	swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

	if (back_buffer)
	{
		device->CreateRenderTargetView(back_buffer, nullptr, &render_targetview);
		back_buffer->Release();

		LOGF(VERBOSE, "Created Device");
		return true;
	}

	LOGF(FATAL, "Failed to Create Device");
	return false;
}

void Window::DestroyDevice()
{
	if (device)
	{
		device->Release();
		device_context->Release();
		swap_chain->Release();
		render_targetview->Release();

		device = nullptr; // If not might cause a crash due to dangling pointers

		LOGF(VERBOSE, "Released Device");
	}
	else
		LOGF(WARNING, "Device Not Found to destroy.");
}

void Window::SpawnWindow()
{
	ImGui_ImplWin32_EnableDpiAwareness();
	wc.cbSize = sizeof(wc);
	wc.style = CS_CLASSDC;
	wc.hInstance = GetModuleHandleA(0);
	wc.lpszClassName = "waaaaaaaaaaaaaa";
	wc.lpfnWndProc = window_procedure;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	//wc.hIcon = 0;
	//wc.hIconSm = 0;
	//wc.hCursor = 0;
	//wc.hbrBackground = 0;
	//wc.lpszMenuName = 0;

	// register our class
	RegisterClassEx(&wc);

	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);

	hwnd = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		"waaaaaaaaaaaaaa",
		"waaaaaaaaaaaaaa",
		WS_POPUP,
		0, 0, screen_width, screen_height,
		NULL,
		NULL,
		wc.hInstance,
		NULL
	);

	if (hwnd == NULL) {
		LOGF(FATAL, "Failed to create Window");
		return;
	}

	
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
	//SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, parent_instance);

	//MARGINS margins = { -1 };
	//DwmExtendFrameIntoClientArea(hwnd, &margins);

	//SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);

	// show + update window
	ShowWindow(hwnd, TRUE);
	UpdateWindow(hwnd);
	SetForeground(hwnd);

	LOGF(VERBOSE, "Window Created with HWND: " + std::to_string((long)hwnd));
}

/**
* @brief Destroys the window and unregisters the class.
* @note This function will cause the program to exit.
*/
void Window::DespawnWindow()
{
	DestroyWindow(hwnd);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
	LOGF(VERBOSE, "Window Despawned");
}

bool Window::CreateImGui()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Set the ImGui IO to the Win32 window
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	
	io.IniFilename = nullptr; // Disable saving to .ini file

	io.ConfigViewportsNoTaskBarIcon = true; // Disable showing in taskbar completely
	io.ConfigViewportsNoAutoMerge = true;

	// Initialize ImGui for the Win32 library
	if (!ImGui_ImplWin32_Init(hwnd)) {
		LOGF(FATAL, "Failed ImGui_ImplWin32_Init");
		return false;
	}

	// Initialize ImGui for DirectX 11.
	if (!ImGui_ImplDX11_Init(device, device_context)) {
		LOGF(FATAL, "Failed ImGui_ImplDX11_Init");
		return false;
	}

	LOGF(VERBOSE, "ImGui Initialized");
	return true;
}

void Window::DestroyImGui()
{
	// Cleanup ImGui by shutting down DirectX11, the Win32 Platform and Destroying the ImGui context.
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	LOGF(VERBOSE, "Imgui Destroyed");
}

void Window::StartRender()
{
	// handle windows messages
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			shouldRun = false;
	}

	// begin a new frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Window::EndRender()
{
	// Render ImGui
	ImGui::Render();

	// Make a color that's clear / transparent
	float color[4]{ 0, 0, 0, 0 };

	// Set the render target and then clear it
	device_context->OMSetRenderTargets(1, &render_targetview, nullptr);
	device_context->ClearRenderTargetView(render_targetview, color);

	// Render ImGui draw data.
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	auto io = ImGui::GetIO();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	// Present rendered frame with V-Sync
	//swap_chain->Present(1U, 0U);

	// Present rendered frame without V-Sync
	swap_chain->Present(0U, 0U);
}

void Window::SetTopMost(HWND window, bool up_down) {
	SetWindowPos(
		window,
		up_down ? HWND_TOPMOST : HWND_NOTOPMOST,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER
	);
}

void Window::SetForeground(HWND window)
{
	if (!IsWindowInForeground(window))
		BringToForeground(window);
}

// declaration of the ImGui_ImplWin32_WndProcHandler function
// basically integrates ImGui with the Windows message loop so ImGui can process input and events
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_procedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// set up ImGui window procedure handler
	if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam))
		return true;

	// switch that disables alt application and checks for if the user tries to close the window.
	switch (msg)
	{
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu (imgui uses it in their example :shrug:)
			return 0;
		break;
	case WM_DESTROY: // We dont handle this event
		LOGF(VERBOSE, "Window procedure WM_DESTROY event triggered"); // We dont want to exit if a child window is closed, as they are when changing tabs
		break;
	case WM_CLOSE:
		LOGF(VERBOSE, "Window procedure WM_CLOSE event triggered");
		//DestroyWindow(window); // Redirect the event to WM_DESTROY
		Window::shouldRun = false; // Exit render thread, it will cleanup on exit
		break;
	case WM_SIZE:
		//DEBUG_LOG(out, "Window procedure WM_SIZE event triggered");
		if (Window::device != nullptr && wParam != SIZE_MINIMIZED)
		{
			if (Window::render_targetview != nullptr) {
				Window::render_targetview->Release();
				Window::render_targetview = nullptr;
			}

			// Resize the swap chain
			Window::swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);

			// Recreate the render target view
			ID3D11Texture2D* back_buffer{ nullptr };
			Window::swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

			// if back buffer is obtained then we can create render target view and release the back buffer again
			if (back_buffer)
			{
				Window::device->CreateRenderTargetView(back_buffer, nullptr, &Window::render_targetview);
				back_buffer->Release();

				return true;
			}
		}
		return 0;

	case WM_DPICHANGED:
		LOGF(VERBOSE, "Window procedure WM_DPICHANGED event triggered");
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			const RECT* suggested_rect = (RECT*)lParam;

			SetWindowPos(
				Window::hwnd, 
				nullptr, 
				suggested_rect->left, suggested_rect->top, 
				suggested_rect->right - suggested_rect->left, 
				suggested_rect->bottom - suggested_rect->top, 
				SWP_NOZORDER | SWP_NOACTIVATE
			);
		}
		break;
	}

	return DefWindowProc(window, msg, wParam, lParam);
}
