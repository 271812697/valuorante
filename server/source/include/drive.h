#pragma once
#include <Windows.h>

HMODULE h = LoadLibraryA("YX_X64.dll"); //这里也可以替换成其他更好的加载dll的方案

typedef struct _MyMEMORY_BASIC_INFORMATION
{
	ULONG64 BaseAddress;
	ULONG64 AllocationBase;
	ULONG64 AllocationProtect;
	ULONG64 RegionSize;
	ULONG64 State;
	ULONG64 Protect;
	ULONG64 Type;
}MyMEMORY_BASIC_INFORMATION, * PMyMEMORY_BASIC_INFORMATION;

extern "C" typedef int(WINAPI* yxInitDriverByCard)(const char* Card, long long* PointNumber, char* LoginState);
extern "C" typedef int(WINAPI* yxInitDriverByCard1)(const char* Card, long long* PointNumber, char* LoginState);
extern "C" typedef int(WINAPI* yxCleanForExit)();
extern "C" typedef int(WINAPI* yxProcessCamouflage)(ULONG fakeProcess, ULONG SrcPid);
extern "C" typedef void(WINAPI* yxDeleteFile)(const char* name);
extern "C" typedef ULONG64(WINAPI* yxProtectFile)(const char* name);
extern "C" typedef void(WINAPI* yxUnProtectFile)(ULONG64 fileOb);
extern "C" typedef void(WINAPI* yxProcessProtect)(ULONG pid);
extern "C" typedef int(WINAPI* yxGetProcessIdByName)(const char* ProcessName);
extern "C" typedef int(WINAPI* yxQueryMemory)(ULONG pid, ULONG64 Address, PMyMEMORY_BASIC_INFORMATION info);
extern "C" typedef ULONG64(WINAPI* yxGetModuleAddress)(ULONG pid, const char* moduleName, PSIZE_T moduleSize);
extern "C" typedef ULONG(WINAPI* yxGetExeModule)(ULONG pid);
extern "C" typedef int(WINAPI* yxSetRWMemoryMode)(ULONG mode);
extern "C" typedef int(WINAPI* yxReadProcessMemory)(ULONG64 pid, ULONG64 TargetAddress, PVOID lpBuffer, SIZE_T size);
extern "C" typedef int(WINAPI* yxWriteProcessMemory)(ULONG64 pid, ULONG64 TargetAddress, PVOID lpBuffer, SIZE_T size);
extern "C" typedef void(WINAPI* yxAntiScreen)(HWND windowHandle);
extern "C" typedef void* (WINAPI* yxAllocateMemory)(ULONG pid, SIZE_T size);
extern "C" typedef int(WINAPI* yxFreeMemory)(ULONG pid, PVOID address, SIZE_T size);
extern "C" typedef int(WINAPI* yxHideProcess1)(ULONG pid);
extern "C" typedef int(WINAPI* yxHideProcess2)(ULONG pid);
extern "C" typedef int(WINAPI* yxUnHideProcess1)(ULONG pid);
extern "C" typedef void(WINAPI* yxHideThread)(ULONG threadId);
extern "C" typedef void(WINAPI* yxMouseMoveRel)(long x, long y);
extern "C" typedef void(WINAPI* yxMouseEvent)(unsigned short flags);
extern "C" typedef ULONG64(WINAPI* yxDecryptCr3)(ULONG pid);

inline yxInitDriverByCard   g_yxInitDriverByCard = (yxInitDriverByCard)GetProcAddress(h, "yxInitDriverByCard");
inline yxInitDriverByCard1  g_yxInitDriverByCard1 = (yxInitDriverByCard1)GetProcAddress(h, "yxInitDriverByCard1");
inline yxCleanForExit       g_yxCleanForExit = (yxCleanForExit)GetProcAddress(h, "yxCleanForExit");
inline yxProcessCamouflage  g_yxProcessCamouflage = (yxProcessCamouflage)GetProcAddress(h, "yxProcessCamouflage");
inline yxDeleteFile         g_yxDeleteFile = (yxDeleteFile)GetProcAddress(h, "yxDeleteFile");
inline yxProtectFile        g_yxProtectFile = (yxProtectFile)GetProcAddress(h, "yxProtectFile");
inline yxUnProtectFile      g_yxUnProtectFile = (yxUnProtectFile)GetProcAddress(h, "yxUnProtectFile");
inline yxProcessProtect     g_yxProcessProtect = (yxProcessProtect)GetProcAddress(h, "yxProcessProtect");
inline yxGetProcessIdByName g_yxGetProcessIdByName = (yxGetProcessIdByName)GetProcAddress(h, "yxGetProcessIdByName");
inline yxQueryMemory        g_yxQueryMemory = (yxQueryMemory)GetProcAddress(h, "yxQueryMemory");
inline yxGetModuleAddress   g_yxGetModuleAddress = (yxGetModuleAddress)GetProcAddress(h, "yxGetModuleAddress");
inline yxGetExeModule       g_yxGetExeModule = (yxGetExeModule)GetProcAddress(h, "yxGetExeModule");
inline yxSetRWMemoryMode    g_yxSetRWMemoryMode = (yxSetRWMemoryMode)GetProcAddress(h, "yxSetRWMemoryMode");
inline yxReadProcessMemory  g_yxReadProcessMemory = (yxReadProcessMemory)GetProcAddress(h, "yxReadProcessMemory");
inline yxWriteProcessMemory g_yxWriteProcessMemory = (yxWriteProcessMemory)GetProcAddress(h, "yxWriteProcessMemory");
inline yxAntiScreen         g_yxAntiScreen = (yxAntiScreen)GetProcAddress(h, "yxAntiScreen");
inline yxAllocateMemory     g_yxAllocateMemory = (yxAllocateMemory)GetProcAddress(h, "yxAllocateMemory");
inline yxFreeMemory         g_yxFreeMemory = (yxFreeMemory)GetProcAddress(h, "yxFreeMemory");
inline yxHideProcess1       g_yxHideProcess1 = (yxHideProcess1)GetProcAddress(h, "yxHideProcess1");
inline yxHideProcess2       g_yxHideProcess2 = (yxHideProcess2)GetProcAddress(h, "yxHideProcess2");
inline yxUnHideProcess1     g_yxUnHideProcess1 = (yxUnHideProcess1)GetProcAddress(h, "yxUnHideProcess1");
inline yxHideThread         g_yxHideThread = (yxHideThread)GetProcAddress(h, "yxHideThread");
inline yxMouseMoveRel       g_yxMouseMoveRel = (yxMouseMoveRel)GetProcAddress(h, "yxMouseMoveRel");
inline yxMouseEvent         g_yxMouseEvent = (yxMouseEvent)GetProcAddress(h, "yxMouseEvent");
inline yxDecryptCr3         g_yxDecryptCr3 = (yxDecryptCr3)GetProcAddress(h, "yxDecryptCr3");

ULONG64 ProcessCr3 = NULL;
//登录并初始化驱动例子
char LoginBuf[100] = { 0 };
long long PointNumber = 0;

template <typename T>
inline T Read(ULONG64 TargetAddress)//读取内存
{
	T Buffer{};
	g_yxReadProcessMemory(ProcessCr3, TargetAddress, &Buffer, sizeof(T));
	return Buffer;
}

template <typename ReadType>
BOOL ReadMemory(ULONG64 TargetAddress, ReadType Buffer, ULONG Size)
{
	if (!g_yxReadProcessMemory(ProcessCr3, TargetAddress, Buffer, Size))return FALSE;
	return TRUE;
}

template <typename ReadType>
BOOL WriteMemory(ULONG64 TargetAddress, ReadType Buffer, ULONG Size)
{
	if (!g_yxWriteProcessMemory(ProcessCr3, TargetAddress, Buffer, Size))return FALSE;
	return TRUE;
}