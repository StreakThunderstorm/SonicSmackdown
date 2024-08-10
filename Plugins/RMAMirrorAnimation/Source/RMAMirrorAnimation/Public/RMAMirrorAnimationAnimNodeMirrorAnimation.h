// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "Animation/AnimNodeBase.h"
#include "RMAMirrorAnimationMirrorTable.h"
#include "RMAMirrorAnimationAnimNodeMirrorAnimation.generated.h"

//Stats
DECLARE_STATS_GROUP(TEXT("RMAMirrorAnimation"), STATGROUP_RMAMirrorAnimation, STATCAT_Advanced);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Mirror Bones"), STAT_MirrorBones, STATGROUP_RMAMirrorAnimation, RMAMIRRORANIMATION_API);

//AnimNode (Runtime) Used To Mirror Animation In Blueprint Animation.
USTRUCT(BlueprintType)
struct RMAMIRRORANIMATION_API FRMAMirrorAnimationAnimNodeMirrorAnimation : public FAnimNode_Base
{
	
	GENERATED_BODY()

	FRMAMirrorAnimationAnimNodeMirrorAnimation();
	
public:

	//Pose
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
		FPoseLink InputPose;

	//Enable MirrorSystem
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AlwaysAsPin), Category = "MirrorAnimation")
		bool Enabled;

	//MirrorTable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MirrorAnimation")
		URMAMirrorAnimationMirrorTable* MirrorTable;

private:

	//Called When The Node First Runs
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;

	//Called To Cache Any Bones That This Node Needs To Track
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;

	//Called To Update The State Of The Graph Relative To This Node
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;

	//Called To Evaluate Local-Space Bones Transforms
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;

	//Called To Gather On-Screen Debug Data
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;

};
