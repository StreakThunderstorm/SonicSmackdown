// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SSDHitbox.h"
#include "SSDHitbox_Grab.generated.h"

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API USSDHitbox_Grab : public USSDHitbox
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void Initialize
	(
		float HalfHeight,
		float Radius,
		bool bGrab,
		bool bForceGrb,
		FName GrabSock,
		UParticleSystem* HitParticle,
		USoundBase* HitSFX,
		bool bBrst,
		int Dmg,
		float HStun,
		float SelfHL,
		float OpponentHL,
		float BStun,
		float PushFactor,
		float Frz,
		bool FrzTw,
		bool bOTG,
		bool bOH,
		bool bAutoChild,
		bool bNoCancel,
		bool bUnBlck,
		bool bAntiA,
		bool bAntiPrj, 
		enum ESSDHitHeight Height,
		enum ESSDInputAttack Att,
		enum ESSDLaunchType Launch,
		bool bForceExtension,
		bool bResetExtensions,
		enum ESSDDirectionMode Direction,
		FVector2D LaunchSpd,
		bool bForceJmp
	) override;
	virtual void DestroyHitbox() override;
	virtual void ResetHitbox() override;

	UFUNCTION()
	void OnOpponentThrowCollided(bool bBrokeOut, FVector InImpactPoint);

protected:
	virtual void MainLoop(float DeltaTime) override;
	// Throws lock the opponent on sustained contact, so they never hit from a sweep.
	virtual bool CanSweepForHits() const override { return false; }

private:
	bool bHasLockedOpponent = false;
	bool bHasLaunched = false;

	bool bOpponentBrokeOut = false;

	float Direction = 0.f;
	FVector2D FinalLaunchSpd = FVector2D::ZeroVector;
	ESSDLaunchType FinalLaunchType;

};
