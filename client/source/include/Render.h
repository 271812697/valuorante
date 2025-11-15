#pragma once
#include "Overlay.h"
#include "MathUtil.h"
#include <format>
#include <future>
#include <mutex>
#include <string>
#include "imgui.h"

struct Config {
  // 玩家
  bool PlayerBOX = false;
  bool PlayerBone = false;
  bool PlayerDis = false;
  bool PlayerHealth = false;
  bool PlayerDetective = false;
  bool PlayerWepon = false;
  bool PlayerArmor = false;
  bool PlayerTeam = false;
  bool PlayerNmae = false;
  int MaxPlayerDistene = 500;
  // 人机
  bool BotBOX = false;
  bool BotBone = false;
  bool BotDis = false;
  bool BotHealth = false;
  bool BotInfo = false;
  bool BotWepon = false;
  bool BotName = false;
  int MaxBotDistene = 100;
  // 物资
  bool Item = false;
  bool WhiteItem = false;
  bool GreenItem = false;
  bool BlueItem = false;
  bool PurpleItem = false;
  bool GoldenItem = false;
  bool RedItem = false;
  // 容器
  bool Container = false;
  bool Booty = false;
  bool PlayerBooty = false;
  bool BotBooty = false;
  bool BirdNest = false;
  bool Computer = false;
  bool ShoppingBag = false;
  bool Server = false;
  bool Safebox = false;
  bool Cloth = false;
  bool Lxrysuitcase = false;
  bool MedSupplyPile = false;
  bool Militarybox = false;
  bool Flightcase = false;
  bool AirCargoContainer = false;
  int MaxContainerDistence = 100;
  int MaxBootyDistence = 100;
  // 自瞄
  bool Aimbot = false;
  bool IsVisable = true;
  bool DrawFOV = false;
  int AimBone = false;
  int FOV = 120;
  int AimSpeed = 30;
  int Aimkey = 0;
  int MaxAimDistence = 300;

  // 大地图雷达
  bool MapRender = false;
  bool MapDetective = false;
  bool MapName = false;
  bool MapWepon = false;
  bool MapArmor = false;
  bool MapTeam = false;

  ImVec4 PlayerVisibleColor = ImColor(255, 48, 48);
  ImVec4 PlayerNoVisibleColor = ImColor(247, 247, 247);
  ImVec4 BotVisibleColor = ImColor(255, 48, 48);
  ImVec4 BotNoVisibleColor = ImColor(245, 180, 20);

};
inline Config settings;

struct Matrix4x4 {
  float Matrix[4][4];
};

struct UGPHealth {
  float Health;
  float MaxHealth;
};

struct UGPArmor {
  INT Head;
  INT Armor;
};

struct TeamCount {
  INT TeamId;
  INT Count;
};

#pragma pack(push, 1)
struct SendPlayerStruct {
  UGPHealth Health;
  UGPArmor Armor;
  char ClassName[18];
  char Detective[18];
  char WeaponName[18];
  char PlayerName[32];
  char BotName[32];
  Vector3 Pos;
  Vector3 Postion;
  Vector3 TargetSpeed;
  float Directionposition;
  INT Distence;
  INT TeamID;
  BOOL IsVisable;
  Vector3 BonePos[KEY_BONE_COUNT];
  INT ValidFlags;
};

struct SendItemsStruct {
  ULONG64 DeadBoxType;
  Vector3 Pos;
  INT Type;
  INT Distance;
  char ItemName[32];
  char ProjectName[18];
  INT ItemMoney;
  INT ItemQuality;
  INT Password;
};

struct UtilsStruct {
  Matrix4x4 Matrix;
  BOOL FightMode;
  BOOL AimBot;
  ULONG64 GetDataPtr;
  float LocalWeaponSpeed;
  bool preShouldDraw;
  Mapinfo Info;
};
#pragma pack(pop)

struct CAimDistance {
  Vector3 Pos;
  Vector3 TargetSpeed;
  INT Distence;
  float Distanc;
};




ImColor LevelGetColor(int L);
BOOL SaveConfig();

BOOL LoadConfig();

BOOL WorldToScreen(const Vector3& Pos, Matrix4x4 Matrix, Vector3& ToPos);

Vector3 WorldToScreen(const Vector3& Pos, Matrix4x4 Matrix);

void DrawPlayerBone(Matrix4x4 Matrix, Vector3 Bones_pos[66], ImVec4 Color,
    float Width);

std::string GetProjectName(std::string ProjectName, INT Password);

BOOL ShouldDisplayItem(INT Quality);

Vector3 GetPrediction(Vector3 TargetSpeed, Vector3 Pos, float ProjectileSpeed,
    float Fall);
Vector3 PredictPosn(Vector3 TargetSpeed, Vector3 Pos, float Distance,
    float WeaponSpeed);

void LoopThread();

void Draw();
void Rander();
