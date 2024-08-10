// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/MotionProcessors/MotionProcessor.h"
#include "QuarterCircleProcessor.generated.h"

/**
 * Reads player input to look for quarter circle motions.
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UQuarterCircleProcessor : public UMotionProcessor
{
	GENERATED_BODY()

public:
	static UQuarterCircleProcessor* Make(UObject* Outer, EOOSInputDir Direction)
	{
		UQuarterCircleProcessor* Result = NewObject<UQuarterCircleProcessor>(Outer);
		Result->SetDirection(Direction);
		return Result;
	}
	// Use when called from the constructor of a UObject
	static UQuarterCircleProcessor* MakeSubobject(UObject* Outer, FName Name, EOOSInputDir Direction)
	{
		UQuarterCircleProcessor* Result = Outer->CreateDefaultSubobject<UQuarterCircleProcessor>(Name);
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
	UQuarterCircleProcessor();

};
