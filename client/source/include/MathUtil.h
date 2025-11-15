#pragma once
struct ImVec2;
struct ImColor;
struct Vector3 {
  float x, y, z;
  bool IsNearlyZero(float Tolerance) const;
};

namespace Bone {
enum Bone : int {
  头部 = 32,
  颈部 = 31,
  胸部 = 4,
  肚子 = 3,
  脊椎 = 2,
  骨盆 = 1,
  左肩 = 34,
  左肘 = 35,
  左手 = 38,
  左腿 = 58,
  左膝 = 59,
  左脚跟 = 60,
  右肩 = 6,
  右肘 = 7,
  右手 = 10,
  右腿 = 62,
  右膝 = 63,
  右脚跟 = 64,
};

} // namespace Bone

constexpr int KEY_BONES[] = {
    Bone::头部, Bone::颈部, Bone::胸部,   Bone::肚子, Bone::脊椎, Bone::骨盆,
    Bone::左肩, Bone::左肘, Bone::左手,   Bone::右肩, Bone::右肘, Bone::右手,
    Bone::左腿, Bone::左膝, Bone::左脚跟, Bone::右腿, Bone::右膝, Bone::右脚跟};

constexpr int KEY_BONE_COUNT = sizeof(KEY_BONES) / sizeof(int);

struct Mapinfo {
  float X = 0.f;
  float Y = 0.f;
  float W = 0.f;
  float H = 0.f;
  int MapX = 0;
  int MapY = 0;
};

void GetDBMapScreenPos(Mapinfo Map, ImVec2 DisplaySize, Vector3 Pos,
    float Pitch, Vector3& ScreenPos, Vector3& Direction);
void GetCGMapScreenPos(Mapinfo Map, ImVec2 DisplaySize, Vector3 Pos,
    float Pitch, Vector3& ScreenPos, Vector3& Direction);

ImColor GetColorForNumber(int teamId);