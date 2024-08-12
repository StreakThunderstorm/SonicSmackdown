// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "Animation/Skeleton.h"
#include "RMAMirrorAnimationUtility.h"
#include "RMAMirrorAnimationMirrorTable.generated.h"

//Settings And Functions That Will Be Used By The Mirror System
UCLASS()
class RMAMIRRORANIMATION_API URMAMirrorAnimationMirrorTable : public UObject
{

	GENERATED_BODY()

public:

	URMAMirrorAnimationMirrorTable();

#if WITH_EDITOR
	virtual void PostLoad() override;
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif

	//Skeleton That Will Be Used To Generate Bone Config
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generator")
		USkeleton* Skeleton;

	//Keyword That Will Be Used To Identify The Side A/B
	UPROPERTY(EditAnywhere, Category = "Generator")
		TArray<FRMAMirrorAnimationBoneKeyword> BoneKeyword;

	//SingleBoneConfig (i.e: Head, Pelvis, Spine)
	UPROPERTY(EditAnywhere, Category = "Bones")
		TArray<FRMAMirrorAnimationSingleBoneConfig> SingleBoneConfig;

	//DoubleBoneConfig (i.e: Foot, Hand, Eye)
	UPROPERTY(EditAnywhere, Category = "Bones")
		TArray<FRMAMirrorAnimationDoubleBoneConfig> DoubleBoneConfig;

	//Component Right Axis
	UPROPERTY(EditAnywhere, Category = "Axis")
		ERMAMirrorAnimationAxis ComponentRightAxis;

	//Location Mirror Axis
	UPROPERTY(EditAnywhere, Category = "Axis")
		ERMAMirrorAnimationAxisWithNull LocationMirrorAxis;

	//Rotation Mirror Axis
	UPROPERTY(EditAnywhere, Category = "Axis")
		ERMAMirrorAnimationAxisWithNull RotationMirrorAxis;

	//Mirror Location Data
	UPROPERTY(EditAnywhere, Category = "Misc")
		bool MirrorLocationData;

	//File Version
	UPROPERTY(VisibleAnywhere, Category = "Misc")
		FName FileVersion;

public:

	//Setter FileVersion
	void SetFileVersion();

	//Getter FileVersion
	UFUNCTION(BlueprintPure, Category = "")
		FName GetFileVersion()
	{

		return FileVersion;

	};

	//Mirror Animations
	UFUNCTION(BlueprintCallable, Category = "")
		bool MirrorAnimations();

	//Getter Anim Selection
	UFUNCTION(BlueprintPure, Category = "")
		TArray<UAnimSequence*> GetAnimSelection();

	//Generate Bone Config
	UFUNCTION(BlueprintCallable, Category = "Generator")
		bool GenerateBoneConfig();

	//Find DoubleBone
	bool FindDoubleBone(const FString& BoneName, FString& OutBoneAName, FString& OutBoneBName);

public:

	//Mirror Location (i.e Head, Pelvis, Spine)
	UFUNCTION()
		FVector MirrorLocation(const FVector& Location, const FVector& RefLocation, const  FQuat4f& RefRotation, const FRMAMirrorAnimationSingleBoneConfig& BoneConfig);

	//Mirror Rotation (i.e Head, Pelvis, Spine)
	UFUNCTION()
		FQuat4f MirrorRotation(const FQuat4f& Rotation, const FQuat4f& RefRotation, const FRMAMirrorAnimationSingleBoneConfig& BoneConfig);

	//Mirror Location To Other Pose (i.e Foot, Hand, Eye)
	UFUNCTION()
		FVector MirrorLocationToOtherPose(const FVector& SourceLocation, const FVector& SourceRefLocation, const FQuat4f& SourceRefRotation, const FVector& TargetRefLocation, const FQuat4f& TargetRefRotation, const FRMAMirrorAnimationDoubleBoneConfig& BoneConfig);

	//Mirror Rotation To Other Pose (i.e Foot, Hand, Eye)
	UFUNCTION()
		FQuat4f MirrorRotationToOtherPose(const FQuat4f& SourceRotation, const FQuat4f& SourceRefRotation, const FQuat4f& TargetRefRotation, const FRMAMirrorAnimationDoubleBoneConfig& BoneConfig);

};
