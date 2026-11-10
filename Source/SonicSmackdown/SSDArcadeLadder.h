// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SSDArcadeLadder.generated.h"

// Forward declarations
class ASSDPawn;
class USSDGameInstance;
/**
 * A series of fights for arcade mode.
 */
UCLASS(Abstract, Blueprintable)
class SONICSMACKDOWN_API USSDArcadeLadder : public UObject
{
	GENERATED_BODY()

public:
	USSDArcadeLadder();

    UFUNCTION(BlueprintCallable)
    USSDGameInstance* GetGameInstance() const;

	UFUNCTION(BLueprintCallable)
		void Initialize();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arcade")
		TArray<TSubclassOf<ASSDPawn>> GetPossibleOpponents();

	UFUNCTION(BlueprintCallable, Category = "Arcade")
		TArray<TSubclassOf<ASSDPawn>> DrawHand(TArray<TSubclassOf<ASSDPawn>> Deck, int Count, TSubclassOf<ASSDPawn> rivalPawn, TSubclassOf<ASSDPawn> bossPawn, TSubclassOf<ASSDPawn> MechaPawn);

	UFUNCTION(BlueprintCallable, Category = "Arcade")
		bool IsFirstMatch() const;
	UFUNCTION(BlueprintCallable, Category = "Arcade")
		bool IsArcadeComplete() const;

	UFUNCTION(BlueprintCallable, Category = "Arcade")
		void MoveToNextOpponent();

	UFUNCTION(BlueprintCallable, Category = "Arcade")
		TSubclassOf<ASSDPawn> CurrentOpponent() const;
	UFUNCTION(BlueprintCallable, Category = "Arcade")
		float CurrentDifficulty() const;

	UPROPERTY(Category = Difficulty, VisibleAnywhere, BlueprintReadWrite)
		float BaseDifficulty;
	UPROPERTY(Category = Difficulty, VisibleAnywhere, BlueprintReadWrite)
		float MaxDifficulty;

	UPROPERTY(Category = Arcade, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<ASSDPawn>> Opponents;

	UPROPERTY(Category = Arcade, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int ArcadeIndex = 0;

  
	
};
