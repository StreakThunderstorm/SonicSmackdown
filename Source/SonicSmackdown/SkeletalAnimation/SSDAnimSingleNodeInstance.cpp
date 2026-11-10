// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	UAnimSingleNodeInstance.cpp: Single Node Tree Instance 
	Only plays one animation at a time. 
=============================================================================*/ 

#include "SSDAnimSingleNodeInstance.h"
#include "SSDAnimSingleNodeInstanceProxy.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void USSDAnimSingleNodeInstance::SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable)
{
	GetProxyOnGameThread<FSSDAnimSingleNodeInstanceProxy>().SetMirrorTable(InTable);
}

void USSDAnimSingleNodeInstance::UpdateOrientation(bool bInOrientation)
{
	GetProxyOnGameThread<FSSDAnimSingleNodeInstanceProxy>().UpdateOrientation(bInOrientation);
}

void USSDAnimSingleNodeInstance::RestartBlending()
{
	SavePoseSnapshot("FDBlendPose");
	if (FParse::Param(FCommandLine::Get(), TEXT("SSDFacingTrace")) && GetSkelMeshComponent() && GetSkelMeshComponent()->GetOwner()
		&& GetSkelMeshComponent()->GetOwner()->GetClass()->GetName().Contains(TEXT("Rouge")))
	{
		const FPoseSnapshot* Snapshot = GetProxyOnGameThread<FSSDAnimSingleNodeInstanceProxy>().GetPoseSnapshot("FDBlendPose");
		if (Snapshot && Snapshot->LocalTransforms.Num() > 0)
			UE_LOG(LogTemp, Display, TEXT("SSDFacing Snapshot Frame=%llu Owner=%s Root=%s Valid=%d"), GFrameCounter,
				*GetSkelMeshComponent()->GetOwner()->GetName(), *Snapshot->LocalTransforms[0].Rotator().ToString(), Snapshot->bIsValid);
	}
	GetProxyOnGameThread<FSSDAnimSingleNodeInstanceProxy>().RestartBlending();
}

void USSDAnimSingleNodeInstance::CancelBlending()
{
	GetProxyOnGameThread<FSSDAnimSingleNodeInstanceProxy>().CancelBlending();
}

FAnimInstanceProxy* USSDAnimSingleNodeInstance::CreateAnimInstanceProxy()
{
	Super::CreateAnimInstanceProxy();

	return new FSSDAnimSingleNodeInstanceProxy(this);
}

