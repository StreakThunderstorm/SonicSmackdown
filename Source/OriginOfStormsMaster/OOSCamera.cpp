// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSCamera.h"
#include "OOSPawn.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Engine.h"

void UOOSCameraArmShaker::Start(const float& InScale)
{
	bIsRunning = true;
	Offset = 0.f;
	Scale = InScale;
}

void UOOSCameraArmShaker::Stop()
{
	bIsRunning = false;
	Offset = 0.f;
}

void UOOSCameraArmShaker::Update(const float& DeltaTime, const FVector& ArmOrigin, FVector& InOutLoc, FRotator& InOutRot)
{
	if (!bIsRunning) return;

	Offset += DeltaTime;

	if (Offset >= Duration)
	{
		Stop();
	}
}

void UOOSCameraArmShaker::Setup(const float& InAmplitude, const float& InSpeed, const float& InDuration)
{
	Offset = 0.f;

	Amplitude = InAmplitude;
	Speed = InSpeed;
	Duration = InDuration;

	EaseIn = Duration * 0.125f;
	EaseOut = Duration * 0.75f;
}

float UOOSCameraArmShaker::GetCurrentValue()
{
	if (!bIsRunning) return 0.f;

	float Val = FMath::Sin(Offset * Speed) * Amplitude * Scale;

	if (Offset < EaseIn)
	{
		return Val * (Offset / EaseIn);
	}
	else if (Offset > EaseOut)
	{
		return Val * ((Duration - Offset) / EaseOut);
	}
	else return Val;
}

void UOOSHitShaker::Update(const float& DeltaTime, const FVector& ArmOrigin, FVector& InOutLoc, FRotator& InOutRot)
{
	FVector Arm = InOutLoc - ArmOrigin;

	float Current = GetCurrentValue();

	FRotator Delta = FVector::ForwardVector.RotateAngleAxis(Current, FVector::UpVector).Rotation();
	Arm = Arm.RotateAngleAxis(Current, FVector::UpVector);

	InOutLoc = ArmOrigin + Arm;
	InOutRot += Delta;

	Super::Update(DeltaTime, ArmOrigin, InOutLoc, InOutRot);
}

void UOOSBounceShaker::Update(const float& DeltaTime, const FVector& ArmOrigin, FVector& InOutLoc, FRotator& InOutRot)
{
	InOutLoc.Z += GetCurrentValue();

	Super::Update(DeltaTime, ArmOrigin, InOutLoc, InOutRot);
}

AOOSCamera::AOOSCamera()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCameraComponent()->bConstrainAspectRatio = false;

	HitShaker = CreateDefaultSubobject<UOOSHitShaker>("HitShaker");
	BounceShaker = CreateDefaultSubobject<UOOSBounceShaker>("BounceShaker");
}

void AOOSCamera::Init(const OOSCameraSettings& Settings)
{
	GetCameraComponent()->SetFieldOfView(Settings.FoV);
	P1 = Settings.P1; P2 = Settings.P2;
	MinDistance = Settings.MinDistance;
	BottomBorderMin = Settings.BottomBorder;
	StageTopEdge = Settings.StageHeight;
	StageLeftEdge = -Settings.StageHalfWidth;
	StageRightEdge = Settings.StageHalfWidth;
	HScrollPadding = Settings.HScrollPadding;
	VScrollPadding = Settings.UpScrollPadding;
	Easing = Settings.Easing;
	ZoomPitchBias = Settings.ZoomPitchBias;
	ZoomLiftBias = Settings.ZoomLiftBias;

	ShakeOrbitOffset = FMath::Clamp(Settings.ShakeOrbitOffset, 0.f, 1.f);

	HitShaker->Setup(Settings.MaxHitShake, Settings.HitShakeSpeed, Settings.HitShakeTime);
	BounceShaker->Setup(Settings.MaxBounceShake, Settings.BounceShakeSpeed, Settings.BounceShakeTime);

	float TanHalfFoV = FMath::Tan(FMath::DegreesToRadians(GetCameraComponent()->FieldOfView * 0.5f));
	float HalfWidth = MinDistance * TanHalfFoV;

	Location = FVector(0, -Settings.MinDistance, BottomBorderMin + HalfWidth * GetInverseAspectRatio());
	FinalLocation = Location;
	Rotation = FRotator::ZeroRotator;

	bInitialized = true;
}

void AOOSCamera::Reset()
{
	Location = GetActorLocation();
	Location.X = 0;

	SetActorLocation(Location);

	PlayerToFollow = nullptr;
}

FVector2D AOOSCamera::GetP1ScreenPos() const
{
	FVector2D Ret;
	GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(P1->GetActorLocation(), Ret);
	return Ret;
}

FVector2D AOOSCamera::GetP2ScreenPos() const
{
	FVector2D Ret;
	GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(P2->GetActorLocation(), Ret);
	return Ret;
}

float AOOSCamera::GetInverseAspectRatio() const
{
	int X, Y;
	GetWorld()->GetFirstPlayerController()->GetViewportSize(X, Y);
	if (Y <= 0 || X <= 0) return 0.5625f; // Default to 16:9 if the PC fails to read the viewport.
	return (float)Y / (float)X;
}

void AOOSCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bInitialized) return;

	FVector P1Loc = P1->GetActorLocation();
	FVector P2Loc = P2->GetActorLocation();

	// Bound extents.
	float P1BSX = P1->Capsule->GetScaledCapsuleRadius();
	float P1BSY = P1->Capsule->GetScaledCapsuleHalfHeight();

	float P2BSX = P2->Capsule->GetScaledCapsuleRadius();
	float P2BSY = P2->Capsule->GetScaledCapsuleHalfHeight();

	// Total bounds extremes.
	float LeftMost, RightMost, TargetTop;
	// Corrected stage edges (edge +/- player H bounds +/- HScrollPadding)
	float C_StageLeft, C_StageRight;

	if (P1Loc.X < P2Loc.X)
	{
		LeftMost = P1Loc.X - P1BSX - HScrollPadding;
		RightMost = P2Loc.X + P2BSX + HScrollPadding;

		C_StageLeft = StageLeftEdge - P1BSX - HScrollPadding;
		C_StageRight = StageRightEdge + P2BSX + HScrollPadding;
	}
	else
	{
		LeftMost = P2Loc.X - P2BSX - HScrollPadding;
		RightMost = P1Loc.X + P1BSX + HScrollPadding;

		C_StageLeft = StageLeftEdge - P2BSX - HScrollPadding;
		C_StageRight = StageRightEdge + P1BSX + HScrollPadding;
	}

	float P1Top = P1Loc.Z + P1BSY;
	float P2Top = P2Loc.Z + P2BSY;
	if ((PlayerToFollow && PlayerToFollow->PlayerIndex == 0) || (!PlayerToFollow && (P1Top > P2Top)))
	{
		TargetTop = P1Top + VScrollPadding;
	}
	else
	{
		TargetTop = P2Top + VScrollPadding;
	}

	float TanHalfFoV = FMath::Tan(FMath::DegreesToRadians(GetCameraComponent()->FieldOfView * 0.5f));
	float HalfDist = FMath::Abs(RightMost - LeftMost) * 0.5f;

	// Adjust zoom.
	float TargetDist = (HalfDist) / TanHalfFoV;
	float FinalDist = FMath::Max(MinDistance, TargetDist);
	Location.Y = FinalDist;

	// Calculate screen edges with corrected zoom.
	float HalfWidth = FinalDist * TanHalfFoV;
	float HalfHeight = HalfWidth * GetInverseAspectRatio();
	float LeftEdge = Location.X - HalfWidth;
	float RightEdge = Location.X + HalfWidth;
	float TopEdge = Location.Z + HalfHeight;
	float BottomEdge = Location.Z - HalfHeight;

	// HScroll.
	if (LeftMost < LeftEdge)
	{
		Location.X += LeftMost - LeftEdge;
	}
	else if (RightMost > RightEdge)
	{
		Location.X += RightMost - RightEdge;
	}

	// VScroll.
	Location.Z += TargetTop - TopEdge;

	// Make sure the camera doesn't scroll past limits.
	Location.X = FMath::Clamp(Location.X, C_StageLeft + (HalfWidth), C_StageRight - (HalfWidth));
	Location.Z = FMath::Clamp(Location.Z, BottomBorderMin + HalfHeight, StageTopEdge - HalfHeight);

	// Calculate zoom out pitch and lift.
	float Pitch = -FinalDist + MinDistance;
	Pitch *= ZoomPitchBias * 0.01f;

	float Lift = FinalDist - MinDistance;
	Lift *= ZoomLiftBias * 0.01f;

	FinalLocation = FMath::VInterpTo(FinalLocation, Location + FVector(0, 0, Lift), DeltaTime, Easing);
	Rotation = FMath::RInterpTo(Rotation, FRotator(Pitch, -90.f, 0.f), DeltaTime, Easing);

	FVector FinalLoc_WithShake = FinalLocation;
	FRotator FinalRot_WithShake = Rotation;

	// Update cam shake.
	FVector ArmOrigin = FVector((RightMost - LeftMost) * 0.5f, 0, Location.Z);
	ArmOrigin = FMath::Lerp(ArmOrigin, FinalLoc_WithShake, ShakeOrbitOffset);
	HitShaker->Update(DeltaTime, ArmOrigin, FinalLoc_WithShake, FinalRot_WithShake);
	BounceShaker->Update(DeltaTime, ArmOrigin, FinalLoc_WithShake, FinalRot_WithShake);

	SetActorTransform(FTransform(FinalRot_WithShake, FinalLoc_WithShake, FVector::OneVector));
}

void AOOSCamera::HitShake(const float& InScale)
{
	BounceShaker->Stop();
	HitShaker->Start(InScale);
}

void AOOSCamera::BounceShake(const float& InScale)
{
	HitShaker->Stop();
	BounceShaker->Start(InScale);
}

float AOOSCamera::GetDistance() const
{
	return GetActorLocation().X;
}
