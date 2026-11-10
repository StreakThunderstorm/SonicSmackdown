// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SSDPawn.h"
#include "SSDAnimNotify_GrabType.generated.h"

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API USSDAnimNotify_GrabType : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(Category = Counter, EditAnywhere) ESSDGrab GrabType = ESSDGrab::SSDGR_Any;
};
