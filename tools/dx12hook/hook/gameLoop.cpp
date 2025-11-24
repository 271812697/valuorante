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
		void loop()
		{
            ImGui::Text("Hello Hook");
			bool flag = UpdateInstance();
			ImGui::Text(flag?"UpdateInstance Success": "UpdateInstance Failed");

            if (!persistentLevel || !localPlayerController || !localPlayerController->AcknowledgedPawn)
                return;
            SDK::FVector localWorldPos;
            SDK::TArray<SDK::AActor*> actors = persistentLevel->Actors;
            SDK::AUEDemoCharacter* closestTarget = nullptr;
            SDK::FVector2D targetHeadScreen;
            float closestDist2 = FLT_MAX;
            ImVec2 screenCenter = GetScreenCenter();
            for (auto* actor : actors) {
				if (!actor || !actor->IsA(SDK::EClassCastFlags::Pawn))continue;
				auto playerChar = static_cast<SDK::AUEDemoCharacter*>(actor);
                if (!playerChar || !playerChar->Mesh || !playerChar->PlayerState) continue;

                // local player
                if (playerChar == localPlayerController->AcknowledgedPawn) {
                    localWorldPos = playerChar->K2_GetActorLocation();
                }

               ;
                auto* skeletalMesh = playerChar->Mesh;
                ImDrawList* drawList = ImGui::GetBackgroundDrawList();
                SDK::FVector enemyPos = playerChar->K2_GetActorLocation();
                float distanceM = Distance3D(localWorldPos, enemyPos);
                std::unordered_map<std::string, SDK::FName> BoneMap;
                int boneCount = skeletalMesh->GetNumBones();
                for (int i = 0; i < boneCount; ++i)
                {
                    ImGui::Text(skeletalMesh->GetBoneName(i).ToString().c_str());
                    BoneMap[skeletalMesh->GetBoneName(i).ToString()] = skeletalMesh->GetBoneName(i);
                    SDK::FVector fromPos = skeletalMesh->GetSocketLocation(skeletalMesh->GetBoneName(i));
                    SDK::FVector2D screenFrom;
                    if (WorldToScreen(*localPlayerController, fromPos, &screenFrom)) {
                        drawList->AddCircleFilled(ImVec2(screenFrom.X, screenFrom.Y),5, IM_COL32(255, 0, 0, 255),30);
                    }
                }
            }
		}		
	}
}