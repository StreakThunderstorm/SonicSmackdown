// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OOSAnimNotify_Hitbox.h"
#include "OOSProjectile.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "OOSAnimNotify_Projectile.generated.h"

/**
*
*/
UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSAnimNotify_Projectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	// Override of methods that receive animation notify begin and end events.
	virtual void Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;

	UPROPERTY(Category = Projectile, EditAnywhere) TSubclassOf<AOOSProjectile> ProjectileClass;
	UPROPERTY(Category = Projectile, EditAnywhere) int NumberOfHits = 1;
	UPROPERTY(Category = Projectile, EditAnywhere) float TimeBetweenHits = 0.1f;
	UPROPERTY(Category = Projectile, EditAnywhere) float Lifetime = 2.f;
	UPROPERTY(Category = Projectile, EditAnywhere) FVector2D Speed = FVector2D(2.f, 0.f);
	UPROPERTY(Category = Transform, EditAnywhere) FVector Location = FVector::ZeroVector;
	UPROPERTY(Category = Transform, EditAnywhere) FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY(Category = Attack, EditAnywhere) bool bGrabMode = false;
	UPROPERTY(Category = Attack, EditAnywhere) bool bForceGrab = false;
	UPROPERTY(Category = Transform, EditAnywhere) FName GrabSocket = "TorsoSocket";
	UPROPERTY(Category = Effects, EditAnywhere) UParticleSystem* HitParticle;
	UPROPERTY(Category = Effects, EditAnywhere) USoundBase* HitSound;
	UPROPERTY(Category = Effects, EditAnywhere) UParticleSystem* DeathParticle;
	UPROPERTY(Category = Effects, EditAnywhere) USoundBase* DeathSound;
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

	void SpawnProjectile(USkeletalMeshComponent* MeshComp);

};
