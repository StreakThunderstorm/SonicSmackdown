// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationAnimSequenceProxy.h"
#include "RMAMirrorAnimationMirrorTable.h"
#include "../Public/RMAMirrorAnimationAnimSequenceCustomData.h"

inline USkeletalMesh* UAnimationAsset::GetPreviewMesh(bool bFindIfNotSet)
{

#if WITH_EDITORONLY_DATA

	USkeletalMesh* PreviewMesh = PreviewSkeletalMesh.LoadSynchronous();

	//If Somehow Skeleton Changes, Just Nullify It. 
	if (PreviewMesh && PreviewMesh->GetSkeleton() != Skeleton)
	{

		PreviewMesh = nullptr;
		SetPreviewMesh(nullptr);

	}

	return PreviewMesh;

#else

	return nullptr;

#endif

}

inline USkeletalMesh* UAnimationAsset::GetPreviewMesh() const
{

#if WITH_EDITORONLY_DATA

	if (!PreviewSkeletalMesh.IsValid())
	{

		PreviewSkeletalMesh.LoadSynchronous();

	}

	return PreviewSkeletalMesh.Get();

#else

	return nullptr;

#endif

}

inline void UAnimationAsset::SetPreviewMesh(USkeletalMesh* PreviewMesh, bool bMarkAsDirty /*=true*/)
{

#if WITH_EDITORONLY_DATA

	if (bMarkAsDirty)
	{

		Modify();

	}

	PreviewSkeletalMesh = PreviewMesh;

#endif

}

bool URMAMirrorAnimationAnimSequenceProxy::MirrorAnimation(URMAMirrorAnimationMirrorTable& MirrorTable)
{
	
	if (MirrorTable.Skeleton)
	{

		ResetAnimation();

		URMAMirrorAnimationAnimSequenceCustomData* LCustomData = Cast<URMAMirrorAnimationAnimSequenceCustomData>(GetAssetUserDataOfClass(URMAMirrorAnimationAnimSequenceCustomData::StaticClass()));

		if (!LCustomData)
		{

			LCustomData = NewObject<URMAMirrorAnimationAnimSequenceCustomData>();

		}

		if (LCustomData)
		{

			if (LCustomData->RawAnimationData.Num() <= 0)
			{

				LCustomData->RawAnimationData = RawAnimationData;

			}

			LCustomData->MirrorTable = &MirrorTable;
			AddAssetUserData(LCustomData);

			//Animation Data
			TArray<FRawAnimSequenceTrack> LAnimationData = RawAnimationData;

			for (int LFrame = 0; LFrame < GetNumberOfFrames(); LFrame++)
			{

				//SingleBone
				for (int LSingleBoneIndex = 0; LSingleBoneIndex < MirrorTable.SingleBoneConfig.Num(); LSingleBoneIndex++)
				{

					int LBoneIndex = AnimationTrackNames.Find(MirrorTable.SingleBoneConfig[LSingleBoneIndex].BoneName);

					if (LBoneIndex >= 0)
					{

						const FTransform LBoneRefTransform = GetSkeleton()->GetRefLocalPoses()[GetSkeletonIndexFromRawDataTrackIndex(LBoneIndex)];

						if (RawAnimationData[LBoneIndex].RotKeys.IsValidIndex(LFrame))
						{

							//Mirror Rotation
							LAnimationData[LBoneIndex].RotKeys[LFrame] = MirrorTable.MirrorRotation(RawAnimationData[LBoneIndex].RotKeys[LFrame],
								LBoneRefTransform.GetRotation(), MirrorTable.SingleBoneConfig[LSingleBoneIndex]);

						}

						if (MirrorTable.MirrorLocationData)
						{

							if (RawAnimationData[LBoneIndex].PosKeys.IsValidIndex(LFrame))
							{

								//Mirror Location
								LAnimationData[LBoneIndex].PosKeys[LFrame] = MirrorTable.MirrorLocation(RawAnimationData[LBoneIndex].PosKeys[LFrame],
									LBoneRefTransform.GetLocation(), LBoneRefTransform.GetRotation(), MirrorTable.SingleBoneConfig[LSingleBoneIndex]);

							}

						}

					}

				}

				//DoubleBone
				for (int LDoubleBoneIndex = 0; LDoubleBoneIndex < MirrorTable.DoubleBoneConfig.Num(); LDoubleBoneIndex++)
				{

					int LBoneAIndex = AnimationTrackNames.Find(MirrorTable.DoubleBoneConfig[LDoubleBoneIndex].BoneAName);
					int LBoneBIndex = AnimationTrackNames.Find(MirrorTable.DoubleBoneConfig[LDoubleBoneIndex].BoneBName);

					if (LBoneAIndex > 0 && LBoneBIndex > 0)
					{

						if (LFrame == 0)
						{

							LAnimationData[LBoneAIndex].RotKeys.Empty();
							LAnimationData[LBoneBIndex].RotKeys.Empty();
							LAnimationData[LBoneAIndex].PosKeys.Empty();
							LAnimationData[LBoneBIndex].PosKeys.Empty();
							LAnimationData[LBoneAIndex].RotKeys.SetNum(RawAnimationData[LBoneBIndex].RotKeys.Num());
							LAnimationData[LBoneBIndex].RotKeys.SetNum(RawAnimationData[LBoneAIndex].RotKeys.Num());
							LAnimationData[LBoneAIndex].PosKeys.SetNum(RawAnimationData[LBoneBIndex].PosKeys.Num());
							LAnimationData[LBoneBIndex].PosKeys.SetNum(RawAnimationData[LBoneAIndex].PosKeys.Num());

						}

						const FTransform LBoneARefTransform = GetSkeleton()->GetRefLocalPoses()[GetSkeletonIndexFromRawDataTrackIndex(LBoneAIndex)];
						const FTransform LBoneBRefTransform = GetSkeleton()->GetRefLocalPoses()[GetSkeletonIndexFromRawDataTrackIndex(LBoneBIndex)];

						if (RawAnimationData[LBoneAIndex].RotKeys.IsValidIndex(LFrame))
						{

							//Mirror Rotation
							LAnimationData[LBoneBIndex].RotKeys[LFrame] = MirrorTable.MirrorRotationToOtherPose(RawAnimationData[LBoneAIndex].RotKeys[LFrame],
								LBoneARefTransform.GetRotation(), LBoneBRefTransform.GetRotation(), MirrorTable.DoubleBoneConfig[LDoubleBoneIndex]);

						}

						if (RawAnimationData[LBoneBIndex].RotKeys.IsValidIndex(LFrame))
						{

							//Mirror Rotation
							LAnimationData[LBoneAIndex].RotKeys[LFrame] = MirrorTable.MirrorRotationToOtherPose(RawAnimationData[LBoneBIndex].RotKeys[LFrame],
								LBoneBRefTransform.GetRotation(), LBoneARefTransform.GetRotation(), MirrorTable.DoubleBoneConfig[LDoubleBoneIndex]);

						}

						if (RawAnimationData[LBoneAIndex].PosKeys.IsValidIndex(LFrame))
						{

							//Mirror Location
							LAnimationData[LBoneBIndex].PosKeys[LFrame] = MirrorTable.MirrorLocationToOtherPose(RawAnimationData[LBoneAIndex].PosKeys[LFrame], LBoneARefTransform.GetLocation(),
								LBoneARefTransform.GetRotation(), LBoneBRefTransform.GetLocation(), LBoneBRefTransform.GetRotation(), MirrorTable.DoubleBoneConfig[LDoubleBoneIndex]);

						}

						if (RawAnimationData[LBoneBIndex].PosKeys.IsValidIndex(LFrame))
						{

							//Mirror Location
							LAnimationData[LBoneAIndex].PosKeys[LFrame] = MirrorTable.MirrorLocationToOtherPose(RawAnimationData[LBoneBIndex].PosKeys[LFrame], LBoneBRefTransform.GetLocation(),
								LBoneBRefTransform.GetRotation(), LBoneARefTransform.GetLocation(), LBoneARefTransform.GetRotation(), MirrorTable.DoubleBoneConfig[LDoubleBoneIndex]);

						}

					}

				}

			}

			//Apply Animation Data
			RawAnimationData = LAnimationData;
			SourceRawAnimationData = LAnimationData;

			return true;

		}

	}

	return false;

}

void URMAMirrorAnimationAnimSequenceProxy::ResetAnimation()
{

	URMAMirrorAnimationAnimSequenceCustomData* LCustomData = Cast<URMAMirrorAnimationAnimSequenceCustomData>(GetAssetUserDataOfClass(URMAMirrorAnimationAnimSequenceCustomData::StaticClass()));

	if (LCustomData)
	{

		if (LCustomData->RawAnimationData.Num() > 0)
		{

			RawAnimationData = LCustomData->RawAnimationData;
			SourceRawAnimationData = LCustomData->RawAnimationData;

		}

	}

}
