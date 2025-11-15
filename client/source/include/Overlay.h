#pragma once
#include <d3d11.h>
#include <dwmapi.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <windows.h>
struct ImVec4;
struct Vector3;
struct ImColor;
struct ImVec2;
inline DXGI_SWAP_CHAIN_DESC DirectX;

inline ID3D11Device *g_pd3dDevice = NULL;

inline ID3D11DeviceContext *g_pd3dDeviceContext = NULL;

inline IDXGISwapChain *g_pSwapChain = NULL;

inline ID3D11RenderTargetView *g_mainRenderTargetView = NULL;

inline HWND MyWindow = NULL;

inline int ScreenWidth = NULL;

inline int ScreenHeight = NULL;

inline float TabAlpha = 0.f;

inline int ActiveTab = 0;

inline int TabPage = 0;

inline BOOL ShowMenu = TRUE;

VOID CleanImGui();
BOOL InitImGui(const char* FonName, float FontSize);

BOOL CreateDeviceD3D(HWND hWnd);

BOOL OverlayInit(const char* FonName, float FontSize);


VOID MainLoop(std::function<VOID()> CallBack);

/// <summary>
/// 画图区域
/// </summary>
/// <param name="Center"></param>
/// <param name="Radius"></param>
/// <param name="Color"></param>
/// <param name="Thickness"></param>
/// <param name="Num"></param>

std::string StringToU8(const std::string& str);
std::string UTF8toANSI(const char* utf8);

VOID DrawLine(float x, float y, float x1, float y1, ImColor Color,
    float Thickness); // 绘制线段;

VOID DrawRectangleA(float x, float y, float w, float h, ImColor Color,
    float Thickness);// 绘制矩形

VOID DrawRectangleB(float x, float y, float w, float h, ImColor Color,
    float Thickness); // 绘制矩形(4棱线);
VOID DrawFilledRectangle(float x, float y, float w, float h,
    ImVec4 Color);// 绘制填充矩形

VOID DrawCircle(float x, float y, float Radius, ImColor Color,
    float Thickness); // 绘制圆形

VOID DrawString(std::string Text, float x, float y, ImColor Color,
    float FontSize);// 绘制文本(居中)

VOID DrawTableString(std::string Text, float x, float y, ImColor Color,
    float FontSize); // 绘制文本(左到右对齐)

VOID DrawTriangle(float distance, float baseSize, float height,
    Vector3 DirectionPos, Vector3 ScreenPos,
    ImColor Color); // 绘制填充三角形

extern const char* KeyNames[];

bool IsKeyPressed(int vKey);
bool Items_ArrayGetter(void* data, int idx, const char** out_text);

void HotkeyButton(const char* label, int* aimkey, const ImVec2& size);