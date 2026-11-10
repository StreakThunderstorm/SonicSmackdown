// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDHitbox_Grab.h"
#include "SSDPawn.h"
#include "SSDMovementComponent.h"
#include "SSDAnimNotify_Hitbox.h"
#include "Engine.h"

void USSDHitbox_Grab::BeginPlay()
{
	Super::BeginPlay();
}



void USSDHitbox_Grab::Initialize
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
	enum ESSDDirectionMode InDirection,
	FVector2D LaunchSpd,
	bool bForceJmp
)
{
	// Call parent init forcing bUnblockable and nullifying stuns.
	Super::Initialize
	(
		HalfHeight,
		Radius,
		bGrab,
		bForceGrb,
		GrabSock,
		HitParticle,
		HitSFX,
		bBrst,
		Dmg,
		0.f,
		SelfHL,
		OpponentHL,
		0.f,
		PushFactor,
		Frz,
		FrzTw,
		bOTG,
		bOH,
		bAutoChild,
		bNoCancel,
		true,
		true,
		false,
		Height,
		Att,
		Launch,
		bForceExtension,
		bResetExtensions,
		InDirection,
		LaunchSpd,
		bForceJmp
	);
}

void USSDHitbox_Grab::DestroyHitbox()
{
	if (FPOwner)
	{
		FPOwner->Opponent->bGrabbed = false;
		FPOwner->MovementComponent->bDoNotPush = false;
		FPOwner->Opponent->MovementComponent->bDoNotPush = false;
		FPOwner->Opponent->MovementComponent->bDontLand = false;
	}

	if (FPOwner && !bOpponentBrokeOut)
	{
		if (FinalLaunchType == ESSDLaunchType::SSDLT_None)
		{
			TryHit(OverlapInfo);
			FPOwner->Opponent->bGrabbed = false;
			Super::DestroyHitbox();
		}
		else
		{
			FPOwner->Opponent->bGrabbed = false;
			FPOwner->Opponent->Launch(FinalLaunchSpd, FinalLaunchType);
			bHasLaunched = true;
			FPOwner->Opponent->MovementComponent->bIsFacingRight = FPOwner->Opponent->MovementComponent->bShouldFaceRight;
		}		
		LaunchType = ESSDLaunchType::SSDLT_None;
	}
	else
	{
		Super::DestroyHitbox();
	}
}

void USSDHitbox_Grab::ResetHitbox()
{
	bHasLockedOpponent = false;
	bHasLaunched = false;
	bOpponentBrokeOut = false;

	Super::ResetHitbox();
}

void USSDHitbox_Grab::OnOpponentThrowCollided(bool bBrokeOut, FVector InImpactPoint)
{
	// This can be executed before the animnotify even destroys the hitbox when the opponent bursts out of a throw and cancels the animation.
	bOpponentBrokeOut = bBrokeOut;

	// Clear all bindings because we assume we have one throw at a time but worth having a close look at.
	FPOwner->Opponent->OnThrowCollision.Clear();

	// Now hit the opponent since we cleared the launch type before.
	if (!bBrokeOut)
	{
		OverlapInfo.ImpactPoint = InImpactPoint;
		TryHit(OverlapInfo);
	}

	Super::DestroyHitbox();
}

void USSDHitbox_Grab::MainLoop(float DeltaTime)
{
	if (bIsMakingContact)
	{
		if (!FPOwner || !FPOwner->Opponent || FPOwner->Opponent->IsInvincible() || (!bOffTheGround && FPOwner->Opponent->IsFloored()) || bHasLaunched)
		{
			return;
		}

		if (!bHasLockedOpponent)
		{
			bHasLockedOpponent = true;
			FPOwner->Opponent->bGrabbed = true;
			FPOwner->Opponent->SetFlightModeEnabled(false);
			FPOwner->Opponent->SetPawnState(ESSDPawnState::SSDPS_Stun);
			FPOwner->MovementComponent->bDoNotPush = true;
			FPOwner->Opponent->MovementComponent->bDoNotPush = true;

			if (LaunchType == ESSDLaunchType::SSDLT_None)
			{
				FPOwner->Opponent->MovementComponent->bOnGround = true;
				FPOwner->Opponent->MovementComponent->bDontLand = true;
				FPOwner->Opponent->PlayAnim(FPOwner->Opponent->St_MidHeavy, true, 5.f);
			}
			else
			{
				FPOwner->Opponent->MovementComponent->bOnGround = false;
				FPOwner->Opponent->MovementComponent->bDontLand = true;
				FPOwner->Opponent->PlayAnim(FPOwner->Opponent->Grabbed, true, 5.f);
			}

			Direction = GetDirection();
			FinalLaunchSpd = LaunchSpeed;
			FinalLaunchSpd.X *= Direction;
			FinalLaunchType = LaunchType;
			if ((DirectionMode == ESSDDirectionMode::SSDDM_Gravitational) && (Push > 0.f))
			{
				if (Push > 0.f)
				{
					FinalLaunchSpd = GravitationalDirection * Push;
				}
				else
				{
					FinalLaunchType = ESSDLaunchType::SSDLT_None;
				}
			}

			if (FinalLaunchType != ESSDLaunchType::SSDLT_None)
			{
				FThrowCollisionDelegate::FDelegate ThrowCollisionDelegate;
				ThrowCollisionDelegate.BindUFunction(this, "OnOpponentThrowCollided");
				FPOwner->Opponent->OnThrowCollision.Add(ThrowCollisionDelegate);
			}
		}
		else if (FPOwner->Opponent->DefensiveTransform)
		{
			FPOwner->Opponent->bGrabbed = false;
			DestroyHitbox();
			return;
		}
		FHitResult Hit = GrabOpponent(false, false, DeltaTime);

		/*if (!bHasMadeContact && Hit.IsValidBlockingHit())
		{
			FVector HitToLoc = Hit.ImpactPoint - FPOwner->Opponent->GetActorLocation();
			HitToLoc = HitToLoc.ProjectOnTo(FPOwner->GetActorForwardVector());
			bool bIsGroundHit = HitToLoc.Size() < FPOwner->Capsule->GetScaledCapsuleRadius();
			if (!bIsGroundHit)
			{
				TryHit(OverlapInfo);
			}
			else
			{
				TryHit(OverlapInfo);
			}
		}*/
	}
}
