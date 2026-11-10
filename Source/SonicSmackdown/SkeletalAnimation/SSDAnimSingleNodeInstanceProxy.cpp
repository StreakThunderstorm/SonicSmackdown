// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SSDAnimSingleNodeInstanceProxy.h"
#include "SSDAnimSingleNodeInstance.h"
#include "Animation/AnimMontage.h"
#include "RMAMirrorAnimationMirrorTable.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void FSSDAnimSingleNodeInstanceProxy::Initialize(UAnimInstance* InAnimInstance)
{
	Super::Initialize(InAnimInstance);
	PreviousPose.Reset();
	PreviousPoseBoneIndices.Reset();
	PreviousPoseAsset.Reset();
	PreviousPoseBoneSerial = 0;
}

void FSSDAnimSingleNodeInstanceProxy::CacheBones()
{
	Super::CacheBones();
	// The engine calls CacheBones before every evaluation, so the held pose must not be cleared here.
	// RestoreEvaluatedPose already rejects a held pose whose skeleton, bone layout or LOD changed.
}

void FSSDAnimSingleNodeInstanceProxy::CacheEvaluatedPose(const FPoseContext& Output)
{
	Output.Pose.CopyBonesTo(PreviousPose);
	PreviousPoseBoneIndices = Output.Pose.GetBoneContainer().GetBoneIndicesArray();
	PreviousCurve.CopyFrom(Output.Curve);
	PreviousAttributes.CopyFrom(Output.CustomAttributes);
	PreviousPoseAsset = Output.Pose.GetBoneContainer().GetAsset();
	PreviousPoseBoneSerial = Output.Pose.GetBoneContainer().GetSerialNumber();
	bPreviousPoseOrientation = bOrientation;
}

bool FSSDAnimSingleNodeInstanceProxy::RestoreEvaluatedPose(FPoseContext& Output)
{
	if (PreviousPose.IsEmpty() || PreviousPose.Num() != Output.Pose.GetNumBones()
		|| PreviousPoseAsset.Get() != Output.Pose.GetBoneContainer().GetAsset()
		|| PreviousPoseBoneSerial == 0 || PreviousPoseBoneSerial != Output.Pose.GetBoneContainer().GetSerialNumber()
		|| PreviousPoseBoneIndices != Output.Pose.GetBoneContainer().GetBoneIndicesArray())
		return false;

	Output.Pose.CopyBonesFrom(PreviousPose);
	Output.Curve.CopyFrom(PreviousCurve);
	Output.CustomAttributes.CopyFrom(PreviousAttributes);
	if (MirrorTable && bPreviousPoseOrientation != bOrientation)
		MirrorPose(Output);
	return true;
}

bool FSSDAnimSingleNodeInstanceProxy::Evaluate(FPoseContext& Output)
{
	Super::Evaluate(Output);

	// Temporary opt-in diagnostics for the reported Rouge recovery-facing snap.
	const bool bTraceFacing = FParse::Param(FCommandLine::Get(), TEXT("SSDFacingTrace")) && GetSkelMeshComponent()
		&& GetSkelMeshComponent()->GetOwner() && GetSkelMeshComponent()->GetOwner()->GetClass()->GetName().Contains(TEXT("Rouge"));
	const FRotator RawRoot = bTraceFacing ? Output.Pose[FCompactPoseBoneIndex(0)].Rotator() : FRotator::ZeroRotator;

	float SlotWeight = 1.f;
	float SourceWeight = 0.f;
	float TotalWeight = 1.f;
	UAnimMontage* Montage = Cast<UAnimMontage>(CurrentAsset);
	if (Montage && !SingleNode.ActiveMontageSlot.IsNone())
		GetSlotWeight(SingleNode.ActiveMontageSlot, SlotWeight, SourceWeight, TotalWeight);
	const bool bEmptyMontageSlot = Montage && SlotWeight <= ZERO_ANIMWEIGHT_THRESH;

	// Temporary body/slot diagnostics complement the root trace during validation.
	auto TraceBody = [&](const TCHAR* Phase)
	{
		if (!bTraceFacing)
			return;
		const FBoneContainer& Bones = Output.Pose.GetBoneContainer();
		for (FCompactPoseBoneIndex BoneIndex : Output.Pose.ForEachBoneIndex())
		{
			const FName BoneName = Bones.GetReferenceSkeleton().GetBoneName(Bones.MakeMeshPoseIndex(BoneIndex).GetInt());
			if (BoneName != TEXT("Hips") && BoneName != TEXT("HipsTranslation") && BoneName != TEXT("Head"))
				continue;
			UE_LOG(LogTemp, Display, TEXT("SSDFacing Body Frame=%llu Phase=%s Asset=%s Bone=%s Slot=%.6f Source=%.6f Weight=%.6f Quat=%s"),
				GFrameCounter, Phase, *GetNameSafe(CurrentAsset), *BoneName.ToString(), SlotWeight, SourceWeight, BlendingWeight,
				*Output.Pose[BoneIndex].GetRotation().ToString());
		}
	};
	TraceBody(TEXT("Raw"));

	// A finished montage evaluates the reference pose before its end callback starts the next animation.
	// Keep the last evaluated pose so the callback snapshots that pose instead of blending from the reference pose.
	if (bEmptyMontageSlot && RestoreEvaluatedPose(Output))
	{
		TraceBody(TEXT("Held"));
		return true;
	}
	if (!CurrentAsset)
		PreviousPose.Reset();

	if(MirrorTable && !bOrientation) MirrorPose(Output);

	const FRotator MirroredRoot = bTraceFacing ? Output.Pose[FCompactPoseBoneIndex(0)].Rotator() : FRotator::ZeroRotator;
	if (bTraceFacing)
		UE_LOG(LogTemp, Display, TEXT("SSDFacing Evaluate Frame=%llu Owner=%s Orientation=%d Weight=%.6f Delta=%.6f Raw=%s Mirrored=%s"),
			GFrameCounter, *GetSkelMeshComponent()->GetOwner()->GetName(), bOrientation, BlendingWeight, GetDeltaSeconds(),
			*RawRoot.ToString(), *MirroredRoot.ToString());
	if (BlendingWeight >= 1.f)
	{
		if (CurrentAsset && !bEmptyMontageSlot)
			CacheEvaluatedPose(Output);
		TraceBody(TEXT("Final"));
		return true;
	}
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

	if (bTraceFacing)
		UE_LOG(LogTemp, Display, TEXT("SSDFacing Blended Frame=%llu Owner=%s Weight=%.6f Root=%s"), GFrameCounter,
			*GetSkelMeshComponent()->GetOwner()->GetName(), BlendingWeight, *Output.Pose[FCompactPoseBoneIndex(0)].Rotator().ToString());
	if (CurrentAsset && !bEmptyMontageSlot)
		CacheEvaluatedPose(Output);
	TraceBody(TEXT("Final"));
	return true;
}

void FSSDAnimSingleNodeInstanceProxy::SetMirrorTable(URMAMirrorAnimationMirrorTable* InTable)
{
	if (MirrorTable != InTable)
		PreviousPose.Reset();
	MirrorTable = InTable;
}

void FSSDAnimSingleNodeInstanceProxy::UpdateOrientation(bool bInOrientation)
{
	bOrientation = bInOrientation;
}

void FSSDAnimSingleNodeInstanceProxy::RestartBlending()
{
	BlendingWeight = 0.f;
}

void FSSDAnimSingleNodeInstanceProxy::CancelBlending()
{
	BlendingWeight = 1.f;
}

void FSSDAnimSingleNodeInstanceProxy::MirrorPose(FPoseContext& Output)
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
				Output.Pose[LBoneIndex].SetRotation(FQuat(MirrorTable->MirrorRotation(FQuat4f(LBoneTransform.GetRotation()),
					FQuat4f(LBoneRefTransform.GetRotation()), MirrorTable->SingleBoneConfig[LIndex])));

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
				Output.Pose[LBoneAIndex].SetRotation(FQuat(MirrorTable->MirrorRotationToOtherPose(FQuat4f(LBoneBTransform.GetRotation()),
					FQuat4f(LBoneBRefTransform.GetRotation()), FQuat4f(LBoneARefTransform.GetRotation()), MirrorTable->DoubleBoneConfig[LIndex])));

				//Mirror Rotation (BoneB)
				Output.Pose[LBoneBIndex].SetRotation(FQuat(MirrorTable->MirrorRotationToOtherPose(FQuat4f(LBoneATransform.GetRotation()),
					FQuat4f(LBoneARefTransform.GetRotation()), FQuat4f(LBoneBRefTransform.GetRotation()), MirrorTable->DoubleBoneConfig[LIndex])));

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

