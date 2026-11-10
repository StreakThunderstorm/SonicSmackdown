// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDAnimNotify_GrabType.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"

void USSDAnimNotify_GrabType::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->GrabType = GrabType;
	}
}

void USSDAnimNotify_GrabType::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->GrabType = ESSDGrab::SSDGR_Any;
	}
}
