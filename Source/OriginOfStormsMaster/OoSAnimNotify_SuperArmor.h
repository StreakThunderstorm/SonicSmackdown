// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "OOSPawn.h"
#include "OoSAnimNotify_SuperArmor.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOoSAnimNotify_SuperArmor : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	UPROPERTY(EditAnywhere) int ArmoredHits = 1;
};
