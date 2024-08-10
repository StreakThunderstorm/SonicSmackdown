// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/MotionProcessors/MotionProcessor.h"
#include "RagingDemonProcessor.generated.h"

/**
 * Reads player input to look for raging demon motions.
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API URagingDemonProcessor : public UMotionProcessor
{
	GENERATED_BODY()

public:
	static URagingDemonProcessor* Make(UObject* Outer, EOOSInputDir Direction)
	{
		URagingDemonProcessor* Result = NewObject<URagingDemonProcessor>(Outer);
		Result->SetDirection(Direction);
		return Result;
	}
	// Use when called from the constructor of a UObject
	static URagingDemonProcessor* MakeSubobject(UObject* Outer, FName Name, EOOSInputDir Direction)
	{
		URagingDemonProcessor* Result = Outer->CreateDefaultSubobject<URagingDemonProcessor>(Name);
		Result->SetDirection(Direction);
		return Result;
	}

	void CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New) override;
	void AttackButton(EOOSInputAttack InputAttack) override;

protected:
	int CompletedMotionIndex() const override;

	EOOSPatternType GetPatternType() const override;
	EOOSDirPattern GetPattern() const override;

	float MotionTimeout() const override;

private:
	// Defined so that the UObject system works
	URagingDemonProcessor();

};
