// Fill out your copyright notice in the Description page of Project Settings.


#include "RagingDemonProcessor.h"

URagingDemonProcessor::URagingDemonProcessor()
{
	NegativeEdgeAllowed = false;
}

void URagingDemonProcessor::CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New)
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

void URagingDemonProcessor::AttackButton(ESSDInputAttack InputAttack)
{
	Super::AttackButton(InputAttack);

	for (FMotionProgress& Motion : InputProgress)
	{
		// Second Light
		if (Motion.GetIndex() == 1)
		{
			// Check second light press first, then add 
			if (InputAttack == ESSDInputAttack::SSDIA_Light)
			{
				Motion.Increment();
			}
		}
		// Medium
		if (Motion.GetIndex() == 3)
		{
			// Check second light press first, then add 
			if (InputAttack == ESSDInputAttack::SSDIA_Medium)
			{
				Motion.Increment();
			}
		}
	}

	// First Light
	if (InputAttack == ESSDInputAttack::SSDIA_Light)
	{
		// New motions on first light press
		AddNewMotion();
	}
}

int URagingDemonProcessor::CompletedMotionIndex() const
{
	return 5;
}

ESSDPatternType URagingDemonProcessor::GetPatternType() const
{
	return ESSDPatternType::SSDPT_None;
}
ESSDDirPattern URagingDemonProcessor::GetPattern() const
{
	return ESSDDirPattern::SSDDP_RagingDemon;
}

float URagingDemonProcessor::MotionTimeout() const
{
	return 60 / 60.f;
}
