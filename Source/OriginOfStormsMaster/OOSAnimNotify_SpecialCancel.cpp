// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSAnimNotify_SpecialCancel.h"
#include "OOSPawn.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"




void UOOSAnimNotify_SpecialCancel::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bSpecialCancel = true;
		FPOwner->bSpecialCancelRequiresAttackLanded = bRequiresAttackLanded;
	}
}

void UOOSAnimNotify_SpecialCancel::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bSpecialCancel = false;
		FPOwner->bSpecialCancelRequiresAttackLanded = false;
	}
}
