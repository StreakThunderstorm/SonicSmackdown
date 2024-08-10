// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSAnimNotify.h"
#include "Runtime/Engine/Classes/Animation/AnimSequenceBase.h"
#include "OOSPawn.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"

void UOOSAnimNotify::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	Super::Notify(MeshComp, Animation);

	AOOSPawn* Owner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (Owner)
	{
		Owner->OnOOSAnimNotify(Animation, NotifyName);
	}
}



