// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDAnimNotify_Counter.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"

void USSDAnimNotify_Counter::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->Counter = Counter;
	}
}

void USSDAnimNotify_Counter::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->Counter = ESSDCounter::SSDCT_None;
	}
}
