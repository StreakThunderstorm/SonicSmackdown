// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "OOSAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	// Override of method that receives animation notify events.
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	UPROPERTY(EditAnywhere) FName NotifyName = TEXT("OOSAnimNotify");
	
};
