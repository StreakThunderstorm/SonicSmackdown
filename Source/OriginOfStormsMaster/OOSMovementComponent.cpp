// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSMovementComponent.h"
#include "SkeletalAnimation/OOSSkeletalMeshComponent.h"
#include "Engine.h"
#include "OOSPawn.h"
#include "OOSHitbox_Grab.h"

#define PUSH_FACTOR	0.45f
#define PUSH_HEIGHT_DELTA 50.f

static const float FindFloorTolerance = 2.f;

UOOSMovementComponent::UOOSMovementComponent()
{
	SetComponentTickEnabled(true);
	bTickInEditor = true;
	bWantsInitializeComponent = true;
}

void UOOSMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);

	if (!PawnOwner) return;

	FPOwner = Cast<AOOSPawn>(PawnOwner);
	Capsule = Cast<UCapsuleComponent>(PawnOwner->GetRootComponent());
	SkeletalMesh = Cast<USkeletalMeshComponent>(FPOwner->SkeletalMesh);

	bOnGround = true;
}

//Called each frame.
void UOOSMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!UpdatedComponent || !Capsule || !FPOwner || !SkeletalMesh || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	if (FPOwner->IsFrozen()) return;

	bool bHeightOK = GetDistanceToFloor() <= FindFloorTolerance;

	// Main physics state switching
	if (bOnGround)
	{
		GndPhysics(DeltaTime);
	}
	else
	{
		if (bHeightOK && (YSpeed < 0.f) && !bDontLand && !bFlightMode)
		{
			if (OnLand.IsBound() && OnLand.Execute()) return;
			Land();
			GndPhysics(DeltaTime);
		}
		else
		{
			AirPhysics(DeltaTime);
		}
	}

	// Root motion.
	FVector RootMotionTranslation = FVector::ZeroVector;

	if (SkeletalMesh->IsPlayingRootMotion())
	{
		FRootMotionMovementParams RootMotionMovementParams = SkeletalMesh->ConsumeRootMotion();
		RootMotionTranslation = RootMotionMovementParams.GetRootMotionTransform().GetTranslation();
		RootMotionTranslation = FRotator(0, (SkeletalMesh->GetForwardVector().Rotation().Yaw), 0).RotateVector(RootMotionTranslation);
	}

	// Add any acceleration coming from the Pawn and reset.
	XSpeed += AccelInputX * DeltaTime;
	YSpeed += AccelInputY * DeltaTime;
	AccelInputX = 0.f;
	AccelInputY = 0.f;

	if (bFlightMode)
	{
		float MaxFlightSpeed = FPOwner->FlightMovementSpeed;
		XSpeed = FMath::Clamp(XSpeed, -MaxFlightSpeed, MaxFlightSpeed);
		YSpeed = FMath::Clamp(YSpeed, -MaxFlightSpeed, MaxFlightSpeed);
	}

	// Integrate component speeds into movement vector.
	// This is old. We managed movement from BP setting X and Y speeds manually. TODO implement InputVector to simplify movement in BP.
	AddInputVector(XSpeed * PawnOwner->GetActorForwardVector());
	AddInputVector(YSpeed * FVector::UpVector);

	// Consume front speed (For Smackdown final throw)
	ZSpeed -= FMath::Min(DeltaTime * 3.f, ZSpeed);
	if (!FMath::IsNearlyZero(ZSpeed))
		AddInputVector(ZSpeed * PawnOwner->GetActorRightVector());

	FVector DesiredMovementThisFrame = (ConsumeInputVector() * DeltaTime * OverallMovementScale) + (RootMotionTranslation * FPOwner->GetActorTimeDilation());

	// Save last states before moving.
	FVector LastPos = PawnOwner->GetActorLocation();
	UpdateOrientation();
	bool bShouldFaceRight_Last = bShouldFaceRight;

	// Perform move.
	if (!DesiredMovementThisFrame.IsNearlyZero())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);

		// If the capsule hit something.
		if (Hit.IsValidBlockingHit())
		{
			FVector HitToLoc = Hit.ImpactPoint - PawnOwner->GetActorLocation();
			HitToLoc = HitToLoc.ProjectOnTo(PawnOwner->GetActorForwardVector());
			bool bIsGroundHit = HitToLoc.Size() < FPOwner->Capsule->GetScaledCapsuleRadius();
			if (!bIsGroundHit)
			{
				if (OnWallTouch.IsBound() && OnWallTouch.Execute()) return;
				WallTouch();
			}

			FHitResult SlideAlongHit = Hit;
			// Project collision normal onto 2D fighting plane to avoid SlideAlongSurface to move the Pawn away.
			FVector SafeSlideNormal = FVector::VectorPlaneProject(Hit.Normal, PawnOwner->GetActorRightVector()).GetSafeNormal();
			float SlideResult = SlideAlongSurface(DesiredMovementThisFrame, 1.f - Hit.Time, SafeSlideNormal, SlideAlongHit);

			if (bIsGroundHit && YSpeed < 0.f && Hit.ImpactPoint.Z < Hit.Location.Z && !bDontLand && !bFlightMode)
			{
				if (OnLand.IsBound() && OnLand.Execute()) return;
				Land();
			}
		}
	}
	
	// We need to update again since we might have pushed through the opponent.
	UpdateOrientation();

	// Pushing against opponent.
	if (!bDoNotPush && FPOwner->Opponent)
	{
		bool Height = IsAtPushHeight();

		if (Height)
		{
			FHitResult Hit;
			bool bLastHeight = IsAtPushHeight(LastPos.Z);

			if ((bShouldFaceRight != bShouldFaceRight_Last) && bLastHeight)
			{
				//We went past the opponent while pushing, Move opponent past us.
				FVector ToOpponent = VectorToOpponent;
				ToOpponent.Z = 0.f;
				FPOwner->Opponent->MovementComponent->SafeMoveUpdatedComponent(-ToOpponent, FPOwner->Opponent->GetActorRotation(), true, Hit);

				// Both players are in the same position now, fix the orientation before updating, since the last check thinks we're on the opposite side and it won't fix the
				// orientation flag (obviously easier than moving the pusher to a non-overlapping distance and also provides a nice push effect after a hard dash or something).
				bShouldFaceRight = bShouldFaceRight_Last;
				UpdateOrientation();
			}

			float SafeDistance = Capsule->GetScaledCapsuleRadius() + FPOwner->Opponent->Capsule->GetScaledCapsuleRadius();
			bool Overlap = DistanceToOpponent < SafeDistance;

			if (Overlap)
			{
				FVector Push = PawnOwner->GetActorForwardVector() * (SafeDistance - DistanceToOpponent) * PUSH_FACTOR * (bShouldFaceRight ? 1.f : -1.f);
				FPOwner->Opponent->MovementComponent->SafeMoveUpdatedComponent(Push, FPOwner->Opponent->GetActorRotation(), true, Hit);
				if (Hit.IsValidBlockingHit())
				{
					SafeMoveUpdatedComponent(-Push * (1.f - Hit.Time), UpdatedComponent->GetComponentRotation(), true, Hit);
				}
			}
		}
	}

	// Pushing from opponent's HitBoxes.
	if (PushSpeed != 0.f)
	{
		FVector Push = PushSpeed * PawnOwner->GetActorForwardVector() * DeltaTime * OverallMovementScale;

		FHitResult Hit;
		SafeMoveUpdatedComponent(Push, UpdatedComponent->GetComponentRotation(), true, Hit);

		float Abs = FMath::Abs(PushSpeed);
		float Decel = (25000.f / (Abs * PushScale * PushDecel)) * DeltaTime;
		PushSpeed -= FMath::Min(FMath::Abs(PushSpeed), Decel) * FMath::Sign(PushSpeed);
	}

	// Check if pawn moved away from its opponent beyond max distance.
	UpdateDistance();
	if (DistanceToOpponent >= MaxDistanceToOpponent)
	{
		// Bring pawn back to max distance to opponent.
		float Correction = (DistanceToOpponent - MaxDistanceToOpponent) * (bShouldFaceRight ? 1.f : -1.f);
		PawnOwner->AddActorWorldOffset(PawnOwner->GetActorForwardVector() * Correction, false, nullptr, ETeleportType::TeleportPhysics);

		// Inform that we hit the camera bounds.
		if (OnWallTouch.IsBound() && OnWallTouch.Execute()) return;
		WallTouch();
	}
}

void UOOSMovementComponent::GndPhysics(float dTime)
{
	Decel(dTime);

	// Adjust floor location.
	FVector NewLoc = PawnOwner->GetActorLocation();
	NewLoc.Z = StageLocation.Z + FindFloorTolerance + FPOwner->Capsule->GetScaledCapsuleHalfHeight();
	PawnOwner->SetActorLocation(NewLoc);

}

void UOOSMovementComponent::AirPhysics(float dTime)
{
	Decel(dTime);
	if ((bUseGravity && !bFlightMode))
	{
		YSpeed -= Gravity * dTime;
	}
}

void UOOSMovementComponent::Decel(float dTime)
{
	if (bFlightMode && FPOwner->MovementNotify == nullptr && !bUseXDecel && !bUseYDecel)
	{
		{
			if (FMath::IsNearlyZero(AccelInputX))
				XSpeed -= FMath::Sign(XSpeed) * FMath::Min(FMath::Abs(XSpeed), (FPOwner->FlightAccel * dTime));
			if (FMath::IsNearlyZero(AccelInputY))
				YSpeed -= FMath::Sign(YSpeed) * FMath::Min(FMath::Abs(YSpeed), (FPOwner->FlightAccel * dTime));
		}
	}
	else
	{
		if (bUseXDecel)
		{
			XSpeed -= FMath::Sign(XSpeed) * FMath::Min(FMath::Abs(XSpeed), (XDecel * dTime));
			if (XSpeed == 0.f)
			{
				bUseXDecel = false; // Automatically turn off decels to avoid problems.
				FPOwner->Stopped();
			}
		}

		if (bUseYDecel)
		{
			YSpeed -= FMath::Sign(YSpeed) * FMath::Min(FMath::Abs(YSpeed), (YDecel * dTime));
			if (YSpeed == 0.f)
			{
				bUseYDecel = false;
			}
		}
	}
}

void UOOSMovementComponent::Land()
{
	if (FPOwner->FlyingCharacter && FPOwner->bIsPerformingMove)
	{
	}
	else
	{
		FVector NewLoc = PawnOwner->GetActorLocation();
		NewLoc.Z = StageLocation.Z + FPOwner->Capsule->GetScaledCapsuleHalfHeight() + FindFloorTolerance;
		PawnOwner->SetActorLocation(NewLoc);

		bOnGround = true;
		YSpeed = 0.f;

		//UpdateOrientation();
		FPOwner->OnLanded(); // Propagate landing to BP.
	}
}

void UOOSMovementComponent::WallTouch()
{
	FPOwner->OnWallTouch();
}

// We only take it out the Tick loop because we need to update as soon as we land for autojumps
void UOOSMovementComponent::UpdateOrientation()
{
	if (!FPOwner->Opponent) return;

	UpdateDistance();

	// Check what direction pawn should look at and set the flag.
	float FoD = VectorToOpponent.GetSafeNormal() | PawnOwner->GetActorForwardVector();

	float Capsules = (FPOwner->Capsule->GetScaledCapsuleRadius() + (FPOwner->Opponent->Capsule->GetScaledCapsuleRadius())) * 0.2f;
	if ((DistanceToOpponent > Capsules) || FMath::IsNearlyEqual(DistanceToOpponent, Capsules, 1.f))
	{
		bShouldFaceRight = FoD > 0.f;
	}
	else return;
	
		
	if ((bIsFacingRight == bShouldFaceRight) || FPOwner->IsKO()) return;

	// Only turn around if completely landed and idle.
	if ((FPOwner->IsNeutral() || FPOwner->IsCompletelyCrouched()) && (bOnGround || bFlightMode))
	{
		FPOwner->SetFacing(bShouldFaceRight);
		FPOwner->OnTurnaround();
	}
}

void UOOSMovementComponent::UpdateDistance()
{
	if (!FPOwner->Opponent) return;

	VectorToOpponent = FPOwner->Opponent->GetActorLocation() - PawnOwner->GetActorLocation();
	DistanceToOpponent = VectorToOpponent.ProjectOnTo(PawnOwner->GetActorForwardVector()).Size();
}

float UOOSMovementComponent::GetDistanceToFloor()
{
	return PawnOwner->GetActorLocation().Z - FPOwner->Capsule->GetScaledCapsuleHalfHeight() - StageLocation.Z;
}

float UOOSMovementComponent::GetFloorZ()
{
	return StageLocation.Z;
}

void UOOSMovementComponent::AccelX(float Accel)
{
	AccelInputX += Accel;
}

void UOOSMovementComponent::AccelY(float Accel)
{
	AccelInputY += Accel;
}

bool UOOSMovementComponent::ApplyPushImpulse(float Push)
{
	if (!FPOwner) return false;

	if (Push != 0.f)
	{
		float Sign = FMath::Sign(Push);

		float Radius = FPOwner->Capsule->GetScaledCapsuleRadius() + 2.f;
		FVector Start = PawnOwner->GetActorLocation();
		FVector End = Start + PawnOwner->GetActorForwardVector() * Sign * Radius;
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, ECollisionChannel::ECC_WorldStatic))
		{
			return false;
		}

		PushSpeed = Sign * InitialPushSpeed;
		PushDecel = FMath::Abs(Push);
	}
	else
	{
		PushSpeed = 0.f;
	}

	return true;
}

FVector UOOSMovementComponent::GetFutureDistance(UOOSMovementComponent* Other, int Frames) const
{
	return Other->GetFuturePosition(Frames) - GetFuturePosition(Frames);
}

FVector UOOSMovementComponent::GetFuturePosition(int Frames) const
{
	float FrameTickScale = OverallMovementScale / 60.f;
	float Scale = Frames  * FrameTickScale;

	FVector FutureVel = YSpeed * Scale * FVector::UpVector + XSpeed * Scale * PawnOwner->GetActorForwardVector();
	//@TODO: Gravity probably shouldn't be multiplied by DeltaTime before it's
	// added to YSpeed, but that's what happens above so
	FVector FutureAccel = FVector::ZeroVector;
	if (!bOnGround && bUseGravity)
	{
		// Triangle numbers function is n * (n + 1) / 2
		int GravityTicks = (Frames * (Frames + 1)) / 2;
		FutureAccel = (Gravity / 60.f) * (GravityTicks * FrameTickScale) * FVector::UpVector;
	}
	return (GetActorLocation() - FPOwner->Capsule->GetScaledCapsuleHalfHeight() * FVector::UpVector) + FutureVel - FutureAccel;
}

int UOOSMovementComponent::FramesUntilApex() const
{
	const float TickScale = 1 / 60.f;

	// If YSpeed is not changing, just return 0
	if (bOnGround || !bUseGravity)
	{
		return 0;
	}
	else
	{
		// Gravity * TickScale subtracted from YSpeed every frame
		float Frames = YSpeed / (Gravity * TickScale);
		// Will return a negative number if YSpeed was 0 in the past
		return FMath::CeilToInt(Frames);
	}
}

int UOOSMovementComponent::FramesUntilFloor() const
{
	const float TickScale = 1 / 60.f;
	float FrameTickScale = OverallMovementScale * TickScale;
	float ZPos = (GetActorLocation() - FPOwner->Capsule->GetScaledCapsuleHalfHeight() * FVector::UpVector).Z;

	// On the ground
	if (bOnGround)
	{
		return 0;
	}
	// Below the ground
	else if (ZPos <= StageLocation.Z)
	{
		return 0;
	}
	else
	{
		float GravityForce = 0.f;
		if (bUseGravity) GravityForce = Gravity * TickScale;

		// If speed and gravity are both 0, we aren't going anywhere so return 0
		if (FMath::IsNearlyZero(YSpeed) && FMath::IsNearlyZero(GravityForce))
		{
			return 0;
		}
		// Will return a negative number if on the floor in the past and not the future
		else
		{
			float FloorOffset = ZPos - StageLocation.Z;
			float SpeedTick = FrameTickScale * YSpeed;
			float GravityTick = FrameTickScale * GravityForce;

			//If no gravity, simple linear test
			if (FMath::IsNearlyZero(GravityForce))
			{
				float Frames = -ZPos / SpeedTick;
				return FMath::CeilToInt(Frames);
			}
			else
			{
				// Alright here's a bunch of really annoying math to solve for frames
				float RootComponent = FrameTickScale * (GravityForce * (8 * FloorOffset + GravityTick) + 4 * YSpeed * (SpeedTick - GravityTick));
				float Root = FMath::Sqrt(RootComponent);
				float Frames1 = (+Root + 2 * SpeedTick - GravityTick) / (2 * GravityTick);
				float Frames2 = (-Root + 2 * SpeedTick - GravityTick) / (2 * GravityTick);
				return FMath::CeilToInt(FMath::Max(Frames1, Frames2));
			}
		}
	}
}

bool UOOSMovementComponent::IsCloseToOpponent(float Threshold) const
{
	float Capsules = FPOwner->Capsule->GetScaledCapsuleRadius() + FPOwner->Opponent->Capsule->GetScaledCapsuleRadius();
	return DistanceToOpponent <= (Capsules + Threshold);
}

bool UOOSMovementComponent::IsAtPushHeight() const
{
	return FMath::IsNearlyEqual(PawnOwner->GetActorLocation().Z, FPOwner->Opponent->GetActorLocation().Z, PUSH_HEIGHT_DELTA);
}

bool UOOSMovementComponent::IsAtPushHeight(float Height) const
{
	return FMath::IsNearlyEqual(Height, FPOwner->Opponent->GetActorLocation().Z, PUSH_HEIGHT_DELTA);
}
