// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/MotionProcessors/MotionProcessor.h"
#include "ReverseQuarterCircleProcessor.generated.h"

/**
 * Reads player input to look for quarter circle motions.
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UReverseQuarterCircleProcessor : public UMotionProcessor
{
	GENERATED_BODY()

public:
	static UReverseQuarterCircleProcessor* Make(UObject* Outer, EOOSInputDir Direction)
	{
		UReverseQuarterCircleProcessor* Result = NewObject<UReverseQuarterCircleProcessor>(Outer);
		Result->SetDirection(Direction);
		return Result;
	}
	// Use when called from the constructor of a UObject
	static UReverseQuarterCircleProcessor* MakeSubobject(UObject* Outer, FName Name, EOOSInputDir Direction)
	{
		UReverseQuarterCircleProcessor* Result = Outer->CreateDefaultSubobject<UReverseQuarterCircleProcessor>(Name);
		Result->SetDirection(Direction);
		return Result;
	}

	void CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New) override;

protected:
	void SetDirection(EOOSInputDir Direction) override;

	int CompletedMotionIndex() const override;

	EOOSPatternType GetPatternType() const override;
	EOOSDirPattern GetPattern() const override;

private:
	// Defined so that the UObject system works
	UReverseQuarterCircleProcessor();

};
