// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSAnimNotify_Invincibility.h"
#include "Runtime/Engine/Classes/Animation/AnimSequenceBase.h"
#include "OOSPawn.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"

void UOOSAnimNotify_Invincibility::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	AOOSPawn* Owner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (Owner)
	{
		Owner->BecomeInvincible();
	}
}

void UOOSAnimNotify_Invincibility::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	AOOSPawn* Owner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (Owner)
	{
		Owner->EndInvincible();
	}
}