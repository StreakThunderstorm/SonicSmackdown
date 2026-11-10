// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SSDMove.h"
#include "SSDFighterInputs.generated.h"

// Forward declarations
class UMotionProcessor;
class ASSDPlayerController;

// Data structure for buffering moves and build combos in BP.
USTRUCT(BlueprintType)
struct FSSDInputMove
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ESSDPatternType PatternType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ESSDDirPattern DirPattern;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ESSDInputDir Direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ESSDInputAttack Attack;

	double Time;

	FSSDInputMove()
	{
		PatternType = ESSDPatternType::SSDPT_None;
		DirPattern = ESSDDirPattern::SSDDP_None;
		Direction = ESSDInputDir::SSDID_None;
		Attack = ESSDInputAttack::SSDIA_None;
		Time = 0;
	};

	FSSDInputMove(ESSDPatternType Type, ESSDDirPattern DP, ESSDInputDir Dir, ESSDInputAttack Att, double T)
	{
		PatternType = Type;
		DirPattern = DP;
		Direction = Dir;
		Attack = Att;
		Time = T;
	};

	// Resets to an empty move.
	void Reset()
	{
		PatternType = ESSDPatternType::SSDPT_None;
		DirPattern = ESSDDirPattern::SSDDP_None;
		Direction = ESSDInputDir::SSDID_None;
		Attack = ESSDInputAttack::SSDIA_None;
		Time = 0;
	}
};

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API USSDFighterInputs : public UObject
{
	GENERATED_BODY()

public:
	USSDFighterInputs();

	void SetMotionProcessors();

	void Tick(float DeltaTime);

	UFUNCTION() void AttackPressed(ESSDInputAttack InputAttack);
	UFUNCTION() void AttackReleased(ESSDInputAttack InputAttack);
	UFUNCTION() void CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New);

	ASSDPlayerController* PCOwner;

	UPROPERTY(Category = InputBufferDir, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FSSDInputMove> MoveBuffer;

	// D-Pad state exposed to BP.
	UPROPERTY(Category = Input, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		ESSDInputDir DPadState = ESSDInputDir::SSDID_None;
	// Pressed attack state exposed to BP.
	UPROPERTY(Category = Input, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		ESSDInputAttack AttackState = ESSDInputAttack::SSDIA_None;

	bool CanSuperJump();

	double SuperJump = 0;

	UPROPERTY()
		TArray<UMotionProcessor*> MotionProcessors;

};
