// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "OOSMovementComponent.generated.h"

/**
 * 
 */

class AOOSPawn;

DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FLandDelegate);
DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FWallTouchDelegate);

UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:

	// Bring tick to this MovementComponent.
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

	// Class Constructor
	UOOSMovementComponent();

	// Movement stats
	UPROPERTY(Category = Stats, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bShouldFaceRight = true; // Reflects which side the Pawn actually is, based on capsules.
	UPROPERTY(Category = Stats, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bIsFacingRight = true; // Reflects which side the Pawn is facing (not always the same to bShouldFaceRight).
	UPROPERTY(Category = Stats, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bOnGround = true;
	UPROPERTY(Category = Stats, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float DistanceToOpponent = 0.f;
	UPROPERTY(Category = Stats, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float XSpeed = 0.f;
	UPROPERTY(Category = Stats, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float YSpeed = 0.f;
	UPROPERTY(Category = Stats, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float ZSpeed = 0.f;

	// Movement parameters
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bUseGravity = true;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bDoNotPush = false;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bCanMove = true;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float OverallMovementScale = 100.f;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float Gravity = 19.6f;

	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bFlightMode = false;
	
	// Temporal deceleration.
	UPROPERTY(Category = Deceleration, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bUseXDecel = false;
	UPROPERTY(Category = Deceleration, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bUseYDecel = false;
		
	UPROPERTY(Category = Deceleration, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float XDecel = 50.f;
	UPROPERTY(Category = Deceleration, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float YDecel = 50.f;

	UPROPERTY(Category = Push, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float InitialPushSpeed = 7.5f;
	UPROPERTY(Category = Push, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float PushScale = 1.f;

private:

	float PushDecel;

public:
	bool bDontLand = false;

	float GetDistanceToFloor();
	float GetFloorZ();

	float MaxDistanceToOpponent = 800.f;

	void AccelX(float Accel);
	void AccelY(float Accel);

	bool ApplyPushImpulse(float Push);

	FVector StageLocation = FVector::ZeroVector;

	FLandDelegate OnLand;
	FWallTouchDelegate OnWallTouch;

private:
	// Complete pointers to owner and its components.
	AOOSPawn *FPOwner;
	UCapsuleComponent* Capsule;
	USkeletalMeshComponent* SkeletalMesh;

	FVector VectorToOpponent;

	float PushSpeed = 0.f;

	void GndPhysics(float dTime);
	void AirPhysics(float dTime);

	void Land();
	void WallTouch();

	void Decel(float dTime);

public:
	void UpdateOrientation();

private:
	void UpdateDistance();

	float AccelInputX;
	float AccelInputY;

public:
	UFUNCTION(BlueprintCallable, Category = Position)
		FVector GetFutureDistance(UOOSMovementComponent* Other, int Frames) const;
	UFUNCTION(BlueprintCallable, Category = Position)
		FVector GetFuturePosition(int Frames) const;

	// Returns how many frames until YSpeed will be 0
	UFUNCTION(BlueprintCallable, Category = Position)
		int FramesUntilApex() const;
	// Returns how many frames until vertical position will be 0
	UFUNCTION(BlueprintCallable, Category = Position)
		int FramesUntilFloor() const;

	bool IsCloseToOpponent(float Threshold) const;
	bool IsAtPushHeight() const;
	bool IsAtPushHeight(float Height) const;
	
};
