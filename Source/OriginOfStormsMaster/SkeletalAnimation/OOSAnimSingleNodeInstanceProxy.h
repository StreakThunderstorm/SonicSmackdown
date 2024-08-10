// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Runtime\Engine\Public\Animation\AnimSingleNodeInstanceProxy.h"
#include "OOSAnimSingleNodeInstanceProxy.generated.h"

class URMAMirrorAnimationMirrorTable;

/** Proxy override for this UAnimInstance-derived class */
USTRUCT()
struct ORIGINOFSTORMSMASTER_API FOOSAnimSingleNodeInstanceProxy : public FAnimSingleNodeInstanceProxy
{
	GENERATED_BODY()

public:
	FOOSAnimSingleNodeInstanceProxy()
	{
	}

	FOOSAnimSingleNodeInstanceProxy(UAnimInstance* InAnimInstance)
		: FAnimSingleNodeInstanceProxy(InAnimInstance)
	{
	}

	virtual ~FOOSAnimSingleNodeInstanceProxy() {};

	// FAnimInstanceProxy interface
	virtual bool Evaluate(FPoseContext& Output) override;
	
	void SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable);
	void UpdateOrientation(bool bInOrientation);

	void RestartBlending();
	void CancelBlending();

private:
	URMAMirrorAnimationMirrorTable* MirrorTable = nullptr;
	bool bOrientation = true;

	float BlendingWeight = 1.f;

	void MirrorPose(FPoseContext& Output);
};
