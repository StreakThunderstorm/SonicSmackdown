// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationAnimGraphNodeMirrorAnimation.h"
#include "Kismet2/CompilerResultsLog.h"

void URMAMirrorAnimationAnimGraphNodeMirrorAnimation::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{

	if (!Node.MirrorTable)
	{

		MessageLog.Warning(TEXT("@@ Pick a MirrorTable"), this);

	}

	else
	{

		if (Node.MirrorTable->SingleBoneConfig.Num() <= 0 && Node.MirrorTable->DoubleBoneConfig.Num() <= 0)
		{

			MessageLog.Warning(TEXT("@@ MirrorTable does not contain definitions of bones!"), this);

		}

		if (!Node.MirrorTable->Skeleton)
		{

			MessageLog.Warning(TEXT("@@ MirrorTable does not have a defined skeleton"), this);

		}

		else if (Node.MirrorTable->Skeleton != ForSkeleton)
		{

			MessageLog.Warning(TEXT("@@ The skeleton defined in the MirrorTable is not compatible with this AnimBlueprint"), this);

		}

	}

	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);

}

void URMAMirrorAnimationAnimGraphNodeMirrorAnimation::PreloadRequiredAssets()
{

	//Load MirrorTable Asset
	PreloadObject(Node.MirrorTable);

	Super::PreloadRequiredAssets();

}
