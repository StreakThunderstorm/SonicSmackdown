// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/MotionProcessors/MotionProcessor.h"
#include "DragonPunchProcessor.generated.h"

/**
 * Reads player input to look for dragon punch motions.
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UDragonPunchProcessor : public UMotionProcessor
{
	GENERATED_BODY()

public:
	static UDragonPunchProcessor* Make(UObject* Outer, EOOSInputDir Direction)
	{
		UDragonPunchProcessor* Result = NewObject<UDragonPunchProcessor>(Outer);
		Result->SetDirection(Direction);
		return Result;
	}
	// Use when called from the constructor of a UObject
	static UDragonPunchProcessor* MakeSubobject(UObject* Outer, FName Name, EOOSInputDir Direction)
	{
		UDragonPunchProcessor* Result = Outer->CreateDefaultSubobject<UDragonPunchProcessor>(Name);
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
	UDragonPunchProcessor();

	void RemoveMotionsOnBack();
	
};
