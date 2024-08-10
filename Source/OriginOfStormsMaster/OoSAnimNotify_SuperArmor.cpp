// Fill out your copyright notice in the Description page of Project Settings.


#include "OoSAnimNotify_SuperArmor.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"

void UOoSAnimNotify_SuperArmor::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		{
			FPOwner->SuperArmor = ArmoredHits;
		}
	}
}

void UOoSAnimNotify_SuperArmor::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->SuperArmor = 0;
	}
}