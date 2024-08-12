// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSAnimNotifyState.h"
#include "OOSPawn.h"

void UOOSAnimNotifyState::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	AOOSPawn* InFPOwner = Cast<AOOSPawn>(MeshComp->GetOwner());
}

void UOOSAnimNotifyState::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
}


