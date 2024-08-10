// Fill out your copyright notice in the Description page of Project Settings.


#include "OOSSkeletalMeshComponent.h"
#include "OOSAnimSingleNodeInstance.h"

void UOOSSkeletalMeshComponent::PlayAnimationWithMirroring(class UAnimationAsset* NewAnimToPlay, bool bLooping, bool bDisableBlending)
{
	if (GetAnimationMode() != EAnimationMode::AnimationCustomMode || !OOSSingleNodeInstance)
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

void UOOSSkeletalMeshComponent::SetCustomSingleNodeInstance()
{
	SetAnimationMode(EAnimationMode::AnimationCustomMode);

	UOOSAnimSingleNodeInstance* SequencerInstance = NewObject<UOOSAnimSingleNodeInstance>(this, UOOSAnimSingleNodeInstance::StaticClass());
	AnimScriptInstance = SequencerInstance;
	AnimScriptInstance->InitializeAnimation();

	OOSSingleNodeInstance = Cast<UOOSAnimSingleNodeInstance>(AnimScriptInstance);
}

void UOOSSkeletalMeshComponent::RestartBlending()
{
	if (GetAnimationMode() != EAnimationMode::AnimationCustomMode || !OOSSingleNodeInstance)
	{
		SetCustomSingleNodeInstance();
	}

	OOSSingleNodeInstance->RestartBlending();
}

void UOOSSkeletalMeshComponent::CancelBlending()
{
	if (GetAnimationMode() != EAnimationMode::AnimationCustomMode || !OOSSingleNodeInstance)
	{
		SetCustomSingleNodeInstance();
	}

	OOSSingleNodeInstance->CancelBlending();
}

void UOOSSkeletalMeshComponent::SetFacing(const bool& bFacing)
{
	if (GetAnimationMode() != EAnimationMode::AnimationCustomMode || !OOSSingleNodeInstance)
	{
		SetCustomSingleNodeInstance();
	}

	OOSSingleNodeInstance->UpdateOrientation(bFacing);
}

void UOOSSkeletalMeshComponent::SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable)
{
	if (GetAnimationMode() != EAnimationMode::AnimationCustomMode || !OOSSingleNodeInstance)
	{
		SetCustomSingleNodeInstance();
	}

	OOSSingleNodeInstance->SetMirrorTable(InTable);
}
