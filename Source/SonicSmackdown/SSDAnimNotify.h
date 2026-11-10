// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SSDAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API USSDAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	// Override of method that receives animation notify events.
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	// Preserve the serialized notification name used by existing animation assets and Blueprint comparisons.
	UPROPERTY(EditAnywhere) FName NotifyName = TEXT("OOSAnimNotify");
	
};
