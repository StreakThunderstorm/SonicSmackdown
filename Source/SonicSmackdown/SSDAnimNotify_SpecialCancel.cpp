// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDAnimNotify_SpecialCancel.h"
#include "SSDPawn.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"




void USSDAnimNotify_SpecialCancel::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bSpecialCancel = true;
		FPOwner->bSpecialCancelRequiresAttackLanded = bRequiresAttackLanded;
	}
}

void USSDAnimNotify_SpecialCancel::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bSpecialCancel = false;
		FPOwner->bSpecialCancelRequiresAttackLanded = false;
	}
}
