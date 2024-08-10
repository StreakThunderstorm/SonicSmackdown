// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "OOSGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	AOOSGameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "GameMode")
		class AOOSPawn* GetPlayer(int Index) const;

	UFUNCTION(BlueprintNativeEvent, Category = "GameMode")
		void Initialized();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
		FVector2D GetP1ScreenPos() const;
	UFUNCTION(BlueprintCallable, Category = "GameMode")
		FVector2D GetP2ScreenPos() const;

	/**
	 * Changes the controller Id for a player and updates their GameInstance controller Id accordingly.
	 * If an Id another player was already using is passed in, swaps them.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameMode")
		bool SetControllerId(int PlayerIndex, int NewControllerId);

	UFUNCTION()
		void UnassignedControllerInput(AOOSUnassignedPlayerController* Controller);
	UFUNCTION()
		void UnassignController(AOOSPlayerController* Controller);

	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void ShowHitboxes(bool Visible);
	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void TrainingRefill();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int LastDamage = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int TotalDamage = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int MaxDamage = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int MaxHits = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float DamageScale = 1.f;

	class AOOSPawn* Fighter1;
	class AOOSPawn* Fighter2;
	class AOOSCamera* Camera;

private:

	UPROPERTY() TArray<APlayerController*> UnassignedControllers;
	
};
