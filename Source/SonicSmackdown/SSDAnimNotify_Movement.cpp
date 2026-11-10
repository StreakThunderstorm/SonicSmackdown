// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDAnimNotify_Movement.h"
#include "Animation/AnimSequenceBase.h"
#include "SSDPawn.h"
#include "Engine.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"

void USSDAnimNotify_Movement::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	ASSDPawn* Owner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (Owner)
	{
		Owner->MovementNotify = this;
		Owner->ExternalMove_Begin(Speed, bAir, bAllowGravity, bKeepMovement, bAddMovement, bUseXDecelerationAtStart, bUseYDecelerationAtStart, bDisablePush);
	}
}

void USSDAnimNotify_Movement::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	ASSDPawn* Owner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (Owner && Owner->MovementNotify && (Owner->MovementNotify == this))
	{
		Owner->ExternalMove_End(bUseXDecelerationAtEnd, bUseYDecelerationAtEnd, bStopAtEnd);
		Owner->MovementNotify = nullptr;
	}	
}



