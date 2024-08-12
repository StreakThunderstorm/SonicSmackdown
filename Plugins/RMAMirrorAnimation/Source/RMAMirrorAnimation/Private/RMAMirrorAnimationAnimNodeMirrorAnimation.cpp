// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationAnimNodeMirrorAnimation.h"

DEFINE_STAT(STAT_MirrorBones);

FRMAMirrorAnimationAnimNodeMirrorAnimation::FRMAMirrorAnimationAnimNodeMirrorAnimation()
{

	Enabled = true;
	MirrorTable = nullptr;

}

void FRMAMirrorAnimationAnimNodeMirrorAnimation::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{

	FAnimNode_Base::Initialize_AnyThread(Context);

	InputPose.Initialize(Context);

}

void FRMAMirrorAnimationAnimNodeMirrorAnimation::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{

	InputPose.CacheBones(Context);

}

void FRMAMirrorAnimationAnimNodeMirrorAnimation::Update_AnyThread(const FAnimationUpdateContext& Context)
{

	GetEvaluateGraphExposedInputs().Execute(Context);

	InputPose.Update(Context);

}

void FRMAMirrorAnimationAnimNodeMirrorAnimation::Evaluate_AnyThread(FPoseContext& Output)
{

	InputPose.Evaluate(Output);

	SCOPE_CYCLE_COUNTER(STAT_MirrorBones);

	if (Enabled && MirrorTable)
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
					Output.Pose[LBoneIndex].SetRotation(FQuat4d(MirrorTable->MirrorRotation(FQuat4f(LBoneTransform.GetRotation()),
						FQuat4f(LBoneRefTransform.GetRotation()),MirrorTable->SingleBoneConfig[LIndex])));

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
					Output.Pose[LBoneAIndex].SetRotation(FQuat4d(MirrorTable->MirrorRotationToOtherPose(FQuat4f(LBoneBTransform.GetRotation()), FQuat4f(LBoneBRefTransform.GetRotation()),
						FQuat4f(LBoneARefTransform.GetRotation()), MirrorTable->DoubleBoneConfig[LIndex])));
					
					//Mirror Rotation (BoneB)
					Output.Pose[LBoneBIndex].SetRotation(FQuat4d(MirrorTable->MirrorRotationToOtherPose(FQuat4f(LBoneATransform.GetRotation()), FQuat4f(LBoneARefTransform.GetRotation()),
						FQuat4f(LBoneBRefTransform.GetRotation()), MirrorTable->DoubleBoneConfig[LIndex])));

					//Mirror Location (BoneA)
					Output.Pose[LBoneAIndex].SetLocation(MirrorTable->MirrorLocationToOtherPose(LBoneBTransform.GetLocation(), LBoneBRefTransform.GetLocation(), 
						FQuat4f(LBoneBRefTransform.GetRotation()), LBoneARefTransform.GetLocation(), FQuat4f(LBoneARefTransform.GetRotation()), MirrorTable->DoubleBoneConfig[LIndex]));

					//Mirror Location (BoneB)
					Output.Pose[LBoneBIndex].SetLocation(MirrorTable->MirrorLocationToOtherPose(LBoneATransform.GetLocation(), LBoneARefTransform.GetLocation(), 
						FQuat4f(LBoneARefTransform.GetRotation()), LBoneBRefTransform.GetLocation(), FQuat4f(LBoneBRefTransform.GetRotation()), MirrorTable->DoubleBoneConfig[LIndex]));

				}

			}

		}

	}

}

void FRMAMirrorAnimationAnimNodeMirrorAnimation::GatherDebugData(FNodeDebugData& DebugData)
{

	InputPose.GatherDebugData(DebugData.BranchFlow(1.f));

}
