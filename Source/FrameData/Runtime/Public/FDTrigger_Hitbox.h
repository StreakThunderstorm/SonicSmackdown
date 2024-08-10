// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FDTrigger.h"
#include "FDTrigger_Hitbox.generated.h"

/**
 * 
 */
UCLASS()
class FRAMEDATARUNTIME_API UFDTrigger_Hitbox : public UFDTrigger
{
	GENERATED_BODY()

public:

	UFDTrigger_Hitbox();

	UPROPERTY(Category = "Hitbox | Transform", EditAnywhere) float Radius = 20.f;
	UPROPERTY(Category = "Hitbox | Transform", EditAnywhere) float HalfHeight = 20.f;
	UPROPERTY(Category = "Hitbox | Transform", EditAnywhere) bool bGrabMode = false;
	UPROPERTY(Category = "Hitbox | Transform", EditAnywhere) FVector Location = FVector::ZeroVector;
	UPROPERTY(Category = "Hitbox | Transform", EditAnywhere) FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY(Category = "Hitbox | Transform", EditAnywhere) FName SocketName = FName("None");
	UPROPERTY(Category = "Hitbox | Effects", EditAnywhere) UParticleSystem* HitParticle;
	UPROPERTY(Category = "Hitbox | Effects", EditAnywhere) USoundBase* HitSound;
	UPROPERTY(Category = "Hitbox | Effects", EditAnywhere) int Damage = 50000;
	UPROPERTY(Category = "Hitbox | Effects", EditAnywhere) float HitStun = 0.5f;
	UPROPERTY(Category = "Hitbox | Effects", EditAnywhere) float SelfHitLag = 0.1;
	UPROPERTY(Category = "Hitbox | Effects", EditAnywhere) float OpponentHitLag = 0.1;
	UPROPERTY(Category = "Hitbox | Effects", EditAnywhere) float BlockStun = 0.1;
	UPROPERTY(Category = "Hitbox | Attack", EditAnywhere) float PushFactor = 1.f;
	UPROPERTY(Category = "Hitbox | Attack", EditAnywhere) float Freeze = 0.f;
	UPROPERTY(Category = "Hitbox | Attack", EditAnywhere) bool bOffTheGround = false;
	UPROPERTY(Category = "Hitbox | Attack", EditAnywhere) bool bOverhead = false;
	UPROPERTY(Category = "Hitbox | Attack", EditAnywhere) bool bAutoPerformChildMove = false;
	//UPROPERTY(Category = "Hitbox | Attack", EditAnywhere) EOOSHitHeight HitHeight = EOOSHitHeight::OOSHH_High;
	//UPROPERTY(Category = "Hitbox | Attack", EditAnywhere) EOOSInputAttack AttackType = EOOSInputAttack::OOSIA_Light;
	//UPROPERTY(Category = "Hitbox | Launch", EditAnywhere) EOOSLaunchType Launch = EOOSLaunchType::OOSLT_None;
	UPROPERTY(Category = "Hitbox | Launch", EditAnywhere) FVector2D LaunchSpeed = FVector2D(2.f, 2.f);
	UPROPERTY(Category = "Hitbox | Launch", EditAnywhere) bool bForceTryPostLaunchJump = false;

	virtual void TriggerStart() override;
	virtual void TriggerTick() override;
	virtual void TriggerEnd() override;
};
