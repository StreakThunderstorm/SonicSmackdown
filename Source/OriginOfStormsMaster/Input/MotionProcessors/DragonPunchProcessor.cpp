// Fill out your copyright notice in the Description page of Project Settings.


#include "DragonPunchProcessor.h"

UDragonPunchProcessor::UDragonPunchProcessor() { }

void UDragonPunchProcessor::SetDirection(EOOSInputDir Direction)
{
	SetHorizontalDirection(Direction);
}

void UDragonPunchProcessor::CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New)
{
	EOOSInputDir NextInput;

	// Forward
	NextInput = EOOSInputDir::OOSID_Right;
	if (MotionDirection == EOOSInputDir::OOSID_Left)
		NextInput = GetNeighborDir(NextInput, EOOSInputNeighbor::OOSIN_Mirror);

	if (Old == NextInput)
	{
		AddNewMotion();
	}
	// If back is pressed after forward, reset the motion
	EOOSInputDir Backwards = EOOSInputDir::OOSID_Left;
	if (MotionDirection == EOOSInputDir::OOSID_Left)
		Backwards = GetNeighborDir(Backwards, EOOSInputNeighbor::OOSIN_Mirror);

	if (New == Backwards)
	{
		RemoveMotionsOnBack();
	}
	for (FMotionProgress& Motion : InputProgress)
	{
		// Down
		if (Motion.GetIndex() == 1)
		{
			if (New == EOOSInputDir::OOSID_Down)
			{
				Motion.Increment();
			}
		}
		// Down-Forward
		if (Motion.GetIndex() == 2)
		{
			NextInput = EOOSInputDir::OOSID_DownRight;
			if (MotionDirection == EOOSInputDir::OOSID_Left)
				NextInput = GetNeighborDir(NextInput, EOOSInputNeighbor::OOSIN_Mirror);

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

EOOSPatternType UDragonPunchProcessor::GetPatternType() const
{
	return EOOSPatternType::OOSPT_Single;
}
EOOSDirPattern UDragonPunchProcessor::GetPattern() const
{
	return EOOSDirPattern::OOSDP_DragonPunch;
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