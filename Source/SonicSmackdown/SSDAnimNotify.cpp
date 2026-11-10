// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDAnimNotify.h"
#include "Animation/AnimSequenceBase.h"
#include "SSDPawn.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"

void USSDAnimNotify::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	ASSDPawn* Owner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (Owner)
	{
		Owner->OnSSDAnimNotify(Animation, NotifyName);
	}
}



