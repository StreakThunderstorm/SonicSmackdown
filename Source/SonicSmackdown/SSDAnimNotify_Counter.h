// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SSDPawn.h"
#include "SSDAnimNotify_Counter.generated.h"

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API USSDAnimNotify_Counter : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(Category = Counter, EditAnywhere) ESSDCounter Counter = ESSDCounter::SSDCT_None;
};
