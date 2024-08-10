// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "OOSStageBounds.generated.h"

UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSStageBounds : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AOOSStageBounds();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(Category = Bounds, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float HorizontalBoundsHalfExtent = 2000.f;
	UPROPERTY(Category = Bounds, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float CeilingHeight = 2000.f;
	UPROPERTY(Category = Bounds, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float VolumeThickness = 2000.f;

	UPROPERTY(Category = PlayerSettings, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float MaxDistance = 800.f;
	UPROPERTY(Category = PlayerSettings, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float SpawnDistance = 200.f;

private:

	UBoxComponent * LeftBound;
	UBoxComponent * RightBound;
	UBoxComponent * FloorBound;
	UBoxComponent * CeilingBound;

};
