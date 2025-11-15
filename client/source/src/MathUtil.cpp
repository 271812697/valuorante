#include "imgui.h"
#include "MathUtil.h"
#include <cmath>

#define M_PI 3.14159265358979323846
bool Vector3::IsNearlyZero(float Tolerance) const {
    return   std::fabs(x) <= Tolerance && std::fabs(y) <= Tolerance && std::fabs(z) <= Tolerance;
}
//struct Vector3 {
//  FLOAT x, y, z;
//
//  bool IsNearlyZero(float Tolerance) const {
//    return fabs(x) <= Tolerance && fabs(y) <= Tolerance && fabs(z) <= Tolerance;
//  }
//};







void GetDBMapScreenPos(Mapinfo Map, ImVec2 DisplaySize, Vector3 Pos,
                       float Pitch, Vector3 &ScreenPos, Vector3 &Direction) {
  ImVec2 MapPos = {float(Map.MapX) + Pos.x - Map.X,
                   float(Map.MapY) + Pos.y - Map.Y};
  ScreenPos = {MapPos.x / Map.W * DisplaySize.x,
               MapPos.y / Map.H * DisplaySize.y};
  Direction = Vector3(ScreenPos.x + std::cos(Pitch * (float)M_PI / 180.0f) * 22,
                      ScreenPos.y + std::sin(Pitch * (float)M_PI / 180) * 22);
}

void GetCGMapScreenPos(Mapinfo Map, ImVec2 DisplaySize, Vector3 Pos,
                       float Pitch, Vector3 &ScreenPos, Vector3 &Direction) {
  ImVec2 MapPos = {float(Map.MapX) * 1.00675f - Pos.y - Map.X,
                   float(Map.MapY) * 0.99805f + Pos.x - Map.Y};
  ScreenPos = {MapPos.x / Map.W * DisplaySize.x,
               MapPos.y / Map.H * DisplaySize.y};
  Direction = Vector3(ScreenPos.x + std::cos(Pitch * (float)M_PI / 180) * 22,
                      ScreenPos.y + std::sin(Pitch * (float)M_PI / 180) * 22);
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