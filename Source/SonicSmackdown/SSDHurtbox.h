// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SSDCapsuleComponent.h"
#include "SSDHurtbox.generated.h"

#define HITBOX_OBJ ECollisionChannel::ECC_GameTraceChannel1
#define PROJECTILE_OBJ ECollisionChannel::ECC_GameTraceChannel1

/**
*
*/

USTRUCT(BlueprintType)
struct FSSDOverlapInfo
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SSDOverlapInfo")
		AActor* OtherActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SSDOverlapInfo")
		UPrimitiveComponent* ThisHB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SSDOverlapInfo")
		UPrimitiveComponent* OtherHB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SSDOverlapInfo")
		float Penetration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SSDOverlapInfo")
		FVector ImpactPoint;
};

UCLASS(meta = (BlueprintSpawnableComponent))
class SONICSMACKDOWN_API USSDHurtbox : public USSDCapsuleComponent
{
	GENERATED_BODY()

public:

	USSDHurtbox();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void BeginPlay() override;
	

	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite) float DebugThickness = 1.f;
};
