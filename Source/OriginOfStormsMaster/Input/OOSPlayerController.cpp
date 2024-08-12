// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSPlayerController.h"
#include "OriginOfStormsMaster/OOSPawn.h"
#include "OriginOfStormsMaster/OOSGameInstance.h"
#include "Engine.h"

AOOSPlayerController::AOOSPlayerController()
{
	bAutoManageActiveCameraTarget = false;

	Inputs = CreateDefaultSubobject<UOOSFighterInputs>(TEXT("FighterInputs"));
	Inputs->PCOwner = this;

}

void AOOSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Inputs->SetMotionProcessors();
}

void AOOSPlayerController::OnPossess(APawn *aPawn)
{
	Super::OnPossess(aPawn);

	PosessedPawn = Cast<AOOSPawn>(aPawn);
}

void AOOSPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	PosessedPawn = nullptr;
	Inputs->DPadState = EOOSInputDir::OOSID_None;
}

void AOOSPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	Inputs->Tick(DeltaTime);

}

void AOOSPlayerController::SetupInputComponent()
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

	InputComponent->BindAxis("HorizontalAxis", this, &AOOSPlayerController::HorizontalAxisPressed);
	InputComponent->BindAxis("VerticalAxis", this, &AOOSPlayerController::VerticalAxisPressed);

}

void AOOSPlayerController::BuildBindingsFromSettings()
{
	UWorld* World = GetWorld();
	if (World)
	{
		UOOSGameInstance* GameInstance = Cast<UOOSGameInstance>(UGameplayStatics::GetGameInstance(World));
		if (GameInstance)
		{
			ULocalPlayer* LP = GetLocalPlayer();
			if (LP)
			{
				int32 ID = LP->GetControllerId();
				if (GameInstance->Bindings.IsValidIndex(ID))
				{
					// Load direction and attack bindings and generate reversed-type maps.
					TArray<EOOSInputDir> DKeys;
					TArray<EOOSInputAttack> AKeys;
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

void AOOSPlayerController::KeyPressed(FKey Key)
{
	EOOSInputDir* Dir = KeyDir.Find(Key);
	if (Dir)
	{
		EOOSInputDir NewDir = *Dir;

		EOOSInputDir ACW = GetCardinalNeighborDir(NewDir, EOOSInputNeighbor::OOSIN_AntiClockwise);
		EOOSInputDir CW = GetCardinalNeighborDir(NewDir, EOOSInputNeighbor::OOSIN_Clockwise);

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
		EOOSInputAttack* Att = KeyAtt.Find(Key);
		if (Att)
		{
			EOOSInputAttack NewAtt = *Att;

			if (NewAtt == EOOSInputAttack::OOSIA_Transform)
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

void AOOSPlayerController::KeyReleased(FKey Key)
{
	EOOSInputDir* Dir = KeyDir.Find(Key);
	if (Dir)
	{
		EOOSInputDir NewDir = *Dir;

		EOOSInputDir ACW = GetCardinalNeighborDir(NewDir, EOOSInputNeighbor::OOSIN_AntiClockwise);
		EOOSInputDir CW = GetCardinalNeighborDir(NewDir, EOOSInputNeighbor::OOSIN_Clockwise);

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
		EOOSInputAttack* Att = KeyAtt.Find(Key);
		if (Att)
		{
			EOOSInputAttack NewAtt = *Att;

			if (NewAtt == EOOSInputAttack::OOSIA_Transform)
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

void AOOSPlayerController::ResetKeyGroupTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(KeyGroupTimer);
	ResetDirPatterns();
}

void AOOSPlayerController::DirPressed(EOOSInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey)
{
	if (!PosessedPawn) return;

	// If down pressed, save timestamp for superjump.
	if (InputDir == EOOSInputDir::OOSID_Down) Inputs->SuperJump = FPlatformTime::Seconds();

	ProcessDirPress(InputDir, ACWNeighborKey, CWNeighborKey);

	// If up pressed, try jumping
	if (InputDir == EOOSInputDir::OOSID_Up) PosessedPawn->Jump();

	// If up pressed, buffer a move for jump cancels.
	if (Inputs->DPadState == EOOSInputDir::OOSID_Up || Inputs->DPadState == EOOSInputDir::OOSID_UpRight || Inputs->DPadState == EOOSInputDir::OOSID_UpLeft)
	{
		FOOSInputMove NewMove = FOOSInputMove(EOOSPatternType::OOSPT_None, EOOSDirPattern::OOSDP_None, Inputs->DPadState, EOOSInputAttack::OOSIA_None, FPlatformTime::Seconds());
		Inputs->MoveBuffer.Insert(NewMove, 0);
	}

}

void AOOSPlayerController::ProcessDirPress(EOOSInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey)
{
	// Check if any neighbor keys are down and buffer a diagonal if any.
	if (IsInputKeyDown(ACWNeighborKey))
	{
		EOOSInputDir NewDir = GetNeighborDir(InputDir, EOOSInputNeighbor::OOSIN_AntiClockwise);
		if (Inputs->DPadState != NewDir)
		{
			InputBuffer.Push(FOOSInput(NewDir, EOOSInputAttack::OOSIA_None, FPlatformTime::Seconds())); // Buffer direction and save timestamp.
			CheckDirPatterns(Inputs->DPadState, NewDir); // We advance our directional pattern state machine every time we buffer a direction.
			Inputs->DPadState = NewDir; // Update D-Pad state for BP.
		}
		return;
	}

	if (IsInputKeyDown(CWNeighborKey))
	{
		EOOSInputDir NewDir = GetNeighborDir(InputDir, EOOSInputNeighbor::OOSIN_Clockwise);
		if (Inputs->DPadState != NewDir)
		{
			InputBuffer.Push(FOOSInput(NewDir, EOOSInputAttack::OOSIA_None, FPlatformTime::Seconds()));
			CheckDirPatterns(Inputs->DPadState, NewDir);
			Inputs->DPadState = NewDir;
		}
		return;
	}

	InputBuffer.Push(FOOSInput(InputDir, EOOSInputAttack::OOSIA_None, FPlatformTime::Seconds()));
	CheckDirPatterns(Inputs->DPadState, InputDir);
	Inputs->DPadState = InputDir;
}

void AOOSPlayerController::DirReleased(EOOSInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey)
{
	if (!PosessedPawn) return;

	ProcessDirRelease(InputDir, ACWNeighborKey, CWNeighborKey);

}

void AOOSPlayerController::ProcessDirRelease(EOOSInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey)
{
	if (IsInputKeyDown(ACWNeighborKey))
	{
		EOOSInputDir NewDir = GetNeighborDir(Inputs->DPadState, EOOSInputNeighbor::OOSIN_AntiClockwise);
		InputBuffer.Push(FOOSInput(NewDir, EOOSInputAttack::OOSIA_None, FPlatformTime::Seconds()));
		CheckDirPatterns(Inputs->DPadState, NewDir);
		Inputs->DPadState = NewDir;
		return;
	}

	if (IsInputKeyDown(CWNeighborKey))
	{
		EOOSInputDir NewDir = GetNeighborDir(Inputs->DPadState, EOOSInputNeighbor::OOSIN_Clockwise);
		InputBuffer.Push(FOOSInput(NewDir, EOOSInputAttack::OOSIA_None, FPlatformTime::Seconds()));
		CheckDirPatterns(Inputs->DPadState, NewDir);
		Inputs->DPadState = NewDir;
		return;
	}

	CheckDirPatterns(Inputs->DPadState, EOOSInputDir::OOSID_None);
	Inputs->DPadState = EOOSInputDir::OOSID_None;
}

FOOSInput AOOSPlayerController::GetInputBufferElement(int Element) const
{
	if (InputBuffer.IsValidIndex(Element))
	{
		return InputBuffer[Element];
	}
	else
	{
		return FOOSInput();
	}
}

int AOOSPlayerController::GetInputBufferSize() const
{
	return InputBuffer.Num();
}

void AOOSPlayerController::HorizontalAxisPressed(float Value)
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

void AOOSPlayerController::VerticalAxisPressed(float Value)
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

void AOOSPlayerController::DispatchDigitalStickEvents_H(float Value)
{
	if (LastStick_H == 0.f)
	{
		if (Value != 0.f)
		{
			if (Value < 0.f)
			{
				DirPressed(EOOSInputDir::OOSID_Left, EKeys::Gamepad_LeftStick_Up, EKeys::Gamepad_LeftStick_Down);
			}
			else
			{
				DirPressed(EOOSInputDir::OOSID_Right, EKeys::Gamepad_LeftStick_Down, EKeys::Gamepad_LeftStick_Up);
			}
		}
	}
	else
	{
		if (Value == 0.f)
		{
			if (LastStick_H < 0.f)
			{
				DirReleased(EOOSInputDir::OOSID_Left, EKeys::Gamepad_LeftStick_Up, EKeys::Gamepad_LeftStick_Down);
			}
			else
			{
				DirReleased(EOOSInputDir::OOSID_Right, EKeys::Gamepad_LeftStick_Down, EKeys::Gamepad_LeftStick_Up);
			}
		}
	}

	LastStick_H = Value;
}

void AOOSPlayerController::DispatchDigitalStickEvents_V(float Value)
{
	if (LastStick_V == 0.f)
	{
		if (Value != 0.f)
		{
			if (Value < 0.f)
			{
				DirPressed(EOOSInputDir::OOSID_Down, EKeys::Gamepad_LeftStick_Left, EKeys::Gamepad_LeftStick_Right);
			}
			else
			{
				DirPressed(EOOSInputDir::OOSID_Up, EKeys::Gamepad_LeftStick_Right, EKeys::Gamepad_LeftStick_Left);
			}
		}
	}
	else
	{
		if (Value == 0.f)
		{
			if (LastStick_V < 0.f)
			{
				DirReleased(EOOSInputDir::OOSID_Down, EKeys::Gamepad_LeftStick_Left, EKeys::Gamepad_LeftStick_Right);
			}
			else
			{
				DirReleased(EOOSInputDir::OOSID_Up, EKeys::Gamepad_LeftStick_Right, EKeys::Gamepad_LeftStick_Left);
			}
		}
	}

	LastStick_V = Value;
}

void AOOSPlayerController::AttackPressed(EOOSInputAttack InputAttack)
{
	if (!PosessedPawn) return;

	if (InputAttack == EOOSInputAttack::OOSIA_None) return;

	Inputs->AttackPressed(InputAttack);

	FOOSInput& LastInput = InputBuffer.Peek(); // Get ref to latest buffer entry.
	if (((FPlatformTime::Seconds() - LastInput.Time) <= KEY_GROUP_TIME) && (LastInput.Attack != EOOSInputAttack::OOSIA_Transform))
	{
		// We pressed an attack fast enough to stack it onto the last attack.
		uint8 NewAttack = (uint8)LastInput.Attack;
		NewAttack |= (uint8)InputAttack;
		if (NewAttack != (uint8)EOOSInputAttack::OOSIA_None)
		{
			LastInput.Attack = (EOOSInputAttack)NewAttack;
			CurrentMove.Attack = (EOOSInputAttack)NewAttack;
		}
	}
	else
	{
		// Too late or we buffered transform, add another buffer entry.
		InputBuffer.Push(FOOSInput(EOOSInputDir::OOSID_None, InputAttack, FPlatformTime::Seconds()));
		CurrentMove.Attack = InputAttack;
	}

	// Pressing an attack button means ending a move, so we shoud buffer it. But first we need to wait KEY_GROUP_TIME seconds
	// to stack another possible attack press.
	if(!GetWorld()->GetTimerManager().IsTimerActive(KeyGroupTimer))
		GetWorld()->GetTimerManager().SetTimer(KeyGroupTimer, this, &AOOSPlayerController::OnAttackGroupTimeout, KEY_GROUP_TIME, false);

}

void AOOSPlayerController::AttackReleased(EOOSInputAttack InputAttack)
{
	if (!PosessedPawn) return;

	if (InputAttack == EOOSInputAttack::OOSIA_None) return;

	Inputs->AttackReleased(InputAttack);

}

void AOOSPlayerController::OnAttackGroupTimeout()
{
	if (!PosessedPawn) return;


	if ((FPlatformTime::Seconds() - CurrentMove.Time) > KEY_MOVE_TIME)
	{
		// Too late to press an attack - we lose the direction pattern and buffer the attacks only.
		CurrentMove.DirPattern = EOOSDirPattern::OOSDP_None;
		CurrentMove.PatternType = EOOSPatternType::OOSPT_None;

		// If there's a direction buffered together with the attack, add it to the move.
		if (Inputs->DPadState != EOOSInputDir::OOSID_None)
			CurrentMove.Direction = Inputs->DPadState;

	}

	// Update timetag now.
	CurrentMove.Time = FPlatformTime::Seconds();
	Inputs->MoveBuffer.Insert(CurrentMove, 0);

	ResetDirPatterns();
}

EOOSInputDir AOOSPlayerController::GetNeighborDir(EOOSInputDir Dir, EOOSInputNeighbor Neighbor)
{
	if (Dir == EOOSInputDir::OOSID_None) return EOOSInputDir::OOSID_None;

	uint8 NewDir = (uint8)Dir;

	switch (Neighbor)
	{
	case EOOSInputNeighbor::OOSIN_AntiClockwise:
		NewDir--;
		if (NewDir == 0) return EOOSInputDir::OOSID_DownRight;
		else return (EOOSInputDir)NewDir;
		break;

	case EOOSInputNeighbor::OOSIN_Clockwise:
		NewDir++;
		if (NewDir == 9) return EOOSInputDir::OOSID_Right;
		else return (EOOSInputDir)NewDir;
		break;
	
	case EOOSInputNeighbor::OOSIN_Opposite:
		if (NewDir == 0) return EOOSInputDir::OOSID_None;

		// Rotate 4 inputs forward
		NewDir = NewDir + 4;
		if (NewDir >= 9) NewDir -= 8;
		return (EOOSInputDir)NewDir;
		break;

	case EOOSInputNeighbor::OOSIN_Mirror:
		switch (Dir)
		{
		case EOOSInputDir::OOSID_DownLeft:
			NewDir = (uint8)EOOSInputDir::OOSID_DownRight;
			break;
		case EOOSInputDir::OOSID_Left:
			NewDir = (uint8)EOOSInputDir::OOSID_Right;
			break;
		case EOOSInputDir::OOSID_UpLeft:
			NewDir = (uint8)EOOSInputDir::OOSID_UpRight;
			break;

		case EOOSInputDir::OOSID_DownRight:
			NewDir = (uint8)EOOSInputDir::OOSID_DownLeft;
			break;
		case EOOSInputDir::OOSID_Right:
			NewDir = (uint8)EOOSInputDir::OOSID_Left;
			break;
		case EOOSInputDir::OOSID_UpRight:
			NewDir = (uint8)EOOSInputDir::OOSID_UpLeft;
			break;
		}
		return (EOOSInputDir)NewDir;
		break;

	default:
		return EOOSInputDir::OOSID_None;
		break;
	}
}

EOOSInputDir AOOSPlayerController::GetCardinalNeighborDir(EOOSInputDir Dir, EOOSInputNeighbor Neighbor)
{
	if (Dir == EOOSInputDir::OOSID_None) return EOOSInputDir::OOSID_None;

	int NewDir = (int)Dir;
	if((NewDir == 2) || (NewDir == 4) || (NewDir == 6) || (NewDir == 8)) return EOOSInputDir::OOSID_None;

	switch (Neighbor)
	{
	case EOOSInputNeighbor::OOSIN_AntiClockwise:
		NewDir -= 2;
		if (NewDir == -1) return EOOSInputDir::OOSID_Down;
		else return (EOOSInputDir)NewDir;
		break;

	case EOOSInputNeighbor::OOSIN_Clockwise:
		NewDir += 2;
		if (NewDir == 9) return EOOSInputDir::OOSID_Right;
		else return (EOOSInputDir)NewDir;
		break;

	case EOOSInputNeighbor::OOSIN_Opposite:
	case EOOSInputNeighbor::OOSIN_Mirror:
		return GetNeighborDir(Dir, Neighbor);
		break;

	default:
		return EOOSInputDir::OOSID_None;
		break;
	}
}

float AOOSPlayerController::GetHorizontalAxisValueFromDirections()
{
	FKey* Right = DirKey.Find(EOOSInputDir::OOSID_Right);
	FKey* Left = DirKey.Find(EOOSInputDir::OOSID_Left);

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

float AOOSPlayerController::GetVerticalAxisValueFromDirections()
{
	FKey* Up = DirKey.Find(EOOSInputDir::OOSID_Up);
	FKey* Down = DirKey.Find(EOOSInputDir::OOSID_Down);

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

void AOOSPlayerController::CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New)
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

void AOOSPlayerController::ResetDirPatterns()
{
	if (!PosessedPawn) return;

	CurrentMove.Reset();

	D = 0;

}

void AOOSPlayerController::TransformPressed()
{
	if (!PosessedPawn) return;

	PosessedPawn->TransformPressed();

	InputBuffer.Push(FOOSInput(EOOSInputDir::OOSID_None, EOOSInputAttack::OOSIA_Transform, FPlatformTime::Seconds()));
}

void AOOSPlayerController::TransformReleased()
{
	if (!PosessedPawn) return;

	PosessedPawn->TransformReleased();
}
