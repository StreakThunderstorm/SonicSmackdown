// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SSDMove.h"
#include "SSDGameInstance.generated.h"

// Forward declarations
class ASSDPawn;
class ASSDPawn_Transformed;
class USSDArcadeLadder;

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API USSDGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USSDGameInstance();

	UPROPERTY(VisibleAnywhere, BLueprintReadWrite)
	int P1ControllerID = -1;
	UPROPERTY(VisibleAnywhere, BLueprintReadWrite)
	int P2ControllerID = -1;
	UPROPERTY(VisibleAnywhere, BLueprintReadWrite)
	int MenuControllerID = -1;

	UPROPERTY(VisibleAnywhere, BLueprintReadWrite)
		bool bDisclaimerAccepted = false;
	UPROPERTY(VisibleAnywhere, BLueprintReadWrite)
		bool bDisclaimerAcceptedOnce = false;

	UPROPERTY(Category = Players, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) TSubclassOf<ASSDPawn> P1Char;
	UPROPERTY(Category = Players, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) TSubclassOf<ASSDPawn_Transformed> P1Transformed;
	UPROPERTY(Category = Players, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) TSubclassOf<ASSDPawn> P2Char;
	UPROPERTY(Category = Players, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) TSubclassOf<ASSDPawn_Transformed> P2Transformed;

	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bTrainingMode;
	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bTrainingAutoRefill = true;
	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bVersusCPU;
	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bCPUVersusCPU;
	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bArcadeMode;
	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float AILevel;
	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) int TrainingRingsRegen = 50;

	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) int TrainingRingsIndex;

	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) USSDArcadeLadder* ArcadeLadder;

	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float TransformGain = 1.f;
	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float SuperGain = 1.f;

	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bAttackDataVisible;
	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bInputsVisible = true;
	UPROPERTY(Category = Mode, BlueprintReadOnly, meta = (AllowPrivateAccess = "true")) bool bHitboxesVisible;
	UPROPERTY(Category = Mode, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bFrameDataVisible;

	UFUNCTION(BlueprintCallable)
		void ResetModeData();

	UFUNCTION(BlueprintCallable)
		bool IsSinglePlayerMode() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FSSDPlayerBindings> Bindings;
	UFUNCTION(BlueprintCallable)
	FSSDPlayerBindings GetKeyboardDefaults() const
	{
		return FSSDPlayerBindings::KeyboardDefaults();
	}

	UFUNCTION(BlueprintCallable)
	FSSDPlayerBindings GetGamepadDefaults() const
	{
		return FSSDPlayerBindings::GamepadDefaults();
	}

	UFUNCTION(BlueprintCallable)
	FSSDPlayerBindings GetNullBindings() const
	{
		return FSSDPlayerBindings::NullBindings();
	}

	UFUNCTION(BlueprintCallable)
	void RestoreBindingDefaults(int Index);

	UFUNCTION(BlueprintCallable)
	void SetNullBindingsToDefaults();

	UFUNCTION(BlueprintCallable)
	FKey GetInvalidKey() const
	{
		return EKeys::Invalid;
	}

}
;