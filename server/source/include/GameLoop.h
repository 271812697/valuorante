#include "Includes.h"
#pragma comment(lib, "winmm.lib")

std::vector<PlayerEntity> EntityList;

std::vector<ItemEntity> ItemList;

ULONG64 LocalPlayerPawn = NULL;

ULONG64 MapPtr = NULL;

BOOL FightMode = FALSE;

static const std::chrono::microseconds MIN_INTERVAL(6167); //4167 240hz 

static std::chrono::steady_clock::time_point lastSendTime = std::chrono::steady_clock::now();


VOID GameLoop()
{

	while (TRUE)
		{
			std::vector<PlayerEntity> EntityArray = {};

			std::vector<ItemEntity> ItemArray = {};

			PlayerEntity PlayerArray = {};

			ItemEntity EntItem = {};

			ULONG64 Uworld = Read<ULONG64>(Offsets::Uworld);
			std::cout << Uworld << std::endl;
			ULONG64 GameInstance = Read<ULONG64>(Uworld + Offsets::LocalPlayers1);

			ULONG64 LocalPlayers = Read<ULONG64>(GameInstance + 8);

			ULONG64 PlayerController = Read<ULONG64>(LocalPlayers);

			LocalPlayerPawn = Read<ULONG64>(PlayerController + Offsets::LocalPlayers2);

			ULONG64 LoaclMesh = Read<ULONG64>(LocalPlayerPawn + Offsets::Mesh);

			ULONG64 LocalState = Read<ULONG64>(LocalPlayerPawn + Offsets::PlayerState);

			ULONG Count = Read<ULONG>(Offsets::ArrayBase + 8);

			ULONG64 AActor = Read<ULONG64>(Offsets::ArrayBase) + 8;

			if (IsUserAddress(AActor) && Count > 0 && Count < 524288)
			{
				for (ULONG i = 0; i <= Count; i++)
				{
					ULONG64 Entity = Read<ULONG64>(AActor + (i - 1) * 32);
					if (!IsUserAddress(Entity))continue;

					if (LocalPlayerPawn == Entity) continue;

					ULONG64 EntMesh = Read<ULONG64>(Entity + Offsets::Mesh);
					if (LoaclMesh == EntMesh) continue;

					INT ObjectID = Read<INT>(Entity + Offsets::ObjectID);
					if (!ObjectID)continue;

					std::string Gname = GetGName(ObjectID);
					if (Gname.empty())continue;

					if (Gname.find(("WBP_Map_Main_PC_C")) != Gname.npos)//大地图雷达
					{
						ULONG64 ViewController = Read<ULONG64>(Entity + Offsets::ViewController);
						if (IsUserAddress(ViewController))
						{
							ULONG64 View = Read<ULONG64>(ViewController + Offsets::View);
							if (IsUserAddress(View))MapPtr = View;
						}
						continue;
					}

					std::string ClassName = GetEntityClassName(Gname);
					if (!ClassName.empty())//玩家|人机
					{
						ULONG64 EntityState = Read<ULONG64>(Entity + Offsets::PlayerState);

						INT EntExitState = Read<INT>(EntityState + Offsets::ExitState);//撤离状态

						if (EntExitState == 3 || EntExitState == 5)continue;

						INT TeamId = GetTeamID(EntityState);

						if (TeamId == GetTeamID(LocalState))continue;

						UGPHealth Health = GetHealth(Entity);

						if (Health.Health <= 0.f || Health.Health > Health.MaxHealth)continue;

						if (strcmp(ClassName.c_str(), ("玩家")) == 0)
						{
							std::string Detective = GetDetectiveName(EntityState);

							std::string PlayerName = GetPlayerName(EntityState);

							PlayerArray.TeamId = TeamId;

							PlayerArray.Detective = Detective;

							PlayerArray.PlayerName = PlayerName;
						}
						else
						{
							std::string BotName = GetBotName(Entity);

							PlayerArray.BotName = BotName;
						}

						PlayerArray.Entity = Entity;

						PlayerArray.ClassName = ClassName;

						EntityArray.push_back(PlayerArray);

						continue;
					}

					BOOL IsItems = (Gname.find(("BP_InventoryPickup_C")) != Gname.npos);

					BOOL IsDeadBody = (Gname.find(("BP_Inventory_DeadBody_C")) != Gname.npos);

					std::string ProjectName = GetProjectName(Entity, Gname);

					if (IsItems || !ProjectName.empty() || IsDeadBody)
					{
						if (IsItems)//物资
						{
							ULONG64 ItemComponent = Read<ULONG64>(Entity + Offsets::ObjectComponent);

							if (ItemComponent != NULL)EntItem.Type = 1;
						}

						if (!ProjectName.empty())//容器
						{
							bool bIsOpened = Read<bool>(Entity + Offsets::bIsOpened);

							if (bIsOpened == true)continue;

							EntItem.Type = 2;

							EntItem.ProjectName = ProjectName;
						}

						if (IsDeadBody)EntItem.Type = 3;//战利品

						EntItem.Entity = Entity;

						ItemArray.push_back(EntItem);

						continue;
					}
				}
			}

			EntityList.clear();

			ItemList.clear();

			EntityList = EntityArray;

			ItemList = ItemArray;

			Sleep(3000);
		}

}

VOID SendLoop()
{
	while (TRUE)
	{

		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - lastSendTime);

		if (elapsed >= MIN_INTERVAL)
		{
			std::vector<SendPlayerStruct> PlayerList = {};
			std::vector<SendItemsStruct> ItemsList = {};

			SendPlayerStruct Player = {};
			SendItemsStruct Items = {};
			UtilsStruct Utils = {};

			Vector3 LocalPos = GetPosition(LocalPlayerPawn, 3);

			FLOAT LocalWeaponSpeed = GetWeaponSpeed(LocalPlayerPawn);
			Utils.LocalWeaponSpeed = LocalWeaponSpeed;

			ULONG64 MatrixTemp = Read<ULONG64>(Offsets::Matrix);
			ULONG64 MatrixPtr = Read<ULONG64>(MatrixTemp + 0x20) + 0x280;
			Matrix4x4 Matrix = Read<Matrix4x4>(MatrixPtr);  //这里更换矩阵
			Utils.Matrix = Matrix;

			for (const PlayerEntity& PlayerArray : EntityList)
			{
				ULONG64 Entity = PlayerArray.Entity;
				if (Entity <= 0)continue;

				ULONG64 EntMesh = Read<ULONG64>(Entity + Offsets::Mesh);
				if (EntMesh <= 0)continue;

				std::string ClassName = PlayerArray.ClassName;
				if (ClassName.empty())continue;

				strncpy_s(Player.ClassName, ClassName.c_str(), sizeof(Player.ClassName) - 1);
				Player.ClassName[sizeof(Player.ClassName) - 1] = '\0';

				UGPHealth Health = GetHealth(Entity);
				if (Health.Health <= 0.f || Health.Health > Health.MaxHealth)continue;
				Player.Health = Health;

				Vector3 Pos = {};
				if (strcmp(ClassName.c_str(), ("玩家")) == 0)Pos = GetPosition(Entity, 3);
				if (strcmp(ClassName.c_str(), ("AI")) == 0)Pos = GetPosition(Entity, 4);
				if (Pos.IsNearlyZero(0.001f)) continue;
				Pos.z -= 90.f;
				Player.Pos = Pos;

				INT Distence = GetDistance(LocalPos, Pos);
				if (Distence < 0)continue;
				Player.Distence = Distence;

				BOOL Visable = IsVisable(EntMesh);
				Player.IsVisable = Visable;

				//Vector3 Postion = GetBonePostion(EntMesh, 31);//Aim部位
				//Player.Postion = Postion;

				ULONG64 RootComponent = Read<ULONG64>(Entity + 0x3e0);

				Vector3 TargetSpeed = Read<Vector3>(RootComponent + Offsets::ComponentVelocity);
				Player.TargetSpeed = TargetSpeed;

				FLOAT Directionposition = Read<FLOAT>(RootComponent + Offsets::Directionposition);
				Player.Directionposition = Directionposition;

				FTransform BoneTransform[66] = {};
				Vector3 Bones_pos[66] = {};
				FTransform ComponentToWorld = Read<FTransform>(EntMesh + Offsets::ComponentToWorld);
				if (strcmp(ClassName.c_str(), ("玩家")) == 0)ComponentToWorld.Translation = Read<Vector3>(Read<ULONG64>(Read<ULONG64>(RootComponent + 0x418) + 0x80) + 0x70);
				if (strcmp(ClassName.c_str(), ("AI")) == 0)ComponentToWorld.Translation = Read<Vector3>(RootComponent + 0x148);
				ComponentToWorld.Translation.z -= 90.f;
				ULONG64 BoneArray = Read<ULONG64>(EntMesh + Offsets::BoneArray);
				if (!BoneArray) BoneArray = Read<ULONG64>(EntMesh + Offsets::BoneArray + 16);
				if (ReadMemory( BoneArray, &BoneTransform[0], 66 * sizeof(FTransform))){
				 
					for (auto a : Bone::拼接骨骼) {
						for (auto bone : a) {
							MATRIX ToMatrix = MatrixMultiplication(BoneTransform[bone].ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());
							Bones_pos[bone] = Vector3(ToMatrix._41, ToMatrix._42, ToMatrix._43);
							if (Bones_pos[bone].IsNearlyZero(0.00001f)) continue;
						}
					}
					for (INT i = 0; i < KEY_BONE_COUNT; i++) {
						INT BoneIdx = KEY_BONES[i];
						if (!Bones_pos[BoneIdx].IsNearlyZero(0.00001f)) {
							Player.BonePos[i] = Bones_pos[BoneIdx];
							Player.ValidFlags |= (1 << i);
						}
					}
				}

				INT TeamID = PlayerArray.TeamId;
				Player.TeamID = TeamID;

				UGPArmor Armor = GetArmor(Entity);
				Player.Armor = Armor;

				std::string Detective = PlayerArray.Detective;
				strncpy_s(Player.Detective, Detective.c_str(), sizeof(Player.Detective) - 1);
				Player.Detective[sizeof(Player.Detective) - 1] = '\0';

				std::string PlayerName = PlayerArray.PlayerName;
				strncpy_s(Player.PlayerName, PlayerName.c_str(), sizeof(Player.PlayerName) - 1);
				Player.PlayerName[sizeof(Player.PlayerName) - 1] = '\0';

				std::string BotName = PlayerArray.BotName;
				strncpy_s(Player.BotName, BotName.c_str(), sizeof(Player.BotName) - 1);
				Player.BotName[sizeof(Player.BotName) - 1] = '\0';

				std::string WeaponName = GetWeaponName(Entity);
				strncpy_s(Player.WeaponName, WeaponName.c_str(), sizeof(Player.WeaponName) - 1);
				Player.WeaponName[sizeof(Player.WeaponName) - 1] = '\0';

				PlayerList.push_back(Player);
			}

			if (!FightMode)
			{
				for (const ItemEntity& List : ItemList)
				{
					ULONG64 Entity = List.Entity;
					if (Entity <= 0) continue;

					INT Type = List.Type;
					if (Type <= 0) continue;
					Items.Type = Type;

					Vector3 Pos = GetPosition(Entity, 2);
					if (Pos.IsNearlyZero(0.001f))continue;
					Items.Pos = Pos;

					INT Distance = GetDistance(LocalPos, Pos);
					if (Distance < 0)continue;
					Items.Distance = Distance;

					if (Type == 1)//物资
					{
						ULONG64 ItemComponent = Read<ULONG64>(Entity + Offsets::ObjectComponent);

						if (ItemComponent != NULL)
						{
							std::string ItemName = GetIteamName(Entity);
							if (ItemName.empty())continue;
							strncpy_s(Items.ItemName, ItemName.c_str(), sizeof(Items.ItemName) - 1);
							Items.ItemName[sizeof(Items.ItemName) - 1] = '\0';

							INT ItemMoney = Read<INT>(ItemComponent + Offsets::ObjectGuidePrice);
							if (ItemMoney <= 5)continue;
							Items.ItemMoney = ItemMoney;

							INT ItemQuality = Read<INT>(ItemComponent + Offsets::ObjectQuality);
							if (ItemQuality == 0)continue;
							Items.ItemQuality = ItemQuality;
						}
					}

					if (Distance <= 100)
					{
						if (Type == 2)//容器
						{
							std::string ProjectName = List.ProjectName;
							if (ProjectName.empty())continue;

							if (strcmp(ProjectName.c_str(), ("电脑")) == 0)
							{
								INT Password = Read<INT>(Entity + Offsets::Password);
								Items.Password = Password;
							}
							strncpy_s(Items.ProjectName, ProjectName.c_str(), sizeof(Items.ProjectName) - 1);
							Items.ProjectName[sizeof(Items.ProjectName) - 1] = '\0';
						}

						if (Type == 3)//战利品
						{
							ULONG64 DeadBoxType = Read<ULONG64>(Entity + Offsets::DeadBoxType);
							Items.DeadBoxType = DeadBoxType;
						}
					}

					ItemsList.push_back(Items);
				}
			}

			if (GetAsyncKeyState(192) & 1)FightMode = !FightMode;//战斗模式
			Utils.FightMode = FightMode;

			bool preShouldDraw = Read<bool>(MapPtr + Offsets::preShouldDraw);
			Utils.preShouldDraw = preShouldDraw;
			if (preShouldDraw == TRUE)
			{
				Mapinfo Info = Read<Mapinfo>(MapPtr + Offsets::pa_0838);
				Utils.Info = Info;
			}

			ULONG64 GetDataPtr = Read<ULONG64>(Offsets::GetDataPtr);
			Utils.GetDataPtr = GetDataPtr;


			 Utils.AimBot = FALSE;
			 if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)Utils.AimBot = TRUE;

			 BYTE writeData[] = { 0xF3, 0x0F, 0x10, 0x40, 0x3C }; // 注意：0xF 建议补全为 0x0F（规范写法）

			
			 if (GetAsyncKeyState(VK_F1) & 0x8000) {
				 if (WriteMemory(0x14199E3A5, writeData, 5)) { // 传递数组指针和长度

					 std::cout << ("写入成功") << std::endl;
					 continue;
				 }
			 }
			
			 BYTE writeData1[] = { 0xF3, 0x0F, 0x10, 0x40, 0x48 }; // 注意：0xF 建议补全为 0x0F（规范写法）
			 if (GetAsyncKeyState(VK_F2) & 0x8000) {
				 if (WriteMemory( 0x14199E3A5, writeData1, 5)) { // 传递数组指针和长度

					 std::cout << ("关闭成功") << std::endl;
					 continue;
				 }
			 }

			SimpleCommand SimpleCmd = SimpleCommand::Create(1, Utils, PlayerList, ItemsList);
			SendSocketMessage(SimpleCmd.Data.data(), SimpleCmd.Data.size());

			lastSendTime = now;
		}	
	}

	closesocket(Socket);

	WSACleanup();
}