// Fill out your copyright notice in the Description page of Project Settings.


#include "OoSAnimNotify_GrabRotate.h"
#include "OOSPawn.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"

void UOoSAnimNotify_GrabRotate::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
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

void UOoSAnimNotify_GrabRotate::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->bGrabRotate = false;
		FPOwner->SocketRot = false;
		FPOwner->RotOffset = FRotator::ZeroRotator;
	}
}