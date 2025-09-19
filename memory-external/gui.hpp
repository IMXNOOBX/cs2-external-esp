#pragma once
#include <windows.h>

extern bool gui_finish;
extern bool show_gui;
extern bool gui_has_focus;

void GuiThread();
bool InitImGui();
void RenderImGui();
void CleanupImGui();
bool CreateGuiWindow();

LRESULT CALLBACK GuiWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);