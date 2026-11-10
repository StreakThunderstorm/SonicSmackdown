// Fill out your copyright notice in the Description page of Project Settings.


#include "FullCircleProcessor.h"

UFullCircleProcessor::UFullCircleProcessor() { }

void UFullCircleProcessor::SetDirection(ESSDInputDir Direction)
{
	SetHorizontalDirection(Direction);
}

void UFullCircleProcessor::CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New)
{
	ESSDInputDir NextInput;

	// Forward
	NextInput = ESSDInputDir::SSDID_Right;
	if (MotionDirection == ESSDInputDir::SSDID_Left)
		NextInput = GetNeighborDir(NextInput, ESSDInputNeighbor::SSDIN_Mirror);

	if (Old == NextInput)
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
		// Down
		if (Motion.GetIndex() == 2)
		{
			if (New == ESSDInputDir::SSDID_Down)
			{
				Motion.Increment();
			}
		}
		// Down-Back
		if (Motion.GetIndex() == 3)
		{
			NextInput = ESSDInputDir::SSDID_DownLeft;
			if (MotionDirection == ESSDInputDir::SSDID_Left)
				NextInput = GetNeighborDir(NextInput, ESSDInputNeighbor::SSDIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
		// Back
		if (Motion.GetIndex() == 4)
		{
			NextInput = ESSDInputDir::SSDID_Left;
			if (MotionDirection == ESSDInputDir::SSDID_Left)
				NextInput = GetNeighborDir(NextInput, ESSDInputNeighbor::SSDIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
		// Up-Back
		if (Motion.GetIndex() == 5)
		{
			NextInput = ESSDInputDir::SSDID_UpLeft;
			if (MotionDirection == ESSDInputDir::SSDID_Left)
				NextInput = GetNeighborDir(NextInput, ESSDInputNeighbor::SSDIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
		// Up
		if (Motion.GetIndex() == 6)
		{
			if (New == ESSDInputDir::SSDID_Up)
			{
				Motion.Increment();
			}
		}
	}
}

int UFullCircleProcessor::CompletedMotionIndex() const
{
	return 8;
}

ESSDPatternType UFullCircleProcessor::GetPatternType() const
{
	return ESSDPatternType::SSDPT_Single;
}
ESSDDirPattern UFullCircleProcessor::GetPattern() const
{
	return ESSDDirPattern::SSDDP_FullCircle;
}

