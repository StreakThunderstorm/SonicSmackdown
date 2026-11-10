// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDAnimNotify_SuperArmor.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"

void USSDAnimNotify_SuperArmor::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		{
			FPOwner->SuperArmor = ArmoredHits;
		}
	}
}

void USSDAnimNotify_SuperArmor::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->SuperArmor = 0;
	}
}
