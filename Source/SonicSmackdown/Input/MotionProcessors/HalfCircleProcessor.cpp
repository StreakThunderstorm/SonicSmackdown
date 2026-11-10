// Fill out your copyright notice in the Description page of Project Settings.


#include "HalfCircleProcessor.h"

UHalfCircleProcessor::UHalfCircleProcessor() { }

void UHalfCircleProcessor::SetDirection(ESSDInputDir Direction)
{
	SetHorizontalDirection(Direction);
}

void UHalfCircleProcessor::CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New)
{
	ESSDInputDir NextInput;

	// Back
	NextInput = ESSDInputDir::SSDID_Left;
	if (MotionDirection == ESSDInputDir::SSDID_Left)
		NextInput = GetNeighborDir(NextInput, ESSDInputNeighbor::SSDIN_Mirror);

	if (Old == NextInput)
	{
		AddNewMotion();
	}
	for (FMotionProgress& Motion : InputProgress)
	{
		// Down-Back
		if (Motion.GetIndex() == 1)
		{
			NextInput = ESSDInputDir::SSDID_DownLeft;
			if (MotionDirection == ESSDInputDir::SSDID_Left)
				NextInput = GetNeighborDir(NextInput, ESSDInputNeighbor::SSDIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
		// Down
		if (Motion.GetIndex() == 2)
		{
			if (New == ESSDInputDir::SSDID_Down)
			{
				Motion.Increment();
			}
		}
		// Down-Forward
		if (Motion.GetIndex() == 3)
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
		if (Motion.GetIndex() == 4)
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

int UHalfCircleProcessor::CompletedMotionIndex() const
{
	return 6;
}

ESSDPatternType UHalfCircleProcessor::GetPatternType() const
{
	return ESSDPatternType::SSDPT_Single;
}
ESSDDirPattern UHalfCircleProcessor::GetPattern() const
{
	return ESSDDirPattern::SSDDP_HalfCircle;
}

