// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OOSArcadeLadder.generated.h"

// Forward declarations
class AOOSPawn;
class UOOSGameInstance;
/**
 * A series of fights for arcade mode.
 */
UCLASS(Abstract, Blueprintable)
class ORIGINOFSTORMSMASTER_API UOOSArcadeLadder : public UObject
{
	GENERATED_BODY()

public:
	UOOSArcadeLadder();

    UFUNCTION(BlueprintCallable)
    UOOSGameInstance* GetGameInstance() const;

	UFUNCTION(BLueprintCallable)
		void Initialize();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arcade")
		TArray<TSubclassOf<AOOSPawn>> GetPossibleOpponents();

	UFUNCTION(BlueprintCallable, Category = "Arcade")
		TArray<TSubclassOf<AOOSPawn>> DrawHand(TArray<TSubclassOf<AOOSPawn>> Deck, int Count, TSubclassOf<AOOSPawn> rivalPawn, TSubclassOf<AOOSPawn> bossPawn, TSubclassOf<AOOSPawn> MechaPawn);

	UFUNCTION(BlueprintCallable, Category = "Arcade")
		bool IsFirstMatch() const;
	UFUNCTION(BlueprintCallable, Category = "Arcade")
		bool IsArcadeComplete() const;

	UFUNCTION(BlueprintCallable, Category = "Arcade")
		void MoveToNextOpponent();

	UFUNCTION(BlueprintCallable, Category = "Arcade")
		TSubclassOf<AOOSPawn> CurrentOpponent() const;
	UFUNCTION(BlueprintCallable, Category = "Arcade")
		float CurrentDifficulty() const;

	UPROPERTY(Category = Difficulty, VisibleAnywhere, BlueprintReadWrite)
		float BaseDifficulty;
	UPROPERTY(Category = Difficulty, VisibleAnywhere, BlueprintReadWrite)
		float MaxDifficulty;

	UPROPERTY(Category = Arcade, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AOOSPawn>> Opponents;

	UPROPERTY(Category = Arcade, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int ArcadeIndex = 0;

  
	
};
