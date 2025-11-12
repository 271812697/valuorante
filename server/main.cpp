#include "GameLoop.h"


int Main()
{
    //SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
    //AddVectoredExceptionHandler(1, MyVectoredHandler);

    HMODULE h = LoadLibraryA("YX_X64.dll"); //这里也可以替换成其他更好的加载dll的方案

    AllocConsole();
    freopen("CONOUT$", "w+t", stdout);

    freopen("CONIN$", "r+t", stdin);

    //if (g_yxInitDriverByCard("YxRwA24EG2GQ56WV35G389T8BN",&PointNumber,LoginBuf) != 0)
      int ret = g_yxInitDriverByCard1("YxRwA24EG2GQ56WV35G389T8BN", &PointNumber, LoginBuf); //这个加载驱动的函数可以过预启动，两个加载驱动的函数选一个用就行了
      if (ret == 0)
      {
          printf("登录状态：%s  点数：%lld \n", LoginBuf, PointNumber);
      }
      else
      {
          printf("失败 返回值：%d 状态：%s\n", ret, LoginBuf);
      }
      {
        std::cout << ("驱动加载失败") << std::endl;
        system("pause");
        ExitProcess(0);
    }

    if (!InitWinSock(12345))
    {
        std::cout << ("网络协议初始化失败") << std::endl;
        system("pause");
        ExitProcess(0);
    }

    ULONG ProcessPid = NULL;
   
    std::cout << ("初始化成功,请输入进程PID:") << std::endl;

    std::cin >> ProcessPid;

    ProcessCr3 = g_yxDecryptCr3(ProcessPid);
     
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessPid);
    if (!hProcess) {
        std::cout << "PID无效，错误码：" << GetLastError() << std::endl;
        system("pause");
        return 1;
    }
    CloseHandle(hProcess); // 用完关闭句柄
    //g_yxProcessCamouflage(ProcessPid, ); //伪装进程例子
    //g_yxSetRWMemoryMode(0); //设置读写模式例子

    FreeConsole();

    fclose(stdin);

    fclose(stdout);

    fclose(stderr);

    std::thread loop(GameLoop);
    loop.detach();

    SendLoop();

    g_yxCleanForExit();//退出drive
    return 0;
}

BOOL DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        std::thread main(Main);
        main.detach();
    }

    return TRUE;
}
