// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSAnimNotify_CinematicScript.h"
#include "OOSPawn.h"
#include "OOSCamera.h"
#include "Camera/CameraActor.h"
#include "Engine.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"


void UOOSAnimNotify_CinematicScript::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	UWorld* World = MeshComp->GetWorld();
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner && World)
	{
		AOOSCinematicScript* Script;
		
		Script = World->SpawnActor<AOOSCinematicScript>(ScriptClass, FPOwner->GetActorTransform());
		if (Script)
		{
			APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
			if (PC)
			{
				AActor* Camera = FPOwner->Camera;
				if (Camera)
				{
					Script->OnCinematicStart(FPOwner, Camera);

					if (FPOwner->PlayerIndex == 0)
					{
						P1Script = Script;
					}
					else if (FPOwner->PlayerIndex == 1)
					{
						P2Script = Script;
					}
				}
			}			
		}
	}
}

void UOOSAnimNotify_CinematicScript::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		AOOSCinematicScript* Script;

		if (FPOwner->PlayerIndex == 0)
		{
			Script = P1Script;
			P1Script = nullptr;
		}
		else if (FPOwner->PlayerIndex == 1)
		{
			Script = P2Script;
			P2Script = nullptr;
		}
		else return;

		if (Script->IsValidLowLevel())
		{
			Script->OnCinematicEnd();
			Script->Destroy();
		}
	}
}