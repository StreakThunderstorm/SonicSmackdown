// Fill out your copyright notice in the Description page of Project Settings.


#include "OOSAnimNotify_CanAct.h"
#include "OOSPawn.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"

void UOOSAnimNotify_CanAct::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	AOOSPawn* Owner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (Owner)
	{
		Owner->bCanAct = true;
	}
}
