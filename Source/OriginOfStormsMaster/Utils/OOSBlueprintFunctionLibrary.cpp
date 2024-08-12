// Fill out your copyright notice in the Description page of Project Settings.


#include "OOSBlueprintFunctionLibrary.h"
#include "OriginOfStormsMaster/OOSPawn.h"

bool UOOSBlueprintFunctionLibrary::PlayAnimAsMontage(USkeletalMeshComponent* ChildMesh, UAnimationAsset* AnimationAsset, bool bLoop, float DesiredDuration, float BlendInTime, float BlendOutTime)
{
	if (!ChildMesh || !AnimationAsset) return false;

	if (AnimationAsset->IsA(UBlendSpace::StaticClass()))
	{
		ChildMesh->PlayAnimation(AnimationAsset, bLoop);
	}
	else
	{
		UAnimMontage* AnimMontage;

		if (AnimationAsset->IsA(UAnimMontage::StaticClass()))
		{
			AnimMontage = Cast<UAnimMontage>(AnimationAsset);
			AnimMontage->BlendIn = 0.f;
			AnimMontage->BlendOut = 0.f;
		}
		else
		{
			UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(AnimationAsset);
			AnimMontage = UAnimMontage::CreateSlotAnimationAsDynamicMontage(AnimSequence, "Default", BlendInTime, BlendOutTime);
		}

		if (!AnimMontage) return false;

		if (DesiredDuration >= 0.f)
		{
			AnimMontage->RateScale = AnimMontage->GetPlayLength() / DesiredDuration;
		}

		ChildMesh->PlayAnimation(AnimMontage, bLoop);
	}
	return true;
}

AOOSPawn* UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(UActorComponent* InComp)
{
	if (!InComp) return nullptr;

	AActor* MeshOwner = InComp->GetOwner();
	if (!MeshOwner) return nullptr;

	AOOSPawn* Owner = Cast<AOOSPawn>(MeshOwner);
	if (!Owner)
	{
		Owner = Cast<AOOSPawn>(MeshOwner->GetOwner());
	}

	return Owner;
}