// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/MotionProcessors/MotionProcessor.h"
#include "HalfCircleProcessor.generated.h"

/**
 * Reads player input to look for half circle motions.
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UHalfCircleProcessor : public UMotionProcessor
{
	GENERATED_BODY()

public:
	static UHalfCircleProcessor* Make(UObject* Outer, EOOSInputDir Direction)
	{
		UHalfCircleProcessor* Result = NewObject<UHalfCircleProcessor>(Outer);
		Result->SetDirection(Direction);
		return Result;
	}
	// Use when called from the constructor of a UObject
	static UHalfCircleProcessor* MakeSubobject(UObject* Outer, FName Name, EOOSInputDir Direction)
	{
		UHalfCircleProcessor* Result = Outer->CreateDefaultSubobject<UHalfCircleProcessor>(Name);
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
	UHalfCircleProcessor();

};
