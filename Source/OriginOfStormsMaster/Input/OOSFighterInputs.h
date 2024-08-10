// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OOSMove.h"
#include "OOSFighterInputs.generated.h"

// Forward declarations
class UMotionProcessor;
class AOOSPlayerController;

// Data structure for buffering moves and build combos in BP.
USTRUCT(BlueprintType)
struct FOOSInputMove
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EOOSPatternType PatternType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EOOSDirPattern DirPattern;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EOOSInputDir Direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EOOSInputAttack Attack;

	double Time;

	FOOSInputMove()
	{
		PatternType = EOOSPatternType::OOSPT_None;
		DirPattern = EOOSDirPattern::OOSDP_None;
		Direction = EOOSInputDir::OOSID_None;
		Attack = EOOSInputAttack::OOSIA_None;
		Time = 0;
	};

	FOOSInputMove(EOOSPatternType Type, EOOSDirPattern DP, EOOSInputDir Dir, EOOSInputAttack Att, double T)
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
		PatternType = EOOSPatternType::OOSPT_None;
		DirPattern = EOOSDirPattern::OOSDP_None;
		Direction = EOOSInputDir::OOSID_None;
		Attack = EOOSInputAttack::OOSIA_None;
		Time = 0;
	}
};

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSFighterInputs : public UObject
{
	GENERATED_BODY()

public:
	UOOSFighterInputs();

	void SetMotionProcessors();

	void Tick(float DeltaTime);

	UFUNCTION() void AttackPressed(EOOSInputAttack InputAttack);
	UFUNCTION() void AttackReleased(EOOSInputAttack InputAttack);
	UFUNCTION() void CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New);

	AOOSPlayerController* PCOwner;

	UPROPERTY(Category = InputBufferDir, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FOOSInputMove> MoveBuffer;

	// D-Pad state exposed to BP.
	UPROPERTY(Category = Input, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EOOSInputDir DPadState = EOOSInputDir::OOSID_None;
	// Pressed attack state exposed to BP.
	UPROPERTY(Category = Input, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EOOSInputAttack AttackState = EOOSInputAttack::OOSIA_None;

	bool CanSuperJump();

	double SuperJump = 0;

	UPROPERTY()
		TArray<UMotionProcessor*> MotionProcessors;

};
