// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "OOSCapsuleComponent.h"
#include "OOSHurtbox.generated.h"

#define HITBOX_OBJ ECollisionChannel::ECC_GameTraceChannel1
#define PROJECTILE_OBJ ECollisionChannel::ECC_GameTraceChannel1

/**
*
*/

USTRUCT(BlueprintType)
struct FOOSOverlapInfo
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OOSOverlapInfo")
		AActor* OtherActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OOSOverlapInfo")
		UPrimitiveComponent* ThisHB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OOSOverlapInfo")
		UPrimitiveComponent* OtherHB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OOSOverlapInfo")
		float Penetration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OOSOverlapInfo")
		FVector ImpactPoint;
};

UCLASS(meta = (BlueprintSpawnableComponent))
class ORIGINOFSTORMSMASTER_API UOOSHurtbox : public UOOSCapsuleComponent
{
	GENERATED_BODY()

public:

	UOOSHurtbox();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void BeginPlay() override;
	

	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite) float DebugThickness = 1.f;
};
