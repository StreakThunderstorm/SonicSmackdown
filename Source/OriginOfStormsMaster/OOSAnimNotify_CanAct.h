// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "OOSAnimNotify_CanAct.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSAnimNotify_CanAct : public UAnimNotify
{
	GENERATED_BODY()

public:
	// Override of methods that receive animation notify begin and end events.
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
};
