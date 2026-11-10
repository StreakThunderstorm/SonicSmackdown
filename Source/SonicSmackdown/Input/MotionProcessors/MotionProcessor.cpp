// Fill out your copyright notice in the Description page of Project Settings.


#include "MotionProcessor.h"
//#include "SonicSmackdown.h"
//#include "Engine.h"

// FMotionProgress
void FMotionProgress::Tick(float DeltaTime)
{
	if (Index > 0)
	{
		MotionTime += DeltaTime;
	}
	if (bBufferringButtons)
	{
		BufferTime += DeltaTime;
	}
}

void FMotionProgress::Increment()
{
	// If incrementing the first step of the motion, reset the time
	if (Index == 0)
	{
		MotionTime = 0.0f;
	}

	Index++;
}

void FMotionProgress::SetBufferTime()
{
	bBufferringButtons = true;
	BufferTime = 0.0f;
}

void FMotionProgress::Reset()
{
	Index = 0;
	MotionTime = 0.0f;
	bBufferringButtons = false;
	BufferTime = 0.0f;
}

bool FMotionProgress::IsExpired(float BufferLength) const
{
	// If move is already bufferred
	if (bBufferringButtons)
	{
		return false;
	}
	// If the move has not started
	if (Index == 0)
	{
		return false;
	}

	return MotionTime > BufferLength;
}

bool FMotionProgress::IsBufferExpired() const
{
	if (!bBufferringButtons)
	{
		return false;
	}

	return BufferTime > KEY_GROUP_TIME;
}

// UMotionProcessor
UMotionProcessor::UMotionProcessor()
{
	InputProgress.Reset(STORED_COMMANDS);
}

void UMotionProcessor::SetDirection(ESSDInputDir Direction)
{
	MotionDirection = Direction;
}
void UMotionProcessor::SetHorizontalDirection(ESSDInputDir Direction)
{
	// Left and right are the only valid directions
	if (Direction == ESSDInputDir::SSDID_Left)
		MotionDirection = Direction;
	else
		MotionDirection = ESSDInputDir::SSDID_Right;
}

ESSDInputDir UMotionProcessor::GetNeighborDir(ESSDInputDir Dir, ESSDInputNeighbor Neighbor)
{
	if (Dir == ESSDInputDir::SSDID_None) return ESSDInputDir::SSDID_None;

	uint8 NewDir = (uint8)Dir;

	switch (Neighbor)
	{
	case ESSDInputNeighbor::SSDIN_AntiClockwise:
		NewDir--;
		if (NewDir == 0) return ESSDInputDir::SSDID_DownRight;
		else return (ESSDInputDir)NewDir;
		break;

	case ESSDInputNeighbor::SSDIN_Clockwise:
		NewDir++;
		if (NewDir == 9) return ESSDInputDir::SSDID_Right;
		else return (ESSDInputDir)NewDir;
		break;

	case ESSDInputNeighbor::SSDIN_Opposite:
		// Rotate 4 inputs forward
		NewDir = NewDir + 4;
		if (NewDir >= 9) NewDir -= 8;
		return (ESSDInputDir)NewDir;
		break;

	case ESSDInputNeighbor::SSDIN_Mirror:
		switch (Dir)
		{
		case ESSDInputDir::SSDID_DownLeft:
			NewDir = (uint8)ESSDInputDir::SSDID_DownRight;
			break;
		case ESSDInputDir::SSDID_Left:
			NewDir = (uint8)ESSDInputDir::SSDID_Right;
			break;
		case ESSDInputDir::SSDID_UpLeft:
			NewDir = (uint8)ESSDInputDir::SSDID_UpRight;
			break;

		case ESSDInputDir::SSDID_DownRight:
			NewDir = (uint8)ESSDInputDir::SSDID_DownLeft;
			break;
		case ESSDInputDir::SSDID_Right:
			NewDir = (uint8)ESSDInputDir::SSDID_Left;
			break;
		case ESSDInputDir::SSDID_UpRight:
			NewDir = (uint8)ESSDInputDir::SSDID_UpLeft;
			break;
		}
		return (ESSDInputDir)NewDir;
		break;

	default:
		return ESSDInputDir::SSDID_None;
		break;
	}
}

void UMotionProcessor::Tick(float DeltaTime)
{
	for (FMotionProgress& Motion : InputProgress)
	{
		Motion.Tick(DeltaTime);

		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red,
		//	FString::Printf(TEXT("%s - %d"),
		//		*GETENUMSTRING("ESSDDirPattern", GetPattern()),
		//		Motion.GetIndex()));
	}
}

FSSDInputMove* UMotionProcessor::GetMoveAndReset()
{
	if (IsMotionCompleted())
	{
		if (InputProgress[0].IsBufferExpired())
		{
			InputProgress.RemoveAt(0);
			return &CurrentMove;
		}
	}

	return nullptr;
}

void UMotionProcessor::RemoveExpired()
{
	for (int i = 0; i < InputProgress.Num();)
	{
		if (InputProgress[i].IsExpired(MotionTimeout()))
		{
			InputProgress.RemoveAt(i);
		}
		else
		{
			i++;
		}
	}
}

void UMotionProcessor::AddNewMotion()
{
	// If we're already waiting on a completely motion, just return
	if (IsMotionCompleted())
	{ 
		return;
	}

	RemoveDuplicateMotions();
	while (InputProgress.Num() >= STORED_COMMANDS)
	{
		InputProgress.RemoveAt(InputProgress.Num() - 1);
	}

	// Add new
	FMotionProgress NewMotion = FMotionProgress();
	NewMotion.Increment();
	InputProgress.Add(NewMotion);
}

bool UMotionProcessor::IsMotionCompleted() const
{
	if (InputProgress.Num() > 0)
	{
		return IsMotionCompleted(InputProgress[0]);
	}

	return false;
}

bool UMotionProcessor::IsMotionCompleted(FMotionProgress Motion) const
{
	return Motion.GetIndex() == CompletedMotionIndex();
}

void UMotionProcessor::RemoveDuplicateMotions()
{
	for (int i = InputProgress.Num() - 2; i >= 0; i--)
	{
		// If two inputs have the same index, remove the older one
		if (InputProgress[i].GetIndex() == InputProgress[i + 1].GetIndex())
		{
			InputProgress.RemoveAt(i);
		}
	}
}

bool UMotionProcessor::IsNegativeEdgeAllowed() const
{
	//@TODO: add a config option to disable per player, disable for buttons/command normals
	return NegativeEdgeAllowed && true;
}

void UMotionProcessor::SetCurrentMove(ESSDInputAttack InputAttack)
{
	CurrentMove.Attack = InputAttack;
	CurrentMove.PatternType = GetPatternType();
	CurrentMove.DirPattern = GetPattern();
	CurrentMove.Direction = MotionDirection;
	CurrentMove.Time = FPlatformTime::Seconds();
}

void UMotionProcessor::AttackPressed(ESSDInputAttack InputAttack)
{
	AttackButton(InputAttack);
}

void UMotionProcessor::AttackReleased(ESSDInputAttack InputAttack)
{
	// If this motion allows negative edge
	if (IsNegativeEdgeAllowed())
	{
		AttackButton(InputAttack);
	}
}

void UMotionProcessor::AttackButton(ESSDInputAttack InputAttack)
{
	if (InputProgress.Num() > 0)
	{
		// On the second to last input
		if (InputProgress[0].GetIndex() == (CompletedMotionIndex() - 1))
		{
			InputProgress[0].Increment();
			InputProgress[0].SetBufferTime();

			SetCurrentMove(InputAttack);
		}
	}

	AddMoveButtons(InputAttack);
}

void UMotionProcessor::AddMoveButtons(ESSDInputAttack InputAttack)
{
	if (IsMotionCompleted())
	{
		uint8 NewAttack = (uint8)CurrentMove.Attack | (uint8)InputAttack;

		CurrentMove.Attack = (ESSDInputAttack)NewAttack;
	}
}

float UMotionProcessor::MotionTimeout() const
{
	return KEY_MOVE_TIME;
}