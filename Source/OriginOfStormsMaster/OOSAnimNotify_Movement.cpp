// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSAnimNotify_Movement.h"
#include "Runtime/Engine/Classes/Animation/AnimSequenceBase.h"
#include "OOSPawn.h"
#include "Engine.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"

void UOOSAnimNotify_Movement::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	AOOSPawn* Owner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (Owner)
	{
		Owner->MovementNotify = this;
		Owner->ExternalMove_Begin(Speed, bAir, bAllowGravity, bKeepMovement, bAddMovement, bUseXDecelerationAtStart, bUseYDecelerationAtStart, bDisablePush);
	}
}

void UOOSAnimNotify_Movement::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	AOOSPawn* Owner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (Owner && Owner->MovementNotify && (Owner->MovementNotify == this))
	{
		Owner->ExternalMove_End(bUseXDecelerationAtEnd, bUseYDecelerationAtEnd, bStopAtEnd);
		Owner->MovementNotify = nullptr;
	}	
}



