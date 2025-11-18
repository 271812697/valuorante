#include <windows.h>
#include <thread>
#include <iostream>
#include <fstream>
#include "hook/hook.h"
// 全局变量定义
char dlldir[320];

void Start()
{
	MOON::Hook::instance()->start(("Dx12Hook"), nullptr);
}


DWORD WINAPI MainThread(LPVOID lpParameter) {
	Start();
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	
	std::ofstream file("testHook.txt");
	file << "DllMain" << std::endl;file << "bengin" << std::endl;file << "start begin" << std::endl;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		MOON::Hook::instance()->getProcessData().Module = hModule;
		GetModuleFileNameA(hModule, dlldir, 512);
		for (size_t i = strlen(dlldir); i > 0; i--) { if (dlldir[i] == '\\') { dlldir[i + 1] = 0; break; } }
		CreateThread(0, 0, MainThread, 0, 0, 0);

		break;

	case DLL_PROCESS_DETACH:
		MOON::Hook::instance()->shutdown();
		FreeLibraryAndExitThread(hModule, TRUE);
		break;
	}
	return TRUE;
}

