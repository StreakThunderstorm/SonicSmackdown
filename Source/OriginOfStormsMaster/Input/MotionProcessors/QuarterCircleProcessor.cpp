// Fill out your copyright notice in the Description page of Project Settings.


#include "QuarterCircleProcessor.h"

UQuarterCircleProcessor::UQuarterCircleProcessor() { }

void UQuarterCircleProcessor::SetDirection(EOOSInputDir Direction)
{
	SetHorizontalDirection(Direction);
}

void UQuarterCircleProcessor::CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New)
{
	EOOSInputDir NextInput;

	// Down
	if (Old == EOOSInputDir::OOSID_Down)
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
		// Forward
		if (Motion.GetIndex() == 2)
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

int UQuarterCircleProcessor::CompletedMotionIndex() const
{
	return 4;
}

EOOSPatternType UQuarterCircleProcessor::GetPatternType() const
{
	return EOOSPatternType::OOSPT_Single;
}
EOOSDirPattern UQuarterCircleProcessor::GetPattern() const
{
	return EOOSDirPattern::OOSDP_QuarterCircle;
}