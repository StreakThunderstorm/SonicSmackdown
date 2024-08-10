// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "OOSCinematicScript.h"
#include "OOSAnimNotify_CinematicScript.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSAnimNotify_CinematicScript : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:

	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;

	UPROPERTY(Category = Projectile, EditAnywhere) TSubclassOf<AOOSCinematicScript> ScriptClass;

	// We need to keep track of both players since animnotifies and animations are not instanced.
	AOOSCinematicScript* P1Script = nullptr;
	AOOSCinematicScript* P2Script = nullptr;	
	
};
