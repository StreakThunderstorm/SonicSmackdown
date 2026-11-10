// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Animation/AnimSingleNodeInstance.h"
#include "SSDAnimSingleNodeInstance.generated.h"

class URMAMirrorAnimationMirrorTable;

UCLASS(transient, NotBlueprintable)
class SONICSMACKDOWN_API USSDAnimSingleNodeInstance : public UAnimSingleNodeInstance
{
	GENERATED_BODY()

public:
	void SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable);
	void UpdateOrientation(bool bInOrientation);

	void RestartBlending();
	void CancelBlending();

protected:
	// UAnimInstance interface
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;


};
