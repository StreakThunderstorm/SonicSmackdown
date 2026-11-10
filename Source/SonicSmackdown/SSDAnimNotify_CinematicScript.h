// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SSDCinematicScript.h"
#include "SSDAnimNotify_CinematicScript.generated.h"

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API USSDAnimNotify_CinematicScript : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:

	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(Category = Projectile, EditAnywhere) TSubclassOf<ASSDCinematicScript> ScriptClass;

	// We need to keep track of both players since animnotifies and animations are not instanced.
	ASSDCinematicScript* P1Script = nullptr;
	ASSDCinematicScript* P2Script = nullptr;	
	
};
