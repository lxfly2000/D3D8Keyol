#pragma once
#include"d3d8.h"
#include "imgui.h"
#include "imgui_impl_dx8.h"

LRESULT CALLBACK KeyOverlayExtraProcess(WNDPROC oldProc, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL KeyOverlayInit(HWND hwnd, LPDIRECT3DDEVICE8 pDevice);
void KeyOverlayUninit();
void KeyOverlayDraw();
