#include "Socket.h"
#include "Render.h"
#include "XorString.h"


int main() {
  std::string IP = "0";
  std::cout << XorString("主机V4ip:") << std::endl;
  std::cin >> IP;
  if (!InitWinSock(IP, 12345)) {
    std::cout << XorString("网络协议初始化失败") << std::endl;
    system("pause");
    ExitProcess(0);
  }

  if (!OverlayInit(XorString("C:\\Windows\\Fonts\\msyhbd.ttc"), 17.f)) {
    std::cout << XorString("绘制初始化失败") << std::endl;
    system("pause");
    ExitProcess(0);
  }

  LoadConfig();
  std::thread loop(LoopThread);
  loop.detach();
  MainLoop(Rander);
  return 0;
}
