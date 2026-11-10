// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDAnimNotify_GrabRotate.h"
#include "SSDPawn.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"

void USSDAnimNotify_GrabRotate::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bGrabRotate = true;
		FPOwner->InterpSpd = InterpSpeed;
		FPOwner->RotOffset = RotOffset;
		if (SocketRotation)
		{
			FPOwner->GrabSocket = SocketName;
			FPOwner->SocketRot = true;
		}
	}
}

void USSDAnimNotify_GrabRotate::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bGrabRotate = false;
		FPOwner->SocketRot = false;
		FPOwner->RotOffset = FRotator::ZeroRotator;
	}
}
