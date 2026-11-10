// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SSDMove.h"
#include "SSDAnimNotify_Hitbox.generated.h"

UENUM(BlueprintType)
enum class ESSDHitHeight : uint8
{
	SSDHH_Low		UMETA(DisplayName = "Low"),
	SSDHH_Mid		UMETA(DisplayName = "Mid"),
	SSDHH_High		UMETA(DisplayName = "High")
};

UENUM(BlueprintType)
enum class ESSDLaunchType : uint8
{
	SSDLT_None				UMETA(DisplayName = "None"),
	SSDLT_UpShort			UMETA(DisplayName = "LaunchUpShort"),
	SSDLT_UpFar				UMETA(DisplayName = "LaunchUpFar"),	
	SSDLT_DownFaceDown		UMETA(DisplayName = "LaunchDownFaceDown"),
	SSDLT_DownFaceUp		UMETA(DisplayName = "LaunchDownFaceUp"),
	SSDLT_BackShort			UMETA(DisplayName = "LaunchBackShort"),
	SSDLT_BackFar			UMETA(DisplayName = "LaunchBackFar"),
	SSDLT_Crumble			UMETA(DisplayName = "Crumble"),
	SSDLT_ForcedUp			UMETA(DisplayName = "Forced Knockdown Up"),
	SSDLT_ForcedDown		UMETA(DisplayName = "Forced Knockdown Down"),
	SSDLT_Spiral			UMETA(DisplayName = "Spiral")
};

UENUM(BlueprintType)
enum class ESSDDirectionMode : uint8
{
	SSDDM_Directional		UMETA(DisplayName = "Directional"),
	SSDDM_TwoSided				UMETA(DisplayName = "Center"),
	SSDDM_Gravitational			UMETA(DisplayName = "Gravitational"),
};

// Forward declarations
class USSDHitbox;

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API USSDAnimNotify_Hitbox : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	USSDAnimNotify_Hitbox();

	// AnimNotify interface.
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(Category = Transform, EditAnywhere) TSubclassOf<USSDHitbox> HitboxType;

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
	UPROPERTY(Category = Attack, EditAnywhere) ESSDHitHeight HitHeight = ESSDHitHeight::SSDHH_High;
	UPROPERTY(Category = Attack, EditAnywhere) ESSDInputAttack AttackType = ESSDInputAttack::SSDIA_Light;
	UPROPERTY(Category = Launch, EditAnywhere) ESSDLaunchType Launch = ESSDLaunchType::SSDLT_None;
	/* Forces ground bounces/wall bounces/etc, even if the limit has been passed */
	UPROPERTY(Category = Launch, EditAnywhere) bool bForceComboExtension;
	/* Resets the target's ground bounce/wall bounce/etc counters */
	UPROPERTY(Category = Launch, EditAnywhere) bool bResetComboExtensions;
	UPROPERTY(Category = Launch, EditAnywhere) ESSDDirectionMode DirectionMode = ESSDDirectionMode::SSDDM_Directional;
	UPROPERTY(Category = Launch, EditAnywhere) FVector2D LaunchSpeed = FVector2D(2.f, 2.f);
	UPROPERTY(Category = Attack, EditAnywhere) bool bForceTryPostLaunchJump = false;

protected:

	class USSDHitbox* P1Hitbox;
	class USSDHitbox* P2Hitbox;

	void SpawnHitbox(USkeletalMeshComponent* MeshComp);
};
