// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "SSDCamera.generated.h"

class ASSDPawn;

struct SSDCameraSettings
{
	SSDCameraSettings(
		ASSDPawn* InP1,
		ASSDPawn* InP2,
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

	ASSDPawn* P1;
	ASSDPawn* P2;
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
class USSDCameraArmShaker : public UObject
{
	GENERATED_BODY()

public:
	USSDCameraArmShaker() {};

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
class USSDHitShaker : public USSDCameraArmShaker
{
	GENERATED_BODY()

public:
	USSDHitShaker() {};

	virtual void Update(const float& DeltaTime, const FVector& ArmOrigin, FVector& InOutLoc, FRotator& InOutRot) override;
};

UCLASS()
class USSDBounceShaker : public USSDCameraArmShaker
{
	GENERATED_BODY()

public:
	USSDBounceShaker() {};

	virtual void Update(const float& DeltaTime, const FVector& ArmOrigin, FVector& InOutLoc, FRotator& InOutRot) override;
};

/**
 *
 */
UCLASS()
class SONICSMACKDOWN_API ASSDCamera : public ACameraActor
{
	GENERATED_BODY()

public:
	ASSDCamera();

	virtual void Tick(float DeltaTime) override;

	void Init(const SSDCameraSettings& Settings);
	void Reset();

	void SetActorToFollow(ASSDPawn* InPawn) { PlayerToFollow = InPawn; };

	void SetP1(ASSDPawn* InP1) { P1 = InP1 ? InP1 : P1; }
	void SetP2(ASSDPawn* InP2) { P2 = InP2 ? InP2 : P2; }

	FVector2D GetP1ScreenPos() const;
	FVector2D GetP2ScreenPos() const;

private:
	ASSDPawn* P1;
	ASSDPawn* P2;
	ASSDPawn* PlayerToFollow = nullptr;

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
		USSDHitShaker* HitShaker;
	UPROPERTY(transient)
		USSDBounceShaker* BounceShaker;

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