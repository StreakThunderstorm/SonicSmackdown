// Fill out your copyright notice in the Description page of Project Settings.


#include "OoSAnimNotify_GrabType.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"

void UOoSAnimNotify_GrabType::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->GrabType = GrabType;
	}
}

void UOoSAnimNotify_GrabType::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->GrabType = EOOSGrab::OOSGR_Any;
	}
}