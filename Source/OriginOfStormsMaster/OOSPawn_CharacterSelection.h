// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OOSPawn_CharacterSelection.generated.h"

UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSPawn_CharacterSelection : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AOOSPawn_CharacterSelection();

protected:
	// Ticks the skeletal mesh with zero delta time to immediately refresh to the current frame
	UFUNCTION(BlueprintCallable, Category = "Super")
		void RefreshAnimation(USkeletalMeshComponent* SkeletalMesh);

};
