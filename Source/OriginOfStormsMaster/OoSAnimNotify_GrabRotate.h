// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "OoSAnimNotify_GrabRotate.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOoSAnimNotify_GrabRotate : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	UPROPERTY(Category = Transform, EditAnywhere) float InterpSpeed = 20.f;
	UPROPERTY(Category = Transform, EditAnywhere) bool SocketRotation = false;
	UPROPERTY(Category = Transform, EditAnywhere) FName SocketName = FName("None");
	UPROPERTY(Category = Transform, EditAnywhere) FRotator RotOffset = FRotator::ZeroRotator;

};
