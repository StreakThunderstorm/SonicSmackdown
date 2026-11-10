// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDFighterInputs.h"
#include "SSDPlayerController.h"
#include "Input/MotionProcessors/QuarterCircleProcessor.h"
#include "Input/MotionProcessors/DragonPunchProcessor.h"
#include "Input/MotionProcessors/HalfCircleProcessor.h"
#include "Input/MotionProcessors/FullCircleProcessor.h"
#include "Input/MotionProcessors/ChargeProcessor.h"
#include "Input/MotionProcessors/RagingDemonProcessor.h"

USSDFighterInputs::USSDFighterInputs()
{

}

void USSDFighterInputs::SetMotionProcessors()
{
	MotionProcessors = TArray<UMotionProcessor*>();

	// Add processors for each motion type;
	MotionProcessors.Add(UQuarterCircleProcessor::Make(this, ESSDInputDir::SSDID_Right));
	MotionProcessors.Add(UQuarterCircleProcessor::Make(this, ESSDInputDir::SSDID_Left));
	MotionProcessors.Add(UDragonPunchProcessor::Make(this, ESSDInputDir::SSDID_Right));
	MotionProcessors.Add(UDragonPunchProcessor::Make(this, ESSDInputDir::SSDID_Left));
	MotionProcessors.Add(UHalfCircleProcessor::Make(this, ESSDInputDir::SSDID_Right));
	MotionProcessors.Add(UHalfCircleProcessor::Make(this, ESSDInputDir::SSDID_Left));
	MotionProcessors.Add(UFullCircleProcessor::Make(this, ESSDInputDir::SSDID_Right));
	MotionProcessors.Add(UFullCircleProcessor::Make(this, ESSDInputDir::SSDID_Left));

	MotionProcessors.Add(UChargeProcessor::Make(this, ESSDInputDir::SSDID_Right));
	MotionProcessors.Add(UChargeProcessor::Make(this, ESSDInputDir::SSDID_DownRight));
	MotionProcessors.Add(UChargeProcessor::Make(this, ESSDInputDir::SSDID_Down));
	MotionProcessors.Add(UChargeProcessor::Make(this, ESSDInputDir::SSDID_DownLeft));
	MotionProcessors.Add(UChargeProcessor::Make(this, ESSDInputDir::SSDID_Left));
	MotionProcessors.Add(UChargeProcessor::Make(this, ESSDInputDir::SSDID_UpLeft));
	MotionProcessors.Add(UChargeProcessor::Make(this, ESSDInputDir::SSDID_Up));
	MotionProcessors.Add(UChargeProcessor::Make(this, ESSDInputDir::SSDID_UpRight));

	MotionProcessors.Add(URagingDemonProcessor::Make(this, ESSDInputDir::SSDID_Right));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, ESSDInputDir::SSDID_DownRight));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, ESSDInputDir::SSDID_Down));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, ESSDInputDir::SSDID_DownLeft));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, ESSDInputDir::SSDID_Left));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, ESSDInputDir::SSDID_UpLeft));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, ESSDInputDir::SSDID_Up));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, ESSDInputDir::SSDID_UpRight));
}

void USSDFighterInputs::Tick(float DeltaTime)
{
	for (UMotionProcessor* Processor : MotionProcessors)
	{
		Processor->RemoveExpired();
	}
	for (UMotionProcessor* Processor : MotionProcessors)
	{
		Processor->Tick(DeltaTime);

		FSSDInputMove* NewMove = Processor->GetMoveAndReset();
		if (NewMove)
		{
			NewMove->Time = FPlatformTime::Seconds();
			MoveBuffer.Insert(*NewMove, 0);

			// We found a motion, so stop the attack stack timeout from buffering a normal.
			if (PCOwner) PCOwner->ResetKeyGroupTimer();
		}
	}
}

void USSDFighterInputs::AttackPressed(ESSDInputAttack InputAttack)
{
	for (UMotionProcessor* Processor : MotionProcessors)
	{
		Processor->AttackPressed(InputAttack);
	}

	// Update controller state.
	uint8 NewAttack = (uint8)AttackState;
	NewAttack |= (uint8)InputAttack;
	AttackState = (ESSDInputAttack)NewAttack;
}

void USSDFighterInputs::AttackReleased(ESSDInputAttack InputAttack)
{
	for (UMotionProcessor* Processor : MotionProcessors)
	{
		Processor->AttackReleased(InputAttack);
	}

	// Update controller state.
	uint8 NewAttack = (uint8)AttackState;
	NewAttack &= ~(uint8)InputAttack;
	AttackState = (ESSDInputAttack)NewAttack;
}

void USSDFighterInputs::CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New)
{
	for (UMotionProcessor* Processor : MotionProcessors)
	{
		Processor->CheckDirPatterns(Old, New);
	}
}

bool USSDFighterInputs::CanSuperJump()
{
	double TimeSinceLastDown = FPlatformTime::Seconds() - SuperJump;
	return TimeSinceLastDown <= KEY_MOVE_TIME;
}
