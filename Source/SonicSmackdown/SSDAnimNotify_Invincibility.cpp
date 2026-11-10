// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDAnimNotify_Invincibility.h"
#include "Animation/AnimSequenceBase.h"
#include "SSDPawn.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"

void USSDAnimNotify_Invincibility::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	ASSDPawn* Owner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (Owner)
	{
		Owner->BecomeInvincible();
	}
}

void USSDAnimNotify_Invincibility::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	ASSDPawn* Owner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (Owner)
	{
		Owner->EndInvincible();
	}
}
