// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "OOSAnimSingleNodeInstanceProxy.h"
#include "OOSAnimSingleNodeInstance.h"
#include "RMAMirrorAnimation/Public/RMAMirrorAnimationMirrorTable.h"
#include "Runtime\Engine\Classes\Kismet\KismetMathLibrary.h"

bool FOOSAnimSingleNodeInstanceProxy::Evaluate(FPoseContext& Output)
{
	Super::Evaluate(Output);

	if(MirrorTable && !bOrientation) MirrorPose(Output);

	if (BlendingWeight >= 1.f) return true;
	BlendingWeight = FMath::Min(BlendingWeight + GetDeltaSeconds() * 10.f, 1.f);

	if (const FPoseSnapshot* BlendPose = GetPoseSnapshot("FDBlendPose"))
	{
		const TArray<FTransform>& LocalTMs = BlendPose->LocalTransforms;
		const FBoneContainer& Bones = Output.Pose.GetBoneContainer();

		for (FCompactPoseBoneIndex PoseBoneIndex : Output.Pose.ForEachBoneIndex())
		{
			const FMeshPoseBoneIndex MeshBoneIndex = Bones.MakeMeshPoseIndex(PoseBoneIndex);
			const int32 Index = MeshBoneIndex.GetInt();

			if (LocalTMs.IsValidIndex(Index))
			{
				Output.Pose[PoseBoneIndex] = UKismetMathLibrary::TLerp(LocalTMs[Index], Output.Pose[PoseBoneIndex], FMath::InterpEaseOut(0.f, 1.f, BlendingWeight, 2.0));
			}
		}
	}

	return true;
}

void FOOSAnimSingleNodeInstanceProxy::SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable)
{
	MirrorTable = InTable;
}

void FOOSAnimSingleNodeInstanceProxy::UpdateOrientation(bool bInOrientation)
{
	bOrientation = bInOrientation;
}

void FOOSAnimSingleNodeInstanceProxy::RestartBlending()
{
	BlendingWeight = 0.f;
}

void FOOSAnimSingleNodeInstanceProxy::CancelBlending()
{
	BlendingWeight = 1.f;
}

void FOOSAnimSingleNodeInstanceProxy::MirrorPose(FPoseContext& Output)
{
	if (MirrorTable->SingleBoneConfig.Num() > 0)
	{
		for (int LIndex = 0; LIndex < MirrorTable->SingleBoneConfig.Num(); LIndex++)
		{
			if (MirrorTable->SingleBoneConfig[LIndex].BoneName == "None")
			{
				continue;
			}

			//Bone Index
			FCompactPoseBoneIndex LBoneIndex(Output.Pose.GetBoneContainer().GetPoseBoneIndexForBoneName(MirrorTable->SingleBoneConfig[LIndex].BoneName));

			if (LBoneIndex.GetInt() >= 0)
			{
				//Transforms
				const FTransform LBoneTransform = Output.Pose[LBoneIndex];
				const FTransform LBoneRefTransform = Output.Pose.GetRefPose(LBoneIndex);

				//Mirror Rotation
				Output.Pose[LBoneIndex].SetRotation(UE::Math::TQuat<double>(MirrorTable->MirrorRotation(FQuat4f(LBoneTransform.GetRotation()),
					FQuat4f(LBoneRefTransform.GetRotation()),
					MirrorTable->SingleBoneConfig[LIndex])));

				if (MirrorTable->MirrorLocationData)
				{
					//Mirror Location
					Output.Pose[LBoneIndex].SetLocation(MirrorTable->MirrorLocation(LBoneTransform.GetLocation(), LBoneRefTransform.GetLocation(),
						FQuat4f(LBoneRefTransform.GetRotation()), MirrorTable->SingleBoneConfig[LIndex]));
				}
			}
		}
	}

	if (MirrorTable->DoubleBoneConfig.Num() > 0)
	{
		for (int LIndex = 0; LIndex < MirrorTable->DoubleBoneConfig.Num(); LIndex++)
		{
			if (MirrorTable->DoubleBoneConfig[LIndex].BoneAName == "None" || MirrorTable->DoubleBoneConfig[LIndex].BoneBName == "None")
			{
				continue;
			}

			//Bone Index
			FCompactPoseBoneIndex LBoneAIndex(Output.Pose.GetBoneContainer().GetPoseBoneIndexForBoneName(MirrorTable->DoubleBoneConfig[LIndex].BoneAName));
			FCompactPoseBoneIndex LBoneBIndex(Output.Pose.GetBoneContainer().GetPoseBoneIndexForBoneName(MirrorTable->DoubleBoneConfig[LIndex].BoneBName));

			if (LBoneAIndex.GetInt() > 0 && LBoneBIndex.GetInt() > 0)
			{
				//Keep Copy Of Bone Transform
				const FTransform LBoneATransform = Output.Pose[LBoneAIndex];
				const FTransform LBoneBTransform = Output.Pose[LBoneBIndex];

				//Keep Copy Of RefPose Transform
				const FTransform LBoneARefTransform = Output.Pose.GetRefPose(LBoneAIndex);
				const FTransform LBoneBRefTransform = Output.Pose.GetRefPose(LBoneBIndex);

				//Mirror Rotation (BoneA)
				Output.Pose[LBoneAIndex].SetRotation(UE::Math::TQuat<double>(MirrorTable->MirrorRotationToOtherPose(FQuat4f(LBoneBTransform.GetRotation()),
					FQuat4f(LBoneBRefTransform.GetRotation()),
					FQuat4f(LBoneARefTransform.GetRotation()),
					MirrorTable->DoubleBoneConfig[LIndex])));

				//Mirror Rotation (BoneB)
				Output.Pose[LBoneBIndex].SetRotation(UE::Math::TQuat<double>(MirrorTable->MirrorRotationToOtherPose(FQuat4f(LBoneATransform.GetRotation()),
					FQuat4f(LBoneARefTransform.GetRotation()),
					FQuat4f(LBoneBRefTransform.GetRotation()),
					MirrorTable->DoubleBoneConfig[LIndex])));

				//Mirror Location (BoneA)
				Output.Pose[LBoneAIndex].SetLocation(MirrorTable->MirrorLocationToOtherPose(LBoneBTransform.GetLocation(), LBoneBRefTransform.GetLocation(),
					FQuat4f(LBoneBRefTransform.GetRotation()), LBoneARefTransform.GetLocation(), FQuat4f(LBoneARefTransform.GetRotation()), MirrorTable->DoubleBoneConfig[LIndex]));

				//Mirror Location (BoneB)
				Output.Pose[LBoneBIndex].SetLocation(MirrorTable->MirrorLocationToOtherPose(LBoneATransform.GetLocation(), LBoneARefTransform.GetLocation(),
					FQuat4f(LBoneARefTransform.GetRotation()), LBoneBRefTransform.GetLocation(), FQuat4f(LBoneBRefTransform.GetRotation()), MirrorTable->DoubleBoneConfig[LIndex]));

				//Mirror Scale (BoneA, only swapping mirror pairs, so it assumes uniform scale atm)
				Output.Pose[LBoneAIndex].SetScale3D(LBoneBTransform.GetScale3D());

				//Mirror Scale (BoneB, only swapping mirror pairs, so it assumes uniform scale atm)
				Output.Pose[LBoneBIndex].SetScale3D(LBoneATransform.GetScale3D());
			}
		}
	}
}

