// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationMirrorTable.h"
#include "RMAMirrorAnimation.h"
#include "UObject/ObjectSaveContext.h"

URMAMirrorAnimationMirrorTable::URMAMirrorAnimationMirrorTable()
{

	Skeleton = nullptr;

	BoneKeyword = { FRMAMirrorAnimationBoneKeyword("_l", "_r"), FRMAMirrorAnimationBoneKeyword("-l", "-r"),
					FRMAMirrorAnimationBoneKeyword("l_", "r_"), FRMAMirrorAnimationBoneKeyword("l-", "r-") };

	SingleBoneConfig.Empty();
	DoubleBoneConfig.Empty();
	ComponentRightAxis = ERMAMirrorAnimationAxis::AxisX;
	LocationMirrorAxis = ERMAMirrorAnimationAxisWithNull::AxisNull;
	RotationMirrorAxis = ERMAMirrorAnimationAxisWithNull::AxisNull;
	MirrorLocationData = true;
	FileVersion = "Out of Date";

}

#if WITH_EDITOR

void URMAMirrorAnimationMirrorTable::PostLoad()
{

	Super::PostLoad();

}

void URMAMirrorAnimationMirrorTable::PreSave(FObjectPreSaveContext SaveContext)
{

	SetFileVersion();
	Super::PreSave(SaveContext);

}

#endif

void URMAMirrorAnimationMirrorTable::SetFileVersion()
{

	FileVersion = URMAMirrorAnimationFunctionLibrary::GetVersion();

}

bool URMAMirrorAnimationMirrorTable::MirrorAnimations()
{

#if WITH_EDITOR

	if (GEditor)
	{

		TArray<UAnimSequence*> LAnimations = GetAnimSelection();

		if (LAnimations.Num() > 0)
		{

			FRMAMirrorAnimation& LMirrorAnimationModule = FModuleManager::Get().LoadModuleChecked<FRMAMirrorAnimation>("RMAMirrorAnimation");
			LMirrorAnimationModule.Get().OnMirrorAnimationsDelegate.Broadcast(LAnimations, *this);

			return true;

		}

	}

#endif

	return false;

}

TArray<UAnimSequence*> URMAMirrorAnimationMirrorTable::GetAnimSelection()
{

#if WITH_EDITOR

	if (GEditor)
	{

		TArray<FAssetData> LSelection;
		TArray<UAnimSequence*> LAnimations;

		GEditor->GetContentBrowserSelections(LSelection);

		for (int LIndex = 0; LIndex < LSelection.Num(); LIndex++)
		{

			if (Cast<UAnimSequence>(LSelection[LIndex].GetAsset()))
			{

				LAnimations.Add(Cast<UAnimSequence>(LSelection[LIndex].GetAsset()));

			}

		}

		return LAnimations;

	}

#endif

	return {};

}

bool URMAMirrorAnimationMirrorTable::GenerateBoneConfig()
{

	if (Skeleton)
	{

		const FReferenceSkeleton LRefSkeleton = Skeleton->GetReferenceSkeleton();
		const int32 LNumBones = Skeleton->GetReferenceSkeleton().GetNum();

		TArray<FString> LBoneNames;
		TArray<FString> LSingleBoneConfigCache;
		TArray<FString> LDoubleBoneConfigCacheA;
		TArray<FString> LDoubleBoneConfigCacheB;

		for (int32 LIndex = 0; LIndex < LNumBones; LIndex++)
		{

			LBoneNames.Add(LRefSkeleton.GetBoneName(LIndex).ToString());

		};

		for (int32 LIndex = 0; LIndex < LBoneNames.Num(); LIndex++)
		{

			FString LBoneName = LBoneNames[LIndex];
			FString LBoneAName = "None";
			FString LBoneBName = "None";

			if (LDoubleBoneConfigCacheA.Contains(LBoneName) || LDoubleBoneConfigCacheB.Contains(LBoneName))
			{

				continue;

			}

			if (FindDoubleBone(LBoneName, LBoneAName, LBoneBName))
			{

				LDoubleBoneConfigCacheA.Add(LBoneAName);
				LDoubleBoneConfigCacheB.Add(LBoneBName);

			}

			else
			{

				LSingleBoneConfigCache.Add(LBoneName);

			}

		}

		SingleBoneConfig.Empty();
		DoubleBoneConfig.Empty();
		SingleBoneConfig.SetNum(LSingleBoneConfigCache.Num());
		DoubleBoneConfig.SetNum(LDoubleBoneConfigCacheA.Num());

		for (int LIndex = 0; LIndex < LSingleBoneConfigCache.Num(); LIndex++)
		{

			SingleBoneConfig[LIndex].BoneName = *LSingleBoneConfigCache[LIndex];

			int LBoneIndex = LRefSkeleton.FindBoneIndex(SingleBoneConfig[LIndex].BoneName);
			int LParentBoneIndex = LRefSkeleton.GetParentIndex(LBoneIndex);

			FVector LComponentRightAxis = FVector(0);
			FTransform LBoneTransform = LRefSkeleton.GetRefBonePose()[LBoneIndex];
			ERMAMirrorAnimationAxis LBoneMirrorAxis = ERMAMirrorAnimationAxis::AxisX;

			while (LParentBoneIndex >= 0)
			{

				//Accumulate Transform
				LBoneTransform.SetLocation(LRefSkeleton.GetRefBonePose()[LParentBoneIndex].GetRotation().RotateVector(LBoneTransform.GetLocation()) + LRefSkeleton.GetRefBonePose()[LParentBoneIndex].GetLocation());
				LBoneTransform.SetRotation(LRefSkeleton.GetRefBonePose()[LParentBoneIndex].GetRotation() * LBoneTransform.GetRotation());

				//Next Parent
				LParentBoneIndex = LRefSkeleton.GetParentIndex(LParentBoneIndex);

			}

			switch (ComponentRightAxis)
			{

			case ERMAMirrorAnimationAxis::AxisX:
				LComponentRightAxis = FVector(1, 0, 0);
				break;
			case ERMAMirrorAnimationAxis::AxisY:
				LComponentRightAxis = FVector(0, 1, 0);
				break;
			case ERMAMirrorAnimationAxis::AxisZ:
				LComponentRightAxis = FVector(0, 0, 1);
				break;

			}

			const FVector LAxisX = LBoneTransform.GetRotation().GetForwardVector() * LComponentRightAxis;
			const FVector LAxisY = LBoneTransform.GetRotation().GetRightVector() * LComponentRightAxis;
			const FVector LAxisZ = LBoneTransform.GetRotation().GetUpVector() * LComponentRightAxis;

			if (LAxisY.Size() > LAxisX.Size())
			{

				LBoneMirrorAxis = ERMAMirrorAnimationAxis::AxisY;

			}

			if (LAxisZ.Size() > LAxisX.Size() && LAxisZ.Size() > LAxisY.Size())
			{

				LBoneMirrorAxis = ERMAMirrorAnimationAxis::AxisZ;

			}

			SingleBoneConfig[LIndex].MirrorAxis = LBoneMirrorAxis;

		}

		for (int LIndex = 0; LIndex < LDoubleBoneConfigCacheA.Num(); LIndex++)
		{

			DoubleBoneConfig[LIndex].BoneAName = *LDoubleBoneConfigCacheA[LIndex];
			DoubleBoneConfig[LIndex].BoneBName = *LDoubleBoneConfigCacheB[LIndex];

		}

		return true;

	}

	return false;

}

bool URMAMirrorAnimationMirrorTable::FindDoubleBone(const FString& BoneName, FString& OutBoneAName, FString& OutBoneBName)
{

	if (Skeleton)
	{

		for (int LIndex = 0; LIndex < BoneKeyword.Num(); LIndex++)
		{

			bool LResult = false;
			const FString LKeywordA = BoneKeyword[LIndex].KeywordA;
			const FString LKeywordB = BoneKeyword[LIndex].KeywordB;
			bool LKeywordIndex = false;
			int LFindOffset = -1;

		Find:

			int LFindIndex = -1;
			FString LBoneAName = BoneName;
			FString LBoneBName = BoneName;

			//Check Keyword A
			LResult = ((LFindIndex = LBoneAName.Find(LKeywordA, ESearchCase::IgnoreCase, ESearchDir::FromStart, LFindOffset)) >= 0);
			LKeywordIndex = false;

			if (!LResult)
			{

				//Check Keyword B
				LResult = ((LFindIndex = LBoneAName.Find(LKeywordB, ESearchCase::IgnoreCase, ESearchDir::FromStart, LFindOffset)) >= 0);
				LKeywordIndex = true;

			}

			if (LResult)
			{

				LBoneAName.RemoveAt(LFindIndex, (LKeywordIndex ? LKeywordB : LKeywordA).Len());
				LBoneBName = LBoneAName;

				LBoneAName.InsertAt(LFindIndex, (LKeywordIndex ? LKeywordB : LKeywordA));
				LBoneBName.InsertAt(LFindIndex, (LKeywordIndex ? LKeywordA : LKeywordB));

				//Check Bones
				if (Skeleton->GetReferenceSkeleton().FindBoneIndex(*LBoneAName) <= -1 ||
					Skeleton->GetReferenceSkeleton().FindBoneIndex(*LBoneBName) <= -1)
				{

					//Restart
					LFindOffset = LFindIndex + 1;
					goto Find;

				}

				OutBoneAName = LBoneAName;
				OutBoneBName = LBoneBName;

				return true;

			}

		};

	}

	return false;

}

FVector URMAMirrorAnimationMirrorTable::MirrorLocation(const FVector& Location, const FVector& RefLocation, const FQuat4f& RefRotation, const FRMAMirrorAnimationSingleBoneConfig& BoneConfig)
{

	FVector LLocationResult = Location - RefLocation;
	LLocationResult = FVector(RefRotation.UnrotateVector(FVector3f(LLocationResult)));

	switch (BoneConfig.MirrorAxis)
	{

	case ERMAMirrorAnimationAxis::AxisX:
		LLocationResult = FVector(-LLocationResult.X, LLocationResult.Y, LLocationResult.Z);
		break;
	case ERMAMirrorAnimationAxis::AxisY:
		LLocationResult = FVector(LLocationResult.X, -LLocationResult.Y, LLocationResult.Z);
		break;
	case ERMAMirrorAnimationAxis::AxisZ:
		LLocationResult = FVector(LLocationResult.X, LLocationResult.Y, -LLocationResult.Z);
		break;

	}

	LLocationResult = FVector(RefRotation.RotateVector(FVector3f(LLocationResult)));
	LLocationResult = RefLocation + LLocationResult;

	return LLocationResult;

}

FQuat4f URMAMirrorAnimationMirrorTable::MirrorRotation(const FQuat4f& Rotation, const FQuat4f& RefRotation, const FRMAMirrorAnimationSingleBoneConfig& BoneConfig)
{

	FQuat4f LRotationResult = RefRotation.Inverse() * Rotation;

	switch (BoneConfig.MirrorAxis)
	{

	case ERMAMirrorAnimationAxis::AxisX:
		LRotationResult = FQuat4f(LRotationResult.X, -LRotationResult.Y, -LRotationResult.Z, LRotationResult.W);
		break;
	case ERMAMirrorAnimationAxis::AxisY:
		LRotationResult = FQuat4f(-LRotationResult.X, LRotationResult.Y, -LRotationResult.Z, LRotationResult.W);
		break;
	case ERMAMirrorAnimationAxis::AxisZ:
		LRotationResult = FQuat4f(-LRotationResult.X, -LRotationResult.Y, LRotationResult.Z, LRotationResult.W);
		break;

	}

	LRotationResult = RefRotation * LRotationResult;

	return LRotationResult;

}

FVector URMAMirrorAnimationMirrorTable::MirrorLocationToOtherPose(const FVector& SourceLocation, const FVector& SourceRefLocation, const FQuat4f& SourceRefRotation, const FVector& TargetRefLocation, const FQuat4f& TargetRefRotation, const FRMAMirrorAnimationDoubleBoneConfig& BoneConfig)
{

	FVector LLocationResult = (SourceLocation - SourceRefLocation);

	switch ((BoneConfig.LocationMirrorAxis != ERMAMirrorAnimationAxisWithNull::AxisNull) 
		? BoneConfig.LocationMirrorAxis : LocationMirrorAxis)
	{

	case ERMAMirrorAnimationAxisWithNull::AxisX:
		LLocationResult = FVector(-LLocationResult.X, LLocationResult.Y, LLocationResult.Z);
		break;
	case ERMAMirrorAnimationAxisWithNull::AxisY:
		LLocationResult = FVector(LLocationResult.X, -LLocationResult.Y, LLocationResult.Z);
		break;
	case ERMAMirrorAnimationAxisWithNull::AxisZ:
		LLocationResult = FVector(LLocationResult.X, LLocationResult.Y, -LLocationResult.Z);
		break;
	default:
		break;

	}

	LLocationResult = TargetRefLocation + LLocationResult;

	return LLocationResult;

}

FQuat4f URMAMirrorAnimationMirrorTable::MirrorRotationToOtherPose(const FQuat4f& SourceRotation, const FQuat4f& SourceRefRotation, const FQuat4f& TargetRefRotation, const FRMAMirrorAnimationDoubleBoneConfig& BoneConfig)
{

	FQuat4f LRotationResult = (SourceRefRotation.Inverse() * SourceRotation);

	switch ((BoneConfig.RotationMirrorAxis != ERMAMirrorAnimationAxisWithNull::AxisNull) 
		? BoneConfig.RotationMirrorAxis : RotationMirrorAxis)
	{

	case ERMAMirrorAnimationAxisWithNull::AxisX:
		LRotationResult = FQuat4f(LRotationResult.X, -LRotationResult.Y, -LRotationResult.Z, LRotationResult.W);
		break;
	case ERMAMirrorAnimationAxisWithNull::AxisY:
		LRotationResult = FQuat4f(-LRotationResult.X, LRotationResult.Y, -LRotationResult.Z, LRotationResult.W);
		break;
	case ERMAMirrorAnimationAxisWithNull::AxisZ:
		LRotationResult = FQuat4f(-LRotationResult.X, -LRotationResult.Y, LRotationResult.Z, LRotationResult.W);
		break;
	default:
		break;

	}

	LRotationResult = TargetRefRotation * LRotationResult;
	return LRotationResult;

}
