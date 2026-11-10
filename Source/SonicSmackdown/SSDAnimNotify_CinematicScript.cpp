// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDAnimNotify_CinematicScript.h"
#include "SSDPawn.h"
#include "SSDCamera.h"
#include "Camera/CameraActor.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"


void USSDAnimNotify_CinematicScript::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	UWorld* World = MeshComp->GetWorld();
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner && World)
	{
		ASSDCinematicScript* Script;
		
		Script = World->SpawnActor<ASSDCinematicScript>(ScriptClass, FPOwner->GetActorTransform());
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

void USSDAnimNotify_CinematicScript::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation,
	const FAnimNotifyEventReference& EventReference)
{
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		ASSDCinematicScript* Script;

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
