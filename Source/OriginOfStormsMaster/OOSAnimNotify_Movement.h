// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "OOSAnimNotify_Movement.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSAnimNotify_Movement : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	// Override of methods that receive animation notify begin and end events.
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;

	UPROPERTY(EditAnywhere) FVector2D Speed = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere) bool bAir = false;
	UPROPERTY(EditAnywhere) bool bKeepMovement = false;
	UPROPERTY(EditAnywhere) bool bAddMovement = false;
	UPROPERTY(EditAnywhere) bool bDisablePush = false;
	UPROPERTY(EditAnywhere) bool bUseXDecelerationAtStart = false;
	UPROPERTY(EditAnywhere) bool bUseXDecelerationAtEnd = false;
	UPROPERTY(EditAnywhere) bool bUseYDecelerationAtStart = false;
	UPROPERTY(EditAnywhere) bool bUseYDecelerationAtEnd = false;
	UPROPERTY(EditAnywhere) bool bAllowGravity = true;
	UPROPERTY(EditAnywhere) bool bStopAtEnd = true;
	
};
