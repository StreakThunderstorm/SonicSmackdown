// Fill out your copyright notice in the Description page of Project Settings.


#include "OOSAnimNotify_BlockSuper.h"
#include "OOSPawn.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"



void UOOSAnimNotify_BlockSuper::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bBlockSuper = true;
	}
}

void UOOSAnimNotify_BlockSuper::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bBlockSuper = false;
	}
}


