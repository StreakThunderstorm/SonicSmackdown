// Fill out your copyright notice in the Description page of Project Settings.


#include "OoSAnimNotify_Counter.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"

void UOoSAnimNotify_Counter::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->Counter = Counter;
	}
}

void UOoSAnimNotify_Counter::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->Counter = EOOSCounter::OOSCT_None;
	}
}