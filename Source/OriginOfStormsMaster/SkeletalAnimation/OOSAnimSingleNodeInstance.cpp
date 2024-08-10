// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	UAnimSingleNodeInstance.cpp: Single Node Tree Instance 
	Only plays one animation at a time. 
=============================================================================*/ 

#include "OOSAnimSingleNodeInstance.h"
#include "OOSAnimSingleNodeInstanceProxy.h"

void UOOSAnimSingleNodeInstance::SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable)
{
	GetProxyOnGameThread<FOOSAnimSingleNodeInstanceProxy>().SetMirrorTable(InTable);
}

void UOOSAnimSingleNodeInstance::UpdateOrientation(bool bInOrientation)
{
	GetProxyOnGameThread<FOOSAnimSingleNodeInstanceProxy>().UpdateOrientation(bInOrientation);
}

void UOOSAnimSingleNodeInstance::RestartBlending()
{
	SavePoseSnapshot("FDBlendPose");
	GetProxyOnGameThread<FOOSAnimSingleNodeInstanceProxy>().RestartBlending();
}

void UOOSAnimSingleNodeInstance::CancelBlending()
{
	GetProxyOnGameThread<FOOSAnimSingleNodeInstanceProxy>().CancelBlending();
}

FAnimInstanceProxy* UOOSAnimSingleNodeInstance::CreateAnimInstanceProxy()
{
	Super::CreateAnimInstanceProxy();

	return new FOOSAnimSingleNodeInstanceProxy(this);
}

