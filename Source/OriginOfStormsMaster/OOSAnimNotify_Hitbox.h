// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "OOSMove.h"
#include "OOSAnimNotify_Hitbox.generated.h"

UENUM(BlueprintType)
enum class EOOSHitHeight : uint8
{
	OOSHH_Low		UMETA(DisplayName = "Low"),
	OOSHH_Mid		UMETA(DisplayName = "Mid"),
	OOSHH_High		UMETA(DisplayName = "High")
};

UENUM(BlueprintType)
enum class EOOSLaunchType : uint8
{
	OOSLT_None				UMETA(DisplayName = "None"),
	OOSLT_UpShort			UMETA(DisplayName = "LaunchUpShort"),
	OOSLT_UpFar				UMETA(DisplayName = "LaunchUpFar"),	
	OOSLT_DownFaceDown		UMETA(DisplayName = "LaunchDownFaceDown"),
	OOSLT_DownFaceUp		UMETA(DisplayName = "LaunchDownFaceUp"),
	OOSLT_BackShort			UMETA(DisplayName = "LaunchBackShort"),
	OOSLT_BackFar			UMETA(DisplayName = "LaunchBackFar"),
	OOSLT_Crumble			UMETA(DisplayName = "Crumble"),
	OOSLT_ForcedUp			UMETA(DisplayName = "Forced Knockdown Up"),
	OOSLT_ForcedDown		UMETA(DisplayName = "Forced Knockdown Down"),
	OOSLT_Spiral			UMETA(DisplayName = "Spiral")
};

UENUM(BlueprintType)
enum class EOOSDirectionMode : uint8
{
	OOSDM_Directional		UMETA(DisplayName = "Directional"),
	OOSDM_TwoSided				UMETA(DisplayName = "Center"),
	OOSDM_Gravitational			UMETA(DisplayName = "Gravitational"),
};

// Forward declarations
class UOOSHitbox;

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSAnimNotify_Hitbox : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UOOSAnimNotify_Hitbox();

	// AnimNotify interface.
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;

	UPROPERTY(Category = Transform, EditAnywhere) TSubclassOf<UOOSHitbox> HitboxType;

	UPROPERTY(Category = Transform, EditAnywhere) float Radius = 20.f;
	UPROPERTY(Category = Transform, EditAnywhere) float HalfHeight = 20.f;
	UPROPERTY(Category = Transform, EditAnywhere, BlueprintReadWrite) bool bGrabMode = false;
	UPROPERTY(Category = Transform, EditAnywhere) bool bForceGrab = false;
	UPROPERTY(Category = Transform, EditAnywhere) FName GrabSocket = "TorsoSocket";
	UPROPERTY(Category = Transform, EditAnywhere) FVector Location = FVector::ZeroVector;
	UPROPERTY(Category = Transform, EditAnywhere) FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY(Category = Transform, EditAnywhere) FName SocketName = FName("None");
	UPROPERTY(Category = Effects, EditAnywhere) UParticleSystem* HitParticle;
	UPROPERTY(Category = Effects, EditAnywhere) USoundBase* HitSound;
	/* The hitbox is a burst, and has special properties */
	UPROPERTY(Category = Effects, EditAnywhere) bool bBurst = false;
	UPROPERTY(Category = Effects, EditAnywhere) int Damage = 50000;
	UPROPERTY(Category = Effects, EditAnywhere) float HitStun = 0.5f;
	UPROPERTY(Category = Effects, EditAnywhere) float BlockStun = 0.5f;
	UPROPERTY(Category = Effects, EditAnywhere) float SelfHitLag = 0.1;
	UPROPERTY(Category = Effects, EditAnywhere) float OpponentHitLag = 0.1;
	UPROPERTY(Category = Attack, EditAnywhere) float PushFactor = 1.f;
	UPROPERTY(Category = Attack, EditAnywhere) float Freeze = 0.f;
	UPROPERTY(Category = Attack, EditAnywhere) bool FreezeTwitch = true;
	UPROPERTY(Category = Attack, EditAnywhere) bool bOffTheGround = false;
	UPROPERTY(Category = Attack, EditAnywhere) bool bOverhead = false;
	UPROPERTY(Category = Attack, EditAnywhere) bool bAutoPerformChildMove = false;
	UPROPERTY(Category = Attack, EditAnywhere) bool bDontCancel = false;
	UPROPERTY(Category = Attack, EditAnywhere) bool bUnblockable = false;
	UPROPERTY(Category = Attack, EditAnywhere) bool bAntiArmor = false;
	UPROPERTY(Category = Attack, EditAnywhere) bool bAntiProjectile = false;
	UPROPERTY(Category = Attack, EditAnywhere) EOOSHitHeight HitHeight = EOOSHitHeight::OOSHH_High;
	UPROPERTY(Category = Attack, EditAnywhere) EOOSInputAttack AttackType = EOOSInputAttack::OOSIA_Light;
	UPROPERTY(Category = Launch, EditAnywhere) EOOSLaunchType Launch = EOOSLaunchType::OOSLT_None;
	/* Forces ground bounces/wall bounces/etc, even if the limit has been passed */
	UPROPERTY(Category = Launch, EditAnywhere) bool bForceComboExtension;
	/* Resets the target's ground bounce/wall bounce/etc counters */
	UPROPERTY(Category = Launch, EditAnywhere) bool bResetComboExtensions;
	UPROPERTY(Category = Launch, EditAnywhere) EOOSDirectionMode DirectionMode = EOOSDirectionMode::OOSDM_Directional;
	UPROPERTY(Category = Launch, EditAnywhere) FVector2D LaunchSpeed = FVector2D(2.f, 2.f);
	UPROPERTY(Category = Attack, EditAnywhere) bool bForceTryPostLaunchJump = false;

protected:

	class UOOSHitbox* P1Hitbox;
	class UOOSHitbox* P2Hitbox;

	void SpawnHitbox(USkeletalMeshComponent* MeshComp);
};
