#define M_PI 3.14159265358979323846

struct Vector3 {
  FLOAT x, y, z;

  bool IsNearlyZero(float Tolerance) const {
    return fabs(x) <= Tolerance && fabs(y) <= Tolerance && fabs(z) <= Tolerance;
  }
};

namespace Bone {
enum Bone : INT {
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
std::list<INT> _上部 = {Bone::头部};
std::list<INT> _右臂 = {Bone::颈部, Bone::右肩, Bone::右肘, Bone::右手};
std::list<INT> _左臂 = {Bone::颈部, Bone::左肩, Bone::左肘, Bone::左手};
std::list<INT> _脊柱 = {Bone::颈部, Bone::胸部, Bone::肚子, Bone::脊椎,
                        Bone::骨盆};
std::list<INT> _右腿 = {Bone::骨盆, Bone::右腿, Bone::右膝, Bone::右脚跟};
std::list<INT> _左腿 = {Bone::骨盆, Bone::左腿, Bone::左膝, Bone::左脚跟};
std::list<std::list<INT>> 拼接骨骼 = {_上部, _右臂, _左臂, _脊柱, _右腿, _左腿};
std::list<INT> _四肢 = {Bone::左肘, Bone::左手, Bone::左膝, Bone::左脚跟,
                        Bone::右肘, Bone::右手, Bone::右膝, Bone::右脚跟};
std::list<INT> _双脚 = {Bone::左膝, Bone::左脚跟, Bone::右膝, Bone::右脚跟};
} // namespace Bone

constexpr int KEY_BONES[] = {
    Bone::头部, Bone::颈部, Bone::胸部,   Bone::肚子, Bone::脊椎, Bone::骨盆,
    Bone::左肩, Bone::左肘, Bone::左手,   Bone::右肩, Bone::右肘, Bone::右手,
    Bone::左腿, Bone::左膝, Bone::左脚跟, Bone::右腿, Bone::右膝, Bone::右脚跟};

constexpr int KEY_BONE_COUNT = sizeof(KEY_BONES) / sizeof(int);

struct Mapinfo {
  FLOAT X = 0.f;
  FLOAT Y = 0.f;
  FLOAT W = 0.f;
  FLOAT H = 0.f;
  INT MapX = 0;
  INT MapY = 0;
};

void GetDBMapScreenPos(Mapinfo Map, ImVec2 DisplaySize, Vector3 Pos,
                       float Pitch, Vector3 &ScreenPos, Vector3 &Direction) {
  ImVec2 MapPos = {float(Map.MapX) + Pos.x - Map.X,
                   float(Map.MapY) + Pos.y - Map.Y};
  ScreenPos = {MapPos.x / Map.W * DisplaySize.x,
               MapPos.y / Map.H * DisplaySize.y};
  Direction = Vector3(ScreenPos.x + cos(Pitch * M_PI / 180) * 22,
                      ScreenPos.y + sin(Pitch * M_PI / 180) * 22);
}

void GetCGMapScreenPos(Mapinfo Map, ImVec2 DisplaySize, Vector3 Pos,
                       float Pitch, Vector3 &ScreenPos, Vector3 &Direction) {
  ImVec2 MapPos = {float(Map.MapX) * 1.00675f - Pos.y - Map.X,
                   float(Map.MapY) * 0.99805f + Pos.x - Map.Y};
  ScreenPos = {MapPos.x / Map.W * DisplaySize.x,
               MapPos.y / Map.H * DisplaySize.y};
  Direction = Vector3(ScreenPos.x + cos(Pitch * M_PI / 180) * 22,
                      ScreenPos.y + sin(Pitch * M_PI / 180) * 22);
}

ImColor GetColorForNumber(int teamId) {
  static const ImColor highContrastColors[] = {
      ImColor(0.2f, 0.8f, 0.2f), // 亮绿色
      ImColor(0.2f, 0.5f, 1.0f), // 亮蓝色
      ImColor(1.0f, 0.8f, 0.2f), // 亮黄色
      ImColor(0.8f, 0.2f, 1.0f), // 亮紫色
      ImColor(0.2f, 0.8f, 0.8f), // 亮青色
      ImColor(1.0f, 0.5f, 0.2f), // 亮橙色
      ImColor(1.0f, 1.0f, 1.0f), // 亮白色
      ImColor(1.0f, 0.6f, 0.8f), // 亮粉色
  };

  const int colorCount =
      sizeof(highContrastColors) / sizeof(highContrastColors[0]);
  return highContrastColors[teamId % colorCount];
}