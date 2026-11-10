// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Animation/AnimSingleNodeInstanceProxy.h"
#include "Animation/AttributesRuntime.h"
#include "SSDAnimSingleNodeInstanceProxy.generated.h"

class URMAMirrorAnimationMirrorTable;

/** Proxy override for this UAnimInstance-derived class */
USTRUCT()
struct SONICSMACKDOWN_API FSSDAnimSingleNodeInstanceProxy : public FAnimSingleNodeInstanceProxy
{
	GENERATED_BODY()

public:
	FSSDAnimSingleNodeInstanceProxy()
	{
	}

	FSSDAnimSingleNodeInstanceProxy(UAnimInstance* InAnimInstance)
		: FAnimSingleNodeInstanceProxy(InAnimInstance)
	{
	}

	virtual ~FSSDAnimSingleNodeInstanceProxy() {};

	// FAnimInstanceProxy interface
	virtual void Initialize(UAnimInstance* InAnimInstance) override;
	virtual void CacheBones() override;
	virtual bool Evaluate(FPoseContext& Output) override;
	
	void SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable);
	void UpdateOrientation(bool bInOrientation);

	void RestartBlending();
	void CancelBlending();

private:
	void MirrorPose(FPoseContext& Output);
	void CacheEvaluatedPose(const FPoseContext& Output);
	bool RestoreEvaluatedPose(FPoseContext& Output);

	URMAMirrorAnimationMirrorTable* MirrorTable = nullptr;
	bool bOrientation = true;
	float BlendingWeight = 1.f;

	TArray<FTransform> PreviousPose;
	TArray<FBoneIndexType> PreviousPoseBoneIndices;
	FBlendedHeapCurve PreviousCurve;
	UE::Anim::FHeapAttributeContainer PreviousAttributes;
	TWeakObjectPtr<UObject> PreviousPoseAsset;
	uint16 PreviousPoseBoneSerial = 0;
	bool bPreviousPoseOrientation = true;
};
