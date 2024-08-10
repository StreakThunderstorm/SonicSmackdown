// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "OOSCamera.generated.h"

class AOOSPawn;

struct OOSCameraSettings
{
	OOSCameraSettings(
		AOOSPawn* InP1,
		AOOSPawn* InP2,
		float InFoV,
		float InHScrollPadding,
		float InUpScrollPadding,
		float InMinDistance,
		float InBottomBorder,
		float InStageHalfWidth,
		float InStageHeight,
		float InEasing,
		float InZoomPitchBias,
		float InZoomLiftBias,
		float InShakeOrbitOffset,
		float InMaxHitShake,
		float InHitShakeSpeed,
		float InHitShakeTime,
		float InMaxBounceShake,
		float InBounceShakeSpeed,
		float InBounceShakeTime
	)
		: P1(InP1)
		, P2(InP2)
		, FoV(InFoV)
		, HScrollPadding(InHScrollPadding)
		, UpScrollPadding(InUpScrollPadding)
		, MinDistance(InMinDistance)
		, BottomBorder(InBottomBorder)
		, StageHalfWidth(InStageHalfWidth)
		, StageHeight(InStageHeight)
		, Easing(InEasing)
		, ZoomPitchBias(InZoomPitchBias)
		, ZoomLiftBias(InZoomLiftBias)
		, ShakeOrbitOffset(InShakeOrbitOffset)
		, MaxHitShake(InMaxHitShake)
		, HitShakeSpeed(InHitShakeSpeed)
		, HitShakeTime(InHitShakeTime)
		, MaxBounceShake(InMaxBounceShake)
		, BounceShakeSpeed(InBounceShakeSpeed)
		, BounceShakeTime(InBounceShakeTime)
	{
	}

	AOOSPawn* P1;
	AOOSPawn* P2;
	float FoV;
	float HScrollPadding;
	float UpScrollPadding;
	float MinDistance;
	float BottomBorder;
	float StageHalfWidth;
	float StageHeight;
	float Easing;
	float ZoomPitchBias;
	float ZoomLiftBias;
	float ShakeOrbitOffset;
	float MaxHitShake;
	float HitShakeSpeed;
	float HitShakeTime;
	float MaxBounceShake;
	float BounceShakeSpeed;
	float BounceShakeTime;
};

UCLASS()
class UOOSCameraArmShaker : public UObject
{
	GENERATED_BODY()

public:
	UOOSCameraArmShaker() {};

	void Start(const float& InScale);
	void Stop();
	virtual void Update(const float& DeltaTime, const FVector& ArmOrigin, FVector& InOutLoc, FRotator& InOutRot);

	void Setup(const float& InAmplitude, const float& InSpeed, const float& InDuration);

	float GetCurrentValue();

private:
	bool bIsRunning = false;
	float Offset;
	float Duration;
	float Speed;
	float Amplitude;
	float Scale = 1.f;
	float EaseIn;
	float EaseOut;
};

UCLASS()
class UOOSHitShaker : public UOOSCameraArmShaker
{
	GENERATED_BODY()

public:
	UOOSHitShaker() {};

	virtual void Update(const float& DeltaTime, const FVector& ArmOrigin, FVector& InOutLoc, FRotator& InOutRot) override;
};

UCLASS()
class UOOSBounceShaker : public UOOSCameraArmShaker
{
	GENERATED_BODY()

public:
	UOOSBounceShaker() {};

	virtual void Update(const float& DeltaTime, const FVector& ArmOrigin, FVector& InOutLoc, FRotator& InOutRot) override;
};

/**
 *
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSCamera : public ACameraActor
{
	GENERATED_BODY()

public:
	AOOSCamera();

	virtual void Tick(float DeltaTime) override;

	void Init(const OOSCameraSettings& Settings);
	void Reset();

	void SetActorToFollow(AOOSPawn* InPawn) { PlayerToFollow = InPawn; };

	void SetP1(AOOSPawn* InP1) { P1 = InP1 ? InP1 : P1; }
	void SetP2(AOOSPawn* InP2) { P2 = InP2 ? InP2 : P2; }

	FVector2D GetP1ScreenPos() const;
	FVector2D GetP2ScreenPos() const;

private:
	AOOSPawn* P1;
	AOOSPawn* P2;
	AOOSPawn* PlayerToFollow = nullptr;

	float MinDistance;
	float BottomBorderMin;
	// We need this to block camera at corners.
	float StageTopEdge;
	float StageLeftEdge;
	float StageRightEdge;
	float HScrollPadding;
	float VScrollPadding;
	float Easing;
	float ZoomPitchBias;
	float ZoomLiftBias;

	float ShakeOrbitOffset;

	bool bInitialized = false;

	float GetInverseAspectRatio() const;

public:
	void HitShake(const float& InScale);
	void BounceShake(const float& InScale);

private:
	UPROPERTY(transient)
		UOOSHitShaker* HitShaker;
	UPROPERTY(transient)
		UOOSBounceShaker* BounceShaker;

	// Target location excluding zoom lift and shake.
	FVector Location;
	// Target location excluding shake.
	FVector FinalLocation;
	// Target rotation excluding shake.
	FRotator Rotation;

public:
	UFUNCTION(BlueprintCallable)
		float GetDistance() const;

};