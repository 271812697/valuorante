#include "imgui.h"
#include "imgui_edited.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "Icon.h"
#include "Math.h"
#include "XorString.h"
#include <d3d11.h>
#include <dwmapi.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <windows.h>

#pragma comment(lib, "d3d11.lib")

DXGI_SWAP_CHAIN_DESC DirectX;

ID3D11Device *g_pd3dDevice = NULL;

ID3D11DeviceContext *g_pd3dDeviceContext = NULL;

IDXGISwapChain *g_pSwapChain = NULL;

ID3D11RenderTargetView *g_mainRenderTargetView = NULL;

HWND MyWindow = NULL;

INT ScreenWidth = NULL;

INT ScreenHeight = NULL;

FLOAT TabAlpha = 0.f;

INT ActiveTab = 0;

INT TabPage = 0;

BOOL ShowMenu = TRUE;

VOID CleanImGui() // 销毁IMGUI
{
  ImGui_ImplDX11_Shutdown();

  ImGui_ImplWin32_Shutdown();

  ImGui::DestroyContext();

  if (g_mainRenderTargetView) {
    g_mainRenderTargetView->Release();
    g_mainRenderTargetView = NULL;
  }

  if (g_pSwapChain) {
    g_pSwapChain->Release();
    g_pSwapChain = NULL;
  }

  if (g_pd3dDeviceContext) {
    g_pd3dDeviceContext->Release();
    g_pd3dDeviceContext = NULL;
  }

  if (g_pd3dDevice) {
    g_pd3dDevice->Release();
    g_pd3dDevice = NULL;
  }

  DestroyWindow(MyWindow);
}

BOOL InitImGui(const char *FonName, FLOAT FontSize) // 初始化IMGUI
{
  ImGui::CreateContext();

  ImGuiIO &ImgIo = ImGui::GetIO();

  ImgIo.IniFilename = NULL;

  ImgIo.LogFilename = NULL;

  ImGui_ImplWin32_Init(MyWindow);

  ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

  ImgIo.Fonts->AddFontFromFileTTF(FonName, FontSize, 0,
                                  ImgIo.Fonts->GetGlyphRangesChineseFull());

  ImFontConfig FontConfig = {};

  Font::icomoon_tabs =
      ImgIo.Fonts->AddFontFromMemoryTTF(Icons, sizeof(Icons), 22.f, &FontConfig,
                                        ImgIo.Fonts->GetGlyphRangesCyrillic());

  return TRUE;
}

BOOL CreateDeviceD3D(HWND hWnd) // 创建D3D
{
  ZeroMemory(&DirectX, sizeof(DirectX));

  DirectX.BufferCount = 2;

  DirectX.BufferDesc.Width = 0;

  DirectX.BufferDesc.Height = 0;

  DirectX.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  DirectX.BufferDesc.RefreshRate.Numerator = 0;

  DirectX.BufferDesc.RefreshRate.Denominator = 1;

  DirectX.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  DirectX.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

  DirectX.OutputWindow = hWnd;

  DirectX.SampleDesc.Count = 1;

  DirectX.SampleDesc.Quality = 0;

  DirectX.Windowed = TRUE;

  DirectX.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT DeviceFlags = 0;

  D3D_FEATURE_LEVEL Level;

  const D3D_FEATURE_LEVEL LevelArray[2] = {D3D_FEATURE_LEVEL_11_0,
                                           D3D_FEATURE_LEVEL_10_0};

  HRESULT Relust = D3D11CreateDeviceAndSwapChain(
      NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, DeviceFlags, LevelArray, 2,
      D3D11_SDK_VERSION, &DirectX, &g_pSwapChain, &g_pd3dDevice, &Level,
      &g_pd3dDeviceContext);

  if (Relust == DXGI_ERROR_UNSUPPORTED)
    Relust = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_WARP, NULL, DeviceFlags, LevelArray, 2,
        D3D11_SDK_VERSION, &DirectX, &g_pSwapChain, &g_pd3dDevice, &Level,
        &g_pd3dDeviceContext);

  if (Relust != S_OK)
    return FALSE;

  ID3D11Texture2D *BackBuffer;

  g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));

  if (BackBuffer != NULL)
    g_pd3dDevice->CreateRenderTargetView(BackBuffer, NULL,
                                         &g_mainRenderTargetView);

  BackBuffer->Release();

  return TRUE;
}

BOOL OverlayInit(const char *FonName, FLOAT FontSize) {
  ScreenWidth = GetSystemMetrics(SM_CXSCREEN);

  ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

  WNDCLASSEXA ClassEx = {sizeof(WNDCLASSEXA),
                         CS_CLASSDC,
                         DefWindowProcA,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         "Overlay",
                         NULL};

  RegisterClassExA(&ClassEx);

  MyWindow = CreateWindowExA(0, ClassEx.lpszClassName, "", WS_POPUP, NULL, NULL,
                             ScreenWidth, ScreenHeight, NULL, NULL, NULL, NULL);

  SetWindowLongPtrA(MyWindow, GWL_STYLE, WS_POPUP);

  SetWindowPos(MyWindow, 0, 0, 0, ScreenWidth, ScreenHeight, SWP_SHOWWINDOW);

  ShowWindow(MyWindow, SW_SHOWNA);

  UpdateWindow(MyWindow);

  MARGINS Margin{-1, -1, -1, -1};

  DwmExtendFrameIntoClientArea(MyWindow, &Margin);

  if (!CreateDeviceD3D(MyWindow))
    return FALSE;

  if (!InitImGui(FonName, FontSize))
    return FALSE;

  return TRUE;
}

// BOOL OverlayInit(const char* FonName, FLOAT FontSize)
//{
//	ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
//
//	ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
//
//	WNDCLASSEXA ClassEx = { sizeof(WNDCLASSEXA), CS_CLASSDC, DefWindowProcA,
//NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Overlay", NULL };
//
//	RegisterClassExA(&ClassEx);
//
//	MyWindow = CreateWindowExA(WS_EX_TOPMOST, ClassEx.lpszClassName, "",
//WS_POPUP, NULL, NULL, ScreenWidth, ScreenHeight, NULL, NULL, NULL, NULL);
//
//	SetWindowLongPtrA(MyWindow, GWL_STYLE, WS_POPUP);
//
//	SetWindowLongPtrA(MyWindow, GWL_EXSTYLE, WS_EX_TRANSPARENT |
//WS_EX_LAYERED | WS_EX_NOACTIVATE);
//
//	SetWindowPos(MyWindow, 0, 1, 1, ScreenWidth, ScreenHeight,
//SWP_SHOWWINDOW);
//
//	SetLayeredWindowAttributes(MyWindow, 0, 255, LWA_ALPHA);
//
//	ShowWindow(MyWindow, SW_SHOWNA);
//
//	UpdateWindow(MyWindow);
//
//	MARGINS Margin{ -1,-1,-1,-1 };
//
//	DwmExtendFrameIntoClientArea(MyWindow, &Margin);
//
//	if (!CreateDeviceD3D(MyWindow))return FALSE;
//
//	if (!InitImGui(FonName, FontSize))return FALSE;
//
//	return TRUE;
// }

VOID MainLoop(std::function<VOID()> CallBack) {
  MSG Message = {NULL};

  while (Message.message != WM_QUIT) {
    if (PeekMessageA(&Message, MyWindow, 0, 0, PM_REMOVE)) {
      TranslateMessage(&Message);

      DispatchMessageA(&Message);
    }

    if (ShowMenu == TRUE) {
      RECT rc = {};

      POINT xy = {};

      GetClientRect(MyWindow, &rc);

      ClientToScreen(MyWindow, &xy);

      rc.left = xy.x;

      rc.top = xy.y;

      ImGuiIO &ImgIo = ImGui::GetIO();

      ImgIo.DeltaTime = 1.0f / 60.0f;

      POINT p;

      GetCursorPos(&p);

      ImgIo.MousePos.x = p.x - xy.x;

      ImgIo.MousePos.y = p.y - xy.y;

      if (GetAsyncKeyState(1)) {
        ImgIo.MouseDown[0] = true;

        ImgIo.MouseClicked[0] = true;

        ImgIo.MouseClickedPos[0].x = ImgIo.MousePos.x;

        ImgIo.MouseClickedPos[0].x = ImgIo.MousePos.y;
      } else
        ImgIo.MouseDown[0] = false;
    }

    ImGui_ImplDX11_NewFrame();

    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    CallBack();

    ImGui::Render();

    const FLOAT ClearColorWithAlpha[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);

    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
                                               ClearColorWithAlpha);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(0, 0);
  }

  CleanImGui();
}

/// <summary>
/// 画图区域
/// </summary>
/// <param name="Center"></param>
/// <param name="Radius"></param>
/// <param name="Color"></param>
/// <param name="Thickness"></param>
/// <param name="Num"></param>

std::string StringToU8(const std::string &str) {
  INT Legnth = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

  wchar_t *Buffer = new wchar_t[Legnth + 1];

  ZeroMemory(Buffer, Legnth * 2 + 2);

  ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), Buffer, Legnth);

  INT Size =
      ::WideCharToMultiByte(CP_UTF8, 0, Buffer, -1, NULL, NULL, NULL, NULL);

  char *NewBuffer = new char[Size + 1];

  ZeroMemory(NewBuffer, Size + 1);

  ::WideCharToMultiByte(CP_UTF8, 0, Buffer, Legnth, NewBuffer, Size, NULL,
                        NULL);

  std::string retStr(NewBuffer);

  delete[] Buffer;

  delete[] NewBuffer;

  Buffer = NULL;

  NewBuffer = NULL;

  return retStr;
}

std::string UTF8toANSI(const char *utf8) {
  int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);

  wchar_t *wstr = new wchar_t[len + 1];

  memset(wstr, 0, len + 1);

  MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);

  len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

  char *str = new char[len + 1];

  memset(str, 0, len + 1);

  WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);

  if (wstr)
    delete[] wstr;

  std::string ret = str;

  if (str)
    delete[] str;

  return ret;
}

VOID DrawLine(FLOAT x, FLOAT y, FLOAT x1, FLOAT y1, ImColor Color,
              FLOAT Thickness) // 绘制线段
{
  ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x1, y1), Color,
                                          Thickness);
}

VOID DrawRectangleA(FLOAT x, FLOAT y, FLOAT w, FLOAT h, ImColor Color,
                    FLOAT Thickness) // 绘制矩形
{
  ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), {x + w, y + h}, Color,
                                          0, 0, Thickness);
}

VOID DrawRectangleB(FLOAT x, FLOAT y, FLOAT w, FLOAT h, ImColor Color,
                    FLOAT Thickness) // 绘制矩形(4棱线)
{
  DrawLine(x, y, x, y + (h / 3), Color, Thickness);
  DrawLine(x, y, x + (w / 3), y, Color, Thickness);
  DrawLine(x + w - (w / 3), y, x + w, y, Color, Thickness);
  DrawLine(x + w, y, x + w, y + (h / 3), Color, Thickness);
  DrawLine(x, y + h - (h / 3), x, y + h, Color, Thickness);
  DrawLine(x, y + h, x + (w / 3), y + h, Color, Thickness);
  DrawLine(x + w - (w / 3), y + h, x + w, y + h, Color, Thickness);
  DrawLine(x + w, y + h - (h / 3), x + w, y + h, Color, Thickness);
}

VOID DrawFilledRectangle(FLOAT x, FLOAT y, FLOAT w, FLOAT h,
                         ImVec4 Color) // 绘制填充矩形
{
  ImGui::GetBackgroundDrawList()->AddRectFilled(
      ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(Color),
      0, 0);
}

VOID DrawCircle(FLOAT x, FLOAT y, FLOAT Radius, ImColor Color,
                FLOAT Thickness) // 绘制圆形
{
  ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(x, y), Radius, Color, 50,
                                            Thickness);
}

VOID DrawString(std::string Text, FLOAT x, FLOAT y, ImColor Color,
                FLOAT FontSize) // 绘制文本(居中)
{
  FLOAT TextWidth =
      ImGui::GetFont()->CalcTextSizeA(FontSize, FLT_MAX, 0.f, Text.c_str()).x;

  ImVec2 Coord = ImVec2((x - TextWidth / 2) + 1, y + 1);

  ImVec2 CoordOut = ImVec2{(x - TextWidth / 2) - 1, y - 1};

  ImGui::GetBackgroundDrawList()->AddTextEx(
      Coord, FontSize, ImGui::ColorConvertFloat4ToU32(ImColor(0, 0, 0, 255)),
      Text.c_str());

  ImGui::GetBackgroundDrawList()->AddTextEx(
      CoordOut, FontSize, ImGui::ColorConvertFloat4ToU32(ImColor(0, 0, 0, 255)),
      Text.c_str());

  ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), FontSize,
                                          ImVec2(x - TextWidth / 2, y), Color,
                                          Text.c_str());
}

VOID DrawTableString(std::string Text, FLOAT x, FLOAT y, ImColor Color,
                     FLOAT FontSize) // 绘制文本(左到右对齐)
{
  ImVec2 Coord = ImVec2(x + 1, y + 1);

  ImVec2 CoordOut = ImVec2{x, y - 1};

  ImGui::GetBackgroundDrawList()->AddTextEx(
      Coord, FontSize, ImGui::ColorConvertFloat4ToU32(ImColor(0, 0, 0, 255)),
      Text.c_str());

  ImGui::GetBackgroundDrawList()->AddTextEx(
      CoordOut, FontSize, ImGui::ColorConvertFloat4ToU32(ImColor(0, 0, 0, 255)),
      Text.c_str());

  ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), FontSize,
                                          ImVec2(x, y), Color, Text.c_str());
}

VOID DrawTriangle(float distance, float baseSize, float height,
                  Vector3 DirectionPos, Vector3 ScreenPos,
                  ImColor Color) // 绘制填充三角形
{
  ImVec2 dir =
      ImVec2(DirectionPos.x - ScreenPos.x, DirectionPos.y - ScreenPos.y);
  float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
  if (length > 0.0f) {
    dir.x /= length;
    dir.y /= length;
  }
  ImVec2 perp = ImVec2(-dir.y, dir.x);
  ImVec2 baseCenter = ImVec2(ScreenPos.x + dir.x * (7 + distance),
                             ScreenPos.y + dir.y * (7 + distance));
  ImVec2 tip =
      ImVec2(baseCenter.x + dir.x * height, baseCenter.y + dir.y * height);
  ImVec2 base1 = ImVec2(baseCenter.x + perp.x * baseSize / 2,
                        baseCenter.y + perp.y * baseSize / 2);
  ImVec2 base2 = ImVec2(baseCenter.x - perp.x * baseSize / 2,
                        baseCenter.y - perp.y * baseSize / 2);
  ImGui::GetForegroundDrawList()->AddTriangleFilled(tip, base1, base2, Color);
  ImGui::GetForegroundDrawList()->AddTriangle(tip, base1, base2,
                                              ImColor{0, 0, 0, 1}, 1.0f);
}

static const char *KeyNames[] = {
    "None",     "Mouse 1",  "Mouse 2",   "Cancel",   "Mouse 3",   "Mouse 5",
    "Mouse 4",  "",         "Backspace", "Tab",      "",          "",
    "Clear",    "Enter",    "",          "",         "Shift",     "Ctrol",
    "Menu",     "Pause",    "Caps",      "",         "",          "",
    "",         "",         "",          "Escape",   "",          "",
    "",         "",         "Space",     "Page Up",  "Page Down", "End",
    "Home",     "Left",     "Up",        "Right",    "Down",      "",
    "",         "",         "Print",     "Insert",   "Delete",    "",
    "0",        "1",        "2",         "3",        "4",         "5",
    "6",        "7",        "8",         "9",        "",          "",
    "",         "",         "",          "",         "",          "A",
    "B",        "C",        "D",         "E",        "F",         "G",
    "H",        "I",        "J",         "K",        "L",         "M",
    "N",        "O",        "P",         "Q",        "R",         "S",
    "T",        "U",        "V",         "W",        "X",         "Y",
    "Z",        "",         "",          "",         "",          "",
    "Numpad 0", "Numpad 1", "Numpad 2",  "Numpad 3", "Numpad 4",  "Numpad 5",
    "Numpad 6", "Numpad 7", "Numpad 8",  "Numpad 9", "Multiply",  "Add",
    "",         "Subtract", "Decimal",   "Divide",   "F1",        "F2",
    "F3",       "F4",       "F5",        "F6",       "F7",        "F8",
    "F9",       "F10",      "F11",       "F12",
};

bool IsKeyPressed(int vKey) {
  static bool prevKeyState[256] = {false};
  bool currentState = (GetAsyncKeyState(vKey) & 0x8000);
  bool isJustPressed = currentState && !prevKeyState[vKey];
  prevKeyState[vKey] = currentState;
  return isJustPressed;
}

static bool Items_ArrayGetter(void *data, int idx, const char **out_text) {
  const char *const *items = (const char *const *)data;
  if (out_text)
    *out_text = items[idx];
  return true;
}

void HotkeyButton(const char *label, int *aimkey, const ImVec2 &size) {
  const char *preview_value = label;
  if (*aimkey > 0 && *aimkey < IM_ARRAYSIZE(KeyNames))
    Items_ArrayGetter(KeyNames, *aimkey, &preview_value);

  if (ImGui::Button(preview_value, size)) {
    while (1) {
      for (int i = 0; i < 0x87; i++) {
        if (IsKeyPressed(i)) {
          *aimkey = i;
          return;
        }
      }
    }
  }
}