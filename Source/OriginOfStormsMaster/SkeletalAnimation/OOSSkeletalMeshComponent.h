// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "OOSSkeletalMeshComponent.generated.h"

class UOOSAnimSingleNodeInstance;
class URMAMirrorAnimationMirrorTable;

/**
 *
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	void PlayAnimationWithMirroring(class UAnimationAsset* NewAnimToPlay, bool bLooping, bool bDisableBlending);
	void SetCustomSingleNodeInstance();

	void RestartBlending();
	void CancelBlending();

	void SetFacing(const bool& bFacing);

	void SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable);

private:
	UOOSAnimSingleNodeInstance* OOSSingleNodeInstance;
};
