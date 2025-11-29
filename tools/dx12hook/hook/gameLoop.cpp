#include "imgui/imgui.h"
#include "CppSDK/SDK.hpp"
#include "gameLoop.h"
#include "Windows.h"
namespace MOON {
	namespace GAME {

		SDK::UWorld* world = nullptr;
		SDK::ULevel* persistentLevel = nullptr;
		SDK::APlayerController* localPlayerController = nullptr;
		SDK::APawn* localPawn = nullptr;
		SDK::ACharacter* playerCharacter = nullptr;

        namespace Menu {
            inline bool bLine = false;
            inline bool bSkeleton = true;
            inline bool bDistance = true;
            inline bool bAimbot = true;
            inline float FOV = 180.0f;
            inline bool dumpbone = false;
            inline bool draw_fov = false;
            inline bool bBox = true;
            inline bool bHealth = true;
            inline bool bNames = false;
            inline bool bAlert360 = true;
            inline float radius = 100.0f;
            inline float maxDistance = 250.0f;
        }
        namespace Color {
            static SDK::FLinearColor red = SDK::FLinearColor(1.f, 0.f, 0.f, 1.f);
            static SDK::FLinearColor green = SDK::FLinearColor(0.f, 1.f, 0.f, 1.f);
            static SDK::FLinearColor white = SDK::FLinearColor(1.f, 1.f, 1.f, 1.f);
            static SDK::FLinearColor yellow = SDK::FLinearColor(1.f, 1.f, 0.f, 1.f);
            static SDK::FLinearColor none = SDK::FLinearColor(0.f, 0.f, 0.f, 0.f);
        }

        bool WorldToScreen(SDK::APlayerController playerController, SDK::FVector& worldPos, SDK::FVector2D* screenPos) {
            return playerController.ProjectWorldLocationToScreen(worldPos, screenPos, true);
        }

        bool IsVisible(SDK::UWorld* world, SDK::FVector& viewPoint, SDK::FVector& targetWorldPos)
        {
            SDK::FHitResult hitResult;
            return (!SDK::UKismetSystemLibrary::LineTraceSingle(
                world, viewPoint, targetWorldPos,
                SDK::ETraceTypeQuery::TraceTypeQuery2, // trace channel
                false,                                 // trace complex
                SDK::TArray<SDK::AActor*>(),           // actors to ignore
                SDK::EDrawDebugTrace::None,
                &hitResult,
                true,
                Color::red, Color::red, NULL
            ));
        }

        std::vector<std::pair<std::string, std::string>> BonePairs = {
              {"HEAD", "neck_01"},
              {"neck_01", "spine_03"},

              // Cột sống
              {"spine_03", "spine_02"},
              {"spine_02", "spine_01"},
              {"spine_01", "pelvis"},

              // Tay trái
              {"spine_03", "clavicle_l"},
              {"clavicle_l", "upperarm_l"},
              {"upperarm_l", "lowerarm_l"},
              {"lowerarm_l", "hand_l"},

              // Tay phải
              {"spine_03", "clavicle_r"},
              {"clavicle_r", "upperarm_r"},
              {"upperarm_r", "lowerarm_r"},
              {"lowerarm_r", "hand_r"},


              {"pelvis", "thigh_l"},
              {"thigh_l", "calf_l"},
              {"calf_l", "foot_l"},

              // Chân phải
              {"pelvis", "thigh_r"},
              {"thigh_r", "calf_r"},
              {"calf_r", "foot_r"},
        };


        float Distance3D(const SDK::FVector& p1, const SDK::FVector& p2) {
            float dx = p1.X - p2.X, dy = p1.Y - p2.Y, dz = p1.Z - p2.Z;
            return SDK::UKismetMathLibrary::Sqrt(dx * dx + dy * dy + dz * dz) / 100.0f;
        }

        float Distance2D(SDK::FVector2D p1, SDK::FVector2D p2) {
            float dx = p1.X - p2.X, dy = p1.Y - p2.Y;
            return SDK::UKismetMathLibrary::Sqrt(dx * dx + dy * dy);
        }
        bool UpdateInstance() {
            world = SDK::UWorld::GetWorld();
            if (!world || !world->OwningGameInstance || world->OwningGameInstance->LocalPlayers.Num() == 0)
                return false;

            localPlayerController = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
            if (!localPlayerController) return false;

            localPawn = localPlayerController->AcknowledgedPawn;
            if (!localPawn) return false;

            persistentLevel = world->PersistentLevel;
            if (!persistentLevel) return false;

            playerCharacter = localPawn->IsA(SDK::ACharacter::StaticClass()) ?
                static_cast<SDK::ACharacter*>(localPawn) : nullptr;

            return true;
        }
        inline ImVec2 GetScreenCenter() {
            static float cx = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)) / 2.0f;
            static float cy = static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) / 2.0f;
            return ImVec2(cx, cy);
        }
        void DrawBox(ImDrawList* drawList, ImVec2 topLeft, ImVec2 bottomRight, ImU32 color, float thickness = 1.5f) {
            float lineX = (bottomRight.x - topLeft.x) * 0.3f;
            float lineY = (bottomRight.y - topLeft.y) * 0.3f;

            // Top Left
            drawList->AddLine(topLeft, ImVec2(topLeft.x + lineX, topLeft.y), color, thickness);
            drawList->AddLine(topLeft, ImVec2(topLeft.x, topLeft.y + lineY), color, thickness);
            // Top Right
            drawList->AddLine(ImVec2(bottomRight.x - lineX, topLeft.y), ImVec2(bottomRight.x, topLeft.y), color, thickness);
            drawList->AddLine(ImVec2(bottomRight.x, topLeft.y), ImVec2(bottomRight.x, topLeft.y + lineY), color, thickness);
            // Bottom Left
            drawList->AddLine(ImVec2(topLeft.x, bottomRight.y - lineY), ImVec2(topLeft.x, bottomRight.y), color, thickness);
            drawList->AddLine(ImVec2(topLeft.x, bottomRight.y), ImVec2(topLeft.x + lineX, bottomRight.y), color, thickness);
            // Bottom Right
            drawList->AddLine(ImVec2(bottomRight.x - lineX, bottomRight.y), bottomRight, color, thickness);
            drawList->AddLine(ImVec2(bottomRight.x, bottomRight.y - lineY), bottomRight, color, thickness);
        }

		void loop()
		{
			bool flag = UpdateInstance();
            ImGui::Begin("NGUYENHUY-Farlight84-v1");
            ImGui::Checkbox("Draw Line", &Menu::bLine);
            ImGui::Checkbox("Skeleton", &Menu::bSkeleton);
            ImGui::Checkbox("Distance", &Menu::bDistance);
            ImGui::Checkbox("Box", &Menu::bBox);
            ImGui::Checkbox("Healthbar", &Menu::bHealth);
            ImGui::Checkbox("AimBot", &Menu::bAimbot);
            ImGui::Checkbox("Draw fov", &Menu::draw_fov);
            ImGui::Checkbox("Names", &Menu::bNames);
            ImGui::SliderFloat("FOV", &Menu::FOV, 50.0f, 500.0f, "%.0f");
            ImGui::Checkbox("Alert360", &Menu::bAlert360);
            ImGui::SliderFloat("Circle Radius", &Menu::radius, 50.0f, 200.0f, "%.1f");
            ImGui::SliderFloat("ESP Distance", &Menu::maxDistance, 50.0f, 1000.0f, "%.0f m");
            ImGui::End();
            if (!persistentLevel || !localPlayerController || !localPlayerController->AcknowledgedPawn)
                return;
            SDK::FVector localWorldPos;
            SDK::TArray<SDK::AActor*> actors = persistentLevel->Actors;
            SDK::ACustomCharacter_C* closestTarget = nullptr;
            SDK::FVector2D targetHeadScreen;
            float closestDist2 = FLT_MAX;
            ImVec2 screenCenter = GetScreenCenter();
            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            for (auto* actor : actors) {
               
				if (!actor || !actor->IsA(SDK::EClassCastFlags::Pawn))continue;
				auto playerChar = static_cast<SDK::ACustomCharacter_C*>(actor);
                if (!playerChar || !playerChar->Mesh ) continue;
                //auto* playerCharState = static_cast<SDK::AInGamePlayerState_C*>(playerChar->PlayerState);
               /// if (!playerCharState ) continue;
                // local player
                if (playerChar == localPlayerController->AcknowledgedPawn) {
                    localWorldPos = playerChar->K2_GetActorLocation();
                    //continue;
                }
                //auto* characterBase = static_cast<SDK::AArchVisCharacter*>(playerChar);
                //if (!characterBase || !characterBase->K2_IsAlive()) continue;

                auto* skeletalMesh = playerChar->Mesh;
                
                SDK::FVector enemyPos = playerChar->K2_GetActorLocation();
                float distanceM = Distance3D(localWorldPos, enemyPos);
                std::unordered_map<std::string, SDK::FName> BoneMap;
                int boneCount = skeletalMesh->GetNumBones();
                for (int i = 0; i < boneCount; ++i)
                {
                    //ImGui::Text(skeletalMesh->GetBoneName(i).ToString().c_str());
                    BoneMap[skeletalMesh->GetBoneName(i).ToString()] = skeletalMesh->GetBoneName(i);
                    SDK::FVector fromPos = skeletalMesh->GetSocketLocation(skeletalMesh->GetBoneName(i));
                    SDK::FVector2D screenFrom;
                    if (WorldToScreen(*localPlayerController, fromPos, &screenFrom)) {
                        drawList->AddCircleFilled(ImVec2(screenFrom.X, screenFrom.Y),2, IM_COL32(255, 0, 0, 255),30);
                    }
                }

                if (Menu::bSkeleton)
                {
                    for (auto& pair :BonePairs) {
                        auto itFrom = BoneMap.find(pair.first);
                        auto itTo = BoneMap.find(pair.second);
                        if (itFrom == BoneMap.end() || itTo == BoneMap.end()) continue;

                        SDK::FVector fromPos = skeletalMesh->GetSocketLocation(itFrom->second);
                        SDK::FVector toPos = skeletalMesh->GetSocketLocation(itTo->second);
                        SDK::FVector2D screenFrom, screenTo;

                        if (WorldToScreen(*localPlayerController, fromPos, &screenFrom) &&
                            WorldToScreen(*localPlayerController, toPos, &screenTo)) {

                            bool visible = IsVisible(world, localWorldPos, fromPos) || IsVisible(world, localWorldPos, toPos);
                            ImU32 color = visible ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
                            drawList->AddLine(ImVec2(screenFrom.X, screenFrom.Y), ImVec2(screenTo.X, screenTo.Y), color, 0.5f);
                        }
                    }
                }

                if (Menu::bAlert360)
                {
                    SDK::FRotator camRot = localPlayerController->PlayerCameraManager->GetCameraRotation();
                    float yawRad = camRot.Yaw * (3.14159265f / 180.0f);

                    SDK::FVector2D forward = { cosf(yawRad), sinf(yawRad) };
                    SDK::FVector2D toEnemy = { enemyPos.X - localWorldPos.X, enemyPos.Y - localWorldPos.Y };

                    float len = SDK::UKismetMathLibrary::Sqrt(toEnemy.X * toEnemy.X + toEnemy.Y * toEnemy.Y);
                    if (len > 1.f) {
                        toEnemy.X /= len;
                        toEnemy.Y /= len;

                        float dot = forward.X * toEnemy.X + forward.Y * toEnemy.Y;
                        float det = forward.X * toEnemy.Y - forward.Y * toEnemy.X;
                        float angle = atan2f(det, dot);

                        ImVec2 enemyPosOnCircle(
                            screenCenter.x + sinf(angle) * Menu::radius,
                            screenCenter.y - cosf(angle) * Menu::radius
                        );

                        bool bVisible = IsVisible(world, localWorldPos, enemyPos);
                        ImU32 color = bVisible ? IM_COL32(0, 255, 0, 255)
                            : IM_COL32(255, 0, 0, 255);

                        drawList->AddCircle(enemyPosOnCircle, 6.0f, color, 64, 2.0f);
                    }
                }

                // Box & Health
                SDK::FVector headPos = skeletalMesh->GetSocketLocation(BoneMap["HEAD"]);
                SDK::FVector footPos = skeletalMesh->GetSocketLocation(BoneMap["foot_l"]);
                SDK::FVector footRPos;
                if (BoneMap.find("foot_r") != BoneMap.end()) {
                    footRPos = skeletalMesh->GetSocketLocation(BoneMap["foot_r"]);
                    if (footRPos.Z < footPos.Z) footPos = footRPos;
                }
                SDK::FVector2D screenHead, screenFoot;
                if (WorldToScreen(*localPlayerController, headPos, &screenHead) &&
                    WorldToScreen(*localPlayerController, footPos, &screenFoot)) 
                {

                    float boxHeight = (screenFoot.Y - screenHead.Y) + 12.0f;
                    float boxWidth = boxHeight / 2.0f + 5.0f;
                    ImVec2 topLeft(screenHead.X - boxWidth / 2.0f, screenHead.Y - 6.0f);
                    ImVec2 bottomRight(screenHead.X + boxWidth / 2.0f, screenFoot.Y + 6.0f);

                    if (Menu::bBox)
                        DrawBox(drawList, topLeft, bottomRight, IsVisible(world, localWorldPos, headPos) ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));

                    if (Menu::bHealth) {
                        float healthPerc = 1.0f;// characterBase->GetCurrentHealth() / characterBase->GetMaxHealth();
                        ImU32 healthColor = healthPerc > 0.5f ? IM_COL32(0, 255, 0, 255)
                            : healthPerc > 0.25f ? IM_COL32(255, 255, 0, 255)
                            : IM_COL32(255, 0, 0, 255);

                        drawList->AddRectFilled(ImVec2(topLeft.x - 8.0f, topLeft.y),
                            ImVec2(topLeft.x - 5.0f, bottomRight.y),
                            IM_COL32(50, 50, 50, 200));
                        drawList->AddRectFilled(ImVec2(topLeft.x - 8.0f, topLeft.y + (1.0f - healthPerc) * boxHeight),
                            ImVec2(topLeft.x - 5.0f, bottomRight.y),
                            healthColor);
                    }
                }
                if (Menu::bLine) {
                    SDK::FVector linePos = headPos;
                    SDK::FVector2D screenPos;
                    if (WorldToScreen(*localPlayerController, linePos, &screenPos)) {


                        ImU32 lineColor = IM_COL32(255, 0, 0, 255);




                        bool visible = IsVisible(world, localWorldPos, linePos);
                        lineColor = visible ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);

                        ImVec2 screenBottom(ImGui::GetIO().DisplaySize.x / 2,
                            ImGui::GetIO().DisplaySize.y);
                        drawList->AddLine(screenBottom,
                            ImVec2(screenPos.X, screenPos.Y),
                            lineColor,
                            0.5f);
                    }
                }


                if (Menu::bDistance) {
                    SDK::FVector2D screenPos;
                    if (WorldToScreen(*localPlayerController, enemyPos, &screenPos)) {
                        ImU32 col = distanceM <= 50.0f ? IM_COL32(0, 255, 0, 255)
                            : distanceM <= 150.0f ? IM_COL32(255, 255, 0, 255)
                            : IM_COL32(255, 100, 0, 255);
                        char buf[64]; sprintf_s(buf, "%.1f m", distanceM);

                        float scale = 1.5f;
                        drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(screenPos.X + 1, screenPos.Y + 1), IM_COL32(0, 0, 0, 150), buf);
                        drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, ImVec2(screenPos.X, screenPos.Y), col, buf);
                    }
                }

                if (Menu::bNames)
                {
                    SDK::FVector2D screenPos;
                    if (WorldToScreen(*localPlayerController, playerChar->RootComponent->RelativeLocation, &screenPos))
                    {
                        std::string name = playerChar->PlayerState->GetPlayerName().ToString();

                        ImGui::GetBackgroundDrawList()->AddText(
                            ImVec2(screenPos.X, screenPos.Y - 15),
                            IM_COL32(255, 255, 0, 255),
                            name.c_str()
                        );
                    }

                }
                // FOV
                if (Menu::draw_fov)
                    drawList->AddCircle(screenCenter, Menu::FOV, IM_COL32(255, 255, 0, 255), 100, 0.5f);

                // Aimbot
                if (Menu::bAimbot && (GetAsyncKeyState(VK_XBUTTON1) & 0x8000)) {
                    SDK::FVector2D headScreen;
                    if (WorldToScreen(*localPlayerController, headPos, &headScreen)) {
                        float dx = headScreen.X - screenCenter.x;
                        float dy = headScreen.Y - screenCenter.y;
                        float dist2 = dx * dx + dy * dy;
                        if (dist2 <= Menu::FOV * Menu::FOV && dist2 < closestDist2) {
                            closestDist2 = dist2;
                            closestTarget = playerChar;
                            targetHeadScreen = headScreen;
                        }
                    }
                }

            }
		}		
	}
}