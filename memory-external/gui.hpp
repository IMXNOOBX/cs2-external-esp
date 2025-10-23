#pragma once
#include <windows.h>
#include <d3d11.h>

extern bool gui_finish;
extern bool show_gui;
extern bool gui_has_focus;

bool InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
void RenderImGui();
void CleanupImGui();
bool IsImGuiInitialized();
void OnWindowResize();