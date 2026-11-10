// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDAnimNotifyState.h"
#include "SSDPawn.h"

void USSDAnimNotifyState::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	ASSDPawn* PawnOwner = Cast<ASSDPawn>(MeshComp->GetOwner());
}

void USSDAnimNotifyState::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}


