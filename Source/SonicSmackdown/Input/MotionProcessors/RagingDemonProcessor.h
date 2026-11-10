// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/MotionProcessors/MotionProcessor.h"
#include "RagingDemonProcessor.generated.h"

/**
 * Reads player input to look for raging demon motions.
 */
UCLASS()
class SONICSMACKDOWN_API URagingDemonProcessor : public UMotionProcessor
{
	GENERATED_BODY()

public:
	static URagingDemonProcessor* Make(UObject* Outer, ESSDInputDir Direction)
	{
		URagingDemonProcessor* Result = NewObject<URagingDemonProcessor>(Outer);
		Result->SetDirection(Direction);
		return Result;
	}
	// Use when called from the constructor of a UObject
	static URagingDemonProcessor* MakeSubobject(UObject* Outer, FName Name, ESSDInputDir Direction)
	{
		URagingDemonProcessor* Result = Outer->CreateDefaultSubobject<URagingDemonProcessor>(Name);
		Result->SetDirection(Direction);
		return Result;
	}

	void CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New) override;
	void AttackButton(ESSDInputAttack InputAttack) override;

protected:
	int CompletedMotionIndex() const override;

	ESSDPatternType GetPatternType() const override;
	ESSDDirPattern GetPattern() const override;

	float MotionTimeout() const override;

private:
	// Defined so that the UObject system works
	URagingDemonProcessor();

};
