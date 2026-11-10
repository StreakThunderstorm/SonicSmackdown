// Fill out your copyright notice in the Description page of Project Settings.


#include "ChargeProcessor.h"

UChargeProcessor::UChargeProcessor() { }

void UChargeProcessor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCharging)
	{
		ChargeTime += DeltaTime;
	}
}

void UChargeProcessor::CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New)
{
	// Check opposite direction held
	ESSDInputDir Opposite = GetNeighborDir(MotionDirection, ESSDInputNeighbor::SSDIN_Opposite);
	bool OppositePressed = New == Opposite;
	// If the final direction is a cardinal direction
	if ((int)MotionDirection % 2 == 1)
	{
		// Check the adjacent diagonals as well
		OppositePressed |= New == GetNeighborDir(Opposite, ESSDInputNeighbor::SSDIN_Clockwise);
		OppositePressed |= New == GetNeighborDir(Opposite, ESSDInputNeighbor::SSDIN_AntiClockwise);
	}

	if (OppositePressed)
	{
		// If opposite direction was just pressed, start charging
		if (!bCharging)
		{
			bCharging = true;
			ChargeTime = 0.0f;
		}
	}
	else
	{
		if (bCharging && ChargeTime > CHARGE_TIMEOUT)
		{
			AddNewMotion();
		}

		bCharging = false;
		ChargeTime = 0.0f;
	}

	for (FMotionProgress& Motion : InputProgress)
	{
		// Final Direction
		if (Motion.GetIndex() == 1)
		{
			if (New == MotionDirection)
			{
				Motion.Increment();
			}
		}
	}
}

int UChargeProcessor::CompletedMotionIndex() const
{
	return 3;
}

ESSDPatternType UChargeProcessor::GetPatternType() const
{
	return ESSDPatternType::SSDPT_Single;
}
ESSDDirPattern UChargeProcessor::GetPattern() const
{
	return ESSDDirPattern::SSDDP_Charge;
}

