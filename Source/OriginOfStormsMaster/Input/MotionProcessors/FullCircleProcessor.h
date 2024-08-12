// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionProcessor.h"
#include "FullCircleProcessor.generated.h"

/**
 * Reads player input to look for full circle motions.
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UFullCircleProcessor : public UMotionProcessor
{
	GENERATED_BODY()

public:
	static UFullCircleProcessor* Make(UObject* Outer, EOOSInputDir Direction)
	{
		UFullCircleProcessor* Result = NewObject<UFullCircleProcessor>(Outer);
		Result->SetDirection(Direction);
		return Result;
	}
	// Use when called from the constructor of a UObject
	static UFullCircleProcessor* MakeSubobject(UObject* Outer, FName Name, EOOSInputDir Direction)
	{
		UFullCircleProcessor* Result = Outer->CreateDefaultSubobject<UFullCircleProcessor>(Name);
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
	UFullCircleProcessor();

};
