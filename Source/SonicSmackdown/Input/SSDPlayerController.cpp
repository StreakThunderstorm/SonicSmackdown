// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDPlayerController.h"
#include "SonicSmackdown.h"
#include "SSDPawn.h"
#include "SSDGameInstance.h"
#include "Engine.h"

ASSDPlayerController::ASSDPlayerController()
{
	bAutoManageActiveCameraTarget = false;

	Inputs = CreateDefaultSubobject<USSDFighterInputs>(TEXT("FighterInputs"));
	Inputs->PCOwner = this;

}

void ASSDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Inputs->SetMotionProcessors();
}

void ASSDPlayerController::OnPossess(APawn *aPawn)
{
	Super::OnPossess(aPawn);

	PosessedPawn = Cast<ASSDPawn>(aPawn);
}

void ASSDPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	PosessedPawn = nullptr;
	Inputs->DPadState = ESSDInputDir::SSDID_None;
}

void ASSDPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	Inputs->Tick(DeltaTime);

}

void ASSDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Any key pressed.
	FInputKeyBinding KBP(FInputChord(EKeys::AnyKey, false, false, false, false), EInputEvent::IE_Pressed);
	KBP.bConsumeInput = false;
	KBP.bExecuteWhenPaused = false;
	KBP.KeyDelegate.GetDelegateWithKeyForManualSet().BindLambda([this](const FKey& Key) {
		this->KeyPressed(Key);
		});
	InputComponent->KeyBindings.Add(KBP);

	// Any key released.
	FInputKeyBinding KBR(FInputChord(EKeys::AnyKey, false, false, false, false), EInputEvent::IE_Released);
	KBR.bConsumeInput = false;
	KBR.bExecuteWhenPaused = false;
	KBR.KeyDelegate.GetDelegateWithKeyForManualSet().BindLambda([this](const FKey& Key) {
		this->KeyReleased(Key);
		});
	InputComponent->KeyBindings.Add(KBR);

	BuildBindingsFromSettings();

	InputComponent->BindAxis("HorizontalAxis", this, &ASSDPlayerController::HorizontalAxisPressed);
	InputComponent->BindAxis("VerticalAxis", this, &ASSDPlayerController::VerticalAxisPressed);

}

void ASSDPlayerController::BuildBindingsFromSettings()
{
	UWorld* World = GetWorld();
	if (World)
	{
		USSDGameInstance* GameInstance = Cast<USSDGameInstance>(UGameplayStatics::GetGameInstance(World));
		if (GameInstance)
		{
			ULocalPlayer* LP = GetLocalPlayer();
			if (LP)
			{
				int32 ID = LP->GetControllerId();
				if (GameInstance->Bindings.IsValidIndex(ID))
				{
					// Load direction and attack bindings and generate reversed-type maps.
					TArray<ESSDInputDir> DKeys;
					TArray<ESSDInputAttack> AKeys;
					TArray<FKey> Values;

					DKeys.Empty();
					Values.Empty();
					DirKey = GameInstance->Bindings[ID].Directions;
					KeyDir.Empty();
					DirKey.GenerateKeyArray(DKeys);
					DirKey.GenerateValueArray(Values);
					for (int i = 0; i < DirKey.Num(); ++i)
					{
						KeyDir.Emplace(Values[i], DKeys[i]);
					}

					AKeys.Empty();
					Values.Empty();
					AttKey = GameInstance->Bindings[ID].Attacks;
					KeyAtt.Empty();
					AttKey.GenerateKeyArray(AKeys);
					AttKey.GenerateValueArray(Values);
					for (int i = 0; i < AttKey.Num(); ++i)
					{
						KeyAtt.Emplace(Values[i], AKeys[i]);
					}
				}
			}
		}
	}
}

void ASSDPlayerController::KeyPressed(FKey Key)
{
	ESSDInputDir* Dir = KeyDir.Find(Key);
	if (Dir)
	{
		ESSDInputDir NewDir = *Dir;

		ESSDInputDir ACW = GetCardinalNeighborDir(NewDir, ESSDInputNeighbor::SSDIN_AntiClockwise);
		ESSDInputDir CW = GetCardinalNeighborDir(NewDir, ESSDInputNeighbor::SSDIN_Clockwise);

		FKey* ACWKey = DirKey.Find(ACW);
		FKey* CWKey = DirKey.Find(CW);

		if (ACWKey && CWKey)
		{
			FKey NewACWKey = *ACWKey;
			FKey NewCWKey = *CWKey;

			DirPressed(NewDir, NewACWKey, NewCWKey);
		}
	}
	else
	{
		ESSDInputAttack* Att = KeyAtt.Find(Key);
		if (Att)
		{
			ESSDInputAttack NewAtt = *Att;

			if (NewAtt == ESSDInputAttack::SSDIA_Transform)
			{
				TransformPressed();
			}
			else
			{
				AttackPressed(NewAtt);
			}
		}
	}
}

void ASSDPlayerController::KeyReleased(FKey Key)
{
	ESSDInputDir* Dir = KeyDir.Find(Key);
	if (Dir)
	{
		ESSDInputDir NewDir = *Dir;

		ESSDInputDir ACW = GetCardinalNeighborDir(NewDir, ESSDInputNeighbor::SSDIN_AntiClockwise);
		ESSDInputDir CW = GetCardinalNeighborDir(NewDir, ESSDInputNeighbor::SSDIN_Clockwise);

		FKey* ACWKey = DirKey.Find(ACW);
		FKey* CWKey = DirKey.Find(CW);

		if (ACWKey && CWKey)
		{
			FKey NewACWKey = *ACWKey;
			FKey NewCWKey = *CWKey;

			DirReleased(NewDir, NewACWKey, NewCWKey);
		}
	}
	else
	{
		ESSDInputAttack* Att = KeyAtt.Find(Key);
		if (Att)
		{
			ESSDInputAttack NewAtt = *Att;

			if (NewAtt == ESSDInputAttack::SSDIA_Transform)
			{
				TransformReleased();
			}
			else
			{
				AttackReleased(NewAtt);
			}
		}
	}
}

void ASSDPlayerController::ResetKeyGroupTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(KeyGroupTimer);
	ResetDirPatterns();
}

void ASSDPlayerController::DirPressed(ESSDInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey)
{
	HandleDirPressed(InputDir, IsInputKeyDown(ACWNeighborKey), IsInputKeyDown(CWNeighborKey));
}

void ASSDPlayerController::HandleDirPressed(ESSDInputDir InputDir, bool bACWNeighborDown, bool bCWNeighborDown)
{
	if (!PosessedPawn) return;

	// If down pressed, save timestamp for superjump.
	if (InputDir == ESSDInputDir::SSDID_Down) Inputs->SuperJump = FPlatformTime::Seconds();

	ProcessDirPress(InputDir, bACWNeighborDown, bCWNeighborDown);

	// If up pressed, try jumping
	if (InputDir == ESSDInputDir::SSDID_Up) PosessedPawn->Jump();

	// If up pressed, buffer a move for jump cancels.
	if (Inputs->DPadState == ESSDInputDir::SSDID_Up || Inputs->DPadState == ESSDInputDir::SSDID_UpRight || Inputs->DPadState == ESSDInputDir::SSDID_UpLeft)
	{
		FSSDInputMove NewMove = FSSDInputMove(ESSDPatternType::SSDPT_None, ESSDDirPattern::SSDDP_None, Inputs->DPadState, ESSDInputAttack::SSDIA_None, FPlatformTime::Seconds());
		Inputs->MoveBuffer.Insert(NewMove, 0);
	}

}

void ASSDPlayerController::ProcessDirPress(ESSDInputDir InputDir, bool bACWNeighborDown, bool bCWNeighborDown)
{
	// Check if any neighbor directions are held and buffer a diagonal if any.
	if (bACWNeighborDown)
	{
		ESSDInputDir NewDir = GetNeighborDir(InputDir, ESSDInputNeighbor::SSDIN_AntiClockwise);
		if (Inputs->DPadState != NewDir)
		{
			InputBuffer.Push(FSSDInput(NewDir, ESSDInputAttack::SSDIA_None, FPlatformTime::Seconds())); // Buffer direction and save timestamp.
			CheckDirPatterns(Inputs->DPadState, NewDir); // We advance our directional pattern state machine every time we buffer a direction.
			Inputs->DPadState = NewDir; // Update D-Pad state for BP.
		}
		return;
	}

	if (bCWNeighborDown)
	{
		ESSDInputDir NewDir = GetNeighborDir(InputDir, ESSDInputNeighbor::SSDIN_Clockwise);
		if (Inputs->DPadState != NewDir)
		{
			InputBuffer.Push(FSSDInput(NewDir, ESSDInputAttack::SSDIA_None, FPlatformTime::Seconds()));
			CheckDirPatterns(Inputs->DPadState, NewDir);
			Inputs->DPadState = NewDir;
		}
		return;
	}

	InputBuffer.Push(FSSDInput(InputDir, ESSDInputAttack::SSDIA_None, FPlatformTime::Seconds()));
	CheckDirPatterns(Inputs->DPadState, InputDir);
	Inputs->DPadState = InputDir;
}

void ASSDPlayerController::DirReleased(ESSDInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey)
{
	HandleDirReleased(InputDir, IsInputKeyDown(ACWNeighborKey), IsInputKeyDown(CWNeighborKey));
}

void ASSDPlayerController::HandleDirReleased(ESSDInputDir InputDir, bool bACWNeighborDown, bool bCWNeighborDown)
{
	if (!PosessedPawn) return;

	ProcessDirRelease(InputDir, bACWNeighborDown, bCWNeighborDown);

}

void ASSDPlayerController::ProcessDirRelease(ESSDInputDir InputDir, bool bACWNeighborDown, bool bCWNeighborDown)
{
	// If a neighbor direction is still held, that direction is what remains. It is taken from the
	// released direction rather than by rotating the current state, so a state that somehow got
	// out of step with the hardware corrects itself here instead of drifting further.
	if (bACWNeighborDown || bCWNeighborDown)
	{
		ESSDInputDir NewDir = GetCardinalNeighborDir(InputDir, bACWNeighborDown ? ESSDInputNeighbor::SSDIN_AntiClockwise : ESSDInputNeighbor::SSDIN_Clockwise);
		if (Inputs->DPadState != NewDir)
		{
			InputBuffer.Push(FSSDInput(NewDir, ESSDInputAttack::SSDIA_None, FPlatformTime::Seconds()));
			CheckDirPatterns(Inputs->DPadState, NewDir);
			Inputs->DPadState = NewDir;
		}
		return;
	}

	CheckDirPatterns(Inputs->DPadState, ESSDInputDir::SSDID_None);
	Inputs->DPadState = ESSDInputDir::SSDID_None;
}

FSSDInput ASSDPlayerController::GetInputBufferElement(int Element) const
{
	if (InputBuffer.IsValidIndex(Element))
	{
		return InputBuffer[Element];
	}
	else
	{
		return FSSDInput();
	}
}

int ASSDPlayerController::GetInputBufferSize() const
{
	return InputBuffer.Num();
}

void ASSDPlayerController::HorizontalAxisPressed(float Value)
{
	if (!PosessedPawn) return;

	DispatchDigitalStickEvents_H(Value);

	float AxisValue = Value;

	if (AxisValue == 0.f)
	{
		AxisValue = GetHorizontalAxisValueFromDirections();
	}

	PosessedPawn->MoveHorizontal(AxisValue);
}

void ASSDPlayerController::VerticalAxisPressed(float Value)
{
	if (!PosessedPawn) return;

	DispatchDigitalStickEvents_V(Value);

	float AxisValue = Value;

	if (AxisValue == 0.f)
	{
		AxisValue = GetVerticalAxisValueFromDirections();
	}

	if (AxisValue == 0)
		PosessedPawn->Uncrouch();
	else if (AxisValue < 0)
		PosessedPawn->Crouch();

	PosessedPawn->MoveVertical(AxisValue);
}

// The stick is turned into digital direction events here. Whether the other axis is held is taken
// from the same dead-zoned axis values that generate those events, NOT from the engine's virtual
// stick keys (Gamepad_LeftStick_Up etc.): those trip at a much smaller deflection than our axis
// dead zone, so a slightly diagonal push would report e.g. Up as held without Up ever being
// pressed or released here, leaving DPadState latched on a direction after the stick recentred.
void ASSDPlayerController::DispatchDigitalStickEvents_H(float Value)
{
	const bool bUp = LastStick_V > 0.f;
	const bool bDown = LastStick_V < 0.f;
	const float OldSign = FMath::Sign(LastStick_H);
	const float NewSign = FMath::Sign(Value);

	// Updated first so the handlers below already see the new state.
	LastStick_H = Value;

	if (OldSign == NewSign) return;

	// A flick can cross from one side to the other without a neutral frame, so release then press.
	if (OldSign < 0.f) HandleDirReleased(ESSDInputDir::SSDID_Left, bUp, bDown);
	else if (OldSign > 0.f) HandleDirReleased(ESSDInputDir::SSDID_Right, bDown, bUp);

	if (NewSign < 0.f) HandleDirPressed(ESSDInputDir::SSDID_Left, bUp, bDown);
	else if (NewSign > 0.f) HandleDirPressed(ESSDInputDir::SSDID_Right, bDown, bUp);
}

void ASSDPlayerController::DispatchDigitalStickEvents_V(float Value)
{
	const bool bLeft = LastStick_H < 0.f;
	const bool bRight = LastStick_H > 0.f;
	const float OldSign = FMath::Sign(LastStick_V);
	const float NewSign = FMath::Sign(Value);

	LastStick_V = Value;

	if (OldSign == NewSign) return;

	if (OldSign < 0.f) HandleDirReleased(ESSDInputDir::SSDID_Down, bLeft, bRight);
	else if (OldSign > 0.f) HandleDirReleased(ESSDInputDir::SSDID_Up, bRight, bLeft);

	if (NewSign < 0.f) HandleDirPressed(ESSDInputDir::SSDID_Down, bLeft, bRight);
	else if (NewSign > 0.f) HandleDirPressed(ESSDInputDir::SSDID_Up, bRight, bLeft);
}

void ASSDPlayerController::AttackPressed(ESSDInputAttack InputAttack)
{
	if (!PosessedPawn) return;

	if (InputAttack == ESSDInputAttack::SSDIA_None) return;

	Inputs->AttackPressed(InputAttack);

	FSSDInput& LastInput = InputBuffer.Peek(); // Get ref to latest buffer entry.
	if (((FPlatformTime::Seconds() - LastInput.Time) <= KEY_GROUP_TIME) && (LastInput.Attack != ESSDInputAttack::SSDIA_Transform))
	{
		// We pressed an attack fast enough to stack it onto the last attack.
		uint8 NewAttack = (uint8)LastInput.Attack;
		NewAttack |= (uint8)InputAttack;
		if (NewAttack != (uint8)ESSDInputAttack::SSDIA_None)
		{
			LastInput.Attack = (ESSDInputAttack)NewAttack;
			CurrentMove.Attack = (ESSDInputAttack)NewAttack;
		}
	}
	else
	{
		// Too late or we buffered transform, add another buffer entry.
		InputBuffer.Push(FSSDInput(ESSDInputDir::SSDID_None, InputAttack, FPlatformTime::Seconds()));
		CurrentMove.Attack = InputAttack;
	}

	// Pressing an attack button means ending a move, so we shoud buffer it. But first we need to wait KEY_GROUP_TIME seconds
	// to stack another possible attack press.
	if(!GetWorld()->GetTimerManager().IsTimerActive(KeyGroupTimer))
		GetWorld()->GetTimerManager().SetTimer(KeyGroupTimer, this, &ASSDPlayerController::OnAttackGroupTimeout, KEY_GROUP_TIME, false);

}

void ASSDPlayerController::AttackReleased(ESSDInputAttack InputAttack)
{
	if (!PosessedPawn) return;

	if (InputAttack == ESSDInputAttack::SSDIA_None) return;

	Inputs->AttackReleased(InputAttack);

}

void ASSDPlayerController::OnAttackGroupTimeout()
{
	if (!PosessedPawn) return;


	if ((FPlatformTime::Seconds() - CurrentMove.Time) > KEY_MOVE_TIME)
	{
		// Too late to press an attack - we lose the direction pattern and buffer the attacks only.
		CurrentMove.DirPattern = ESSDDirPattern::SSDDP_None;
		CurrentMove.PatternType = ESSDPatternType::SSDPT_None;

		// If there's a direction buffered together with the attack, add it to the move.
		if (Inputs->DPadState != ESSDInputDir::SSDID_None)
			CurrentMove.Direction = Inputs->DPadState;

	}

	// Update timetag now.
	CurrentMove.Time = FPlatformTime::Seconds();
	Inputs->MoveBuffer.Insert(CurrentMove, 0);

	ResetDirPatterns();
}

ESSDInputDir ASSDPlayerController::GetNeighborDir(ESSDInputDir Dir, ESSDInputNeighbor Neighbor)
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
		if (NewDir == 0) return ESSDInputDir::SSDID_None;

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

ESSDInputDir ASSDPlayerController::GetCardinalNeighborDir(ESSDInputDir Dir, ESSDInputNeighbor Neighbor)
{
	if (Dir == ESSDInputDir::SSDID_None) return ESSDInputDir::SSDID_None;

	int NewDir = (int)Dir;
	if((NewDir == 2) || (NewDir == 4) || (NewDir == 6) || (NewDir == 8)) return ESSDInputDir::SSDID_None;

	switch (Neighbor)
	{
	case ESSDInputNeighbor::SSDIN_AntiClockwise:
		NewDir -= 2;
		if (NewDir == -1) return ESSDInputDir::SSDID_Down;
		else return (ESSDInputDir)NewDir;
		break;

	case ESSDInputNeighbor::SSDIN_Clockwise:
		NewDir += 2;
		if (NewDir == 9) return ESSDInputDir::SSDID_Right;
		else return (ESSDInputDir)NewDir;
		break;

	case ESSDInputNeighbor::SSDIN_Opposite:
	case ESSDInputNeighbor::SSDIN_Mirror:
		return GetNeighborDir(Dir, Neighbor);
		break;

	default:
		return ESSDInputDir::SSDID_None;
		break;
	}
}

float ASSDPlayerController::GetHorizontalAxisValueFromDirections()
{
	FKey* Right = DirKey.Find(ESSDInputDir::SSDID_Right);
	FKey* Left = DirKey.Find(ESSDInputDir::SSDID_Left);

	if (Right)
	{
		FKey RightValue = *Right;
		if (IsInputKeyDown(RightValue))
			return 1.f;
	}
	if (Left)
	{
		FKey LeftValue = *Left;
		if (IsInputKeyDown(LeftValue))
			return -1.f;
	}

	return 0.f;
}

float ASSDPlayerController::GetVerticalAxisValueFromDirections()
{
	FKey* Up = DirKey.Find(ESSDInputDir::SSDID_Up);
	FKey* Down = DirKey.Find(ESSDInputDir::SSDID_Down);

	if (Up)
	{
		FKey UpValue = *Up;
		if (IsInputKeyDown(UpValue))
			return 1.f;
	}
	if (Down)
	{
		FKey DownValue = *Down;
		if (IsInputKeyDown(DownValue))
			return -1.f;
	}

	return 0.f;
}

void ASSDPlayerController::CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New)
{
	if (!PosessedPawn) return;

	Inputs->CheckDirPatterns(Old, New);

	int CurrentIndex = InputBuffer.Size - 1;

	// If new entry was too slow, reset patterns.
	if ((InputBuffer[CurrentIndex].Time - InputBuffer[CurrentIndex - 1].Time) > KEY_MOVE_TIME)
		ResetDirPatterns();

	// Dash
	switch (D)
	{
	case 0:
		D++;
		break;

	case 1:
		if (InputBuffer[CurrentIndex].Direction == InputBuffer[CurrentIndex - 1].Direction)
		{
			// Same direction pressed twice, do a dash
			D++;
			PosessedPawn->Dash(InputBuffer[CurrentIndex].Direction);
			ResetDirPatterns();
		}
		else D = 0;
		break;
	}

}

void ASSDPlayerController::ResetDirPatterns()
{
	if (!PosessedPawn) return;

	CurrentMove.Reset();

	D = 0;

}

void ASSDPlayerController::TransformPressed()
{
	if (!PosessedPawn) return;

	PosessedPawn->TransformPressed();

	InputBuffer.Push(FSSDInput(ESSDInputDir::SSDID_None, ESSDInputAttack::SSDIA_Transform, FPlatformTime::Seconds()));
}

void ASSDPlayerController::TransformReleased()
{
	if (!PosessedPawn) return;

	PosessedPawn->TransformReleased();
}
