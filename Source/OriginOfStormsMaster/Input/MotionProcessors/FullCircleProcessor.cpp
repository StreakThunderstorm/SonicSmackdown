// Fill out your copyright notice in the Description page of Project Settings.


#include "FullCircleProcessor.h"

UFullCircleProcessor::UFullCircleProcessor() { }

void UFullCircleProcessor::SetDirection(EOOSInputDir Direction)
{
	SetHorizontalDirection(Direction);
}

void UFullCircleProcessor::CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New)
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
	for (FMotionProgress& Motion : InputProgress)
	{
		// Down-Forward
		if (Motion.GetIndex() == 1)
		{
			NextInput = EOOSInputDir::OOSID_DownRight;
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
		// Down-Back
		if (Motion.GetIndex() == 3)
		{
			NextInput = EOOSInputDir::OOSID_DownLeft;
			if (MotionDirection == EOOSInputDir::OOSID_Left)
				NextInput = GetNeighborDir(NextInput, EOOSInputNeighbor::OOSIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
		// Back
		if (Motion.GetIndex() == 4)
		{
			NextInput = EOOSInputDir::OOSID_Left;
			if (MotionDirection == EOOSInputDir::OOSID_Left)
				NextInput = GetNeighborDir(NextInput, EOOSInputNeighbor::OOSIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
		// Up-Back
		if (Motion.GetIndex() == 5)
		{
			NextInput = EOOSInputDir::OOSID_UpLeft;
			if (MotionDirection == EOOSInputDir::OOSID_Left)
				NextInput = GetNeighborDir(NextInput, EOOSInputNeighbor::OOSIN_Mirror);

			if (New == NextInput)
			{
				Motion.Increment();
			}
		}
		// Up
		if (Motion.GetIndex() == 6)
		{
			if (New == EOOSInputDir::OOSID_Up)
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

EOOSPatternType UFullCircleProcessor::GetPatternType() const
{
	return EOOSPatternType::OOSPT_Single;
}
EOOSDirPattern UFullCircleProcessor::GetPattern() const
{
	return EOOSDirPattern::OOSDP_FullCircle;
}

