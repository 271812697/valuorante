#pragma once
#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <string_view>
#include <vector>
#include <cstdint>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")


// RC4加密算法实现（加密和解密通用）
class RC4Cipher {
private:
    uint8_t m_state[256];
    uint8_t m_i, m_j;

    void init(const uint8_t* key, size_t keyLen) {
        for (int i = 0; i < 256; ++i) m_state[i] = i;
        uint8_t j = 0;
        for (int i = 0; i < 256; ++i) {
            j = (j + m_state[i] + key[i % keyLen]) % 256;
            std::swap(m_state[i], m_state[j]);
        }
        m_i = m_j = 0;
    }

public:
    RC4Cipher(const uint8_t* key, size_t keyLen) { init(key, keyLen); }
    RC4Cipher(const char* key) { init((const uint8_t*)key, strlen(key)); }

    void crypt(uint8_t* data, size_t len) {
        for (size_t k = 0; k < len; ++k) {
            m_i = (m_i + 1) % 256;
            m_j = (m_j + m_state[m_i]) % 256;
            std::swap(m_state[m_i], m_state[m_j]);
            uint8_t kstream = m_state[(m_state[m_i] + m_state[m_j]) % 256];
            data[k] ^= kstream;
        }
    }

    void crypt(std::vector<uint8_t>& data) {
        if (!data.empty()) crypt(data.data(), data.size());
    }
};

// 全局RC4密钥（需与接收方保持一致）
const char* g_rc4Key = "YourSecretKey123";  // 自定义密钥


SOCKET Socket = NULL;
sockaddr_in SockAddr = {};

typedef struct _SocStruct
{
    BOOL State;
    ULONG Token;
} SocStruct;

BOOL InitWinSock(USHORT HostPort)
{
    WSADATA WsaData = {};
    if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) return FALSE;

    Socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (Socket == INVALID_SOCKET)
    {
        WSACleanup();
        return FALSE;
    }

    sockaddr_in ServerAddr = {};
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(HostPort);
    ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(Socket, reinterpret_cast<sockaddr*>(&ServerAddr), sizeof(ServerAddr)) == SOCKET_ERROR)
    {
        closesocket(Socket);
        WSACleanup();
        return FALSE;
    }

    printf(("等待副机链接... \n"));

    SocStruct Data = {};
    INT Size = sizeof(SockAddr);
    recvfrom(Socket, (char*)&Data, sizeof(Data), 0, (sockaddr*)&SockAddr, &Size);
    Sleep(1000);

    if (Data.State != TRUE || Data.Token != 0x1897084) return FALSE;

    printf(("适配成功 \n"));
    return TRUE;
}

struct SimpleCommand
{
    UINT8 Type;
    std::vector<UINT8> Data;

    template<typename Type1, typename Type2, typename Type3>
    static SimpleCommand Create(UINT8 Type, const Type1& Data1, const Type2& Data2, const Type3& Data3)
    {
        SimpleCommand SimlpeCmd = {};
        SimlpeCmd.Type = Type;

        const UINT8* Bytes1 = reinterpret_cast<const UINT8*>(&Data1);
        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), Bytes1, Bytes1 + sizeof(Type1));

        const UINT8* Bytes2 = reinterpret_cast<const UINT8*>(&Data2);
        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), Bytes2, Bytes2 + sizeof(Type2));

        const UINT8* Bytes3 = reinterpret_cast<const UINT8*>(&Data3);
        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), Bytes3, Bytes3 + sizeof(Type3));

        return SimlpeCmd;
    }

    template<typename CustomType, typename VectorType1, typename VectorType2>
    static SimpleCommand Create(UINT8 Type, const CustomType& CustomData, const std::vector<VectorType1>& Vector1, const std::vector<VectorType2>& Vector2)
    {
        SimpleCommand SimlpeCmd = {};
        SimlpeCmd.Type = Type;

        const UINT8* CustomBytes = reinterpret_cast<const UINT8*>(&CustomData);
        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), CustomBytes, CustomBytes + sizeof(CustomType));

        SIZE_T Count1 = Vector1.size();
        const UINT8* CountBytes1 = reinterpret_cast<const UINT8*>(&Count1);
        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), CountBytes1, CountBytes1 + sizeof(SIZE_T));

        if (!Vector1.empty())
        {
            const UINT8* Elements1 = reinterpret_cast<const UINT8*>(Vector1.data());
            SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), Elements1, Elements1 + Vector1.size() * sizeof(VectorType1));
        }

        SIZE_T Count2 = Vector2.size();
        const UINT8* CountBytes2 = reinterpret_cast<const UINT8*>(&Count2);
        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), CountBytes2, CountBytes2 + sizeof(SIZE_T));

        if (!Vector2.empty())
        {
            const UINT8* Elements2 = reinterpret_cast<const UINT8*>(Vector2.data());
            SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), Elements2, Elements2 + Vector2.size() * sizeof(VectorType2));
        }

        return SimlpeCmd;
    }

    // 序列化命令（包含Type和Data，用于加密传输）
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buf;
        buf.push_back(Type);  // 先写入命令类型
        buf.insert(buf.end(), Data.begin(), Data.end());  // 再写入数据部分
        return buf;
    }
};

BOOL SendSocketMessage(PVOID Data, ULONG64 Size)
{
    if (!Data || Size == 0 || Socket == INVALID_SOCKET) return FALSE;

    // 1. 复制数据到临时缓冲区
    std::vector<uint8_t> dataBuf((uint8_t*)Data, (uint8_t*)Data + Size);

    // 2. 使用RC4加密
    RC4Cipher cipher(g_rc4Key);
    cipher.crypt(dataBuf);

    // 3. 发送加密后的数据
    INT sendResult = sendto(
        Socket,
        (const char*)Data,
        (int)Size,
        0,
        (sockaddr*)&SockAddr,
        sizeof(SockAddr)
    );

    return sendResult == (int)dataBuf.size();
}

// 便捷发送命令的函数（自动序列化并加密）
BOOL SendSimpleCommand(const SimpleCommand& cmd) {
    std::vector<uint8_t> serialized = cmd.serialize();
    return SendSocketMessage(serialized.data(), serialized.size());
}









//#pragma once
//#include <winsock2.h>
//#include <iostream>
//#include <ws2tcpip.h>
//#include <string_view>
//#include <vector>
//#include ".h"
//#pragma comment(lib, "ws2_32.lib")
//
//
//SOCKET Socket = NULL;
//
//sockaddr_in SockAddr = {};
//
//typedef struct _SocStruct
//{
//    BOOL State;
//    ULONG Token;
//} SocStruct, * PSocStruct;
//
//BOOL InitWinSock(USHORT HostPort)
//{
//    WSADATA WsaData = {};
//
//    if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)return FALSE;
//
//    Socket = socket(AF_INET, SOCK_DGRAM, 0);
//
//    if (Socket == INVALID_SOCKET)
//    {
//        WSACleanup();
//        return FALSE;
//    }
//
//    sockaddr_in ServerAddr = {};
//
//    ServerAddr.sin_family = AF_INET;
//
//    ServerAddr.sin_port = htons(HostPort);
//
//    ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
//
//    if (bind(Socket, reinterpret_cast<sockaddr*>(&ServerAddr), sizeof(ServerAddr)) == SOCKET_ERROR)
//    {
//        closesocket(Socket);
//        WSACleanup();
//        return FALSE;
//    }
//
//    printf(("等待副机链接... \n"));
//
//    SocStruct Data = {};
//
//    INT Size = sizeof(SockAddr);
//
//    recvfrom(Socket, (char*)&Data, sizeof(Data), 0, (sockaddr*)&SockAddr, &Size);
//
//    Sleep(1000);
//
//    if (Data.State != TRUE || Data.Token != 0x1897084)return FALSE;
//
//    printf(("适配成功 \n"));
//
//    return TRUE;
//}
//
//struct SimpleCommand
//{
//    UINT8 Type;
//
//    std::vector<UINT8> Data;
//
//    template<typename Type1, typename Type2, typename Type3>
//    static SimpleCommand Create(UINT8 Type, const Type1& Data1, const Type2& Data2, const Type3& Data3)
//    {
//        SimpleCommand SimlpeCmd = {};
//
//        SimlpeCmd.Type = Type;
//
//        const UINT8* Bytes1 = reinterpret_cast<const UINT8*>(&Data1);
//
//        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), Bytes1, Bytes1 + sizeof(Type1));
//
//        const UINT8* Bytes2 = reinterpret_cast<const UINT8*>(&Data2);
//
//        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), Bytes2, Bytes2 + sizeof(Type2));
//
//        const UINT8* Bytes3 = reinterpret_cast<const UINT8*>(&Data3);
//
//        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), Bytes3, Bytes3 + sizeof(Type3));
//
//        return SimlpeCmd;
//    }
//
//    template<typename CustomType, typename VectorType1, typename VectorType2>
//    static SimpleCommand Create(UINT8 Type, const CustomType& CustomData, const std::vector<VectorType1>& Vector1, const std::vector<VectorType2>& Vector2)
//    {
//        SimpleCommand SimlpeCmd = {};
//
//        SimlpeCmd.Type = Type;
//
//        const UINT8* CustomBytes = reinterpret_cast<const UINT8*>(&CustomData);
//
//        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), CustomBytes, CustomBytes + sizeof(CustomType));
//
//        SIZE_T Count1 = Vector1.size();
//
//        const UINT8* CountBytes1 = reinterpret_cast<const UINT8*>(&Count1);
//
//        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), CountBytes1, CountBytes1 + sizeof(SIZE_T));
//
//        if (!Vector1.empty())
//        {
//            const UINT8* Elements1 = reinterpret_cast<const UINT8*>(Vector1.data());
//
//            SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), Elements1, Elements1 + Vector1.size() * sizeof(VectorType1));
//        }
//
//        SIZE_T Count2 = Vector2.size();
//
//        const UINT8* CountBytes2 = reinterpret_cast<const UINT8*>(&Count2);
//
//        SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), CountBytes2, CountBytes2 + sizeof(SIZE_T));
//
//        if (!Vector2.empty())
//        {
//            const UINT8* Elements2 = reinterpret_cast<const UINT8*>(Vector2.data());
//
//            SimlpeCmd.Data.insert(SimlpeCmd.Data.end(), Elements2, Elements2 + Vector2.size() * sizeof(VectorType2));
//        }
//
//        return SimlpeCmd;
//    }
//};
//
//BOOL SendSocketMessage(PVOID Data, ULONG64 Size)
//{
//    return sendto(Socket, (const char*)Data, Size, 0, (sockaddr*)&SockAddr, sizeof(SockAddr));
//}