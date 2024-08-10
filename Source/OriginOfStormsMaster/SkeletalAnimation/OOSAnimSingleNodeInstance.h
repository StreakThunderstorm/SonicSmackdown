// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Runtime\Engine\Classes\Animation\AnimSingleNodeInstance.h"
#include "OOSAnimSingleNodeInstance.generated.h"

class URMAMirrorAnimationMirrorTable;

UCLASS(transient, NotBlueprintable)
class ORIGINOFSTORMSMASTER_API UOOSAnimSingleNodeInstance : public UAnimSingleNodeInstance
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
