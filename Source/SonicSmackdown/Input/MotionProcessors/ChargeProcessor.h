// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/MotionProcessors/MotionProcessor.h"
#include "ChargeProcessor.generated.h"

/**
 * Reads player input to look for charge motions.
 */
UCLASS()
class SONICSMACKDOWN_API UChargeProcessor : public UMotionProcessor
{
	GENERATED_BODY()

public:
	static UChargeProcessor* Make(UObject* Outer, ESSDInputDir Direction)
	{
		UChargeProcessor* Result = NewObject<UChargeProcessor>(Outer);
		Result->SetDirection(Direction);
		return Result;
	}
	// Use when called from the constructor of a UObject
	static UChargeProcessor* MakeSubobject(UObject* Outer, FName Name, ESSDInputDir Direction)
	{
		UChargeProcessor* Result = Outer->CreateDefaultSubobject<UChargeProcessor>(Name);
		Result->SetDirection(Direction);
		return Result;
	}

	void Tick(float DeltaTime) override;

	void CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New) override;

protected:
	int CompletedMotionIndex() const override;

	ESSDPatternType GetPatternType() const override;
	ESSDDirPattern GetPattern() const override;

private:
	// Defined so that the UObject system works
	UChargeProcessor();

	bool bCharging;
	float ChargeTime;

};
