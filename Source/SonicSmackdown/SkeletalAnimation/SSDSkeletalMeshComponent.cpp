// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDSkeletalMeshComponent.h"
#include "SSDAnimSingleNodeInstance.h"

void USSDSkeletalMeshComponent::PlayAnimationWithMirroring(class UAnimationAsset* NewAnimToPlay, bool bLooping, bool bDisableBlending)
{
	if (GetAnimationMode() != EAnimationMode::AnimationCustomMode || !SSDSingleNodeInstance)
	{
		SetCustomSingleNodeInstance();
	}

	if (bDisableBlending)
	{
		CancelBlending();
	}
	else
	{
		RestartBlending();
	}

	SetAnimation(NewAnimToPlay);
	Play(bLooping);
}

void USSDSkeletalMeshComponent::SetCustomSingleNodeInstance()
{
	SetAnimationMode(EAnimationMode::AnimationCustomMode);

	USSDAnimSingleNodeInstance* SequencerInstance = NewObject<USSDAnimSingleNodeInstance>(this, USSDAnimSingleNodeInstance::StaticClass());
	AnimScriptInstance = SequencerInstance;
	AnimScriptInstance->InitializeAnimation();

	SSDSingleNodeInstance = Cast<USSDAnimSingleNodeInstance>(AnimScriptInstance);
}

void USSDSkeletalMeshComponent::RestartBlending()
{
	if (GetAnimationMode() != EAnimationMode::AnimationCustomMode || !SSDSingleNodeInstance)
	{
		SetCustomSingleNodeInstance();
	}

	SSDSingleNodeInstance->RestartBlending();
}

void USSDSkeletalMeshComponent::CancelBlending()
{
	if (GetAnimationMode() != EAnimationMode::AnimationCustomMode || !SSDSingleNodeInstance)
	{
		SetCustomSingleNodeInstance();
	}

	SSDSingleNodeInstance->CancelBlending();
}

void USSDSkeletalMeshComponent::SetFacing(const bool& bFacing)
{
	if (GetAnimationMode() != EAnimationMode::AnimationCustomMode || !SSDSingleNodeInstance)
	{
		SetCustomSingleNodeInstance();
	}

	SSDSingleNodeInstance->UpdateOrientation(bFacing);
}

void USSDSkeletalMeshComponent::SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable)
{
	if (GetAnimationMode() != EAnimationMode::AnimationCustomMode || !SSDSingleNodeInstance)
	{
		SetCustomSingleNodeInstance();
	}

	SSDSingleNodeInstance->SetMirrorTable(InTable);
}

void USSDSkeletalMeshComponent::RefreshBoneTransforms(FActorComponentTickFunction* TickFunction)
{
	Super::RefreshBoneTransforms(TickFunction);

	// If evaluation went to a worker thread, CompleteParallelAnimationEvaluation handles it instead.
	if (!IsRunningParallelEvaluation())
	{
		DispatchNotifiesHeldForPhysics();
	}
}

void USSDSkeletalMeshComponent::CompleteParallelAnimationEvaluation(bool bDoPostAnimEvaluation)
{
	Super::CompleteParallelAnimationEvaluation(bDoPostAnimEvaluation);

	if (bDoPostAnimEvaluation)
	{
		DispatchNotifiesHeldForPhysics();
	}
}

void USSDSkeletalMeshComponent::DispatchNotifiesHeldForPhysics()
{
	// Without physics blending the engine has already dispatched them. Nothing is dispatched twice:
	// the engine clears its pending flag, so its own call after the physics step finds nothing left.
	if (ShouldBlendPhysicsBones())
	{
		ConditionallyDispatchQueuedAnimEvents();
	}
}
