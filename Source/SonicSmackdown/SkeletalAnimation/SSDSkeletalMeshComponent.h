// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "SSDSkeletalMeshComponent.generated.h"

class USSDAnimSingleNodeInstance;
class URMAMirrorAnimationMirrorTable;

/**
 *
 */
UCLASS()
class SONICSMACKDOWN_API USSDSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	void PlayAnimationWithMirroring(class UAnimationAsset* NewAnimToPlay, bool bLooping, bool bDisableBlending);
	void SetCustomSingleNodeInstance();

	void RestartBlending();
	void CancelBlending();

	void SetFacing(const bool& bFacing);

	void SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable);

	virtual void RefreshBoneTransforms(FActorComponentTickFunction* TickFunction = nullptr) override;
	virtual void CompleteParallelAnimationEvaluation(bool bDoPostAnimEvaluation) override;

private:
	// When a mesh blends physics into its pose (any simulated body, e.g. tendrils or hair), the
	// engine holds back the animation notifies until the physics step has finished, later in the
	// frame. Fighters drive gameplay from notifies (movement, hitboxes, state changes), so on those
	// characters a notify could land after input and movement had already been handled that frame
	// and overwrite them. This dispatches the notifies right after the animation is evaluated, as
	// happens for every mesh without physics.
	void DispatchNotifiesHeldForPhysics();

	USSDAnimSingleNodeInstance* SSDSingleNodeInstance;
};
