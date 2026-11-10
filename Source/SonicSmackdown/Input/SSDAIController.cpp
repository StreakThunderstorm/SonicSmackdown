// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDAIController.h"
#include "SSDGameInstance.h"
#include "SSDGameMode.h"
#include "SSDMovementComponent.h"
#include "SSDPawn_Transformed.h"
#include "SSDPlayerController.h"
#include "MNComboSet.h"
#include "Engine.h"
#include "SonicSmackdown.h"

ASSDAIController::ASSDAIController() 
{
	// Set this pawn to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Inputs = CreateDefaultSubobject<USSDFighterInputs>(TEXT("FighterInputs"));
	ComboSet = CreateDefaultSubobject<UMNComboSet>(TEXT("ComboSet"));

}

void ASSDAIController::SetPlayer(ASSDPlayerController* NewPlayerController)
{
	if (PlayerController || !NewPlayerController)
	{
		return;
	}

	PlayerController = NewPlayerController;
}

bool ASSDAIController::WantsPlayerController() const
{
	return !PlayerController && Action == ESSDAIAction::SSDAIA_Human;
}

void ASSDAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!World || !ControlledPawn || !ControlledPawn->MovementComponent) return;

	if (PlayerController)
	{
		// Cancel player control if set to another action
		if (Action != ESSDAIAction::SSDAIA_Human)
		{
			if (PlayerController->PosessedPawn)
			{
				PlayerController->PosessedPawn = nullptr;
			}

			const UWorld* CurrentWorld = GetWorld();
			if (!CurrentWorld) return;

			ASSDGameMode* GameMode = Cast<ASSDGameMode>(UGameplayStatics::GetGameMode(CurrentWorld));
			if (!GameMode) return;

			// Remove player controller, set an unassigned controller to the game mode
			GameMode->UnassignController(PlayerController);
			PlayerController = nullptr;
		}
	}

	// We must get rid of this here in case we "humanize" a training pawn in the middle of a transform press.
	if (bPressedTransform)
	{
		ControlledPawn->TransformReleased();
		bPressedTransform = false;
	}

	if (Action == ESSDAIAction::SSDAIA_Human)
	{
		// If has a player controller, we can try to have human control
		if (PlayerController)
		{
			if (PlayerController->PosessedPawn != ControlledPawn)
			{
				PlayerController->PosessedPawn = ControlledPawn;
			}
		}
	}
	else if (Action == ESSDAIAction::SSDAIA_CPU)
	{
		TickAI(DeltaTime);
	}
	else
	{
		switch (Action)
		{
		case ESSDAIAction::SSDAIA_Stand:
			ControlledPawn->MoveHorizontal(0.f);
			ControlledPawn->Uncrouch();
			break;

		case ESSDAIAction::SSDAIA_Crouch:
			ControlledPawn->Crouch();
			break;

		case ESSDAIAction::SSDAIA_Jump:
			ControlledPawn->MoveHorizontal(0.f);
			ControlledPawn->Uncrouch();
			if (ControlledPawn->MovementComponent->bOnGround)
			{
				if (!World->GetTimerManager().IsTimerActive(AutoJump))
					World->GetTimerManager().SetTimer(AutoJump, this, &ASSDAIController::DoAutoJump, 0.05f);
			}
			break;

		case ESSDAIAction::SSDAIA_SuperJump:
			ControlledPawn->MoveHorizontal(0.f);
			ControlledPawn->Uncrouch();
			if (ControlledPawn->MovementComponent->bOnGround)
			{
				if (!World->GetTimerManager().IsTimerActive(AutoJump))
					World->GetTimerManager().SetTimer(AutoJump, this, &ASSDAIController::DoAutoSuperJump, 0.05f);
			}
			break;

		case ESSDAIAction::SSDAIA_Forward:
		{
			float AxisValue = ControlledPawn->MovementComponent->bIsFacingRight ? 1.f : -1.f;
			if (ControlledPawn->MovementComponent->bOnGround) ControlledPawn->MoveHorizontal(AxisValue);
			ControlledPawn->Uncrouch();
		}
		break;

		default:
			break;
		}

		if (bPushBlock && (ControlledPawn->IsBlocking() || ControlledPawn->IsCrouchBlocking()))
		{
			ControlledPawn->PushBlock();
		}

		/* Training autoburst */
		if ((AutoBurst != ESSDAIAutoBurst::SSDAIAB_Off) && ControlledPawn->IsTransformReady())
		{
			int ABHits = -1;
			switch (AutoBurst)
			{
			case ESSDAIAutoBurst::SSDAIAB_Random:
				if (ControlledPawn->Opponent->HitCount > 0)
				{
					if (!bRandomAutoBurstSet)
					{
						RandomAutoBurst = FMath::RandRange(ControlledPawn->Opponent->HitCount, FixedAutoBurst);
						bRandomAutoBurstSet = true;
					}
					ABHits = RandomAutoBurst;
				}
				else
				{
					bRandomAutoBurstSet = false;
				}
				break;

			case ESSDAIAutoBurst::SSDAIAB_Fixed:
				ABHits = FixedAutoBurst;
				break;
			}

			if (ControlledPawn->Opponent->HitCount == ABHits)
			{
				ControlledPawn->TransformPressed();
				bRandomAutoBurstSet = false;
			}
		}
	}
}

void ASSDAIController::BeginPlay()
{
	Super::BeginPlay();

	World = GetWorld();

	Inputs->SetMotionProcessors();
}

void ASSDAIController::OnPossess(APawn * InPawn)
{
	Super::OnPossess(InPawn);

	ControlledPawn = Cast<ASSDPawn>(InPawn);
	ComboSet->SetMoveNetwork(ControlledPawn->MoveNetwork);
	// Get super moves
	SuperMoves.Reset();
	UltraMoves.Reset();
	if (ControlledPawn->MoveNetwork)
	{
		for (size_t i = 0; i < ControlledPawn->MoveNetwork->RootNodes.Num(); i++)
		{
			UMNNode* Node = ControlledPawn->MoveNetwork->RootNodes[i];
			UMNNode_Move* MoveNode = Cast<UMNNode_Move>(Node);

			if (MoveNode)
			{
				if (MoveNode->Move.MoveType == ESSDMoveType::SSDMT_Super)
				{
					SuperMoves.Add(MoveNode);
				}
				else if (MoveNode->Move.MoveType == ESSDMoveType::SSDMT_Ultra)
				{
					UltraMoves.Add(MoveNode);
				}
			}
		}
	}

	bTransformedPawn = ControlledPawn->IsA(ASSDPawn_Transformed::StaticClass());
}

void ASSDAIController::OnUnPossess()
{
	Super::OnUnPossess();

	ControlledPawn = nullptr;
	ComboSet->Reset();
	SuperMoves.Reset();
	UltraMoves.Reset();
}

ESSDMoveDir ASSDAIController::NotifyHit(ESSDHitHeight Height, bool bOH)
{
	bool bCrossup = ControlledPawn->MovementComponent->bIsFacingRight !=
		ControlledPawn->MovementComponent->bShouldFaceRight;

	switch (Action)
	{
	case ESSDAIAction::SSDAIA_Human:
		return ESSDMoveDir::SSDMD_None;
		break;
	case ESSDAIAction::SSDAIA_CPU:
		bool BlockSuccess;
		// Block chance is higher when on defense
		//@TODO: CPU should switch to SSDAIFS_Defense more readily
		//@TODO: for example, after hitting with a very negative button that can't be cancelled
		if (FightState == ESSDAIFightState::SSDAIFS_Defense)
		{
			if (bCrossup)
				BlockSuccess = RollAIOdds(0.3f, 0.9f);
			else if (bOH)
				BlockSuccess = RollAIOdds(0.4f, 0.95f);
			else if (Height == ESSDHitHeight::SSDHH_Low)
				BlockSuccess = RollAIOdds(0.5f, 0.98f);
			else
				BlockSuccess = RollAIOdds(0.7f, 0.99f);
		}
		// Block chance lower if in neutral
		else
		{
			if (bCrossup)
				BlockSuccess = RollAIOdds(0.1f, 0.6f);
			else
				BlockSuccess = RollAIOdds(0.15f, 0.85f);
		}

		if (BlockSuccess)
		{
			if (Height == ESSDHitHeight::SSDHH_Low && !bOH)
			{
				return ESSDMoveDir::SSDMD_DownBack;
			}
			else
			{
				return ESSDMoveDir::SSDMD_Back;
			}
		}
		return ESSDMoveDir::SSDMD_None;
		break;

	default:
		switch (Block)
		{
		case ESSDAIBlock::SSDAIB_None:
			return ESSDMoveDir::SSDMD_None;
			break;
		case ESSDAIBlock::SSDAIB_All:
			if (Height == ESSDHitHeight::SSDHH_Low && !bOH)
			{
				return ESSDMoveDir::SSDMD_DownBack;
			}
			else
			{
				return ESSDMoveDir::SSDMD_Back;
			}
			break;
		case ESSDAIBlock::SSDAIB_Crouch:
			return ESSDMoveDir::SSDMD_DownBack;
			break;
		case ESSDAIBlock::SSDAIB_Standing:
			return ESSDMoveDir::SSDMD_Back;
			break;
		}
		break;
	}

	return ESSDMoveDir::SSDMD_None;
}

void ASSDAIController::DoAutoJump()
{
	ControlledPawn->Jump();
}

void ASSDAIController::DoAutoSuperJump()
{
	ControlledPawn->SuperJump();
}

void ASSDAIController::UpdateFightState()
{
	if (!ControlledPawn || !ControlledPawn->Opponent) return;

	// If hitting the opponent and they're not blocking
	if (ControlledPawn->HitCount > 0 &&
		(!ControlledPawn->Opponent->IsBlocking() || !ControlledPawn->Opponent->IsCrouchBlocking()) &&
		!ControlledPawn->Opponent->IsRecoveringOnGround())
	{
		// 80-2 frames to reliably react, from AI 0-1
		if (RollAIOdds(0.055f, 0.85f))
		{
			ChangeFightState(ESSDAIFightState::SSDAIFS_Combo);
		}
	}
	// If hitting the opponent and they're blocking or downed
	else if (ControlledPawn->Opponent->IsBlocking() || ControlledPawn->Opponent->IsCrouchBlocking() ||
		ControlledPawn->Opponent->IsRecoveringOnGround())
	{
		// 80-2 frames to reliably react, from AI 0-1
		if (RollAIOdds(0.055f, 0.85f))
		{
			ChangeFightState(ESSDAIFightState::SSDAIFS_Pressure);
		}
	}
	// If being hit
	else if (ControlledPawn->Opponent->HitCount > 0 ||
		ControlledPawn->IsBlocking() ||
		ControlledPawn->IsCrouchBlocking() ||
		ControlledPawn->IsRecoveringOnGround())
	{
		// 120-8 frames to reliably react, from AI 0-1
		if (RollAIOdds(0.0375f, 0.44f))
		{
			ChangeFightState(ESSDAIFightState::SSDAIFS_Defense);
		}
	}
	else
	{
		// 120-12 frames to reliably react, from AI 0-1
		if (RollAIOdds(0.0375f, 0.32f))
		{
			ChangeFightState(ESSDAIFightState::SSDAIFS_Neutral);
		}
	}
}

void ASSDAIController::ChangeFightState(ESSDAIFightState NewState)
{
	if (FightState != NewState)
	{
		FightState = NewState;
		StateTime = 0.f;
	}
}

void ASSDAIController::TickAI(float DeltaTime)
{
	FSSDInput OldInputBuffer = InputBuffer;
	InputBuffer.Attack = ESSDInputAttack::SSDIA_None; // Reset attack button

	// If we're detransformed, just force reset the timer.
	TransformTimer = FMath::Max(0.f, bTransformedPawn ? (TransformTimer - DeltaTime) : 0.f);

	// Figure out how the fight is going
	UpdateFightState();

	// Get some values that the rest of the function will use
	// Don't hit opponents who will take forever to land when on the ground
	bool bJuggleInPosition = false;
	bool bValidJuggle = false;
	if (ControlledPawn->Opponent->MovementComponent->bOnGround)
	{
		// If opponent is also on the ground, fine to press buttons unless downed
		bJuggleInPosition = !ControlledPawn->Opponent->IsFloored();
		bValidJuggle = bJuggleInPosition;
	}
	else
	{
		// Otherwise juggling, need them to be close to the ground
		int FramesUntilOpponentLands = ControlledPawn->Opponent->MovementComponent->FramesUntilFloor();
		int FramesUntilOpponentRecovers = ControlledPawn->Opponent->GetHitStunRemaining();
		if (FramesUntilOpponentLands < 25 || FramesUntilOpponentRecovers < 25)
		{
			int Frames = FMath::Min(FramesUntilOpponentLands, FramesUntilOpponentRecovers);
			// 10% + 15% per frame after 25
			float JuggleOdds = 0.1f + (25 - Frames) * 15 / 100.f;
			// 100% - 20% per frame after 12 until landing
			float LandingOdds = 1.f - (12 - FramesUntilOpponentLands) / 5.f;

			bJuggleInPosition = JuggleOdds > 0 && LandingOdds > 0;

			if (RollAIOdds(JuggleOdds / 2.f, JuggleOdds) &&
				RollAIOdds(LandingOdds, LandingOdds))
			{
				bValidJuggle = true;
			}
		}
	}

	// This is checked once per second after changing to a new fight state
	if (FMath::IsNearlyZero(StateTime) || (int)StateTime != (int)(StateTime + DeltaTime))
	{
		// Update whether it's a good idea to do a super
		ASSDPawn* Opponent = ControlledPawn->Opponent;

		// Near having full meter, spend it if you got it
		float MeterOdds = (ControlledPawn->SuperPts / (float)MAX_SUPER_POINTS_PER_LV) / MAX_SUPER_LEVELS;
		// Do supers if the opponent is near full burst to pre-empt them, but not if they're ready
		float BurstOdds = 0.5f + 0.5f * Opponent->Transform / (float)MAX_TRANSFORM_POINTS;
		if (Opponent->Transform >= MAX_TRANSFORM_POINTS)
			BurstOdds = 0.5f;
		// Do supers if the opponent is nearly dead to finish them off
		float LifeOdds = 0.5f + 0.5f * (Opponent->MaxHealth - Opponent->Health) / (float)Opponent->MaxHealth;

		// Combine the odds
		// 1.f if max meter, opponent has 1 hp, and opponent one hit away from burst
		// 0.1f if one bar of meter, opponent at full health and one hit from burst
		// 0.05f if one bar of meter, opponent at full health and with burst
		float SuperOdds = MeterOdds * BurstOdds * LifeOdds;
		//float SuperOdds = MeterOdds * 0.3f + BurstOdds * 0.4f + LifeOdds * 0.3f;

		// Do supers at a much lower rate during training
		UWorld* CurrentWorld = GetWorld();
		USSDGameInstance* GameInstance = Cast<USSDGameInstance>(UGameplayStatics::GetGameInstance(CurrentWorld));
		//@TODO: allow supers at the normal rate during training if super meter isn't infinite?
		if (GameInstance->bTrainingMode && !false)
		{
			SuperOdds *= 0.1f;
		}

		bWantsToDoSuper = false;
		// Over the first 5 seconds of a combo, max 25-99% chance to occur, from AI 0-1
		// If full meter, opponent has half burst and full health, 10-72%
		// Three bars, opponent full burst and half health, 6-51%
		if (RollAIOdds(0.0559f * SuperOdds, 0.6019f * SuperOdds))
		{
			bWantsToDoSuper = true;
		}


		/*
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red,
			FString::Printf(TEXT("Super Odds: %f"),
				SuperOdds));*/
	}


	/*
	GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red,
		FString::Printf(bWantsToDoSuper ? TEXT("Big do Super") : TEXT("No Super")));*/


	/* // Debug messages
	GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red,
		FString::Printf(TEXT("%s"),
			*GETENUMSTRING("ESSDAIFightState", FightState)));
	GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red,
		FString::Printf(TEXT("Buffered moves: %d"),
			Inputs->MoveBuffer.Num()));
	GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green,
		FString::Printf(bValidJuggle ? TEXT("Can Juggle") : TEXT("Juggle Disabled")));
	GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red,
		FString::Printf(bJuggleInPosition ? TEXT("Can Juggle") : TEXT("Juggle Disabled")));*/

	// Press some buttons
	switch (FightState)
	{
	case ESSDAIFightState::SSDAIFS_Neutral:
		// Press random directions
		// 44 frames to reliably react
		if (RollAIOdds(0.1f, 0.1f))
		{
			// Roll odds for walking towards opponent or new direction.
			ESSDInputDir Direction;
			if (RollAIOdds(0.2f, 0.5f))
			{
				Direction = ControlledPawn->MovementComponent->bShouldFaceRight ? ESSDInputDir::SSDID_Right : ESSDInputDir::SSDID_Left;
			}
			else
			{			
				if (ControlledPawn->MovementComponent->bFlightMode)
				{
					float DistanceX = FMath::Abs(ControlledPawn->GetActorLocation().X - ControlledPawn->Opponent->GetActorLocation().X);
					if (DistanceX > 100.f)
					{
						if (ControlledPawn->GetActorLocation().Z > ControlledPawn->Opponent->GetActorLocation().Z)
						{
							if (ControlledPawn->GetActorLocation().X > ControlledPawn->Opponent->GetActorLocation().X)
								Direction = ESSDInputDir::SSDID_DownLeft;
							else if (ControlledPawn->GetActorLocation().X < ControlledPawn->Opponent->GetActorLocation().X)
								Direction = ESSDInputDir::SSDID_DownRight;
						}
						else
						{
							if (ControlledPawn->GetActorLocation().X > ControlledPawn->Opponent->GetActorLocation().X)
								Direction = ESSDInputDir::SSDID_UpLeft;
							else if (ControlledPawn->GetActorLocation().X < ControlledPawn->Opponent->GetActorLocation().X)
								Direction = ESSDInputDir::SSDID_UpRight;
						}
					}
					else
					{
						if (ControlledPawn->GetActorLocation().Z > ControlledPawn->Opponent->GetActorLocation().Z)
						{
							Direction = ESSDInputDir::SSDID_Down;
						}
						else
						{
							Direction = ESSDInputDir::SSDID_Up;
						}
					}
				}
				else
					Direction = (ESSDInputDir)FMath::RandRange(0, 8); //@TODO: use noise instead of rng
			}
			InputBuffer.Direction = Direction;
		}

		// Press random buttons
		// 44-10 frames to reliably react, from AI 0-1
		if (RollAIOdds(0.1f, 0.37f))
		{
			// 10% at JAB_RANGE
			if (RollJabRange(0.1f))
			{
				int StrengthIndex = RollWeightedIndex(3, 2.f); // Prefer lower strengths
				int Strength = (int)FMath::Pow(2.f, static_cast<float>(StrengthIndex)); //@TODO: use noise instead of rng

				//@TODO: This line isn't enough to press buttons just yet
				InputBuffer.AddAttackFlags((ESSDInputAttack)Strength);

				FSSDInputMove CurrentMove = FSSDInputMove();
				CurrentMove.Attack = (ESSDInputAttack)Strength;
				CurrentMove.PatternType = ESSDPatternType::SSDPT_None;
				CurrentMove.DirPattern = ESSDDirPattern::SSDDP_None;
				CurrentMove.Direction = RollAIOdds(0.1f, 0.4f) ?
					ESSDInputDir::SSDID_Down : InputBuffer.Direction;
				CurrentMove.Time = FPlatformTime::Seconds();

				Inputs->MoveBuffer.Insert(CurrentMove, 0);
			}
		}

		// Dashing
		// 1000-60 frames to reliably react, from AI 0-1
		if (ControlledPawn->MovementComponent->bFlightMode)
		{
			if (RollAIOdds(0.001f, 0.001f))
			{
				//@TODO: This line isn't enough to press buttons just yet
				InputBuffer.AddAttackFlags(ESSDInputAttack::SSDIA_MediumHeavy);

				FSSDInputMove CurrentMove = FSSDInputMove();
				CurrentMove.Attack = ESSDInputAttack::SSDIA_MediumHeavy;
				CurrentMove.PatternType = ESSDPatternType::SSDPT_None;
				CurrentMove.DirPattern = ESSDDirPattern::SSDDP_None;
				CurrentMove.Direction = ESSDInputDir::SSDID_None;
				CurrentMove.Time = FPlatformTime::Seconds();

				Inputs->MoveBuffer.Insert(CurrentMove, 0);
			}
		}
		else
		{
			if (RollAIOdds(0.001f, 0.025f))
			{
				//@TODO: This line isn't enough to press buttons just yet
				InputBuffer.AddAttackFlags(ESSDInputAttack::SSDIA_MediumHeavy);

				FSSDInputMove CurrentMove = FSSDInputMove();
				CurrentMove.Attack = ESSDInputAttack::SSDIA_MediumHeavy;
				CurrentMove.PatternType = ESSDPatternType::SSDPT_None;
				CurrentMove.DirPattern = ESSDDirPattern::SSDDP_None;
				CurrentMove.Direction = ESSDInputDir::SSDID_None;
				CurrentMove.Time = FPlatformTime::Seconds();

				Inputs->MoveBuffer.Insert(CurrentMove, 0);
			}
		}
		

		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Magenta, FString::SanitizeFloat(TransformTimer));

		// (De)transform chance on neutral.
		if (bTransformedPawn)
		{
			if (RollTransformOdds(0.002f, 0.007f, 2.f) && (TransformTimer <= 0.f))
			{
				PressTransform();
			}

			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, "Transformed");
		}
		else
		{
			// @TODO: Add AI transform charging support.
			if (ControlledPawn->IsTransformReady())
			{
				if (RollTransformOdds(0.002f, 0.007f, 2.f)) PressTransform();
				TransformTimer = 15.f; // "Intentional" transformed time should be longer to be credible.
			}
			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, "Normal");
		}
		break;
	case ESSDAIFightState::SSDAIFS_Defense:
		// Pushblock
		if (!ControlledPawn->IsPushBlocking() &&
			(ControlledPawn->IsBlocking() || ControlledPawn->IsCrouchBlocking()))
		{
			// Only pushblock if there's a lot of recovery left in block
			int PushblockFrames = FMath::Max(0, ControlledPawn->AnimationFramesRemaining() - 15);
			// 2% per frame above 15
			float PushblockOdds = PushblockFrames / 50.f;
			if (RollAIOdds(PushblockOdds / 10.f, PushblockOdds))
			{
				//@TODO: This line isn't enough to press buttons just yet
				InputBuffer.AddAttackFlags(ESSDInputAttack::SSDIA_MediumHeavy);

				FSSDInputMove CurrentMove = FSSDInputMove();
				CurrentMove.Attack = ESSDInputAttack::SSDIA_MediumHeavy;
				CurrentMove.PatternType = ESSDPatternType::SSDPT_None;
				CurrentMove.DirPattern = ESSDDirPattern::SSDDP_None;
				CurrentMove.Direction = ESSDInputDir::SSDID_None;
				CurrentMove.Time = FPlatformTime::Seconds();

				Inputs->MoveBuffer.Insert(CurrentMove, 0);
			}
		}

		// Burst (defensive transform)
		// Don't burst on blocking for now
		if (!ControlledPawn->IsBlocking() && !ControlledPawn->IsCrouchBlocking() && !bPressedTransform)
		{
			// (SSD-only) Also don't burst if we don't have a full enough gauge for now
			if (ControlledPawn->IsTransformReady())
			{
				// Don't burst during training
				UWorld* CurrentWorld = GetWorld();
				USSDGameInstance* GameInstance = Cast<USSDGameInstance>(UGameplayStatics::GetGameInstance(CurrentWorld));
				//@TODO: allow bursting during training if burst gauge isn't infinite?
				if (!GameInstance->bTrainingMode || false)
				{
					//@TODO: should also much prefer bursting when the opponent is
					// close and attacking and won't recover soon, to hopfully hit
					// them with the burst

					// Being hit and first 10 frames of hit state
					if ((ControlledPawn->GetHitStunRemaining() > 0) &&
						(ControlledPawn->AnimationFramesElapsed() < 10)
						&& !bTransformedPawn)
					{
						// 458-10 frames to reliably react, from AI 0-1
						if (RollTransformOdds(0.002f, 0.007f, 2.f)) PressTransform();
						TransformTimer = IsHealthLowerThan(0.3f) ? 5.f : 10.f; // Defensive transform time is longer when not in danger, shorter when under pressure.
					}
				}
			}
		}
		break;
	case ESSDAIFightState::SSDAIFS_Pressure:
	case ESSDAIFightState::SSDAIFS_Combo:
		bool bChoseInput = false;
		bool bAir = !ControlledPawn->MovementComponent->bOnGround;
		bool bOpponentAir = !ControlledPawn->Opponent->MovementComponent->bOnGround;

		// Stop pressing unneeded directions after landing hits
		// 120-44 frames to reliably react
		if (InputBuffer.Direction != ESSDInputDir::SSDID_None && RollAIOdds(0.0375f, 0.1f))
		{
			InputBuffer.Direction = ESSDInputDir::SSDID_None;
		}

		// Hit with launcher
		if (FightState == ESSDAIFightState::SSDAIFS_Combo &&
			ControlledPawn->IsPerformingMove() &&
			ControlledPawn->CurrentNode &&
			ControlledPawn->CurrentNode->Move.Attack == ESSDInputAttack::SSDIA_Special &&
			ControlledPawn->CurrentNode->Move.DirPattern == ESSDDirPattern::SSDDP_None &&
			!bAir)
		{
			if (RollAIOdds(0.1f, 0.99f))
			{
				// Hold S during launcher
				InputBuffer.AddAttackFlags(ESSDInputAttack::SSDIA_Special);
				InputBuffer.Direction = ESSDInputDir::SSDID_None;
				bChoseInput = true;


				//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue,
				//	FString::Printf(TEXT("Launch")));
			}
		}

		//if (ControlledPawn->CurrentNode && Inputs->MoveBuffer.Num() == 0 && !ControlledPawn->ReadiedInputNode)
		if (ControlledPawn->CurrentNode && !ControlledPawn->ReadiedInputNode)
		{
			// Get the child moves of the current move
			TArray<UMNNode_Move*> Moves = ComboSet->ComboNodes(ControlledPawn->CurrentNode);
			bool bShouldCancel = bAir || bValidJuggle;
			bool bLandedAttack = ControlledPawn->bHasLandedAttack || ControlledPawn->bIsPerformingSuper;

			if (bShouldCancel && !bChoseInput && Moves.Num() > 0 && bLandedAttack)
			{
				// 458-10 frames to reliably react, from AI 0-1
				if (!bChoseInput && RollAIOdds(0.01f, 0.37f))
				{
					// Get supers possible in the current air state
					TArray<UMNNode_Move*> PossibleSupers = TArray<UMNNode_Move*>();
					for (UMNNode_Move* Super : SuperMoves)
					{
						if (Super->Move.bAir == bAir)
							PossibleSupers.Add(Super);
					}
					if (ControlledPawn->GetSuperLevel() >= 3)
						for (UMNNode_Move* Ultra : UltraMoves)
						{
							//if (Ultra->Move.bAir == bAir)
							//	PossibleSupers.Add(Ultra);
						}

					// If wants to do a super
					if (bWantsToDoSuper && PossibleSupers.Num() > 0 &&
						FightState == ESSDAIFightState::SSDAIFS_Combo &&
						!ControlledPawn->bIsPerformingSuper)
					{
						int MoveIndex = RollWeightedIndex(PossibleSupers.Num() - 1, 1.f);
						UMNNode_Move* SuperMove = PossibleSupers[MoveIndex];

						FSSDInputMove CurrentMove = FSSDInputMove(
							SuperMove->Move.PatternType,
							SuperMove->Move.DirPattern,
							SuperMove->GetInputDir(),
							SuperMove->Move.Attack == ESSDInputAttack::SSDIA_AA ?
								ESSDInputAttack::SSDIA_MediumHeavy : SuperMove->Move.Attack,
							FPlatformTime::Seconds());

						Inputs->MoveBuffer.Insert(CurrentMove, 0);
						bChoseInput = true;
					}
					else
					{
						// Greatly prefer moves higher on the move list
						//@TODO: separate handling of different move types, eg Normal, Special, etc
						int MoveIndex = RollWeightedIndex(Moves.Num() - 1, 10.f);
						UMNNode_Move* NextMove = Moves[MoveIndex];

						if (bWantsToDoSuper || NextMove->Move.RequiredBars == 0)
						{
							FSSDInputMove CurrentMove = FSSDInputMove(
								NextMove->Move.PatternType,
								NextMove->Move.DirPattern,
								NextMove->GetInputDir(),
								NextMove->Move.Attack == ESSDInputAttack::SSDIA_AA ? ESSDInputAttack::SSDIA_MediumHeavy :
								(NextMove->Move.Attack == ESSDInputAttack::SSDIA_A ? ESSDInputAttack::SSDIA_Light : NextMove->Move.Attack),
								FPlatformTime::Seconds());

							Inputs->MoveBuffer.Insert(CurrentMove, 0);
							bChoseInput = true;
						}
					}


					//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue,
					//	FString::Printf(TEXT("Cancel")));
				}
			}
		}

		if (!ControlledPawn->ReadiedInputNode && !ControlledPawn->IsPerformingMove())
		{
			float DistanceX = FMath::Abs(ControlledPawn->GetActorLocation().X - ControlledPawn->Opponent->GetActorLocation().X);

			if (!bChoseInput)
			{
				// Move forward if too far away and neutral
				if (!bAir && !ControlledPawn->IsDashing())
				{
					// Dash
					// -20% + 1% per distance above 100
					float DashOdds = -0.2f + (DistanceX - 100) / 100.f;
					if (RollAIOdds(DashOdds / 10.f, DashOdds))
					{
						//@TODO: This line isn't enough to press buttons just yet
						InputBuffer.AddAttackFlags(ESSDInputAttack::SSDIA_MediumHeavy);

						FSSDInputMove CurrentMove = FSSDInputMove();
						CurrentMove.Attack = ESSDInputAttack::SSDIA_MediumHeavy;
						CurrentMove.PatternType = ESSDPatternType::SSDPT_None;
						CurrentMove.DirPattern = ESSDDirPattern::SSDDP_None;
						CurrentMove.Direction = ControlledPawn->MovementComponent->bIsFacingRight ?
							ESSDInputDir::SSDID_Right : ESSDInputDir::SSDID_Left;
						CurrentMove.Time = FPlatformTime::Seconds();

						Inputs->MoveBuffer.Insert(CurrentMove, 0);
						InputBuffer.Direction = CurrentMove.Direction;
						bChoseInput = true;


						//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue,
						//	FString::Printf(TEXT("Dash")));
					}
					// Walk
					// 120-44 frames to reliably react
					else if (RollAIOdds(0.0375f, 0.1f))
					{
						InputBuffer.Direction = ControlledPawn->MovementComponent->bIsFacingRight ?
							ESSDInputDir::SSDID_Right : ESSDInputDir::SSDID_Left;


						//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue,
						//	FString::Printf(TEXT("Walk")));
					}
				}				
			}
			
			if (!bChoseInput)
			{
				// If dashing and too close to a juggled opponent, crouch to cancel
				if (!bAir && ControlledPawn->IsDashing())
				{
					// 20% + 1% per distance below 80
					float DashCrouchOdds = 0.2f + (80 - DistanceX) / 100.f;

					if (RollAIOdds(DashCrouchOdds / 10.f, DashCrouchOdds))
					{
						InputBuffer.Direction = ESSDInputDir::SSDID_Down;
						bChoseInput = true;


						//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue,
						//	FString::Printf(TEXT("Crouch")));
					}
				}
			}

			if (!bChoseInput)
			{
				// Press random buttons in combos

				bool bPressNormals = false;
				// On the ground
				if (!bAir)
				{
					bPressNormals = bValidJuggle;
				}
				// In the air though
				else
				{
					const int Frames = 8; // About 8 frames into the future
					float FutureY = ControlledPawn->MovementComponent->GetFutureDistance(
						ControlledPawn->Opponent->MovementComponent, Frames).Z;

					// This is a pretty good range; some normals go up to 120, some go down to -112
					const int MaxAirDistanceY = 64;
					const int MinAirDistanceY = -96;
					if (FutureY < MaxAirDistanceY && FutureY > MinAirDistanceY)
					{
						bPressNormals = true;
					}
					// Well below opponent
					else if (false)//FutureY >= MaxAirDistanceY)
					{
						if (RollDistanceOdds(1.f, MaxAirDistanceY, 20.f, FutureY, 1.f))
							bPressNormals = true;
					}
					// Well above opponent
					else if (false)
					{
						if (RollDistanceOdds(1.f, MinAirDistanceY, -20.f, FutureY, 1.f))
							bPressNormals = true;
					}
				}

				if (FightState == ESSDAIFightState::SSDAIFS_Combo &&
					bPressNormals && OldInputBuffer.Attack == ESSDInputAttack::SSDIA_None)
				{
					// 458-20 or 10 frames to reliably react, from AI 0-1
					if (RollAIOdds(0.01f, (!bAir && !bOpponentAir) ? 0.2f : 0.37f))
					{
						// 25% at 100 distance
						if (RollXDistanceOdds(0.25f, 100.f, 80.f))
						{
							int StrengthIndex;
							if (bAir)
								// Mostly press heavy and sometimes medium, I guess
								StrengthIndex = RollWeightedIndex(1, 0.5f) + 1;
							else if (bOpponentAir)
								// Launcher when juggling, AI isn't good enough to do anything else yet
								StrengthIndex = 3;
							else
								StrengthIndex = RollWeightedIndex(3, 2.f); // Prefer lower strengths
							int Strength = (int)FMath::Pow(2.f, static_cast<float>(StrengthIndex)); //@TODO: use noise instead of rng

							//@TODO: This line isn't enough to press buttons just yet
							InputBuffer.AddAttackFlags((ESSDInputAttack)Strength);

							FSSDInputMove CurrentMove = FSSDInputMove();
							CurrentMove.Attack = (ESSDInputAttack)Strength;
							CurrentMove.PatternType = ESSDPatternType::SSDPT_None;
							CurrentMove.DirPattern = ESSDDirPattern::SSDDP_None;
							CurrentMove.Direction =
								RollAIOdds(0.1f, 0.4f) && !bOpponentAir ?
								ESSDInputDir::SSDID_Down : ESSDInputDir::SSDID_None;
							CurrentMove.Time = FPlatformTime::Seconds();

							Inputs->MoveBuffer.Insert(CurrentMove, 0);


							//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue,
							//	FString::Printf(TEXT("Buttons")));
						}
					}
				}
			}
		}


		/*
		// Just randomly do 214H on hit, they're all good ;Y
		// 458-10 frames to reliably react, from AI 0-1
		if (RollAIOdds(0.01f, 0.37f))
		{
			FSSDInputMove CurrentMove = FSSDInputMove();
			CurrentMove.Attack = ESSDInputAttack::SSDIA_Heavy;
			CurrentMove.PatternType = ESSDPatternType::SSDPT_Single;
			CurrentMove.DirPattern = ESSDDirPattern::SSDDP_QuarterCircle;
			CurrentMove.Direction = ControlledPawn->MovementComponent->bIsFacingRight ?
				ESSDInputDir::SSDID_Left : ESSDInputDir::SSDID_Right;
			CurrentMove.Time = FPlatformTime::Seconds();

			Inputs->MoveBuffer.Insert(CurrentMove, 0);
		}*/
		break;
	}

	// Tick inputs
	UpdateInputBuffer(OldInputBuffer);

	ControlledPawn->MoveHorizontal(InputBuffer.GetHorizontalAxis());
	ControlledPawn->MoveVertical(InputBuffer.GetVerticalAxis());

	Inputs->Tick(DeltaTime);

	// Force adding moves, if needed
	float Random = FMath::RandRange(0.f, 1.f); //@TODO: use noise instead of rng
	if (Random < 0)//0.2f)
	{
		FSSDInputMove CurrentMove = FSSDInputMove();
		CurrentMove.Attack = ESSDInputAttack::SSDIA_Heavy;
		CurrentMove.PatternType = ESSDPatternType::SSDPT_Single;
		CurrentMove.DirPattern = ESSDDirPattern::SSDDP_QuarterCircle;
		CurrentMove.Direction = ControlledPawn->MovementComponent->bIsFacingRight ?
			ESSDInputDir::SSDID_Left : ESSDInputDir::SSDID_Right;
		CurrentMove.Time = FPlatformTime::Seconds();

		Inputs->MoveBuffer.Insert(CurrentMove, 0);
	}

	// Update time in the current state
	StateTime += DeltaTime;
}

void ASSDAIController::UpdateInputBuffer(FSSDInput OldInputBuffer)
{
	// Directional inputs
	Inputs->CheckDirPatterns(OldInputBuffer.Direction, InputBuffer.Direction);
	Inputs->DPadState = InputBuffer.Direction;

	// Crouch/uncrouch
	if ((InputBuffer.Direction == ESSDInputDir::SSDID_Down || InputBuffer.Direction == ESSDInputDir::SSDID_DownRight || InputBuffer.Direction == ESSDInputDir::SSDID_DownLeft))
		ControlledPawn->Crouch();
	else
		ControlledPawn->Uncrouch();

	// Attack inputs
	ESSDInputAttack Pressed = (ESSDInputAttack)((uint8)InputBuffer.Attack & (~(uint8)OldInputBuffer.Attack));
	ESSDInputAttack Released = (ESSDInputAttack)((uint8)OldInputBuffer.Attack & (~(uint8)InputBuffer.Attack));

	Inputs->AttackReleased(Released);
	Inputs->AttackPressed(Pressed);
}

bool ASSDAIController::RollAIOdds(float MinLevelOdds, float MaxLevelOdds)
{
	float Odds = FMath::Lerp(MinLevelOdds, MaxLevelOdds, AILevel);
	float Random = FMath::RandRange(0.f, 1.f); //@TODO: use noise instead of rng
	return Random <= Odds;
}

bool ASSDAIController::RollDistanceOdds(float Odds, float Distance, float MaxAddedDistance, float DistanceToOpponent, float ZeroDistanceOdds)
{
	float DistanceOdds;
	// Odds% at Distance
	if (DistanceToOpponent < Distance)
		// ZeroDistanceOdds at zero distance; default 50%
		DistanceOdds = FMath::Lerp(Odds, ZeroDistanceOdds, (Distance - DistanceToOpponent) / Distance);
	else
		// 0% at Distance + MaxAddedDistance
		DistanceOdds = FMath::Lerp(Odds, 0.f, (DistanceToOpponent - Distance) / MaxAddedDistance);
	return RollAIOdds(DistanceOdds, DistanceOdds);
}

bool ASSDAIController::RollAbsoluteDistanceOdds(float Odds, float Distance, float MaxAddedDistance)
{
	float DistanceToOpponent = ControlledPawn->GetDistanceTo(ControlledPawn->Opponent);

	return RollDistanceOdds(Odds, Distance, MaxAddedDistance, DistanceToOpponent);
}
bool ASSDAIController::RollXDistanceOdds(float Odds, float Distance, float MaxAddedDistance)
{
	float DistanceX = FMath::Abs(ControlledPawn->GetActorLocation().X - ControlledPawn->Opponent->GetActorLocation().X);

	return RollDistanceOdds(Odds, Distance, MaxAddedDistance, DistanceX);
}
bool ASSDAIController::RollZDistanceOdds(float Odds, float Distance, float MaxAddedDistance)
{
	float DistanceZ = FMath::Abs(ControlledPawn->GetActorLocation().Z - ControlledPawn->Opponent->GetActorLocation().Z);

	return RollDistanceOdds(Odds, Distance, MaxAddedDistance, DistanceZ);
}

bool ASSDAIController::RollJabRange(float JabRangeOdds)
{
	return RollAbsoluteDistanceOdds(JabRangeOdds, JAB_RANGE, 120.f);
}

// Roll chances of transforming, attempt to make less likely to defensive transform if we're good on health, otherwise it's annoying.
bool ASSDAIController::RollTransformOdds(float MinLevelOdds, float MaxLevelOdds, float DesperationOddsScale)
{
	float Min = MinLevelOdds;
	float Max = MaxLevelOdds;

	// More aggressive transformation under pressure.
	if (IsHealthLowerThan(0.3f))
	{
		Min *= DesperationOddsScale;
		Max *= DesperationOddsScale;
	}

	return RollAIOdds(Min, Max);
}

bool ASSDAIController::IsHealthLowerThan(float Threshold)
{
	float HealthPercent = (float)ControlledPawn->Health / (float)ControlledPawn->MaxHealth;
	return HealthPercent < Threshold;
}

void ASSDAIController::PressTransform()
{
	FSSDInputMove CurrentMove = FSSDInputMove();
	CurrentMove.Attack = ESSDInputAttack::SSDIA_Transform;
	CurrentMove.PatternType = ESSDPatternType::SSDPT_None;
	CurrentMove.DirPattern = ESSDDirPattern::SSDDP_None;
	CurrentMove.Direction = ESSDInputDir::SSDID_None;
	CurrentMove.Time = FPlatformTime::Seconds();

	Inputs->MoveBuffer.Insert(CurrentMove, 0);

	//@TODO: Bursts should also work with the MoveBuffer
	ControlledPawn->TransformPressed();
	bPressedTransform = true;
}

int ASSDAIController::RollWeightedIndex(int Num, float Weight)
{
	float Random = FMath::RandRange(0.f, 1.f); //@TODO: use noise instead of rng
	Random = FMath::Pow(Random, Weight); // Bias heavily toward if greater than 1
	float FloatIndex = FMath::Lerp(0, Num + 1, Random);
	return FMath::Min(Num, (int)FloatIndex);
}
