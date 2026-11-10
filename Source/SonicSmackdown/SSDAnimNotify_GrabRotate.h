// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SSDAnimNotify_GrabRotate.generated.h"

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API USSDAnimNotify_GrabRotate : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(Category = Transform, EditAnywhere) float InterpSpeed = 20.f;
	UPROPERTY(Category = Transform, EditAnywhere) bool SocketRotation = false;
	UPROPERTY(Category = Transform, EditAnywhere) FName SocketName = FName("None");
	UPROPERTY(Category = Transform, EditAnywhere) FRotator RotOffset = FRotator::ZeroRotator;

};
