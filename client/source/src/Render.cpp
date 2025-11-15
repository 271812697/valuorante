#include "Socket.h"
#include "Render.h"
#include "XorString.h"

#include "imgui_internal.h"
#include "imgui_edited.h"
UtilsStruct Utils = {};

std::vector<SendPlayerStruct> PlayerList = {};

std::vector<SendItemsStruct> ItemsList = {};

CAimDistance ResultAim = {};

ImColor LevelGetColor(int L) {

  if (L == 1)
    return ImColor(255, 255, 255, 255);
  if (L == 2)
    return ImColor(0, 255, 0, 255);
  if (L == 3)
    return ImColor(0, 100, 255, 255);
  if (L == 4)
    return ImColor(226, 0, 255, 255);
  if (L == 5)
    return ImColor(255, 215, 0, 255);
  if (L == 6)
    return ImColor(255, 0, 0, 255);

  return ImColor(255, 255, 255, 255);
}
BOOL SaveConfig() {
  HANDLE RequestFile =
      CreateFileA(XorString("C:\\Configs"), GENERIC_WRITE, 0, NULL,
                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (RequestFile == INVALID_HANDLE_VALUE)
    return FALSE;

  if (!WriteFile(RequestFile, &settings, sizeof(settings), NULL, NULL))
    return FALSE;

  CloseHandle(RequestFile);

  return TRUE;
}

BOOL LoadConfig() {
  HANDLE RequestFile =
      CreateFileA(XorString("C:\\Configs"), GENERIC_READ, FILE_SHARE_READ, NULL,
                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (RequestFile == INVALID_HANDLE_VALUE)
    return FALSE;

  if (!ReadFile(RequestFile, &settings, sizeof(settings), NULL, NULL))
    return FALSE;

  CloseHandle(RequestFile);

  return TRUE;
}

BOOL WorldToScreen(const Vector3 &Pos, Matrix4x4 Matrix, Vector3 &ToPos) {
  float View = 0.f;

  float SightX = ScreenWidth / 2, SightY = ScreenHeight / 2;

  View = Matrix.Matrix[0][3] * Pos.x + Matrix.Matrix[1][3] * Pos.y +
         Matrix.Matrix[2][3] * Pos.z + Matrix.Matrix[3][3];

  if (View <= 0.01)
    return FALSE;

  ToPos.x =
      SightX + (Matrix.Matrix[0][0] * Pos.x + Matrix.Matrix[1][0] * Pos.y +
                Matrix.Matrix[2][0] * Pos.z + Matrix.Matrix[3][0]) /
                   View * SightX;

  ToPos.y =
      SightY - (Matrix.Matrix[0][1] * Pos.x + Matrix.Matrix[1][1] * Pos.y +
                Matrix.Matrix[2][1] * Pos.z + Matrix.Matrix[3][1]) /
                   View * SightY;

  return TRUE;
}

Vector3 WorldToScreen(const Vector3 &Pos, Matrix4x4 Matrix) {
  FLOAT View = 0.f;

  FLOAT SightX = ScreenWidth / 2, SightY = ScreenHeight / 2;

  View = Matrix.Matrix[0][3] * Pos.x + Matrix.Matrix[1][3] * Pos.y +
         Matrix.Matrix[2][3] * Pos.z + Matrix.Matrix[3][3];

  Vector3 ToPos = {};

  if (View <= 0.01)
    return ToPos;

  ToPos.x =
      SightX + (Matrix.Matrix[0][0] * Pos.x + Matrix.Matrix[1][0] * Pos.y +
                Matrix.Matrix[2][0] * Pos.z + Matrix.Matrix[3][0]) /
                   View * SightX;

  ToPos.y =
      SightY - (Matrix.Matrix[0][1] * Pos.x + Matrix.Matrix[1][1] * Pos.y +
                Matrix.Matrix[2][1] * Pos.z + Matrix.Matrix[3][1]) /
                   View * SightY;

  return ToPos;
}
namespace Bone {
    std::list<INT> _上部 = { Bone::头部 };
    std::list<INT> _右臂 = { Bone::颈部, Bone::右肩, Bone::右肘, Bone::右手 };
    std::list<INT> _左臂 = { Bone::颈部, Bone::左肩, Bone::左肘, Bone::左手 };
    std::list<INT> _脊柱 = { Bone::颈部, Bone::胸部, Bone::肚子, Bone::脊椎, Bone::骨盆 };
    std::list<INT> _右腿 = { Bone::骨盆, Bone::右腿, Bone::右膝, Bone::右脚跟 };
    std::list<INT> _左腿 = { Bone::骨盆, Bone::左腿, Bone::左膝, Bone::左脚跟 };
    std::list<std::list<INT>> 拼接骨骼 = { _上部, _右臂, _左臂, _脊柱, _右腿, _左腿 };
    std::list<INT> _四肢 = { Bone::左肘, Bone::左手, Bone::左膝, Bone::左脚跟,Bone::右肘, Bone::右手, Bone::右膝, Bone::右脚跟 };
    std::list<INT> _双脚 = { Bone::左膝, Bone::左脚跟, Bone::右膝, Bone::右脚跟 };

}
VOID DrawPlayerBone(Matrix4x4 Matrix, Vector3 Bones_pos[66], ImVec4 Color,
                    FLOAT Width) {
  Vector3 Bones_screen[66] = {};

  Vector3 Previous = {};

  Vector3 Current = {};

  Vector3 p1, c1;

  for (auto a : Bone::拼接骨骼) {
    Previous = Vector3(0, 0, 0);

    for (auto bone : a) {
      if (Bones_pos[bone].IsNearlyZero(0.00001f))
        continue;

      if (!WorldToScreen(Bones_pos[bone], Matrix, Bones_screen[bone]))
        continue;

      Current = Bones_screen[bone];

      if (Previous.x == 0.f) {
        Previous = Current;

        continue;
      }

      p1 = Previous;

      c1 = Current;

      DrawLine(p1.x, p1.y, c1.x, c1.y, Color, Width);

      Previous = Current;
    }
  }

  Vector3 HeadPosition = Bones_screen[Bone::头部];

  Vector3 NeckPosition = Bones_screen[Bone::颈部];

  DrawCircle(HeadPosition.x, HeadPosition.y, NeckPosition.y - HeadPosition.y,
             Color, Width);
}

std::string GetProjectName(std::string ProjectName, INT Password) {
  if (ProjectName.find(XorString("鸟窝")) != std::string::npos &&
      settings.BirdNest)
    return XorString("[鸟窝]");

  else if (ProjectName.find(XorString("电脑")) != std::string::npos &&
           settings.Computer) {
    std::string ComputerName = std::format("[电脑:{:03d}]", Password);
    return ComputerName;
  }

  else if (ProjectName.find(XorString("旅行包")) != std::string::npos &&
           settings.ShoppingBag)
    return XorString("[旅行包]");

  else if (ProjectName.find(XorString("服务器")) != std::string::npos &&
           settings.Server)
    return XorString("[服务器]");

  else if (ProjectName.find(XorString("小保险箱")) != std::string::npos &&
           settings.Safebox)
    return XorString("[小保险箱]");

  else if (ProjectName.find(XorString("保险箱")) != std::string::npos &&
           settings.Safebox)
    return XorString("[保险箱]");

  else if (ProjectName.find(XorString("一件衣服")) != std::string::npos &&
           settings.Cloth)
    return XorString("[一件衣服]");

  else if (ProjectName.find(XorString("高级旅行箱")) != std::string::npos &&
           settings.Lxrysuitcase)
    return XorString("[高级旅行箱]");

  else if (ProjectName.find(XorString("医疗物资堆")) != std::string::npos &&
           settings.MedSupplyPile)
    return XorString("[医疗物资堆]");

  else if (ProjectName.find(XorString("野外物资箱")) != std::string::npos &&
           settings.Flightcase)
    return XorString("[野外物资箱]");

  else if (ProjectName.find(XorString("高级储物箱")) != std::string::npos &&
           settings.Flightcase)
    return XorString("[高级储物箱]");

  else if (ProjectName.find(XorString("航空存储箱")) != std::string::npos &&
           settings.AirCargoContainer)
    return XorString("[航空存储箱]");

  return XorString("");
}

BOOL ShouldDisplayItem(INT Quality) {
  switch (Quality) {
  case 1:
    return settings.WhiteItem;
  case 2:
    return settings.GreenItem;
  case 3:
    return settings.BlueItem;
  case 4:
    return settings.PurpleItem;
  case 5:
    return settings.GoldenItem;
  case 6:
    return settings.RedItem;
  default:
    return FALSE;
  }
}

Vector3 GetPrediction(Vector3 TargetSpeed, Vector3 Pos, FLOAT ProjectileSpeed,
                      FLOAT Fall) {
  FLOAT gravity = 9.72f;

  FLOAT drop = 0.5f * gravity * ProjectileSpeed * ProjectileSpeed * 50 * Fall;

  Vector3 Vec3SpeedShift;

  Vec3SpeedShift.x = TargetSpeed.x * ProjectileSpeed * 1.33;

  Vec3SpeedShift.y = TargetSpeed.y * ProjectileSpeed * 1.33;

  Vec3SpeedShift.z = TargetSpeed.z * ProjectileSpeed * 1.33;

  Vector3 predictedBonePos;

  predictedBonePos.x = Vec3SpeedShift.x + Pos.x;

  predictedBonePos.y = Vec3SpeedShift.y + Pos.y;

  predictedBonePos.z = Vec3SpeedShift.z + Pos.z + drop;

  return predictedBonePos;
}

Vector3 PredictPosn(Vector3 TargetSpeed, Vector3 Pos, FLOAT Distance,
                    FLOAT WeaponSpeed) {
  if (WeaponSpeed <= 0)
    WeaponSpeed = 55000.f;

  FLOAT flyTime = Distance / (WeaponSpeed / 100.f);

  FLOAT Fall = 1;

  Vector3 CalcPos =
      GetPrediction(TargetSpeed, Pos, flyTime, Fall); // 计算预判坐标

  return CalcPos;
}

VOID LoopThread() {
  std::vector<uint8_t> Buffer(65536 * 2);

  fd_set readfds;

  timeval timeout{0, 1000};

  while (TRUE) {
    FD_ZERO(&readfds);

    FD_SET(Socket, &readfds);

    int ready = select(Socket + 1, &readfds, NULL, NULL, &timeout);

    if (ready <= 0)
      continue;
   
    int Received =
        recvfrom(Socket, (char *)Buffer.data(), Buffer.size(), 0, 0, 0);

    if (Received <= 0)
      continue;

    std::async(
        std::launch::async,
        [buffer = std::vector(Buffer.begin(), Buffer.begin() + Received)]() {
          std::string_view EncodedData(
              reinterpret_cast<const char *>(buffer.data()), buffer.size());

          ParseComplexData(
              std::vector<uint8_t>(EncodedData.begin(), EncodedData.end()),
              Utils, PlayerList, ItemsList);
        });
  }

  closesocket(Socket);

  WSACleanup();
}

VOID Draw() {
  std::vector<CAimDistance> AimDistanceVector = {};

  float CrosshairX = (float)ScreenWidth / 2;

  float CrosshairY = (float)ScreenHeight / 2;

  Matrix4x4 Matrix = Utils.Matrix;

  bool preShouldDraw = Utils.preShouldDraw;

  Mapinfo Info = Utils.Info;

  FLOAT LocalWeaponSpeed = Utils.LocalWeaponSpeed;

  INT PlayerCount = 0; // 玩家数量

  std::list<TeamCount> Count = {}; // 队伍数量

  for (const auto &PlayerArray : PlayerList) {
    std::string ClassName = PlayerArray.ClassName;
    if (ClassName.empty())
      continue;

    UGPHealth Health = PlayerArray.Health;

    if (Health.Health > 0.f && Health.Health <= Health.MaxHealth) {
      if (ClassName == XorString("玩家")) {
        BOOL Create = TRUE;
        PlayerCount++;
        for (TeamCount Team : Count) {
          if (Team.TeamId == PlayerArray.TeamID) {
            Create = FALSE;
            Team.Count++;
            break;
          }
        }

        if (Create) {
          TeamCount Team = {0};
          Team.TeamId = PlayerArray.TeamID;
          Team.Count++;
          Count.push_back(Team);
        }
      }

      Vector3 Pos = PlayerArray.Pos;
      if (Pos.IsNearlyZero(0.001f))
        continue;

      INT Distence = PlayerArray.Distence;
      if (Distence < 0)
        continue;

      if (settings.MapRender) // 大地图雷达
      {
        if (ClassName != XorString("AI")) {
          if (preShouldDraw == TRUE) {
            std::string Detective = PlayerArray.Detective;
            INT TeamID = PlayerArray.TeamID;
            FLOAT Directionposition = PlayerArray.Directionposition;
            Vector3 ScreenPos = {};
            Vector3 DirectionPos = {};
            if (Info.MapX == 330062)
              GetCGMapScreenPos(Info, {(float)ScreenWidth, (float)ScreenHeight},
                                Pos, Directionposition, ScreenPos,
                                DirectionPos);
            else
              GetDBMapScreenPos(Info, {(float)ScreenWidth, (float)ScreenHeight},
                                Pos, Directionposition, ScreenPos,
                                DirectionPos);
            ImColor TeamColor = GetColorForNumber(TeamID);

            if (ClassName == XorString("玩家")) {
              FLOAT Dis = 13.f;

              ImGui::GetForegroundDrawList()->AddCircleFilled(
                  ImVec2(ScreenPos.x, ScreenPos.y), 7, TeamColor);
              ImGui::GetForegroundDrawList()->AddCircle(
                  ImVec2(ScreenPos.x, ScreenPos.y), 8, ImColor{0, 0, 0, 1}, 0,
                  2);
              DrawTriangle(1.0f, 10.f, 7.0f, DirectionPos, ScreenPos,
                           TeamColor);

              if (settings.MapDetective) // 雷达探员
              {
                DrawString(StringToU8(Detective).c_str(), ScreenPos.x,
                           ScreenPos.y - 27.f, TeamColor, 14.0f);
              }

              if (settings.MapWepon) // 雷达手持
              {
                std::string WeaponName = PlayerArray.WeaponName;
                DrawString(StringToU8(WeaponName).c_str(), ScreenPos.x,
                           ScreenPos.y + 12.f, TeamColor, 14.f);
                Dis += 13.f;
              }

              if (settings.MapArmor) // 玩家头甲
              {
                UGPArmor Armor = PlayerArray.Armor;
                std::string Level =
                    XorString("头:") + std::to_string(Armor.Head) +
                    XorString(" 甲:") + std::to_string(Armor.Armor);
                if (settings.MapTeam || settings.MapDetective)
                  DrawString(StringToU8(Level).c_str(), ScreenPos.x,
                             ScreenPos.y - 41.f, TeamColor, 14.f);
                if (!settings.MapTeam && !settings.MapDetective)
                  DrawString(StringToU8(Level).c_str(), ScreenPos.x,
                             ScreenPos.y - 27.f, TeamColor, 14.f);
              }

              if (settings.MapName) // 雷达名称
              {
                DrawString(PlayerArray.PlayerName, ScreenPos.x,
                           ScreenPos.y + Dis, TeamColor, 14.0f);
              }

              if (settings.MapTeam && !settings.MapDetective) // 雷达队伍
              {
                std::string BetaName =
                    XorString("(") + std::to_string(TeamID) + XorString(")");
                DrawString(StringToU8(BetaName).c_str(), ScreenPos.x,
                           ScreenPos.y - 27.f, TeamColor, 14.f);
              }

              if (settings.MapTeam && settings.MapDetective) // 雷达探员|队伍
              {
                std::string BetaName = XorString("(") + std::to_string(TeamID) +
                                       XorString(")") + Detective;
                DrawString(StringToU8(BetaName).c_str(), ScreenPos.x,
                           ScreenPos.y - 27.f, TeamColor, 14.f);
              }
            } else // BOOS
            {
              ImGui::GetForegroundDrawList()->AddCircleFilled(
                  ImVec2(ScreenPos.x, ScreenPos.y), 8,
                  ImColor(1.0f, 0.2f, 0.2f));
              ImGui::GetForegroundDrawList()->AddCircle(
                  ImVec2(ScreenPos.x, ScreenPos.y), 9, ImColor{0, 0, 0, 1}, 0,
                  2);
              DrawTriangle(1.0f, 10.f, 7.0f, DirectionPos, ScreenPos,
                           ImColor(1.0f, 0.2f, 0.2f));
              DrawString(StringToU8(ClassName).c_str(), ScreenPos.x,
                         ScreenPos.y - 27.f, ImColor(1.0f, 0.2f, 0.2f), 14.0f);
            }
          }
        }
      }

      Vector3 HeadPos = {};
      HeadPos.x = Pos.x;
      HeadPos.y = Pos.y;
      HeadPos.z = Pos.z + 175.f;

      Vector3 Ws2Pos = WorldToScreen(Pos, Matrix);

      Vector3 Ws2HeadPos = WorldToScreen(HeadPos, Matrix);

      if (Ws2Pos.x != 0.f && Ws2HeadPos.x != 0.f) {
        FLOAT Height = Ws2HeadPos.y - Ws2Pos.y;

        FLOAT Width = Height / 2.f;

        ImColor PlayerBoneColor = {};
        ImColor BotBoneColor = {};

        ImColor PlayerFontColor = {};
        ImColor BotFontColor = {};

        if (ClassName == XorString("玩家")) {
          PlayerBoneColor = settings.PlayerNoVisibleColor;
          PlayerFontColor = settings.PlayerNoVisibleColor;
        } else if (ClassName == XorString("AI")) {
          BotBoneColor = settings.BotNoVisibleColor;
          BotFontColor = settings.BotNoVisibleColor;
        } else {
          BotBoneColor = ImColor(50, 255, 247); // BOSS
          BotFontColor = ImColor(50, 255, 247);
        }

        BOOL IsVisable = PlayerArray.IsVisable;

        if (IsVisable) {
          PlayerBoneColor = settings.PlayerVisibleColor;
          BotBoneColor = settings.BotVisibleColor;
        }

        Vector3 Postion = PlayerArray.Postion;

        Vector3 TargetSpeed = PlayerArray.TargetSpeed;

        Vector3 Ws2AimPos = WorldToScreen(Postion, Matrix);

        if (!Ws2AimPos.IsNearlyZero(0.001f)) {
          FLOAT AimDistence = sqrt(pow(std::abs(CrosshairX - Ws2AimPos.x), 2.0f) +
                                   pow(std::abs(CrosshairY - Ws2AimPos.y), 2.0f));

          if (AimDistence < 120.f && Distence < 200 && IsVisable) {
            CAimDistance AimDistance = {0};
            AimDistance.Pos = Postion;
            AimDistance.Distence = Distence;
            AimDistance.Distanc = AimDistence;
            AimDistance.TargetSpeed = TargetSpeed;
            AimDistanceVector.emplace_back(AimDistance);
          }

          CAimDistance AimDistance = {0};
          FLOAT Distanc = FLT_MAX;
          for (std::vector<CAimDistance>::iterator iter =
                   AimDistanceVector.begin();
               iter != AimDistanceVector.end(); iter++) {
            if ((*iter).Distanc < Distanc) {
              AimDistance = (*iter);
              Distanc = AimDistance.Distanc;
            }
          }
          ResultAim = AimDistance;
        }

        if (!preShouldDraw) {
          if (ClassName == XorString("玩家")) {
            FLOAT Dis = 0.f;

            if (Distence <= settings.MaxPlayerDistene) {
              std::string Detective = PlayerArray.Detective;

              INT TeamID = PlayerArray.TeamID;

              if (settings.PlayerBOX) // 玩家方框
              {
                DrawRectangleB(Ws2HeadPos.x - Width / 2.f, Ws2Pos.y, Width,
                               Height, PlayerBoneColor, 2);
              }

              if (settings.PlayerBone) // 玩家骨骼
              {
                Vector3 BonePos[18];
                for (int i = 0; i < KEY_BONE_COUNT; i++) {
                  if (PlayerArray.ValidFlags & (1 << i)) {
                    BonePos[KEY_BONES[i]] = PlayerArray.BonePos[i];
                  }
                }
                DrawPlayerBone(Matrix, BonePos, PlayerBoneColor, 1.5f);
              }

              if (settings.PlayerDis) // 玩家距离
              {
                std::string PlayerDistence = XorString("[") +
                                             std::to_string(Distence) +
                                             XorString(" m]");
                DrawString(PlayerDistence.c_str(), Ws2Pos.x, Ws2Pos.y + Dis,
                           PlayerFontColor, 15.f);
                Dis += 15.f;
              }

              if (settings.PlayerHealth) // 玩家血量
              {
                ImColor HealColor = ImColor(0, 217, 0);

                if (Health.Health <= Health.MaxHealth / 2)
                  HealColor = ImColor(255, 185, 14);

                if (Health.Health <= Health.MaxHealth / 4)
                  HealColor = ImColor(238, 65, 65);

                DrawFilledRectangle(Ws2HeadPos.x + (Width / 1.4f), Ws2Pos.y,
                                    2.0f, Height,
                                    ImVec4(30 / 255, 30 / 255, 0, 1));

                DrawFilledRectangle(
                    Ws2HeadPos.x + (Width / 1.4f), Ws2Pos.y, 2.0f,
                    Health.Health / Health.MaxHealth * Height, HealColor);
              }

              if (settings.PlayerDetective && !settings.PlayerTeam) // 玩家探员
              {
                DrawString(StringToU8(Detective).c_str(), Ws2HeadPos.x,
                           Ws2HeadPos.y - 17.f, PlayerFontColor, 15.f);
              }

              if (settings.PlayerTeam && !settings.PlayerDetective) // 玩家队伍
              {
                std::string BetaName =
                    XorString("(") + std::to_string(TeamID) + XorString(")");
                DrawString(StringToU8(BetaName).c_str(), Ws2HeadPos.x,
                           Ws2HeadPos.y - 17.f, LevelGetColor(TeamID), 15.f);
              }

              if (settings.PlayerTeam &&
                  settings.PlayerDetective) // 玩家探员|队伍
              {
                std::string BetaName = XorString("(") + std::to_string(TeamID) +
                                       XorString(")") + Detective;
                DrawString(StringToU8(BetaName).c_str(), Ws2HeadPos.x,
                           Ws2HeadPos.y - 17.f, PlayerFontColor, 15.f);
              }

              if (settings.PlayerWepon) // 玩家手持
              {
                std::string WeaponName = PlayerArray.WeaponName;
                DrawString(StringToU8(WeaponName).c_str(), Ws2Pos.x,
                           Ws2Pos.y + Dis, PlayerFontColor, 15.f);
                Dis += 15.f;
              }

              if (settings.PlayerArmor) // 玩家头甲
              {

                UGPArmor Armor = PlayerArray.Armor;
                std::string Level =
                    XorString("头:") + std::to_string(Armor.Head) +
                    XorString(" 甲:") + std::to_string(Armor.Armor);
                if (settings.PlayerTeam || settings.PlayerDetective)
                  DrawString(StringToU8(Level).c_str(), Ws2HeadPos.x,
                             Ws2HeadPos.y - 32.f, LevelGetColor(Armor.Head),
                             15.f);
                if (!settings.PlayerTeam && !settings.PlayerDetective)
                  DrawString(StringToU8(Level).c_str(), Ws2HeadPos.x,
                             Ws2HeadPos.y - 17.f, LevelGetColor(Armor.Armor),
                             15.f);
              }

              if (settings.PlayerNmae) // 玩家名字
              {
                std::string EntityName = PlayerArray.PlayerName;
                DrawString(EntityName, Ws2Pos.x, Ws2Pos.y + Dis,
                           PlayerFontColor, 15.f);
                Dis += 15.f;
              }
            }
          } else {
            FLOAT Dis = 0.f;

            if (Distence <= settings.MaxBotDistene) {
              if (settings.BotBOX) // Bot方框
              {
                DrawRectangleB(Ws2HeadPos.x - Width / 2.f, Ws2Pos.y, Width,
                               Height, BotBoneColor, 2);
              }

              if (settings.BotBone) // Bot骨骼
              {
                Vector3 BonePos[18];
                for (int i = 0; i < KEY_BONE_COUNT; i++) {
                  if (PlayerArray.ValidFlags & (1 << i)) {
                    BonePos[KEY_BONES[i]] = PlayerArray.BonePos[i];
                  }
                }
                DrawPlayerBone(Matrix, BonePos, BotBoneColor, 1.5f);
              }

              if (settings.BotDis) // Bot距离
              {
                std::string AIDistence = XorString("[") +
                                         std::to_string(Distence) +
                                         XorString(" m]");
                DrawString(AIDistence.c_str(), Ws2Pos.x, Ws2Pos.y + Dis,
                           BotFontColor, 15.f);
                Dis += 15.f;
              }

              if (settings.BotHealth) // Bot血量
              {
                ImColor HealColor = ImColor(0, 217, 0);

                if (Health.Health <= Health.MaxHealth / 2)
                  HealColor = ImColor(255, 185, 14);

                if (Health.Health <= Health.MaxHealth / 4)
                  HealColor = ImColor(238, 65, 65);

                DrawFilledRectangle(Ws2HeadPos.x + (Width / 1.4f), Ws2Pos.y,
                                    2.0f, Height,
                                    ImVec4(30 / 255, 30 / 255, 0, 1));

                DrawFilledRectangle(
                    Ws2HeadPos.x + (Width / 1.4f), Ws2Pos.y, 2.0f,
                    Health.Health / Health.MaxHealth * Height, HealColor);
              }

              if (settings.BotInfo) // Bot信息
              {
                DrawString(StringToU8(ClassName).c_str(), Ws2HeadPos.x,
                           Ws2HeadPos.y - 17.f, BotFontColor, 15.f);
              }

              if (settings.BotWepon) // Bot手持
              {
                std::string WeaponName = PlayerArray.WeaponName;
                DrawString(StringToU8(WeaponName).c_str(), Ws2Pos.x,
                           Ws2Pos.y + Dis, BotFontColor, 15.f);
                Dis += 15.f;
              }

              if (settings.BotName) // Bot名字
              {
                std::string BotName = PlayerArray.BotName;
                DrawString(BotName, Ws2Pos.x, Ws2Pos.y + Dis, BotFontColor,
                           15.f);
                Dis += 15.f;
              }
            }
          }
        }
      }
    }
  }

  BOOL FightMode = Utils.FightMode;

  if (!FightMode && !preShouldDraw &&
      (settings.Item || settings.Computer || settings.Booty)) {
    for (const auto &ItemsArray : ItemsList) {
      Vector3 Pos = ItemsArray.Pos;
      if (Pos.IsNearlyZero(0.001f))
        continue;

      INT Type = ItemsArray.Type;
      if (Type <= 0)
        continue;

      Vector3 Ws2Pos = WorldToScreen(Pos, Matrix);

      if (Ws2Pos.x != 0.0f && Ws2Pos.y != 0.0f) {
        INT Distance = ItemsArray.Distance;
        if (Distance < 0)
          continue;

        if (Type == 1) // 物资
        {
          std::string ItemName = ItemsArray.ItemName;
          if (ItemName.empty())
            continue;

          INT ItemMoney = ItemsArray.ItemMoney;
          if (ItemMoney <= 5)
            continue;

          INT ItemQuality = ItemsArray.ItemQuality;
          if (ItemQuality == 0)
            continue;

          if (ShouldDisplayItem(ItemQuality)) {
            ImColor Color = {};

            if (ItemQuality == 1)
              Color = ImColor(247, 247, 247);

            if (ItemQuality == 2)
              Color = ImColor(20, 217, 20);

            if (ItemQuality == 3)
              Color = ImColor(72, 118, 255);

            if (ItemQuality == 4)
              Color = ImColor(224, 102, 255);

            if (ItemQuality == 5)
              Color = ImColor(255, 255, 0);

            if (ItemQuality == 6)
              Color = ImColor(255, 48, 48);

            INT RmbK = ItemMoney / 1000;

            INT Remainder = (ItemMoney % 1000) / 100;

            std::string Name = ItemName + (" ") + std::to_string(RmbK) + (".") +
                               std::to_string(Remainder) + ("K") + ("[") +
                               std::to_string(Distance) + ("m]");

            DrawString(StringToU8(Name).c_str(), Ws2Pos.x, Ws2Pos.y, Color,
                       14.f);
          }
        }

        if (Type == 2 && settings.Container &&
            Distance <= settings.MaxContainerDistence) // 容器
        {
          std::string Gname = ItemsArray.ProjectName;
          if (Gname.empty())
            continue;

          INT Password = ItemsArray.Password;
          std::string ProjectName = GetProjectName(Gname, Password);
          if (ProjectName.empty())
            continue;

          ImColor Color = {};
          if (ProjectName == XorString("[小保险箱]") ||
              ProjectName == XorString("[保险箱]"))
            Color = ImColor(255, 48, 48);
          else
            Color = ImColor(203, 183, 0);

          std::string ContainerName = ProjectName + XorString("[") +
                                      std::to_string(Distance) +
                                      XorString(" m]");
          DrawString(StringToU8(ContainerName).c_str(), Ws2Pos.x, Ws2Pos.y,
                     Color, 14.f);
        }

        if (Type == 3 && settings.Booty &&
            Distance <= settings.MaxContainerDistence) // 战利品
        {
          ULONG64 DeadBoxType = ItemsArray.DeadBoxType;

          if (DeadBoxType > 0 && settings.PlayerBooty) {
            std::string BootyName = XorString("玩家|战利品[") +
                                    std::to_string(Distance) + XorString(" m]");

            DrawString(StringToU8(BootyName).c_str(), Ws2Pos.x, Ws2Pos.y,
                       ImColor(50, 215, 217), 14.f);
          } else if (DeadBoxType <= 0 && settings.BotBooty) {
            std::string BootyName = XorString("AI|战利品[") +
                                    std::to_string(Distance) + XorString(" m]");

            DrawString(StringToU8(BootyName).c_str(), Ws2Pos.x, Ws2Pos.y,
                       ImColor(50, 215, 217), 14.f);
          }
        }
      }
    }
  }

  if (FightMode)
    DrawTableString(StringToU8(XorString("~战斗模式:开")).c_str(), 5, 43,
                    ImColor(255, 48, 48), 18.f);
  else
    DrawTableString(StringToU8(XorString("~战斗模式:关")).c_str(), 5, 43,
                    ImColor(247, 247, 247), 18.f);

  std::string CountString =
      XorString("剩余队伍: ") + std::to_string(Count.size()) +
      XorString(" 剩余玩家: ") + std::to_string(PlayerCount);
  DrawString(StringToU8(CountString).c_str(), ScreenWidth / 2,
             ScreenHeight - 40.f, ImColor(247, 247, 247), 20.f);

  ULONG GetDataPtr = Utils.GetDataPtr;
  std::cout << GetDataPtr << std::endl;

  std::string String = XorString("");
  ImColor StringColor = {};

  if (GetDataPtr == 0x1412B12E0) {
    String = XorString("账号状态:未加密");
    StringColor = ImColor(247, 247, 247);
  } else {
    String = XorString("账号状态:已加密");
    StringColor = ImColor(255, 48, 48);
  }

  DrawTableString(StringToU8(String).c_str(), 5, 23, StringColor, 18.f);

  BOOL AimBoting = Utils.AimBot;
  float ClosestX = 99999.f;
  float ClosestY = 99999.f;
  if (AimBoting) {
    Vector3 Postion = ResultAim.Pos;
    if (!Postion.IsNearlyZero(0.001f)) {
      INT Distence = ResultAim.Distence;
      Vector3 TargetSpeed = ResultAim.TargetSpeed;
      Vector3 PredictPos =
          PredictPosn(TargetSpeed, Postion, Distence, LocalWeaponSpeed); // 预判
      Vector3 Ws2Predict = WorldToScreen(PredictPos, Matrix);
      ;
      if (Ws2Predict.x != 0.0f && Ws2Predict.y != 0.0f && Ws2Predict.x >= 0 &&
          Ws2Predict.x <= ScreenWidth && Ws2Predict.y >= 0 &&
          Ws2Predict.y <= ScreenHeight) {
        ClosestX = Ws2Predict.x;
        ClosestY = Ws2Predict.y;
        if (ClosestX != 99999.f && ClosestY != 99999.f) {
          int targetX = ClosestX - CrosshairX;
          int targetY = ClosestY - CrosshairY;
          int randomOffsetX = (rand() % 20) - 10;
          int randomOffsetY = (rand() % 20) - 10;
          int ctrl1X = targetX * 0.4f + randomOffsetX;
          int ctrl1Y = targetY * 0.4f + randomOffsetY;
          int ctrl2X = targetX * 0.6f - randomOffsetX;
          int ctrl2Y = targetY * 0.6f - randomOffsetY;
          int moveTime = 15 + (abs(targetX) + abs(targetY)) / 20;
          // kmNet_mouse_move_beizer(targetX, targetY, moveTime, ctrl1X, ctrl1Y,
          // ctrl2X, ctrl2Y);
        }
      }
    }
  }
}

VOID Rander() {
  if (GetAsyncKeyState(VK_HOME) & 1) {
    ShowMenu = !ShowMenu;
  }

  if (ShowMenu == TRUE) {
    ImGuiIO &ImgIo = ImGui::GetIO();
    (VOID) ImgIo;

    ImGuiStyle *Style = &ImGui::GetStyle();

    Style->WindowPadding = ImVec2(0, 0);

    Style->ItemSpacing = ImVec2(20, 0);

    Style->WindowBorderSize = 0;

    Style->ScrollbarSize = 9.f;

    ImGui::Begin(XorString("1"), 0,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);
    {
      const ImVec2 &Pos = ImGui::GetWindowPos();

      const ImVec2 &Spacing = Style->ItemSpacing;

      ImGui::GetBackgroundDrawList()->AddRectFilled(
          Pos, Pos + ImVec2(c::bg::size), ImGui::GetColorU32(c::bg::background),
          c::bg::rounding + 1);

      ImGui::GetBackgroundDrawList()->AddRectFilled(
          Pos, Pos + ImVec2(100, c::bg::size.y),
          ImGui::GetColorU32(c::bg::border), c::bg::rounding,
          ImDrawFlags_RoundCornersLeft);

      ImGui::GetBackgroundDrawList()->AddLine(
          Pos + ImVec2(0, 100), Pos + ImVec2(100, 100),
          ImGui::GetColorU32(c::widget::background), 1.f);

      ImGui::SetCursorPos(ImVec2((100 - 47) / 2, 100 + (47 / 2)));

      ImGui::BeginGroup();
      {
        if (Edited::Tab(0 == TabPage, 1, XorString("b"), ImVec2(47, 47)))
          TabPage = 0;

        if (Edited::Tab(1 == TabPage, 2, XorString("f"), ImVec2(47, 47)))
          TabPage = 1;

        // if (Edited::Tab(2 == TabPage, 3, XorString("c"), ImVec2(47, 47)))
        // TabPage = 2;

        if (Edited::Tab(3 == TabPage, 4, XorString("o"), ImVec2(47, 47)))
          TabPage = 3;
      }

      ImGui::EndGroup();

      ImGui::SetCursorPos(ImVec2(100 + Spacing.x, 0));

      TabAlpha = ImClamp(TabAlpha + (4.f * ImGui::GetIO().DeltaTime *
                                     (TabPage == ActiveTab ? 1.f : -1.f)),
                         0.f, 1.f);

      if (TabAlpha == 0.f)
        ActiveTab = TabPage;

      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, TabAlpha * Style->Alpha);

      ImGui::BeginChild(XorString("2"),
                        ImVec2(c::bg::size) - ImVec2(100 + Spacing.x, 0));
      {
        if (ActiveTab == 0) // 人物透视
        {
          ImGui::BeginGroup();
          {
            Edited::BeginChild(
                StringToU8(XorString("玩家")).c_str(),
                ImVec2(c::bg::size.x - (100 + Spacing.x * 3), 0) / 2);
            {
              Edited::Checkbox(StringToU8(XorString("显示方框")).c_str(),
                               &settings.PlayerBOX);

              Edited::Checkbox(StringToU8(XorString("显示骨骼")).c_str(),
                               &settings.PlayerBone);

              Edited::Checkbox(StringToU8(XorString("显示距离")).c_str(),
                               &settings.PlayerDis);

              Edited::Checkbox(StringToU8(XorString("显示血量")).c_str(),
                               &settings.PlayerHealth);

              Edited::Checkbox(StringToU8(XorString("显示探员")).c_str(),
                               &settings.PlayerDetective);

              Edited::Checkbox(StringToU8(XorString("显示手持")).c_str(),
                               &settings.PlayerWepon);

              Edited::Checkbox(StringToU8(XorString("显示头甲")).c_str(),
                               &settings.PlayerArmor);

              Edited::Checkbox(StringToU8(XorString("显示队伍")).c_str(),
                               &settings.PlayerTeam);

              Edited::Checkbox(StringToU8(XorString("显示名字")).c_str(),
                               &settings.PlayerNmae);

              Edited::ColorEdit4(StringToU8(XorString("可见颜色")).c_str(),
                                 settings.PlayerVisibleColor,
                                 ImGuiColorEditFlags_NoInputs);

              Edited::ColorEdit4(StringToU8(XorString("不可见颜色")).c_str(),
                                 settings.PlayerNoVisibleColor,
                                 ImGuiColorEditFlags_NoInputs);

              Edited::SliderInt(StringToU8(XorString("最远距离")).c_str(),
                                &settings.MaxPlayerDistene, 50, 1000);
            }

            Edited::EndChild();
          }

          ImGui::EndGroup();

          ImGui::SameLine();

          ImGui::BeginGroup();
          {
            Edited::BeginChild(
                StringToU8(XorString("人机")).c_str(),
                ImVec2(c::bg::size.x - (100 + Spacing.x * 3), 0) / 2);
            {
              Edited::Checkbox(StringToU8(XorString("显示方框")).c_str(),
                               &settings.BotBOX);

              Edited::Checkbox(StringToU8(XorString("显示骨骼")).c_str(),
                               &settings.BotBone);

              Edited::Checkbox(StringToU8(XorString("显示距离")).c_str(),
                               &settings.BotDis);

              Edited::Checkbox(StringToU8(XorString("显示血量")).c_str(),
                               &settings.BotHealth);

              Edited::Checkbox(StringToU8(XorString("显示信息")).c_str(),
                               &settings.BotInfo);

              Edited::Checkbox(StringToU8(XorString("显示手持")).c_str(),
                               &settings.BotWepon);

              Edited::Checkbox(StringToU8(XorString("显示名字")).c_str(),
                               &settings.BotName);

              Edited::ColorEdit4(StringToU8(XorString("可见颜色")).c_str(),
                                 settings.BotVisibleColor,
                                 ImGuiColorEditFlags_NoInputs);

              Edited::ColorEdit4(StringToU8(XorString("不可见颜色")).c_str(),
                                 settings.BotNoVisibleColor,
                                 ImGuiColorEditFlags_NoInputs);

              Edited::SliderInt(StringToU8(XorString("最远距离")).c_str(),
                                &settings.MaxBotDistene, 50, 1000);
            }

            Edited::EndChild();
          }

          ImGui::EndGroup();
        }

        else if (ActiveTab == 1) // 物资透视
        {
          ImGui::BeginGroup();
          {
            Edited::BeginChild(
                StringToU8(XorString("物资")).c_str(),
                ImVec2(c::bg::size.x - (100 + Spacing.x * 3), 0) / 2);
            {
              Edited::Checkbox(StringToU8(XorString("显示物资")).c_str(),
                               &settings.Item);

              Edited::CheckboxPicker(
                  StringToU8(XorString("显示白级物品")).c_str(),
                  &settings.WhiteItem, ImColor(247, 247, 247), 0);

              Edited::CheckboxPicker(
                  StringToU8(XorString("显示绿级物品")).c_str(),
                  &settings.GreenItem, ImColor(20, 217, 20), 0);

              Edited::CheckboxPicker(
                  StringToU8(XorString("显示蓝级物品")).c_str(),
                  &settings.BlueItem, ImColor(72, 118, 255), 0);

              Edited::CheckboxPicker(
                  StringToU8(XorString("显示紫级物品")).c_str(),
                  &settings.PurpleItem, ImColor(224, 102, 255), 0);

              Edited::CheckboxPicker(
                  StringToU8(XorString("显示金级物品")).c_str(),
                  &settings.GoldenItem, ImColor(255, 255, 0), 0);

              Edited::CheckboxPicker(
                  StringToU8(XorString("显示红级物品")).c_str(),
                  &settings.RedItem, ImColor(255, 48, 48), 0);

              Edited::Checkbox(StringToU8(XorString("显示战利品")).c_str(),
                               &settings.Booty);

              if (settings.Booty) {
                Edited::Checkbox(StringToU8(XorString("玩家战利品")).c_str(),
                                 &settings.PlayerBooty);

                Edited::Checkbox(StringToU8(XorString("人机战利品")).c_str(),
                                 &settings.BotBooty);

                if (settings.Booty)
                  Edited::SliderInt(StringToU8(XorString("最远距离")).c_str(),
                                    &settings.MaxBootyDistence, 50, 1000);
              }
            }

            Edited::EndChild();
          }

          ImGui::EndGroup();

          ImGui::SameLine();

          ImGui::BeginGroup();
          {
            Edited::BeginChild(
                StringToU8(XorString("容器")).c_str(),
                ImVec2(c::bg::size.x - (100 + Spacing.x * 3), 0) / 2);
            {
              Edited::Checkbox(StringToU8(XorString("显示容器")).c_str(),
                               &settings.Container);

              Edited::Checkbox(StringToU8(XorString("鸟窝")).c_str(),
                               &settings.BirdNest);

              Edited::Checkbox(StringToU8(XorString("电脑")).c_str(),
                               &settings.Computer);

              Edited::Checkbox(StringToU8(XorString("旅行包")).c_str(),
                               &settings.ShoppingBag);

              Edited::Checkbox(StringToU8(XorString("服务器")).c_str(),
                               &settings.Server);

              Edited::Checkbox(StringToU8(XorString("保险箱")).c_str(),
                               &settings.Safebox);

              Edited::Checkbox(StringToU8(XorString("一件衣服")).c_str(),
                               &settings.Cloth);

              Edited::Checkbox(StringToU8(XorString("高级旅行箱")).c_str(),
                               &settings.Lxrysuitcase);

              Edited::Checkbox(StringToU8(XorString("医疗物资堆")).c_str(),
                               &settings.MedSupplyPile);

              Edited::Checkbox(StringToU8(XorString("野外物资箱")).c_str(),
                               &settings.Militarybox);

              Edited::Checkbox(StringToU8(XorString("高级储物箱")).c_str(),
                               &settings.Flightcase);

              Edited::Checkbox(StringToU8(XorString("航空存储箱")).c_str(),
                               &settings.AirCargoContainer);

              Edited::SliderInt(StringToU8(XorString("最远距离")).c_str(),
                                &settings.MaxContainerDistence, 50, 1000);
            }

            Edited::EndChild();
          }

          ImGui::EndGroup();
        }

        else if (ActiveTab == 2) // 自瞄
        {
          ImGui::BeginGroup();
          {
            Edited::BeginChild(
                StringToU8(XorString("瞄准")).c_str(),
                ImVec2(c::bg::size.x - (100 + Spacing.x * 3), 0) / 2);
            {
              Edited::Checkbox(StringToU8(XorString("预判瞄准")).c_str(),
                               &settings.Aimbot);

              if (settings.Aimbot) {
                Edited::Checkbox(StringToU8(XorString("障碍判断")).c_str(),
                                 &settings.IsVisable);

                Edited::Checkbox(StringToU8(XorString("显示范围")).c_str(),
                                 &settings.DrawFOV);

                Edited::HotkeyButton(StringToU8(XorString("瞄准按键")).c_str(),
                                     &settings.Aimkey, ImVec2(83, 24));

                const char *Items[6]{"Head",    "Neck", "Chest",
                                     "Stomach", "Foot", "\0"};

                Edited::Combo(StringToU8(XorString("瞄准部位")).c_str(),
                              &settings.AimBone, Items, IM_ARRAYSIZE(Items), 3);

                Edited::SliderInt(StringToU8(XorString("自瞄速度")).c_str(),
                                  &settings.AimSpeed, 10, 50);

                Edited::SliderInt(StringToU8(XorString("瞄准范围")).c_str(),
                                  &settings.FOV, 10, 300);

                Edited::SliderInt(StringToU8(XorString("最远距离")).c_str(),
                                  &settings.MaxAimDistence, 0, 1000);
              }
            }

            Edited::EndChild();
          }

          ImGui::EndGroup();
        }

        else if (ActiveTab == 3) // 杂项
        {
          ImGui::BeginGroup();
          {
            Edited::BeginChild(
                StringToU8(XorString("大地图雷达")).c_str(),
                ImVec2(c::bg::size.x - (100 + Spacing.x * 3), 0) / 2);
            {
              Edited::Checkbox(StringToU8(XorString("显示雷达")).c_str(),
                               &settings.MapRender);

              Edited::Checkbox(StringToU8(XorString("显示探员")).c_str(),
                               &settings.MapDetective);

              Edited::Checkbox(StringToU8(XorString("显示名称")).c_str(),
                               &settings.MapName);

              Edited::Checkbox(StringToU8(XorString("显示手持")).c_str(),
                               &settings.MapWepon);

              Edited::Checkbox(StringToU8(XorString("显示头甲")).c_str(),
                               &settings.MapArmor);

              Edited::Checkbox(StringToU8(XorString("显示队伍")).c_str(),
                               &settings.MapTeam);
            }

            Edited::EndChild();

            Edited::BeginChild(
                StringToU8(XorString("杂项")).c_str(),
                ImVec2(c::bg::size.x - (100 + Spacing.x * 3), 0) / 2);
            {
              if (Edited::Button(StringToU8(XorString("配置选项")).c_str(),
                                 StringToU8(XorString("保存配置")).c_str(),
                                 ImVec2(83, 24))) {
                SaveConfig();
              }
            }

            Edited::EndChild();
          }

          ImGui::EndGroup();
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + Spacing.x);
      }

      ImGui::EndChild();

      ImGui::PopStyleVar();
    }

    ImGui::End();
  }

  Draw();

  std::string FPSName =
      XorString("FPS:") + std::to_string((int)ImGui::GetIO().Framerate);
  DrawTableString(FPSName, 5, 5, ImColor(247, 247, 247), 18.f);

  if (settings.Aimbot && settings.DrawFOV) {
    DrawCircle(ScreenWidth / 2, ScreenHeight / 2, settings.FOV,
               ImColor(255, 255, 255, 255), 1.2);

    DrawCircle(ScreenWidth / 2, ScreenHeight / 2, 1,
               ImColor(255, 255, 255, 255), 3);
  }

  if (GetAsyncKeyState(VK_END)) {
    ExitProcess(0);
  }
}
