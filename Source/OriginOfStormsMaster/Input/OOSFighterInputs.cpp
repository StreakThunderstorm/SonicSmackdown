// Fill out your copyright notice in the Description page of Project Settings.


#include "OOSFighterInputs.h"
#include "OOSPlayerController.h"
#include "OriginOfStormsMaster/Input/MotionProcessors/QuarterCircleProcessor.h"
#include "OriginOfStormsMaster/Input/MotionProcessors/DragonPunchProcessor.h"
#include "OriginOfStormsMaster/Input/MotionProcessors/HalfCircleProcessor.h"
#include "OriginOfStormsMaster/Input/MotionProcessors/FullCircleProcessor.h"
#include "OriginOfStormsMaster/Input/MotionProcessors/ChargeProcessor.h"
#include "OriginOfStormsMaster/Input/MotionProcessors/RagingDemonProcessor.h"

UOOSFighterInputs::UOOSFighterInputs()
{

}

void UOOSFighterInputs::SetMotionProcessors()
{
	MotionProcessors = TArray<UMotionProcessor*>();

	// Add processors for each motion type;
	MotionProcessors.Add(UQuarterCircleProcessor::Make(this, EOOSInputDir::OOSID_Right));
	MotionProcessors.Add(UQuarterCircleProcessor::Make(this, EOOSInputDir::OOSID_Left));
	MotionProcessors.Add(UDragonPunchProcessor::Make(this, EOOSInputDir::OOSID_Right));
	MotionProcessors.Add(UDragonPunchProcessor::Make(this, EOOSInputDir::OOSID_Left));
	MotionProcessors.Add(UHalfCircleProcessor::Make(this, EOOSInputDir::OOSID_Right));
	MotionProcessors.Add(UHalfCircleProcessor::Make(this, EOOSInputDir::OOSID_Left));
	MotionProcessors.Add(UFullCircleProcessor::Make(this, EOOSInputDir::OOSID_Right));
	MotionProcessors.Add(UFullCircleProcessor::Make(this, EOOSInputDir::OOSID_Left));

	MotionProcessors.Add(UChargeProcessor::Make(this, EOOSInputDir::OOSID_Right));
	MotionProcessors.Add(UChargeProcessor::Make(this, EOOSInputDir::OOSID_DownRight));
	MotionProcessors.Add(UChargeProcessor::Make(this, EOOSInputDir::OOSID_Down));
	MotionProcessors.Add(UChargeProcessor::Make(this, EOOSInputDir::OOSID_DownLeft));
	MotionProcessors.Add(UChargeProcessor::Make(this, EOOSInputDir::OOSID_Left));
	MotionProcessors.Add(UChargeProcessor::Make(this, EOOSInputDir::OOSID_UpLeft));
	MotionProcessors.Add(UChargeProcessor::Make(this, EOOSInputDir::OOSID_Up));
	MotionProcessors.Add(UChargeProcessor::Make(this, EOOSInputDir::OOSID_UpRight));

	MotionProcessors.Add(URagingDemonProcessor::Make(this, EOOSInputDir::OOSID_Right));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, EOOSInputDir::OOSID_DownRight));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, EOOSInputDir::OOSID_Down));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, EOOSInputDir::OOSID_DownLeft));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, EOOSInputDir::OOSID_Left));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, EOOSInputDir::OOSID_UpLeft));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, EOOSInputDir::OOSID_Up));
	MotionProcessors.Add(URagingDemonProcessor::Make(this, EOOSInputDir::OOSID_UpRight));
}

void UOOSFighterInputs::Tick(float DeltaTime)
{
	for (UMotionProcessor* Processor : MotionProcessors)
	{
		Processor->RemoveExpired();
	}
	for (UMotionProcessor* Processor : MotionProcessors)
	{
		Processor->Tick(DeltaTime);

		FOOSInputMove* NewMove = Processor->GetMoveAndReset();
		if (NewMove)
		{
			NewMove->Time = FPlatformTime::Seconds();
			MoveBuffer.Insert(*NewMove, 0);

			// We found a motion, so stop the attack stack timeout from buffering a normal.
			if (PCOwner) PCOwner->ResetKeyGroupTimer();
		}
	}
}

void UOOSFighterInputs::AttackPressed(EOOSInputAttack InputAttack)
{
	for (UMotionProcessor* Processor : MotionProcessors)
	{
		Processor->AttackPressed(InputAttack);
	}

	// Update controller state.
	uint8 NewAttack = (uint8)AttackState;
	NewAttack |= (uint8)InputAttack;
	AttackState = (EOOSInputAttack)NewAttack;
}

void UOOSFighterInputs::AttackReleased(EOOSInputAttack InputAttack)
{
	for (UMotionProcessor* Processor : MotionProcessors)
	{
		Processor->AttackReleased(InputAttack);
	}

	// Update controller state.
	uint8 NewAttack = (uint8)AttackState;
	NewAttack &= ~(uint8)InputAttack;
	AttackState = (EOOSInputAttack)NewAttack;
}

void UOOSFighterInputs::CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New)
{
	for (UMotionProcessor* Processor : MotionProcessors)
	{
		Processor->CheckDirPatterns(Old, New);
	}
}

bool UOOSFighterInputs::CanSuperJump()
{
	double TimeSinceLastDown = FPlatformTime::Seconds() - SuperJump;
	return TimeSinceLastDown <= KEY_MOVE_TIME;
}
