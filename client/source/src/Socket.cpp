#pragma once
#include "Socket.h"

#pragma comment(lib, "ws2_32.lib")




BOOL InitWinSock(std::string Ip, USHORT Port) {
  WSADATA WsaData = {};

  if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
    return FALSE;

  Socket = socket(AF_INET, SOCK_DGRAM, 0);

  if (Socket == INVALID_SOCKET) {
    WSACleanup();
    return FALSE;
  }

  SockAddr.sin_family = AF_INET;
  SockAddr.sin_port = htons(Port);
  inet_pton(AF_INET, Ip.c_str(), &SockAddr.sin_addr);

  SocStruct Data = {};
  Data.State = TRUE;
  Data.Token = 0x1897084;
  if (sendto(Socket, (char *)&Data, sizeof(Data), 0, (sockaddr *)&SockAddr,
             sizeof(SockAddr)) == INVALID_SOCKET)
    return FALSE;

  return TRUE;
}



// -------------------------- 新增：接收数据的便捷函数（可选，简化调用）
// --------------------------
// 从Socket接收数据并返回（需外部处理接收逻辑时可直接调用）
std::vector<uint8_t> ReceiveSocketData() {
  if (Socket == INVALID_SOCKET)
    return {};

  const int BUF_SIZE = 4096; // 缓冲区大小，可根据实际数据长度调整
  uint8_t buf[BUF_SIZE] = {0};
  sockaddr_in senderAddr = {};
  int senderAddrLen = sizeof(senderAddr);

  // 接收数据
  int recvLen =
      recvfrom(Socket, reinterpret_cast<char *>(buf), BUF_SIZE, 0,
               reinterpret_cast<sockaddr *>(&senderAddr), &senderAddrLen);
  if (recvLen <= 0)
    return {};

  // 返回接收到的原始加密数据（后续调用ParseComplexData时会解密）
  return std::vector<uint8_t>(buf, buf + recvLen);
}