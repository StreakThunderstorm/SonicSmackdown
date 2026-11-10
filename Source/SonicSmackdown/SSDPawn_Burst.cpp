// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDPawn_Burst.h"
#include "SSDSonicRing.h"
#include "SSDMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

bool ASSDPawn_Burst::IsTransformReady() const
{
	bool Amount = Transform >= MAX_TRANSFORM_POINTS;

	return !IsTransformCoolingDown() && !IsTransformLockedOut() && Amount;
}

void ASSDPawn_Burst::TransformPressed()
{
	StartTransformation();
}

void ASSDPawn_Burst::TransformReleased()
{

}

void ASSDPawn_Burst::Kill()
{
	//Disable collision.
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MovementComponent->bDontLand = true;

	SetActorToFollow(Opponent);// Snap camera to winner.
	MovementComponent->bOnGround = false;
	MovementComponent->XSpeed = 0.f;
	MovementComponent->YSpeed = 12.f;
	MovementComponent->ZSpeed = 3.f;

	PlayAnim(InstantDeath, true);

	MovementComponent->UpdateOrientation();
}

bool ASSDPawn_Burst::IgnoreDeathAnims() const
{
	return true;
}

void ASSDPawn_Burst::ScatterRings(int Amt)
{
	UWorld* World = GetWorld();
	if (!World || !GameMode || !RingClass || (Amt <= 0)) return;

	if (RingLossSound) UGameplayStatics::PlaySound2D(World, RingLossSound);

	// Rings in the outer circle spawn faster.
	float Speed = 4.f;

	// Rotating cursor for setting the ring spawn velocity.
	FVector Cursor1 = FVector::UpVector;
	FVector Cursor2 = FVector::UpVector;
	Cursor1 = Cursor1.RotateAngleAxis(11.25f, GetActorRightVector());
	Cursor2 = Cursor2.RotateAngleAxis(-11.25f, GetActorRightVector());

	FTransform SpawnTransform = FTransform(GetActorRotation(), GetActorLocation());

	for (int i = 0; i < Amt; i++)
	{
		// This is used to move rings to both sides of the pawn alternately.
		bool bOdd = bool(i % 2);

		// Spawn new ring using deferred spawning so we can disable and set the ring up before it's registered overlapping the pawn and counting as a pickup.
		ASSDSonicRing* Ring = World->SpawnActorDeferred<ASSDSonicRing>(RingClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Ring->SetDisableTimer();
		Ring->Mode = ESSDSonicRingMode::RM_Free;
		Ring->Box->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Ring->FinishSpawning(SpawnTransform);
		Ring->SetVelocity((bOdd ? Cursor1 : Cursor2) * Speed);

		// Rotate cursors.
		if (bOdd)
		{
			Cursor1 = Cursor1.RotateAngleAxis(22.5f, GetActorRightVector());
			Cursor2 = Cursor2.RotateAngleAxis(-22.5f, GetActorRightVector());
		}

		// Reduce speed and reset cursors for inner circle.
		if (i == 15)
		{
			Speed = 2.f;
			Cursor1 = FVector::UpVector;
			Cursor2 = FVector::UpVector;
			Cursor1 = Cursor1.RotateAngleAxis(11.25f, GetActorRightVector());
			Cursor2 = Cursor2.RotateAngleAxis(-11.25f, GetActorRightVector());
		}
	}
}

void ASSDPawn_Burst::StartTransformation()
{
	if (IsRecoveringOnGround() || IsKO() || CanOnlyWalk() || !IsTransformReady() || Opponent->PawnState == ESSDPawnState::SSDPS_Transform || bBlockBurst) return;
	// No bursts during supers or ultras or if the opponent is bursting
	if (IsPerformingUltra() || bIsPerformingSuper || Opponent->IsPerformingUltra() || Opponent->bIsPerformingSuper) return;

	// If this is a defensive burst
	bool InRecovery = IsStunned() || IsLaunched() ||
		IsChargingTransform() || IsTransforming() || IsTurning() ||
		IsRecoveringOnGround();
	if (InRecovery || IsBlocking() || IsCrouchBlocking() || IsFloored())
	{
		DefensiveTransform = true;
	}

	// We may be bursting out of a grab box mid-throw.
	OnThrowCollision.Broadcast(true, FVector::ZeroVector);

	SetPawnState(ESSDPawnState::SSDPS_Transform);
	PlayAnim(Transformation);
	
	Transform = 0;
}

void ASSDPawn_Burst::FinishTransformation()
{
	SetPawnState(ESSDPawnState::SSDPS_Idle);
}
