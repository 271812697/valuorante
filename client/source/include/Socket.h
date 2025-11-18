#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>



// -------------------------- RC4解密核心类（与发送端完全一致）
// --------------------------
class RC4Cipher {
private:
  uint8_t m_state[256]; // RC4状态盒（S盒）
  uint8_t m_i;          // 状态变量i
  uint8_t m_j;          // 状态变量j

  // 初始化S盒（KSA算法，与发送端逻辑一致）
  void init(const uint8_t *key, size_t keyLen) {
    // 初始化S盒为0-255
    for (int i = 0; i < 256; ++i) {
      m_state[i] = static_cast<uint8_t>(i);
    }
    // 用密钥打乱S盒
    uint8_t j = 0;
    for (int i = 0; i < 256; ++i) {
      j = static_cast<uint8_t>((j + m_state[i] + key[i % keyLen]) % 256);
      std::swap(m_state[i], m_state[j]);
    }
    // 重置状态变量
    m_i = 0;
    m_j = 0;
  }

public:
  // 构造函数：支持字节数组密钥
  RC4Cipher(const uint8_t *key, size_t keyLen) { init(key, keyLen); }

  // 重载构造函数：支持字符串密钥（更易用）
  RC4Cipher(const char *key) {
    init(reinterpret_cast<const uint8_t *>(key), std::strlen(key));
  }

  // 加解密统一接口（RC4解密=再次加密，逻辑完全相同）
  void crypt(uint8_t *data, size_t dataLen) {
    if (data == nullptr || dataLen == 0)
      return;

    for (size_t k = 0; k < dataLen; ++k) {
      // 更新状态变量
      m_i = static_cast<uint8_t>((m_i + 1) % 256);
      m_j = static_cast<uint8_t>((m_j + m_state[m_i]) % 256);
      // 交换S盒元素
      std::swap(m_state[m_i], m_state[m_j]);
      // 生成密钥流并异或数据（核心解密步骤）
      uint8_t keyStreamByte =
          m_state[static_cast<uint8_t>(m_state[m_i] + m_state[m_j]) % 256];
      data[k] ^= keyStreamByte;
    }
  }

  // 重载：支持std::vector<uint8_t>直接解密（无需手动处理指针）
  void crypt(std::vector<uint8_t> &data) {
    if (!data.empty()) {
      crypt(data.data(), data.size());
    }
  }
};

// -------------------------- 全局配置（需与发送端严格一致）
// --------------------------
// RC4密钥：必须和发送端使用完全相同的密钥，否则解密失败
inline const char* g_rc4Key =
"YourSecretKey123"; // 示例密钥，实际使用时建议修改为复杂密钥
inline SOCKET Socket = NULL;
inline sockaddr_in SockAddr = {};
// -------------------------- 原始结构体与初始化函数（无修改）
// --------------------------
typedef struct _SocStruct {
  BOOL State;
  ULONG Token;
} SocStruct, *PSocStruct;

BOOL InitWinSock(std::string Ip, USHORT Port);
template <typename CustomType, typename VectorType1, typename VectorType2>
bool ParseComplexData(const std::vector<uint8_t> &ReceivedData,
                      CustomType &OutCustomData,
                      std::vector<VectorType1> &OutVec1,
                      std::vector<VectorType2> &OutVec2) {
  if (ReceivedData.size() < sizeof(CustomType))
    return false;

  const UINT8 *Entity = ReceivedData.data();

  SIZE_T Offset = 0;

  OutCustomData = *reinterpret_cast<const CustomType *>(Entity);

  Offset += sizeof(CustomType);

  if (Offset + sizeof(SIZE_T) > ReceivedData.size())
    return false;

  SIZE_T Count1 = *reinterpret_cast<const SIZE_T *>(Entity + Offset);

  Offset += sizeof(SIZE_T);

  if (Count1 > 0) {
    SIZE_T Vec1Bytes = Count1 * sizeof(VectorType1);

    if (Offset + Vec1Bytes > ReceivedData.size())
      return false;

    OutVec1.resize(Count1);

    memcpy(OutVec1.data(), Entity + Offset, Vec1Bytes);

    Offset += Vec1Bytes;
  }

  if (Offset + sizeof(SIZE_T) > ReceivedData.size())
    return false;

  SIZE_T Count2 = *reinterpret_cast<const SIZE_T *>(Entity + Offset);

  Offset += sizeof(SIZE_T);

  if (Count2 > 0) {
    SIZE_T vec2Bytes = Count2 * sizeof(VectorType2);

    if (Offset + vec2Bytes > ReceivedData.size())
      return false;

    OutVec2.resize(Count2);

    memcpy(OutVec2.data(), Entity + Offset, vec2Bytes);
  }

  return true;
}

// -------------------------- 新增：接收数据的便捷函数（可选，简化调用）
// --------------------------
// 从Socket接收数据并返回（需外部处理接收逻辑时可直接调用）
std::vector<uint8_t> ReceiveSocketData();