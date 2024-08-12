// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OOSSonicRing.generated.h"

#define FREEMODE_GRAVITY 0.09375f
#define MASTER_SCALE 120 // This one comes from S3KRM's attempt to keep original games' values explicit

class AOOSPawn;

UENUM(BlueprintType)
enum class EOOSSonicRingMode : uint8
{
	RM_Stationary	UMETA(DisplayName = "Stationary"),
	RM_Attraction	UMETA(DisplayName = "Attraction"),
	RM_Free 		UMETA(DisplayName = "Free")
};

UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSSonicRing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOOSSonicRing();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent *Mesh;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent *Sparkle;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent *Box;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
		class USphereComponent *Sphere;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
		class UAudioComponent *RingSound;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	EOOSSonicRingMode Mode = EOOSSonicRingMode::RM_Stationary;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
		float Acceleration = 15.f;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
		float MaxSpeed = 750.f;

	UPROPERTY(EditAnywhere, Category = Properties, meta = (AllowPrivateAccess = "true"))
		int HealthRecover = 1000;
	UPROPERTY(EditAnywhere, Category = Properties, meta = (AllowPrivateAccess = "true"))
		int TransformRecover = 500;

	void SetDisableTimer(float Time = -1.f);

	void SetVelocity(FVector NewVelocity) { Velocity = NewVelocity; };

	UFUNCTION(BlueprintNativeEvent)
		void OnCollected(AOOSPawn* Pawn);

	UFUNCTION(BlueprintCallable)
		void Freeze(float Duration);

	UFUNCTION(BlueprintCallable)
		void Unfreeze();

	UFUNCTION(BlueprintCallable)
		bool IsTooFarFromPawn(AOOSPawn* Pawn);

	UFUNCTION(BlueprintCallable)
		float DistanceFromPawn(AOOSPawn* Pawn);

private:

	UFUNCTION(meta = (BlueprintInternalUseOnly))
		void OnBoxOverlap(UPrimitiveComponent *Component, AActor *OtherActor, UPrimitiveComponent *OtherComponent);

	UFUNCTION(meta = (BlueprintInternalUseOnly))
		void OnSparkleFinish();

	UFUNCTION(meta = (BlueprintInternalUseOnly))
		void OnSphereOverlap(UPrimitiveComponent *Component, AActor *OtherActor, UPrimitiveComponent *OtherComponent);

	class AOOSPawn *PawnToFollow;

	float RotationSpeed = 1.f;
	FVector Velocity = FVector::ZeroVector;
	FVector Accel = FVector::ZeroVector;

public:
	UPROPERTY(EditAnywhere)
	float DefaultDisableTime = 16.f;

	UPROPERTY(EditAnywhere)
	float LifeTime = 256.f;

private:
	float DisableTimer = 0.f;
	float LifeTimer = 256.f;
	FTimerHandle FreezeTimer;

	bool bFrozen;
};
