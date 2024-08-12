// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationAnimSequenceProxy.h"
#include "RMAMirrorAnimationAnimSequenceCustomData.h"
#include "RMAMirrorAnimationMirrorTable.h"
#include "AnimPose.h"

TArray<FBoneAnimationTrack> GetAnimationData(UAnimSequence* Seq)
{

	TArray<FBoneAnimationTrack> LResult;

	if (Seq)
	{

		TMap<FName, FBoneAnimationTrack> LMap;

		for (int32 LFrame = 0; LFrame < Seq->GetNumberOfSampledKeys(); LFrame++)
		{

			FAnimPose LPose;
			UAnimPoseExtensions::GetAnimPoseAtFrame(Seq, LFrame, FAnimPoseEvaluationOptions(), LPose);

			FPermissionListOwners LBones;
			UAnimPoseExtensions::GetBoneNames(LPose, LBones);

			for (auto LBone : LBones)
			{

				const auto LTransform = UAnimPoseExtensions::GetBonePose(LPose, LBone);
				auto LTrack = LMap.FindOrAdd(LBone);
				if (LTrack.Name.IsNone())
				{
					LTrack.Name = LBone;
				}

				LTrack.InternalTrackData.PosKeys.Add(FVector3f(LTransform.GetLocation()));
				LTrack.InternalTrackData.RotKeys.Add(FQuat4f(LTransform.GetRotation()));
				LTrack.InternalTrackData.ScaleKeys.Add(FVector3f(LTransform.GetScale3D()));

				LMap.Add(LBone, LTrack);

			}

		}

		LMap.GenerateValueArray(LResult);

	}

	return LResult;

}

FTransform GetRefPose(USkeleton* Skeleton, FName BoneName)
{

	FTransform LResult;

	if (Skeleton && !BoneName.IsNone())
	{

		FAnimPose LPose;
		UAnimPoseExtensions::GetReferencePose(Skeleton, LPose);
		LResult = UAnimPoseExtensions::GetRefBonePose(LPose, BoneName);

	}

	return LResult;

}

bool URMAMirrorAnimationAnimSequenceProxy::MirrorAnimation(URMAMirrorAnimationMirrorTable& MirrorTable)
{

	if (GetDataModel() && GetSkeleton() && MirrorTable.Skeleton)
	{

		auto LCustomData = Cast<URMAMirrorAnimationAnimSequenceCustomData>(GetAssetUserDataOfClass(URMAMirrorAnimationAnimSequenceCustomData::StaticClass()));
		if (!LCustomData)
		{

			LCustomData = NewObject<URMAMirrorAnimationAnimSequenceCustomData>(this);

		}

		if (LCustomData)
		{

			ResetAnimation();
			if (LCustomData->AnimationData.Num() <= 0)
			{

				LCustomData->AnimationData = GetAnimationData(this);
				LCustomData->MirrorTable = &MirrorTable;
				AddAssetUserData(LCustomData);

			}

			if (LCustomData->AnimationData.Num() > 0)
			{

				//Animation Data
				auto LAnimationData = LCustomData->AnimationData;
				const auto LAnimationDataConst = LAnimationData;

				//Getter Bone Index
				auto GetBoneIndex = [&](FName Name) -> int
				{

					for (int LIndex = 0; LIndex < LAnimationData.Num(); LIndex++)
					{

						if (LAnimationData[LIndex].Name == Name)
						{

							return LIndex;

						}

					}

					return -1;

				};

				//Track Names
				TArray<FName> LTrackNames;
				GetDataModel()->GetBoneTrackNames(LTrackNames);

				//Mirror
				for (int32 LKey = 0; LKey < GetNumberOfSampledKeys(); LKey++)
				{

					//Single Bone
					for (int LSingleBoneIndex = 0; LSingleBoneIndex < MirrorTable.SingleBoneConfig.Num(); LSingleBoneIndex++)
					{

						const auto LBoneConfig = MirrorTable.SingleBoneConfig[LSingleBoneIndex];
						const auto LBoneIndex = GetBoneIndex(LBoneConfig.BoneName);
						if (LBoneIndex >= 0)
						{

							const auto LBoneRefTransform = GetRefPose(GetSkeleton(), LBoneConfig.BoneName);

							if (LAnimationDataConst[LBoneIndex].InternalTrackData.RotKeys.IsValidIndex(LKey))
							{

								//Mirror Rotation
								LAnimationData[LBoneIndex].InternalTrackData.RotKeys[LKey] = MirrorTable.MirrorRotation(LAnimationDataConst[LBoneIndex].InternalTrackData.RotKeys[LKey],
									FQuat4f(LBoneRefTransform.GetRotation()), LBoneConfig);

							}

							if (MirrorTable.MirrorLocationData)
							{

								if (LAnimationDataConst[LBoneIndex].InternalTrackData.PosKeys.IsValidIndex(LKey))
								{

									//Mirror Location
									LAnimationData[LBoneIndex].InternalTrackData.PosKeys[LKey] = FVector3f(MirrorTable.MirrorLocation(FVector(LAnimationDataConst[LBoneIndex].InternalTrackData.PosKeys[LKey]),
										LBoneRefTransform.GetLocation(), FQuat4f(LBoneRefTransform.GetRotation()), LBoneConfig));

								}

							}

						}

					}

					//Double Bone
					for (int LDoubleBoneIndex = 0; LDoubleBoneIndex < MirrorTable.DoubleBoneConfig.Num(); LDoubleBoneIndex++)
					{

						const auto LBoneConfig = MirrorTable.DoubleBoneConfig[LDoubleBoneIndex];
						const auto LBoneAIndex = GetBoneIndex(LBoneConfig.BoneAName);
						const auto LBoneBIndex = GetBoneIndex(LBoneConfig.BoneBName);
						if (LBoneAIndex > 0 && LBoneBIndex > 0)
						{

							if (LKey == 0)
							{

								LAnimationData[LBoneAIndex].InternalTrackData.RotKeys.Empty();
								LAnimationData[LBoneBIndex].InternalTrackData.RotKeys.Empty();
								LAnimationData[LBoneAIndex].InternalTrackData.PosKeys.Empty();
								LAnimationData[LBoneBIndex].InternalTrackData.PosKeys.Empty();
								LAnimationData[LBoneAIndex].InternalTrackData.RotKeys.SetNum(LAnimationDataConst[LBoneBIndex].InternalTrackData.RotKeys.Num());
								LAnimationData[LBoneBIndex].InternalTrackData.RotKeys.SetNum(LAnimationDataConst[LBoneAIndex].InternalTrackData.RotKeys.Num());
								LAnimationData[LBoneAIndex].InternalTrackData.PosKeys.SetNum(LAnimationDataConst[LBoneBIndex].InternalTrackData.PosKeys.Num());
								LAnimationData[LBoneBIndex].InternalTrackData.PosKeys.SetNum(LAnimationDataConst[LBoneAIndex].InternalTrackData.PosKeys.Num());

							}

							const auto LBoneARefTransform = GetRefPose(GetSkeleton(), LBoneConfig.BoneAName);
							const auto LBoneBRefTransform = GetRefPose(GetSkeleton(), LBoneConfig.BoneBName);

							if (LAnimationDataConst[LBoneAIndex].InternalTrackData.RotKeys.IsValidIndex(LKey))
							{

								//Mirror Rotation
								LAnimationData[LBoneBIndex].InternalTrackData.RotKeys[LKey] = MirrorTable.MirrorRotationToOtherPose(LAnimationDataConst[LBoneAIndex].InternalTrackData.RotKeys[LKey],
									FQuat4f(LBoneARefTransform.GetRotation()), FQuat4f(LBoneBRefTransform.GetRotation()), LBoneConfig);

							}

							if (LAnimationDataConst[LBoneBIndex].InternalTrackData.RotKeys.IsValidIndex(LKey))
							{

								//Mirror Rotation
								LAnimationData[LBoneAIndex].InternalTrackData.RotKeys[LKey] = MirrorTable.MirrorRotationToOtherPose(LAnimationDataConst[LBoneBIndex].InternalTrackData.RotKeys[LKey],
									FQuat4f(LBoneBRefTransform.GetRotation()), FQuat4f(LBoneARefTransform.GetRotation()), LBoneConfig);

							}

							if (LAnimationDataConst[LBoneAIndex].InternalTrackData.PosKeys.IsValidIndex(LKey))
							{

								//Mirror Location
								LAnimationData[LBoneBIndex].InternalTrackData.PosKeys[LKey] = FVector3f(MirrorTable.MirrorLocationToOtherPose(FVector(LAnimationDataConst[LBoneAIndex].InternalTrackData.PosKeys[LKey]), LBoneARefTransform.GetLocation(),
									FQuat4f(LBoneARefTransform.GetRotation()), LBoneBRefTransform.GetLocation(), FQuat4f(LBoneBRefTransform.GetRotation()), LBoneConfig));

							}

							if (LAnimationDataConst[LBoneBIndex].InternalTrackData.PosKeys.IsValidIndex(LKey))
							{

								//Mirror Location
								LAnimationData[LBoneAIndex].InternalTrackData.PosKeys[LKey] = FVector3f(MirrorTable.MirrorLocationToOtherPose(FVector(LAnimationDataConst[LBoneBIndex].InternalTrackData.PosKeys[LKey]), LBoneBRefTransform.GetLocation(),
									FQuat4f(LBoneBRefTransform.GetRotation()), LBoneARefTransform.GetLocation(), FQuat4f(LBoneARefTransform.GetRotation()), LBoneConfig));

							}

						}

					}

				}

				//Controller Scoped Bracket
				//IAnimationDataController::FScopedBracket LControllerSB(GetController(), FText::FromString("Mirroring Animation"), false);

				//Apply Animation Data
				for (int32 LTrack = 0; LTrack < LAnimationDataConst.Num(); LTrack++)
				{

					GetController().SetBoneTrackKeys(LAnimationData[LTrack].Name, LAnimationData[LTrack].InternalTrackData.PosKeys,
						LAnimationData[LTrack].InternalTrackData.RotKeys, LAnimationData[LTrack].InternalTrackData.ScaleKeys, false);

				}

				//Cache Derived Data
				CacheDerivedDataForCurrentPlatform();

				return true;

			}

		}

	}

	return false;

}

void URMAMirrorAnimationAnimSequenceProxy::ResetAnimation()
{
	
	auto LCustomData = Cast<URMAMirrorAnimationAnimSequenceCustomData>(GetAssetUserDataOfClass(URMAMirrorAnimationAnimSequenceCustomData::StaticClass()));
	if (LCustomData && LCustomData->AnimationData.Num() > 0)
	{

		//Controller Scoped Bracket
		IAnimationDataController::FScopedBracket LControllerSB(GetController(), FText::FromString("Reseting Animation"), false);

		//Apply Animation Data
		for (int32 LTrack = 0; LTrack < LCustomData->AnimationData.Num(); LTrack++)
		{

			GetController().SetBoneTrackKeys(LCustomData->AnimationData[LTrack].Name, LCustomData->AnimationData[LTrack].InternalTrackData.PosKeys,
				LCustomData->AnimationData[LTrack].InternalTrackData.RotKeys, LCustomData->AnimationData[LTrack].InternalTrackData.ScaleKeys, false);

		}

		//Cache Derived Data
		CacheDerivedDataForCurrentPlatform();

	}

}
