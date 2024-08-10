// Fill out your copyright notice in the Description page of Project Settings.


#include "HalfCircleProcessor.h"

UHalfCircleProcessor::UHalfCircleProcessor() { }

void UHalfCircleProcessor::SetDirection(EOOSInputDir Direction)
{
	SetHorizontalDirection(Direction);
}

void UHalfCircleProcessor::CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New)
{
	EOOSInputDir NextInput;

	// Back
	NextInput = EOOSInputDir::OOSID_Left;
	if (MotionDirection == EOOSInputDir::OOSID_Left)
		NextInput = GetNeighborDir(NextInput, EOOSInputNeighbor::OOSIN_Mirror);

	if (Old == NextInput)
	{
		AddNewMotion();
	}
	for (FMotionProgress& Motion : InputProgress)
	{
		// Down-Back
		if (Motion.GetIndex() == 1)
		{
			NextInput = EOOSInputDir::OOSID_DownLeft;
			if (MotionDirection == EOOSInputDir::OOSID_Left)
				NextInput = GetNeighborDir(NextInput, EOOSInputNeighbor::OOSIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
		// Down
		if (Motion.GetIndex() == 2)
		{
			if (New == EOOSInputDir::OOSID_Down)
			{
				Motion.Increment();
			}
		}
		// Down-Forward
		if (Motion.GetIndex() == 3)
		{
			NextInput = EOOSInputDir::OOSID_DownRight;
			if (MotionDirection == EOOSInputDir::OOSID_Left)
				NextInput = GetNeighborDir(NextInput, EOOSInputNeighbor::OOSIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
		// Forward
		if (Motion.GetIndex() == 4)
		{
			NextInput = EOOSInputDir::OOSID_Right;
			if (MotionDirection == EOOSInputDir::OOSID_Left)
				NextInput = GetNeighborDir(NextInput, EOOSInputNeighbor::OOSIN_Mirror);

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

EOOSPatternType UHalfCircleProcessor::GetPatternType() const
{
	return EOOSPatternType::OOSPT_Single;
}
EOOSDirPattern UHalfCircleProcessor::GetPattern() const
{
	return EOOSDirPattern::OOSDP_HalfCircle;
}

