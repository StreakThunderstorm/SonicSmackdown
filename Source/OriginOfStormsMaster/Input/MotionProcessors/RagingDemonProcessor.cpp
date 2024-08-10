// Fill out your copyright notice in the Description page of Project Settings.


#include "RagingDemonProcessor.h"

URagingDemonProcessor::URagingDemonProcessor()
{
	NegativeEdgeAllowed = false;
}

void URagingDemonProcessor::CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New)
{
	for (FMotionProgress& Motion : InputProgress)
	{
		// Directional Input
		if (Motion.GetIndex() == 2)
		{
			if (New == MotionDirection)
			{
				Motion.Increment();
			}
		}
	}
}

void URagingDemonProcessor::AttackButton(EOOSInputAttack InputAttack)
{
	Super::AttackButton(InputAttack);

	for (FMotionProgress& Motion : InputProgress)
	{
		// Second Light
		if (Motion.GetIndex() == 1)
		{
			// Check second light press first, then add 
			if (InputAttack == EOOSInputAttack::OOSIA_Light)
			{
				Motion.Increment();
			}
		}
		// Medium
		if (Motion.GetIndex() == 3)
		{
			// Check second light press first, then add 
			if (InputAttack == EOOSInputAttack::OOSIA_Medium)
			{
				Motion.Increment();
			}
		}
	}

	// First Light
	if (InputAttack == EOOSInputAttack::OOSIA_Light)
	{
		// New motions on first light press
		AddNewMotion();
	}
}

int URagingDemonProcessor::CompletedMotionIndex() const
{
	return 5;
}

EOOSPatternType URagingDemonProcessor::GetPatternType() const
{
	return EOOSPatternType::OOSPT_None;
}
EOOSDirPattern URagingDemonProcessor::GetPattern() const
{
	return EOOSDirPattern::OOSDP_RagingDemon;
}

float URagingDemonProcessor::MotionTimeout() const
{
	return 60 / 60.f;
}
