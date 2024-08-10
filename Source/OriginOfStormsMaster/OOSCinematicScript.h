// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraActor.h"
#include "OOSCinematicScript.generated.h"

class AOOSPawn;

UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSCinematicScript : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOOSCinematicScript();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, Category = "CinematicScript")
		void OnCinematicStart(AOOSPawn* Pawn, AActor* Camera);
	UFUNCTION(BlueprintNativeEvent, Category = "CinematicScript")
		void OnCinematicInterrupted();
	UFUNCTION(BlueprintNativeEvent, Category = "CinematicScript")
		void OnCinematicEnd();
	
};
