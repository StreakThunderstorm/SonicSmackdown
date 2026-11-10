// Fill out your copyright notice in the Description page of Project Settings.


#include "ReverseQuarterCircleProcessor.h"

UReverseQuarterCircleProcessor::UReverseQuarterCircleProcessor() { }

void UReverseQuarterCircleProcessor::SetDirection(ESSDInputDir Direction)
{
	SetHorizontalDirection(Direction);
}

void UReverseQuarterCircleProcessor::CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New)
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

int UReverseQuarterCircleProcessor::CompletedMotionIndex() const
{
	return 4;
}

ESSDPatternType UReverseQuarterCircleProcessor::GetPatternType() const
{
	return ESSDPatternType::SSDPT_Single;
}
ESSDDirPattern UReverseQuarterCircleProcessor::GetPattern() const
{
	return ESSDDirPattern::SSDDP_QuarterCircle;
}