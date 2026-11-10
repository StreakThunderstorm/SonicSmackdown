// Fill out your copyright notice in the Description page of Project Settings.


#include "QuarterCircleProcessor.h"

UQuarterCircleProcessor::UQuarterCircleProcessor() { }

void UQuarterCircleProcessor::SetDirection(ESSDInputDir Direction)
{
	SetHorizontalDirection(Direction);
}

void UQuarterCircleProcessor::CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New)
{
	ESSDInputDir NextInput;

	// Down
	if (Old == ESSDInputDir::SSDID_Down)
	{
		AddNewMotion();
	}
	for (FMotionProgress& Motion : InputProgress)
	{
		// Down-Forward
		if (Motion.GetIndex() == 1)
		{
			NextInput = ESSDInputDir::SSDID_DownRight;
			if (MotionDirection == ESSDInputDir::SSDID_Left)
				NextInput = GetNeighborDir(NextInput, ESSDInputNeighbor::SSDIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
		// Forward
		if (Motion.GetIndex() == 2)
		{
			NextInput = ESSDInputDir::SSDID_Right;
			if (MotionDirection == ESSDInputDir::SSDID_Left)
				NextInput = GetNeighborDir(NextInput, ESSDInputNeighbor::SSDIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
	}
}

int UQuarterCircleProcessor::CompletedMotionIndex() const
{
	return 4;
}

ESSDPatternType UQuarterCircleProcessor::GetPatternType() const
{
	return ESSDPatternType::SSDPT_Single;
}
ESSDDirPattern UQuarterCircleProcessor::GetPattern() const
{
	return ESSDDirPattern::SSDDP_QuarterCircle;
}