// Fill out your copyright notice in the Description page of Project Settings.


#include "DragonPunchProcessor.h"

UDragonPunchProcessor::UDragonPunchProcessor() { }

void UDragonPunchProcessor::SetDirection(ESSDInputDir Direction)
{
	SetHorizontalDirection(Direction);
}

void UDragonPunchProcessor::CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New)
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
	// If back is pressed after forward, reset the motion
	ESSDInputDir Backwards = ESSDInputDir::SSDID_Left;
	if (MotionDirection == ESSDInputDir::SSDID_Left)
		Backwards = GetNeighborDir(Backwards, ESSDInputNeighbor::SSDIN_Mirror);

	if (New == Backwards)
	{
		RemoveMotionsOnBack();
	}
	for (FMotionProgress& Motion : InputProgress)
	{
		// Down
		if (Motion.GetIndex() == 1)
		{
			if (New == ESSDInputDir::SSDID_Down)
			{
				Motion.Increment();
			}
		}
		// Down-Forward
		if (Motion.GetIndex() == 2)
		{
			NextInput = ESSDInputDir::SSDID_DownRight;
			if (MotionDirection == ESSDInputDir::SSDID_Left)
				NextInput = GetNeighborDir(NextInput, ESSDInputNeighbor::SSDIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
	}
}

int UDragonPunchProcessor::CompletedMotionIndex() const
{
	return 4;
}

ESSDPatternType UDragonPunchProcessor::GetPatternType() const
{
	return ESSDPatternType::SSDPT_Single;
}
ESSDDirPattern UDragonPunchProcessor::GetPattern() const
{
	return ESSDDirPattern::SSDDP_DragonPunch;
}

void UDragonPunchProcessor::RemoveMotionsOnBack()
{
	for (int i = 0; i < InputProgress.Num();)
	{
		if (!IsMotionCompleted(InputProgress[i]))
		{
			InputProgress.RemoveAt(i);
		}
		else
		{
			i++;
		}
	}
}