// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDAnimNotify_BlockSuper.h"
#include "SSDPawn.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"



void USSDAnimNotify_BlockSuper::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bBlockSuper = true;
	}
}

void USSDAnimNotify_BlockSuper::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bBlockSuper = false;
	}
}


