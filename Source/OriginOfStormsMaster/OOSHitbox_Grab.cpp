// Fill out your copyright notice in the Description page of Project Settings.


#include "OOSHitbox_Grab.h"
#include "OOSPawn.h"
#include "OOSMovementComponent.h"
#include "OOSAnimNotify_Hitbox.h"
#include "Engine.h"

void UOOSHitbox_Grab::BeginPlay()
{
	Super::BeginPlay();
}



void UOOSHitbox_Grab::Initialize
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
	enum EOOSHitHeight Height,
	enum EOOSInputAttack Att,
	enum EOOSLaunchType Launch,
	bool bForceExtension,
	bool bResetExtensions,
	enum EOOSDirectionMode InDirection,
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

void UOOSHitbox_Grab::DestroyHitbox()
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
		if (FinalLaunchType == EOOSLaunchType::OOSLT_None)
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
		LaunchType = EOOSLaunchType::OOSLT_None;
	}
	else
	{
		Super::DestroyHitbox();
	}
}

void UOOSHitbox_Grab::ResetHitbox()
{
	bHasLockedOpponent = false;
	bHasLaunched = false;
	bOpponentBrokeOut = false;

	Super::ResetHitbox();
}

void UOOSHitbox_Grab::OnOpponentThrowCollided(bool bBrokeOut, FVector InImpactPoint)
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

void UOOSHitbox_Grab::MainLoop(float DeltaTime)
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
			FPOwner->Opponent->SetPawnState(EOOSPawnState::OOSPS_Stun);
			FPOwner->MovementComponent->bDoNotPush = true;
			FPOwner->Opponent->MovementComponent->bDoNotPush = true;

			if (LaunchType == EOOSLaunchType::OOSLT_None)
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
			if ((DirectionMode == EOOSDirectionMode::OOSDM_Gravitational) && (Push > 0.f))
			{
				if (Push > 0.f)
				{
					FinalLaunchSpd = GravitationalDirection * Push;
				}
				else
				{
					FinalLaunchType = EOOSLaunchType::OOSLT_None;
				}
			}

			if (FinalLaunchType != EOOSLaunchType::OOSLT_None)
			{
				FPOwner->Opponent->OnThrowCollision.AddUObject(this, &UOOSHitbox_Grab::OnOpponentThrowCollided);
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
