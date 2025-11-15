#include "Socket.h"
#include <thread>
#include <string.h>   

#include "Drive.h"
#include "Offsets.h"	
#include "MathUtil.h"

struct PlayerEntity
{
	ULONG64 Entity;//实体
	ULONG64 MapPtr;//实体
	INT TeamId;//队伍ID
	std::string ClassName;//类名
	std::string Detective;//探员
	std::string PlayerName;//玩家名称
	std::string BotName;//人机名字
};

struct ItemEntity
{
	ULONG64 Entity;//实体
	INT Type;//物品类型
	std::string ProjectName;//容器名称
};

struct Mapinfo
{
	FLOAT X = 0.f;
	FLOAT Y = 0.f;
	FLOAT W = 0.f;
	FLOAT H = 0.f;
	INT MapX = 0;
	INT MapY = 0;
};

struct FNameEntryHandle
{
	USHORT Length;
};

struct FNameEntry
{
	FNameEntryHandle Header;
	union
	{
		char AnsiName[1024];
		wchar_t UnicodeName[1024];
	};
	char const* GetAnsiName() const { return AnsiName; }
	wchar_t const* GetWideName() const { return UnicodeName; }
	bool IsWide() const { return (Header.Length & 1) != 0; }
};

struct Matrix4x4
{
	float Matrix[4][4];
};

struct UGPHealth
{
	FLOAT Health;
	FLOAT MaxHealth;
};

struct UGPArmor
{
	INT Head;
	INT Armor;
};

#pragma pack(push, 1)
struct SendPlayerStruct
{
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
	FLOAT Directionposition;
	INT Distence;
	INT TeamID;
	BOOL IsVisable;
	Vector3 BonePos[KEY_BONE_COUNT];
	INT ValidFlags;
};

struct SendItemsStruct
{
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

struct UtilsStruct
{
	Matrix4x4 Matrix;
	BOOL FightMode;
	BOOL AimBot;
	ULONG64 GetDataPtr;
	FLOAT LocalWeaponSpeed;
	bool preShouldDraw;
	Mapinfo Info;
};
#pragma pack(pop)

BOOL IsUserAddress(ULONG64 Address)
{
	if (Address < 0xFFFF || Address > 0x7FFFFFFF0000) return FALSE;

	return TRUE;
}

BOOL IsVisable(ULONG64 Mesh)
{
	FLOAT fvisiontick = 0.07f;

	FLOAT fLastSubmitTime = Read<FLOAT>(Mesh + Offsets::LastSumbitTime);

	FLOAT fLastRenderTimeOnScreen = Read<FLOAT>(Mesh + Offsets::LastTimeOnScreen);

	return fLastRenderTimeOnScreen + fvisiontick >= fLastSubmitTime;
}

UGPHealth GetHealth(ULONG64 Entity)
{
	UGPHealth Health = {};

	ULONG64 AttributesCenter = Read<ULONG64>(Entity + Offsets::DFMAttributesCenter);

	ULONG64 SetHealth = Read<ULONG64>(AttributesCenter + Offsets::AttributeSetHealth);

	Health.Health = Read<FLOAT>(SetHealth + Offsets::AttributeHealth + 8);

	Health.MaxHealth = Read<FLOAT>(SetHealth + Offsets::AttributeHMaxHealth + 12);

	return Health;
}

UGPArmor GetArmor(ULONG64 Entity)
{
	UGPArmor Armor = {};

	ULONG64 CharacterEquipComponent = Read<ULONG64>(Entity + Offsets::EquipComponent);

	ULONG64 EquipedArmorInfoArray = Read<ULONG64>(CharacterEquipComponent + Offsets::ArmorInfoArray);

	Armor.Head = GetArmorLevel(Read<BYTE>(EquipedArmorInfoArray + Offsets::EquipLevel));

	Armor.Armor = GetArmorLevel(Read<BYTE>(EquipedArmorInfoArray + Offsets::ArmorLevel));

	return Armor;
}

FLOAT GetDistance(Vector3 MyPos, Vector3 ObjPos)
{
	FLOAT DeltaX = ObjPos.x - MyPos.x;

	FLOAT DeltaY = ObjPos.y - MyPos.y;

	FLOAT DeltaZ = ObjPos.z - MyPos.z;

	FLOAT DistanceSquared = DeltaX * DeltaX + DeltaY * DeltaY + DeltaZ * DeltaZ;

	FLOAT Distance = sqrt(DistanceSquared);

	return Distance / 100.f;
}

FLOAT GetWeaponSpeed(ULONG64 Entity)
{
	ULONG64 CacheWeapon = Read<ULONG64>(Entity + Offsets::CacheCurWeapon);

	if (CacheWeapon > 0)
	{
		ULONG64 WeaponDataComponentFiring = Read<ULONG64>(CacheWeapon + Offsets::WeaponFiring);

		FLOAT WeaponSpeed = Read<FLOAT>(WeaponDataComponentFiring + Offsets::FireInfo + Offsets::InitSpeed);

		return WeaponSpeed;
	}

	return 0.f;
}

Vector3 坐标换算(Vector3 假坐标, Vector3 换算坐标) {
	Vector3 真坐标;
	真坐标.x = 假坐标.x - 换算坐标.x;  // 修正全角等号为半角=，并添加分号
	真坐标.y = 假坐标.y - 换算坐标.y;  // 统一成员变量大小写为小写y（符合常规命名）
	真坐标.z = 假坐标.z - 换算坐标.z;  // 统一成员变量大小写为小写z
	return 真坐标;
}
Vector3 获取坐标换算() {

	Vector3 v1;
	v1.x = Read<int>(Read<uint64_t>(Offsets::Uworld) + 0x640);
	v1.y = Read<int>(Read<uint64_t>(Offsets::Uworld) + 0x644);
	v1.z = Read<int>(Read<uint64_t>(Offsets::Uworld) + 0x648);
	return  v1;
}

Vector3 GetPosition(ULONG64 Entity, INT Type)
{
	if (Type == 3)//玩家
	{
		ULONG64 RootComponent = Read<ULONG64>(Entity + 0x3e0);

		return Read<Vector3>(Read<ULONG64>(Read<ULONG64>(RootComponent + 0x418) + 0x80) + 0x70);
	}
	else if (Type == 4)//人机
	{
		ULONG64 RootComponent = Read<ULONG64>(Entity + 0x3e0);

		return Read<Vector3>(RootComponent + 0x148);
	}
	else if (Type == 2)//物资
	{
		Vector3 RootComponent = Read<Vector3>(Entity + 0x384);
		Vector3 pos = 坐标换算(RootComponent, 获取坐标换算());
		return  pos;
	}
	return Vector3(0, 0, 0);
}


INT GetTeamID(ULONG64 State)
{
	return Read<INT>(State + Offsets::TeamID);
}

USHORT GetXorKey(INT length)
{
	switch (length % 9)
	{
	case 0: return (length + (length & 0x1F) + 0x80) | 0x7F;
	case 1: return (length + (length ^ 0xDF) + 0x80) | 0x7F;
	case 2: return (length + (length | 0xCF) + 0x80) | 0x7F;
	case 3:return (33 * length + 128) | 0x7F;
	case 4: return (length + (length >> 2) + 128) | 0x7F;
	case 5: return (3 * length + 133) | 0x7F;
	case 6:return (length + ((4 * length) | 5) + 128) | 0x7F;
	case 7: return (length + ((length >> 4) | 7) + 128) | 0x7F;
	case 8: return (length + (length ^ 0xC) + 128) | 0x7F;
	}
	return 0;
}

std::string GetGName(ULONG Key)
{
	if (!Key)return ("");

	ULONG ChunkOffset = Key >> 18;

	ULONG NameOffset = Key & 262143;

	ULONG64 NamePoolChunk = Read<ULONG64>(Offsets::FNames + ChunkOffset * 8 + 8) + (NameOffset * 2);

	FNameEntry NameEntry = Read<FNameEntry>(NamePoolChunk);

	INT NameLength = NameEntry.Header.Length >> 6;

	if (NameLength > 0 && NameLength <= 256)
	{
		if (!NameEntry.IsWide())
		{
			char Name[1024] = { 0 };

			memcpy(Name, NameEntry.AnsiName, NameLength);

			ULONG Index = 0;

			do {
				Name[Index] ^= GetXorKey(NameLength);

				Index++;

			} while (Index < NameLength);

			Name[NameLength] = 0;

			return std::string(Name);
		}
		else
		{
			char Name[1024] = { 0 };

			wchar_t Unicode[1024] = { 0 };

			memcpy(Unicode, NameEntry.UnicodeName, NameLength * sizeof(WCHAR));

			ULONG Index = 0;

			do {
				Unicode[Index] ^= GetXorKey(NameLength);

				Index += 2;

			} while (Index < NameLength);

			Unicode[NameLength] = 0;

			int requiredSize = WideCharToMultiByte(CP_UTF8, 0, Unicode, -1, NULL, 0, NULL, NULL);

			if (requiredSize > 0 && requiredSize <= sizeof(Name))
			{
				WideCharToMultiByte(CP_UTF8, 0, Unicode, -1, Name, requiredSize, NULL, NULL);

				return std::string(Name);
			}
		}
	}

	return ("");
}

std::string GetEntityClassName(std::string Gname)
{
	std::string ClassName = ("");

	auto Name = ClassNameMap.find(Gname);

	if (Name != ClassNameMap.end())
	{
		ClassName = Name->second;
	}

	return ClassName;
}

std::string GetDetectiveName(ULONG64 State)
{
	std::string DetectiveName = ("");

	ULONG64 DetectiveId = Read<ULONG64>(State + Offsets::HeroID);

	auto Detective = DetectiveMap.find(DetectiveId);

	if (Detective != DetectiveMap.end())
	{
		DetectiveName = Detective->second;
	}

	return DetectiveName;
}

std::string GetPlayerName(ULONG64 State)
{
	WCHAR Unicode[32] = L"\0";

	ULONG64 PlayerNamePtr = Read<ULONG64>(State + Offsets::PlayerNamePrivate);

	if (ReadMemory( PlayerNamePtr, Unicode, sizeof(Unicode)))
	{
		 
		char Name[64] = {};

		WideCharToMultiByte(CP_UTF8, 0, Unicode, -1, Name, WideCharToMultiByte(CP_UTF8, 0, Unicode, -1, NULL, 0, NULL, NULL), NULL, NULL);

		return Name;
	}

	return ("Unknown");
}

std::string GetBotName(ULONG64 Entity)
{
	WCHAR Unicode[32] = L"\0";

	ULONG64 ChosenName = Read<ULONG64>(Entity + Offsets::PoolChosenName);

	ULONG64 AINamePda1 = Read<ULONG64>(ChosenName + Offsets::PoolChosenName_Pad1);

	ULONG64 AINamePda2 = Read<ULONG64>(AINamePda1 + Offsets::PoolChosenName_Pad2);

	ULONG64 AIName = Read<ULONG64>(AINamePda2 + Offsets::PoolAiName);

	if (ReadMemory(AIName, Unicode, sizeof(Unicode)))
	{
		char Name[64] = {};

		WideCharToMultiByte(CP_UTF8, 0, Unicode, -1, Name, WideCharToMultiByte(CP_UTF8, 0, Unicode, -1, NULL, 0, NULL, NULL), NULL, NULL);

		return Name;
	}

	return ("Unknown");
}

std::string GetWeaponName(ULONG64 Entity)
{
	std::string WeaponName = ("Unknown");

	ULONG64 CacheWeapon = Read<ULONG64>(Entity + Offsets::CacheCurWeapon);

	if (CacheWeapon > 0)
	{
		USHORT WeaponId = Read<USHORT>(CacheWeapon + Offsets::WeaponID);

		auto Weapon = WeaponMap.find(WeaponId);

		if (Weapon != WeaponMap.end())
		{
			WeaponName = Weapon->second;
		}
	}

	return WeaponName;
}

std::string GetIteamName(ULONG64 Entity)
{
	std::string IteamName = ("");

	ULONG Category = Read<ULONG>(Entity + Offsets::PickupItemInfo + 0x10);

	ULONG Sequence = Read<ULONG>(Entity + Offsets::PickupItemInfo + 0x14);

	ULONG64 IteamId = (ULONG64)Category * 10000 + Sequence;

	auto Iteams = IteamsMap.find(IteamId);

	if (Iteams != IteamsMap.end())
	{
		IteamName = Iteams->second;
	}

	return IteamName;
}

std::string GetProjectName(ULONG64 Entity, std::string Gname)
{
	if (Gname.find(("BP_Interact_Nest_C")) != std::string::npos) return ("鸟窝");

	else if ((Gname.find(("BP_Interact_Computer_")) != std::string::npos || Gname.find(("BP_Interact_PC_FireMissile")) != std::string::npos || Gname.find(("BP_Interact_PC_OpenDoor")) != std::string::npos))return ("电脑");

	else if (Gname.find(("BP_Interact_ShoppingBag_C")) != std::string::npos) return ("旅行包");

	else if (Gname.find(("BP_InteractorContainer_Server_C")) != std::string::npos) return ("服务器");

	else if (Gname.find(("BP_Interact_SmallSafebox_C")) != std::string::npos) return ("小保险箱");

	else if (Gname.find(("BP_Interactor_Container_SafeBox_C")) != std::string::npos) return ("保险箱");

	else if (Gname.find(("BP_InteractorContainer_Cloth_")) != std::string::npos) return ("一件衣服");

	else if (Gname.find(("BP_InteractorContainer_Lxrysuitcase_C")) != std::string::npos) return ("高级旅行箱");

	else if (Gname.find(("BP_InteractorContainer_MedSupplyPile_C")) != std::string::npos) return ("医疗物资堆");

	else if ((Gname.find(("BP_InteractorContainer_Militarybox7_C")) != std::string::npos || Gname.find(("BP_InteractorContainer_Militarybox8_C")) != std::string::npos)) return ("野外物资箱");

	else if ((Gname.find(("BP_InteractorContainer_flightcase_C")) != std::string::npos || Gname.find(("BP_InteractorContainer_OfficeBox2_C")) != std::string::npos)) return ("高级储物箱");

	else if ((Gname.find(("BP_InteractorContainer_Rocket_C")) != std::string::npos || Gname.find(("BP_InteractorContainer_AirCargoContainer_C")) != std::string::npos)) return ("航空存储箱");

	return ("");
}
