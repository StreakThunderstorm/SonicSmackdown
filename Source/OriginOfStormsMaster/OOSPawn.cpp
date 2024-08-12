// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSPawn.h"

#include "AnimCustomInstanceHelper.h"
#include "Engine.h"
#include "NiagaraComponent.h"
#include "OOSAnimNotify_Hitbox.h"
#include "OOSAnimNotify_Movement.h"
#include "OOSCamera.h"
#include "OOSCinematicScript.h"
#include "OOSGameInstance.h"
#include "OOSGameMode.h"
#include "OOSHitbox.h"
#include "OOSHurtbox.h"
#include "OOSMovementComponent.h"
#include "OOSProjectile.h"
#include "../Plugins/RMAMirrorAnimation/Source/RMAMirrorAnimation/Public/RMAMirrorAnimationMirrorTable.h"
#include "Input/OOSAIController.h"
#include "Input/OOSPlayerController.h"
#include "Particles/ParticleSystemComponent.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "Runtime/Engine/Classes/Animation/AnimMontage.h"
#include "Runtime/Engine/Classes/Animation/AnimSingleNodeInstance.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "SkeletalAnimation/OOSSkeletalMeshComponent.h"

// Sets default values
AOOSPawn::AOOSPawn()
{
	// Set this pawn to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;

	// Capsule Component for player's interaction with the world.
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	Capsule->InitCapsuleSize(42.0f, 96.0f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	Capsule->bEditableWhenInherited = true;
	Capsule->SetHiddenInGame(false); // For physics test, disable once everything's OK.
	RootComponent = Capsule;

	// Add skeletal mesh.
	SkeletalMesh = CreateOptionalDefaultSubobject<UOOSSkeletalMeshComponent>(TEXT("Mesh"));
	SkeletalMesh->bEditableWhenInherited = true;
	SkeletalMesh->SetupAttachment(Capsule);

	// Add skeletal mesh.
	SecondaryMesh = CreateDefaultSubobject<UOOSSkeletalMeshComponent>(TEXT("SecondaryMesh"));
	SecondaryMesh->bEditableWhenInherited = true;
	SecondaryMesh->SetupAttachment(SkeletalMesh);

	// Add transform charge sound.
	ChargeSoundLoop = CreateDefaultSubobject<UAudioComponent>(TEXT("ChargeSound"));
	ChargeSoundLoop->SetupAttachment(Capsule);
	ChargeSoundLoop->SetAutoActivate(false);

	// Add instance of FighterMovementComponent to the pawn.
	MovementComponent = CreateDefaultSubobject<UOOSMovementComponent>(TEXT("OOSMovementComponent"));
	MovementComponent->SetUpdatedComponent(RootComponent);

}

void AOOSPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AOOSPawn::UnPossessed()
{
	Super::UnPossessed();


}

AOOSPlayerController* AOOSPawn::GetPlayerController() const
{
	// If the controller is already a player controller, just return it
	AOOSPlayerController* PlayerController = GetController<AOOSPlayerController>();
	if (PlayerController) return PlayerController;

	// If the AI is using player control
	AOOSAIController* AIController = GetController<AOOSAIController>();
	if (AIController && AIController->Action == EOOSAIAction::OOSAIA_Human)
	{
		return AIController->PlayerController;
	}

	return nullptr;
}

UOOSFighterInputs* AOOSPawn::GetFighterInputs() const
{
	AOOSPlayerController* PlayerController = GetPlayerController();
	if (PlayerController)
	{
		return PlayerController->Inputs;
	}

	AOOSAIController* AIController = GetController<AOOSAIController>();
	if (AIController && AIController->Action == EOOSAIAction::OOSAIA_CPU)
	{
		return AIController->Inputs;
	}

	return nullptr;
}

bool AOOSPawn::SetControllerId(int Id)
{
	AOOSPlayerController* PlayerController = GetPlayerController();
	if (PlayerController)
	{
		PlayerController->GetLocalPlayer()->SetControllerId(Id);
		return true;
	}

	return false;
}

bool AOOSPawn::WantsPlayerController() const
{
	// If the AI is using player control
	AOOSAIController* AIController = GetController<AOOSAIController>();
	if (AIController)
	{
		return AIController->WantsPlayerController();
	}

	return false;
}


// Called when the game starts or when spawned
void AOOSPawn::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World) return;

	GameInstance = Cast<UOOSGameInstance>(UGameplayStatics::GetGameInstance(World));
	
	SkeletalMesh->SetCustomSingleNodeInstance();
	SkeletalMesh->SetMirrorTable(MirrorTable);
	FScriptDelegate Delegate;
	Delegate.BindUFunction(this, TEXT("CallAnimEndEvent"));
	UAnimInstance* SNI = SkeletalMesh->GetAnimInstance();
	if (SNI)
	{
		SNI->OnMontageBlendingOut.Add(Delegate);
		SNI->RootMotionMode = ERootMotionMode::RootMotionFromEverything;
	}

	Health = MaxHealth;
	Regen = MaxHealth;
	Rings = 50;

	RemainingJumps = MaxAirJumps;

	LastDetransform = UGameplayStatics::GetTimeSeconds(GetWorld()) - DETRANSFORM_TIME_PENALTY;
}

// Called every frame
void AOOSPawn::Tick(float DeltaTime)
{
	if (bWantsAnimRefresh)
	{
		SkeletalMesh->TickAnimation(0.f, false);
		SkeletalMesh->RefreshBoneTransforms();
		bWantsAnimRefresh = false;
	}

	Super::Tick(DeltaTime);

	if (!Opponent) return;

	TickHitStop(DeltaTime);

	// Invincibility over
	if (bEndInvincible)
	{
		bEndInvincible = false;
		bInvincible = false;
	}

	if (!MovementComponent || !MoveNetwork) return;

	if (!Opponent->bGrabRotate)
	{
		RefreshMirror();
	}
	else
		SkeletalMesh->SetRelativeRotation(FRotator(Opponent->GrabRot));

	if (!bGrabRotate)
	{
		GrabRot = FRotator(0, MovementComponent->bIsFacingRight ? 90 : -90, 0);
		InterpSpd = 0.f;
	}

	// Perform automatic followup moves
	if (IsHitStopOver())
		PerformChildMove();

	// After attempting child move, to finish automatics on round end
	if (IsKO() || !GetController()) return;

	UOOSFighterInputs* Inputs = GetFighterInputs();
	if (CanOnlyWalk())
	{
		if (Inputs) Inputs->MoveBuffer.Empty();
		return;
	}

	//@TODO: allow inputs while turning
	bool InRecovery = IsStunned() || IsLaunched() || IsFloored() ||
		IsChargingTransform() || IsTransforming() || IsTurning() ||
		IsRecoveringOnGround() || IsRecoveringInAir();

	if (Inputs && !InRecovery)
	{
		// If blocking
		if (IsBlocking() || IsCrouchBlocking())
		{
			// Check if should pushblock
			TryPushBlock(Inputs);
		}
		else
		{
			// Find moves the player input
			TickMoveNetwork(Inputs);
		}
	}

	// Perform combo moves
	if (IsHitStopOver())
	{
		if (!InRecovery)
		{
			if (ReadiedInputNode)
			{
				// Force the next node.
				PerformMove(ReadiedInputNode);
			}
			else
			{
				// If either jump or dash succeed they'll clear the
				// other's readied value on state change, so there's
				// no need for an if-else
				if (ReadiedJumpCancel != EOOSInputDir::OOSID_None)
				{
					Jump();
					ReadiedJumpCancel = EOOSInputDir::OOSID_None;
				}
				if (ReadiedDashCancel != EOOSInputDir::OOSID_None)
				{
					Dash(ReadiedDashCancel);
					ReadiedDashCancel = EOOSInputDir::OOSID_None;
				}
			}
		}
	}

	if (MovementComponent->bFlightMode)
	{
		UWorld* World = GetWorld();
		FTimerManager& TimerManager = World->GetTimerManager();
		if (!TimerManager.IsTimerActive(UnflyTimer) && !FlyingCharacter)
			SetFlightModeEnabled(false);
	}

	// Freeze twitching effect
	if (IsFrozen() && bFreezeTwitch)
	{
		FVector NewLoc = FreezeLocation + (GetActorForwardVector() * FMath::RandRange(-2.5f, 2.5f));
		SetActorLocation(NewLoc);
	}

	// Transform Drain.
	if (IsChargingTransform())
	{
		if (TransformChargeDrainTimer < ChargeDrainInterval)
		{
			TransformChargeDrainTimer += DeltaTime;
		}
		else
		{
			if (Health > ChargeDrainAmount)
			{
				TransformChargeDrainTimer = 0.f;
				Health -= ChargeDrainAmount;
				Transform += ChargeDrainAmount;
			}
			else
			{
				TransformReleased();
			}

			if (Transform >= MAX_TRANSFORM_POINTS)
			{
				Transform = MAX_TRANSFORM_POINTS;
				ChargeSoundLoop->Deactivate();
				StartTransformation();
			}
		}
	}

	if (TransformLockout > 0)
	{
		TransformLockout -= DeltaTime;
		if (TransformLockout <= 0)
			TransformLockout = 0;
	}

	// Defeat.
	bool bIsDefeated = !bDefeated && (Health <= 0);
	if ((bIsDefeated && !GameInstance->bTrainingMode) || (bIsDefeated && !GameInstance->bTrainingAutoRefill))
	{
		SetPawnState(EOOSPawnState::OOSPS_Defeat);
	}

	//Launch Recover
	if (IsLaunched())
	{
		if (AnimationSequence == LaunchUpFar)
		{
			if (MovementComponent->YSpeed <= -4 && IsPawnHigherThan(50.f, 50.f))
			{
				PlayAnim(AirComboBreak);
				SetPawnState(EOOSPawnState::OOSPS_Idle, true);
			}
		}
		else if (AnimationSequence == Spiral)
		{
			if (MovementComponent->YSpeed <= -4 && !bSpiralFallCounted)
			{
				if (!bForceComboExtension && TimesSpiralled >= 1)
				{
					PlayAnim(AirComboBreak);
					SetPawnState(EOOSPawnState::OOSPS_Idle, true);
				}
				else
				{
					if (!bComboExtensionsReset) TimesSpiralled++;
					bSpiralFallCounted = true;
				}
			}
		}
	}

	// Update any blendspace anim asset that may be running.
	UAnimSingleNodeInstance* SNI = SkeletalMesh->GetSingleNodeInstance();
	if (SNI)
	{
		SmoothBlendSpaceInput = FMath::VInterpTo(SmoothBlendSpaceInput, BlendSpaceInput, DeltaTime, 20.f);
		SNI->SetBlendSpacePosition(SmoothBlendSpaceInput);
	}

	// Techs
	AOOSPlayerController* PC = GetPlayerController();
	if (PC)
	{
		FOOSInput LastInput = PC->InputBuffer.Peek();

		/* Throw tech */
		if (Opponent->bTechWindow)
		{
			if (LastInput.Time >= Opponent->TechWindowTime)
			{
				if (LastInput.Attack == EOOSInputAttack::OOSIA_MediumHeavy)
				{
					if (TechParticle)
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TechParticle, GetActorLocation());

					if (TechSound)
						UGameplayStatics::PlaySound2D(GetWorld(), TechSound);

					// We're about to buffer a dash that will come out after we break the throw if we don't flush it.
					PC->ResetDirPatterns();

					OnThrowCollision.Broadcast(true, FVector::ZeroVector); // Destroy the throwing hitbox

					if (Opponent->MovementComponent->bOnGround)
					{
						Unfreeze();
						MovementComponent->bIsFacingRight = MovementComponent->bShouldFaceRight;
						SetPawnState(EOOSPawnState::OOSPS_Idle);
						PlayAnim(AirComboBreak, false, 0.5f);
						MovementComponent->YSpeed = 6.f;
						MovementComponent->bOnGround = false;

						Opponent->SetPawnState(EOOSPawnState::OOSPS_Stun, true);
						Opponent->PlayAnim(Opponent->St_HighHeavy, false, 0.5f);
					}
					else
					{
						Unfreeze();
						MovementComponent->bIsFacingRight = MovementComponent->bShouldFaceRight;
						SetPawnState(EOOSPawnState::OOSPS_Idle);
						PlayAnim(AirComboBreak, false, 0.5f);

						Opponent->SetPawnState(EOOSPawnState::OOSPS_Idle);
						Opponent->PlayAnim(Opponent->AirComboBreak, false, 0.5f);
					}

					// Apply a little push to both pawns.
					float PushAmt = 8.f;
					float Direction = MovementComponent->bShouldFaceRight ? 1.f : -1.f;
					MovementComponent->ApplyPushImpulse(PushAmt * -Direction);
					Opponent->MovementComponent->ApplyPushImpulse(PushAmt * Direction);
				}
				else // If we pressed anything but MH, it's over...
				{
					//Opponent->bTechWindow = false;
				}
			}
		}

		// Wall tech
		if (WallTechWindow > 0.f)
		{
			if ((LastInput.Time >= LastWallBounce) && (LastInput.Attack == EOOSInputAttack::OOSIA_MediumHeavy))
			{
				WallTechWindow = 0.f;
				OnWallHit(true); // Teched wall hit

				if (TechParticle)
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TechParticle, GetActorLocation());

				if (TechSound)
					UGameplayStatics::PlaySound2D(GetWorld(), TechSound);

				// We're about to buffer a dash that will come out after we break the throw if we don't flush it.
				PC->ResetDirPatterns();

				MovementComponent->bIsFacingRight = MovementComponent->bShouldFaceRight;
				SetPawnState(EOOSPawnState::OOSPS_Idle, true);
				PlayAnim(AirComboBreak, false, 0.5f, 0.f, 0.f, true);
				MovementComponent->YSpeed = 8.f;
				MovementComponent->XSpeed = MovementComponent->bShouldFaceRight ? 1.5f : -1.5f;
				MovementComponent->bOnGround = false;
			}
			else
			{
				WallTechWindow -= DeltaTime;
				if (WallTechWindow <= 0.f)
				{
					OnWallHit(false); // Non-teched
				}
			}
		}
	}

	if (MovementComponent->bFlightMode && bIsPerformingMove && GetWorld()->GetTimerManager().IsTimerActive(UnflyTimer))
	{
		UWorld* World = GetWorld();
		//World->GetTimerManager().PauseTimer(UnflyTimer);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Paused TIMER"));
	}
	if (MovementComponent->bFlightMode && !bIsPerformingMove && GetWorld()->GetTimerManager().IsTimerPaused(UnflyTimer))
	{
		UWorld* World = GetWorld();
		World->GetTimerManager().UnPauseTimer(UnflyTimer);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("UNPAUSED TIMER"));
	}

}

void AOOSPawn::TryPushBlock(UOOSFighterInputs* Inputs)
{
	for (int i = 0; i < Inputs->MoveBuffer.Num(); ++i)
	{
		// Ignore if too old, and the rest must be too old too
		if ((FPlatformTime::Seconds() - Inputs->MoveBuffer[i].Time) >= MOVE_BUFFER_TIME)
		{
			break;
		}

		EOOSInputAttack Att = Inputs->MoveBuffer[i].Attack;
		// Check for AA press.
		bool AA = (Att == EOOSInputAttack::OOSIA_LightMedium) || (Att == EOOSInputAttack::OOSIA_MediumHeavy) || (Att == EOOSInputAttack::OOSIA_LightHeavy)
			|| (Att == EOOSInputAttack::OOSIA_LightMediumHeavy);

		// If the pushblock input
		if (AA)
		{
			Inputs->MoveBuffer.RemoveAt(i);
			PushBlock();
			break;
		}
	}
}

void AOOSPawn::TickMoveNetwork(UOOSFighterInputs* Inputs)
{
	uint8 FirstOldEntry = -1;

	for (int i = 0; i < Inputs->MoveBuffer.Num();)
	{
		FOOSInputMove MoveInput = Inputs->MoveBuffer[i];

		// if we come across a buffered move that is too old, we're safe marking its index and breaking, since anything greater is older and will get deleted after.
		if ((FPlatformTime::Seconds() - MoveInput.Time) >= MOVE_BUFFER_TIME)
		{
			FirstOldEntry = i;
			break;
		}

		if (CheckMoveInput(MoveInput, Inputs))
		{
			// If found a move that matched this input, remove the input
			Inputs->MoveBuffer.RemoveAt(i);
		}
		else
		{
			// Else increment to check the next move
			i++;
		}
	}

	if (FirstOldEntry >= 0)
	{
		for (int i = FirstOldEntry; i < Inputs->MoveBuffer.Num(); ++i)
		{
			Inputs->MoveBuffer.RemoveAt(i);
		}
	}
}

bool AOOSPawn::CheckMoveInput(FOOSInputMove& MoveInput, UOOSFighterInputs* Inputs)
{
	// Check for AA press.
	EOOSInputAttack Att = MoveInput.Attack;
	bool AA = (Att == EOOSInputAttack::OOSIA_LightMedium) || (Att == EOOSInputAttack::OOSIA_MediumHeavy) || (Att == EOOSInputAttack::OOSIA_LightHeavy)
		|| (Att == EOOSInputAttack::OOSIA_LightMediumHeavy);

	// Check jump.
	EOOSInputDir BufDir = MoveInput.Direction;
	bool Jmp = (BufDir == EOOSInputDir::OOSID_Up) || (BufDir == EOOSInputDir::OOSID_UpRight) || (BufDir == EOOSInputDir::OOSID_UpLeft);

	UMNNode_Move* NewNode = nullptr;

	// First, look for supers.
	if (FindMatchingMove(MoveNetwork->RootNodes, NewNode, MoveInput, !MovementComponent->bOnGround, true, true))
	{
		SetReadiedMove(NewNode);
		return true;
	}
	else
	{
		// Search our first move.
		if (!bIsPerformingMove || bCanAct)
		{
			if (FindMatchingMove(MoveNetwork->AllNodes, NewNode, MoveInput, !MovementComponent->bOnGround, true))
			{
				SetReadiedMove(NewNode);
				return true;
			}
			// If no move was found, dash if two normals pressed.
			else
			{
				if (AA)
				{
					EOOSInputDir Dir = Inputs->DPadState;
					Dash(Dir);
					return true;
				}
			}
		}
		// We're not in an idle state, get current move's children and search.
		else
		{
			if (bHasLandedAttack || bSpecialCancel)
			{
				if (CurrentNode && FindMatchingMove(CurrentNode->ChildrenNodes, NewNode, MoveInput, !MovementComponent->bOnGround, false))
				{
					if ((CurrentNode->Move.MoveType == EOOSMoveType::OOSMT_Special) && (NewNode->Move.MoveType == EOOSMoveType::OOSMT_Special))
					{
						// We check for the special cancel flag inside the Special into Special scope to avoid cancel specials to be executed as regular combos.
						if (bSpecialCancel)
						{
							// If special cancel requires a hit
							if (!bSpecialCancelRequiresAttackLanded || bHasLandedAttack)
							{
								SetReadiedMove(NewNode);
								return true;
							}
						}
					}
					else
					{
						SetReadiedMove(NewNode);
						return true;
					}
				}
				// If no move was found, see if we're jump/dash canceling
				else if (CurrentNode)
				{
					if (AA && CurrentNode->Move.bDashCancel)
					{
						if (MoveInput.Direction == EOOSInputDir::OOSID_None)
						{
							ReadiedDashCancel = MovementComponent->bShouldFaceRight ? EOOSInputDir::OOSID_Right : EOOSInputDir::OOSID_Left;
						}
						else
						{
							ReadiedDashCancel = MoveInput.Direction;
						}
					}

					if (Jmp && CurrentNode->Move.bJumpCancel)
					{
						ReadiedJumpCancel = MoveInput.Direction;
					}
				}
			}
		}
	}

	// No move matched the input
	return false;
}

void AOOSPawn::SetReadiedMove(UMNNode_Move* NewNode)
{
	// Can't cancel a super or ultra with another one.
	bool bIsPerformingSuperOrUltra = bIsPerformingMove && CurrentNode && ((CurrentNode->Move.MoveType == EOOSMoveType::OOSMT_Super) || (CurrentNode->Move.MoveType == EOOSMoveType::OOSMT_Ultra));
	bool bIsNextMoveSuperOrUltra = (NewNode->Move.MoveType == EOOSMoveType::OOSMT_Super) || (NewNode->Move.MoveType == EOOSMoveType::OOSMT_Ultra);
	if (bIsPerformingSuperOrUltra && bIsNextMoveSuperOrUltra) return;

	if (Opponent->bGrabbed && bIsNextMoveSuperOrUltra) return;
	if (bBlockSuper && bIsNextMoveSuperOrUltra)return;

	if (!ReadiedInputNode || UMNNode_Move::Compare(ReadiedInputNode, NewNode))
		ReadiedInputNode = NewNode;
}

void AOOSPawn::TickHitStop(float DeltaTime)
{
	if (HitStop > 0)
	{
		HitStop -= DeltaTime;
		TryHitStopOver();
	}
}

void AOOSPawn::PerformMove(UMNNode_Move* Node, bool bForce)
{
	if (bCanAct)
	{
		bCanAct = false;
	}	

	if (!Node || !MovementComponent || !Opponent || !Opponent->MovementComponent) return;

	if (Node->Move.bBlockMoveWithActorTag)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			TArray<AActor*> Actors;
			UGameplayStatics::GetAllActorsWithTag(World, Node->Move.BlockActorTag, Actors);
			for (AActor* Actor : Actors)
			{
				if (Actor->GetOwner() != Opponent)
				{
					ResetReadiedMoves();
					return;
				}
			}
		}
	}

	if (!FlyingCharacter && bForce || MovementComponent->bOnGround || MovementComponent->bFlightMode || IsPawnHigherThan(HeightThresholdUp, HeightThresholdDown))
	{
		if ((Node->Move.MoveType == EOOSMoveType::OOSMT_Flight) && !bCanFly)
		{
			SetFlightModeEnabled(false);
			AOOSPlayerController* PC = GetPlayerController();
			if (PC) PC->ResetDirPatterns();
			return;
		}
		
		if (Node->Move.bAir)
		{
			if (Node->Move.MoveType == EOOSMoveType::OOSMT_EX)
			{
				if ((AirEXs < MAX_AIR_EXS))
				{
					AirEXs++;
				}
				else
				{
					return;
				}
			}

			if (Node->Move.MoveType == EOOSMoveType::OOSMT_Special)
			{
				if ((AirSpecials < MAX_AIR_SPECIALS))
				{
					AirSpecials++;
				}
				else
				{
					return;
				}
			}
		}

		// Use horizontal decel if on ground, turn it off if in air to conserve momentum.
		MovementComponent->bUseXDecel = MovementComponent->bOnGround;

		// If the move is not a normal (has directional input previously evaluated with the enemy location in mind), force orientation.
		if (Node->Move.Direction != EOOSMoveDir::OOSMD_None)
		{
			SetFacing(MovementComponent->bShouldFaceRight);
		}

		// if the move has a directional pattern, force idle (we can get locked in any ground state while doing the move, which causes problems when it ends).
		if (!(Node->Move.DirPattern == EOOSDirPattern::OOSDP_None))
		{
			SetPawnState(EOOSPawnState::OOSPS_Idle, true);
		}

		if (MovementComponent->bOnGround)
		{
			SetFacing(MovementComponent->bShouldFaceRight);
		}
		bHasLandedAttack = false;
		bHasMadeContact = false;
		bIsPerformingMove = true;
		HitStop = 0.f;
		ResetReadiedMoves();

		PlayAnim(Node->Move.Animation);
		SuperPts -= Node->Move.RequiredBars * MAX_SUPER_POINTS_PER_LV;
		CurrentNode = Node;
		Opponent->MovementComponent->ApplyPushImpulse(0.f);
	}
}

void AOOSPawn::ResetReadiedMoves()
{
	ReadiedChildNode = nullptr;
	ReadiedInputNode = nullptr;
	ReadiedJumpCancel = EOOSInputDir::OOSID_None;
	ReadiedDashCancel = EOOSInputDir::OOSID_None;
}

void AOOSPawn::PrepareChildMove(UMNNode* Parent)
{
	if (!Parent || !Parent->ChildrenNodes.IsValidIndex(0)) return;

	// Get first child.
	UMNNode* ChildNode = Parent->ChildrenNodes[0];
	ReadiedChildNode = Cast<UMNNode_Move>(ChildNode);

	// If no move for this node, reroute node, recall.
	if (!ReadiedChildNode)
	{
		PrepareChildMove(ChildNode);
	}
}

bool AOOSPawn::PerformChildMove()
{
	if (ReadiedChildNode)
	{
		// Force the next node.
		PerformMove(ReadiedChildNode, true);
		return true;
	}

	return false;
}

// Return value is playback successful or not.
bool AOOSPawn::PlayAnim(UAnimationAsset* AnimationAsset, bool bLoop /*= false*/, float DesiredDuration /*= -1.f*/, float BlendInTime /*= 0.1f*/, float BlendOutTime /*= 0.1f*/, bool bIgnoreMovNotifs /*= false*/, bool bDisableBlending /*= false*/)
{
	if (!AnimationAsset || !MovementComponent || !Opponent || !Opponent->MovementComponent || WasKilled()) return false;

	bIgnoreMovementNotifies = bIgnoreMovNotifs;

	MovementNotify = nullptr;
	bEndInvincible = true;

	// Kill all hitboxes.
	/*TArray<UActorComponent*> HBList = GetComponentsByClass(UOOSHitbox::StaticClass());
	for (int i = 0; i < HBList.Num(); i++)
	{
		HBList[i]->Deactivate();
		HBList[i]->DestroyComponent();
	}*/

	// We need to force gravity and push again, since we may be interrupting a OOSAAnimNotify_Movement that disabled it.
	MovementComponent->bUseGravity = true;
	//MovementComponent->bDoNotPush = false;
	//Opponent->MovementComponent->bDoNotPush = false;

	if (AnimationAsset->IsA(UBlendSpace::StaticClass()))
	{
		bHasLandedAttack = false;
		bHasMadeContact = false;
		SkeletalMesh->PlayAnimationWithMirroring(AnimationAsset, bLoop, bDisableBlending);
	}
	else
	{

		UAnimMontage* AnimMontage;

		if (AnimationAsset->IsA(UAnimMontage::StaticClass()))
		{
			AnimMontage = Cast<UAnimMontage>(AnimationAsset);
			AnimMontage->BlendIn = BlendInTime;
			AnimMontage->BlendOut = BlendOutTime;
		}
		else
		{
			UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(AnimationAsset);
			AnimMontage = UAnimMontage::CreateSlotAnimationAsDynamicMontage(AnimSequence, "Default", BlendInTime, BlendOutTime);
		}

		if (!AnimMontage) return false;

		if (DesiredDuration >= 0.f)
		{
			AnimMontage->RateScale = AnimMontage->GetPlayLength() / DesiredDuration;
		}

		bHasLandedAttack = false;
		bHasMadeContact = false;
		SkeletalMesh->PlayAnimationWithMirroring(AnimMontage, bLoop, bDisableBlending);
	}

	// Store animation sequence for OnAnimationEnd event to return it.
	AnimationSequence = AnimationAsset;

	if (MovementComponent->bOnGround && !MovementComponent->bUseXDecel)
	{
		MovementComponent->XSpeed = 0.f;
	}

	return true;
}

bool AOOSPawn::PlayAnim_NoBlend(UAnimationAsset* AnimationAsset, bool bLoop /*= false*/, float DesiredDuration /*= -1.f*/, float BlendInTime /*= 0.f*/, float BlendOutTime /*= 0.f*/, bool bIgnoreMovNotifs /*= false*/)
{
	return PlayAnim(AnimationAsset, bLoop, DesiredDuration, BlendInTime, BlendOutTime, bIgnoreMovNotifs, true);
}

void AOOSPawn::CallAnimEndEvent(UAnimMontage* Montage, bool Interrupted)
{
	if (!AnimationSequence) return;

	UOOSFighterInputs* Inputs = GetFighterInputs();

	bool bJump = false;
	UAnimationAsset* AS = AnimationSequence;

	if (MovementComponent)
	{
		if (!Interrupted)
		{
			// Clear AnimSequence so that some state getters based on current anim don't get confused.
			AnimationSequence = nullptr;

			EOOSInputDir Dir = EOOSInputDir::OOSID_None;
			if (Inputs) Dir = Inputs->DPadState;

			if (AS == Victory)
			{
				SetPawnState(EOOSPawnState::OOSPS_Idle);
			}
			else if (IsKO())
			{
				if (!IgnoreDeathAnims())
				{
					if (AS == Air_Hit)
					{
						PlayAnim(LaunchDownFaceUp, false, -1, 0.f);
					}
					else if (AS == FloorBounceUpMedium)
					{
						PlayAnim(LyingUp, true);
						bIsLyingDown = true;
					}
					else
					{
						PlayAnim(LyingDown, true);
						bIsLyingDown = true;
					}
				}
			}
			else if (IsTransforming())
			{
				FinishTransformation();
			}
			else if (IsPerformingMove())
			{

				bIsPerformingMove = false;
				if (!MovementComponent->bOnGround && (IsJumping() || IsSuperJumping()))
				{
					if (MovementComponent->XSpeed == 0.f)
					{
						PlayAnim(Fall, true);
					}
					else if (MovementComponent->bIsFacingRight)
						// Fall forward
					{
						MovementComponent->XSpeed > 0.f ? PlayAnim(FallF) : PlayAnim(FallB);
					}
					else
						// Fall back
					{
						MovementComponent->XSpeed > 0.f ? PlayAnim(FallB) : PlayAnim(FallF);
					}
				}
				else
				{
					if (IsCrouching())
					{
						if ((Dir == EOOSInputDir::OOSID_Down || Dir == EOOSInputDir::OOSID_DownRight || Dir == EOOSInputDir::OOSID_DownLeft))
						{
							PlayAnim(CrouchLoop, true);
						}
						else if (!MovementComponent->bOnGround)
						{
							Uncrouch();
							PlayAnim(Fall);
						}
						else
						{
							Uncrouch();
						}
					}
					else SetPawnState(EOOSPawnState::OOSPS_Idle);
				}
				SetFacing(MovementComponent->bShouldFaceRight);
				// Defer next anim frame update to next tick since we're in middle of skeletalmesh transform updates at this point.
				bWantsAnimRefresh = true;
			}
			else if (IsBlocking())
			{
				SetPawnState(EOOSPawnState::OOSPS_Idle);
			}
			else if (IsCrouchBlocking())
			{
				SetPawnState(EOOSPawnState::OOSPS_Crouch);
				if ((Dir == EOOSInputDir::OOSID_Down || Dir == EOOSInputDir::OOSID_DownRight || Dir == EOOSInputDir::OOSID_DownLeft))
				{
					PlayAnim(CrouchLoop, true);
				}
				else
				{
					Uncrouch();
				}
			}
			else if (AS == CrouchTurn)
			{
				SetPawnState(EOOSPawnState::OOSPS_Crouch);
				if ((Dir == EOOSInputDir::OOSID_Down || Dir == EOOSInputDir::OOSID_DownRight || Dir == EOOSInputDir::OOSID_DownLeft))
				{
					PlayAnim(CrouchLoop, true);
				}
				else
				{
					Uncrouch();
				}
			}
			// Dash finished.
			else if ((AS == DashFW) || (AS == DashBW))
			{
				SetPawnState(EOOSPawnState::OOSPS_Idle);
			}			
			// Land finished.
			else if (IsLanding() && IsFacingOpponent())
			{
				SetPawnState(EOOSPawnState::OOSPS_Idle);
			}
			else if (IsTurning())
			{
				SetPawnState(EOOSPawnState::OOSPS_Idle);
			}
			else if ((AS == LaunchUpFar) || (AS == LaunchUpShort) || (AS == Air_Hit))
			{
				if (IsPawnHigherThan(50.f, 50.f))
				{
					PlayAnim(AirComboBreak);
					SetPawnState(EOOSPawnState::OOSPS_Idle, true);
				}
				else
				{
					if (MovementComponent->YSpeed <= 4.f)
						PlayAnim(Air_Hit);									
				}
			}
			else if (AS == AirComboBreak)
			{
				{
					if (FlyingCharacter == true)
					{
						SetFlightModeEnabled(true);
						SetPawnState(EOOSPawnState::OOSPS_Idle);
					}
					else
					{
						PlayAnim(Fall);
						SetPawnState(EOOSPawnState::OOSPS_Idle, true);
					}
				}
			}
			else if (AS == LaunchBackShort)
			{
			PlayAnim(AirComboBreak);
			SetPawnState(EOOSPawnState::OOSPS_Idle, true);
			}
			else if (AS == FloorBounceUpMedium)
			{
				WakeUpRecovery(false);
			}
			else if (AS == FloorBounceDownMedium)
			{
				WakeUpRecovery(true);
			}
			else if (AS == FloorBounceDownShort)
			{
				WakeUpRecovery(true);
			}
			else if (AS == LyingDown)
			{
				PlayAnim(LyingDownStand);
			}
			else if (AS == FloorBounceUpShort)
			{
				WakeUpRecovery(false);
			}
			else if (AS == LyingUp)
			{
				PlayAnim(LyingUpStand);
			}
			else if (AS == Crumble)
			{
				PlayAnim(LyingDownStand);
				SetFacing(MovementComponent->bShouldFaceRight);
				SetPawnState(EOOSPawnState::OOSPS_Idle, true);
			}
			else if ((AS == LyingDownStand) || (AS == LyingUpStand))
			{
				SetPawnState(EOOSPawnState::OOSPS_Idle);
			}
			else if (AS == IdleToCrouch)
			{
				PlayAnim(CrouchLoop, true);
			}
			else if (AS == CrouchToIdle)
			{
				SetPawnState(EOOSPawnState::OOSPS_Idle);
			}
			else if ((AS == SingleJump) && !MovementComponent->bOnGround)
			{
				PlayAnim(Fall, true);
			}
			else if ((AS == SingleJumpF) && !MovementComponent->bOnGround)
			{
				PlayAnim(FallF, true);
			}
			else if ((AS == SingleJumpB) && !MovementComponent->bOnGround)
			{
				PlayAnim(FallB, true);
			}
			else if ((AS == DoubleJump) && !MovementComponent->bOnGround)
			{
				PlayAnim(DoubleJumpFall, true);
			}
			else if ((AS == DoubleJumpF) && !MovementComponent->bOnGround)
			{
				PlayAnim(DoubleJumpFallF, true);
			}
			else if ((AS == DoubleJumpB) && !MovementComponent->bOnGround)
			{
				PlayAnim(DoubleJumpFallB, true);
			}
			else if ((AS == SupJump) && !MovementComponent->bOnGround)
			{
				PlayAnim(SupJumpFall, true);
			}
			else if (AS == QuickRecoverB)
			{
				SetPawnState(EOOSPawnState::OOSPS_Idle);
			}
			else
			{
				SetPawnState(EOOSPawnState::OOSPS_Idle);
			}
		}
		else
		{
			if (CinematicScript)
			{
				CinematicScript->OnCinematicInterrupted();
			}
		}
	}

	if (bJump) Jump(true);
	if (Interrupted) return;
	OnAnimationEnd(AS);
}

EOOSMoveDir AOOSPawn::ResolveDirection(EOOSInputDir InputDirection, bool bUseCurrentFacing)
{
	if (!MovementComponent) return EOOSMoveDir::OOSMD_None;

	bool bIFR = MovementComponent->bShouldFaceRight;
	if (bUseCurrentFacing) bIFR = MovementComponent->bIsFacingRight;

	switch (InputDirection)
	{
	case EOOSInputDir::OOSID_None: default:
		return EOOSMoveDir::OOSMD_None;
		break;

	case EOOSInputDir::OOSID_Right:
		return bIFR ? EOOSMoveDir::OOSMD_Forward : EOOSMoveDir::OOSMD_Back;
		break;

	case EOOSInputDir::OOSID_UpRight:
		return bIFR ? EOOSMoveDir::OOSMD_UpForward : EOOSMoveDir::OOSMD_UpBack;
		break;

	case EOOSInputDir::OOSID_Up:
		return EOOSMoveDir::OOSMD_Up;
		break;

	case EOOSInputDir::OOSID_UpLeft:
		return bIFR ? EOOSMoveDir::OOSMD_UpBack : EOOSMoveDir::OOSMD_UpForward;
		break;

	case EOOSInputDir::OOSID_Left:
		return bIFR ? EOOSMoveDir::OOSMD_Back : EOOSMoveDir::OOSMD_Forward;
		break;

	case EOOSInputDir::OOSID_DownLeft:
		return bIFR ? EOOSMoveDir::OOSMD_DownBack : EOOSMoveDir::OOSMD_DownForward;
		break;

	case EOOSInputDir::OOSID_Down:
		return EOOSMoveDir::OOSMD_Down;
		break;

	case EOOSInputDir::OOSID_DownRight:
		return bIFR ? EOOSMoveDir::OOSMD_DownForward : EOOSMoveDir::OOSMD_DownBack;
		break;
	}
}

bool AOOSPawn::ResolveAttack(EOOSInputAttack Moveset, EOOSInputAttack Buffered)
{
	switch (Moveset)
	{
	case EOOSInputAttack::OOSIA_None:
		return false;
		break;

	case EOOSInputAttack::OOSIA_A:
		if ((Buffered != EOOSInputAttack::OOSIA_None) && (Buffered != EOOSInputAttack::OOSIA_Special)) return true;
		else return false;
		break;

	case EOOSInputAttack::OOSIA_AA:
		if ((Buffered == EOOSInputAttack::OOSIA_LightMedium) || (Buffered == EOOSInputAttack::OOSIA_MediumHeavy) || (Buffered == EOOSInputAttack::OOSIA_LightHeavy)
			|| (Buffered == EOOSInputAttack::OOSIA_LightMediumSpecial) || (Buffered == EOOSInputAttack::OOSIA_MediumHeavySpecial)
			|| (Buffered == EOOSInputAttack::OOSIA_LightHeavySpecial) || (Buffered == EOOSInputAttack::OOSIA_LightMediumHeavySpecial))
			return true;
		else return false;
		break;

	default:
		if (((uint8)Buffered & 8) && (Moveset == EOOSInputAttack::OOSIA_Special)) return true;
		else
			return Buffered == Moveset;
		break;
	}
}

bool AOOSPawn::FindMatchingMove(const TArray<UMNNode*> &InputMoves, UMNNode_Move* &OutputMove, const FOOSInputMove BufferedMove, bool bAir, bool bFirstMove, bool bSupersOnly)
{
	//@TODO: Would have to refactor this method if it runs on multiple threads at once
	static bool Found = false;
	bool FoundExactDirNormal = false;

	for (int i = 0; i < InputMoves.Num(); i++)
	{
		UMNNode_Move* Node = Cast<UMNNode_Move>(InputMoves[i]);
		if (Node)
		{
			FOOSMove Move = Node->Move;

			if (!bSupersOnly || (Move.MoveType == EOOSMoveType::OOSMT_Super) || (Move.MoveType == EOOSMoveType::OOSMT_Ultra))
			{
				if (!bFirstMove || !Move.bCantStartSeries)
				{
					if (bAir == Move.bAir)
					{
						// Check for just attacks ignoring buffered directions
						if ((Move.PatternType == EOOSPatternType::OOSPT_None) && (Move.DirPattern == EOOSDirPattern::OOSDP_None)
							&& (Move.Direction == EOOSMoveDir::OOSMD_None) && ResolveAttack(Move.Attack, BufferedMove.Attack)
							&& (GetSuperLevel() >= Move.RequiredBars))
						{
							// We may have found a move using the buffered LMHS already, so skip searching for a normal if so, because we might be overwriting a special or super.
							// Also quick and embarrassing fix to bold cancels.
							if (!Found || (Move.Attack == EOOSInputAttack::OOSIA_LightSpecial))
							{
								Found = true;
								OutputMove = Node;
							}
						}
						else
						{
							// For non-directional crouch moves, DownFW and DownBW are valid as well.
							// Same thing for jump cancel buffers: Up admits UpFW and UpBW.
							bool Dir;
							EOOSMoveDir ResolvedDir = ResolveDirection(BufferedMove.Direction);
							Dir = (Move.Direction == ResolvedDir);
							bool ExactDirNormal = false;
							if (Dir)
							{
								ExactDirNormal = (Move.DirPattern == EOOSDirPattern::OOSDP_None) && (Move.Direction != EOOSMoveDir::OOSMD_None);
							}
							else if (Move.Direction == EOOSMoveDir::OOSMD_Down)
							{
								Dir = (ResolvedDir == EOOSMoveDir::OOSMD_DownBack) || (ResolvedDir == EOOSMoveDir::OOSMD_DownForward);
							}
							else if (Move.Direction == EOOSMoveDir::OOSMD_Up)
							{
								Dir = (ResolvedDir == EOOSMoveDir::OOSMD_UpBack) || (ResolvedDir == EOOSMoveDir::OOSMD_UpForward);
							}

							// Now check for exact matches.
							if ((Move.PatternType == BufferedMove.PatternType) && (Move.DirPattern == BufferedMove.DirPattern)
								&& Dir && ResolveAttack(Move.Attack, BufferedMove.Attack)
								&& (GetSuperLevel() >= Move.RequiredBars))
							{
								bool bIsThrowMove = Move.MoveType == EOOSMoveType::OOSMT_Throw;
								if ((!FoundExactDirNormal && !bIsThrowMove) || (bIsThrowMove && bFirstMove && CanPerformThrow()))
								{
									Found = true;
									FoundExactDirNormal = ExactDirNormal;
									OutputMove = Node;
								}
							}
						}
					}
				}
			}
		}
		// We found a reroute node. Look into its children only if not searching for initial move.
		else if (!bFirstMove)
		{
			if (FindMatchingMove(InputMoves[i]->ChildrenNodes, OutputMove, BufferedMove, bAir, bFirstMove, bSupersOnly))
			{
				Found = true;
			}
		}
	}

	bool Out = Found;
	Found = false;
	return Out;
}

bool AOOSPawn::IsPawnHigherThan(float HeightUp, float HeightDown)
{
	float H = (MovementComponent->YSpeed > 0.f) ? HeightUp : HeightDown;
	return MovementComponent && (MovementComponent->GetDistanceToFloor() > H);
}

void AOOSPawn::SetActorToFollow(AOOSPawn* ActorToFollow)
{
	if (!Camera) return;
	Camera->SetActorToFollow(ActorToFollow);
}

FVector AOOSPawn::GetStageOrigin() const
{
	if (!MovementComponent) return FVector::ZeroVector;

	return MovementComponent->StageLocation;
}

void AOOSPawn::SetFlightModeEnabled(bool bEnabled)
{
	if (!MovementComponent) return;
	if (bEnabled && !bCanFly) return;

	UWorld* World = GetWorld();
	if (!World) return;
	if(!FlyingCharacter)
	World->GetTimerManager().SetTimer(UnflyTimer, this, &AOOSPawn::UnFly, 3.f);

	MovementComponent->bFlightMode = bEnabled;
	if (bEnabled)
	{
		MovementComponent->bOnGround = false;
		if (!FlyingCharacter)
		{
			bCanFly = false;
		}
	}
	else 
	{
		//MovementComponent->bUseGravity = true;
	}
	if(bEnabled || IsNeutral()) SetPawnState(EOOSPawnState::OOSPS_Idle); // This will play the flight blend space anim for us when enabled.
}

void AOOSPawn::UnFly()
{
	if (!FlyingCharacter)
	{
		if (!MovementComponent) return;
		UWorld* World = GetWorld();
		if (!World) return;
		FTimerManager& TimerManager = World->GetTimerManager();
		if (MovementComponent->bFlightMode && !TimerManager.IsTimerPaused(UnflyTimer))
		{
			if (bIsPerformingSuper)
			{
				return;
			}
			else if (bIsPerformingMove)
			{
				return;
			}
			else
			{
				MovementComponent->bFlightMode = false;
				PawnState = EOOSPawnState::OOSPS_Idle;
				PlayAnim(Fall);
			}			
		}
		TimerManager.ClearTimer(UnflyTimer);
	}

}

void AOOSPawn::SetFacing(bool FacingRight)
{
	MovementComponent->bIsFacingRight = FacingRight;

	RefreshMirror();
}

void AOOSPawn::RefreshMirror()
{
	if (!MovementComponent || !SkeletalMesh)
		return;
	if (bMirrored)
	{
		// Mirror on the right side
		SkeletalMesh->SetFacing(MovementComponent->bIsFacingRight);
		FRotator Rot = FRotator(0, MovementComponent->bIsFacingRight ? -90 : 90, 0);
		FRotator DeadRot = FRotator::ZeroRotator;

		SkeletalMesh->SetRelativeRotation(WasKilled() ? DeadRot : Rot);
	}
	else
	{
		FVector Scale = FVector(MovementComponent->bIsFacingRight ? 1 : -1, 1, 1);
		FRotator Rot = FRotator(0, MovementComponent->bIsFacingRight ? -90 : 90, 0);
		FRotator DeadRot = FRotator::ZeroRotator;
		SkeletalMesh->SetRelativeScale3D(Scale);
		SkeletalMesh->SetRelativeRotation(WasKilled() ? DeadRot : Rot);
	}
}

void AOOSPawn::BecomeInvincible()
{
	bInvincible = true;
	bEndInvincible = false;
}
void AOOSPawn::EndInvincible()
{
	bEndInvincible = true;
}

UOOSHurtbox* AOOSPawn::AddHurtbox(FName BoneSocket, float CapsuleRadius, float CapsuleHalfHeight)
{
	UOOSHurtbox* NewHB = NewObject<UOOSHurtbox>(this);
	if (!NewHB || !SkeletalMesh) return nullptr;

	NewHB->SetCollisionObjectType(HITBOX_OBJ);
	NewHB->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	NewHB->SetCollisionResponseToChannel(HITBOX_OBJ, ECollisionResponse::ECR_Overlap);
	NewHB->SetCollisionResponseToChannel(PROJECTILE_OBJ, ECollisionResponse::ECR_Overlap);
	NewHB->AttachToComponent(SkeletalMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false), BoneSocket);
	NewHB->PrimaryComponentTick.bCanEverTick = true;
	NewHB->SetComponentTickEnabled(true);

	NewHB->SetCapsuleRadius(CapsuleRadius);
	NewHB->SetCapsuleHalfHeight(CapsuleHalfHeight);

	NewHB->SetHiddenInGame(!bDebugHurtboxes);
	NewHB->bDebug = bDebugHurtboxes;
	NewHB->DebugColor = HurtboxDebugColor;
	NewHB->DebugThickness = HBDebugThickness;

	NewHB->RegisterComponent();

	return NewHB;
}

class UOOSHitbox* AOOSPawn::AddHitbox(
	USkeletalMeshComponent* InMesh,
	TSubclassOf<UOOSHitbox> HitboxType,
	FVector Offset,
	FRotator Rotation,
	float HalfHeight,
	float Radius,
	bool bGrab,
	bool bForceGrab,
	FName GrabSock,
	UParticleSystem* HitParticle,
	USoundBase* HitSound,
	bool bBurst,
	int Damage,
	float HitStun,
	float SelfHitLag,
	float OpponentHitLag,
	float BlockStun,
	float PushFactor,
	float Frz,
	bool FrzTw,
	bool bOTG,
	bool bOH,
	bool bAutoChild,
	bool bDontCancel,
	bool bUnblockable,
	bool bAntiArmor,
	bool bAntiProjectile,
	EOOSHitHeight HitHeight,
	EOOSInputAttack Attack,
	EOOSLaunchType Launch,
	bool bInForceComboExtension,
	bool bResetComboExtensions,
	EOOSDirectionMode Direction,
	FVector2D LaunchSpd,
	bool bForceTryPostLaunchJump,
	FName SocketName
)
{
	UOOSHitbox* NewHB = NewObject<UOOSHitbox>(this, HitboxType);
	if (!NewHB || !SkeletalMesh) return nullptr;

	USkeletalMeshComponent* UsedMesh = InMesh ? InMesh : SkeletalMesh;

	FName FinalSocketName = SocketName;

	if (bMirrored)
	{
		if (!MovementComponent->bIsFacingRight && MirrorTable && SocketName != NAME_None)
		{
			FString S_SocketName = SocketName.ToString();

			int32 Count = S_SocketName.ReplaceInline(TEXT("RSocket"), TEXT("LSocket"), ESearchCase::IgnoreCase);
			if (Count <= 0)
			{
				Count = S_SocketName.ReplaceInline(TEXT("LSocket"), TEXT("RSocket"), ESearchCase::IgnoreCase);
			}

			if (Count > 0)
			{
				FinalSocketName = FName(S_SocketName);
			}
		}
	}	

	NewHB->AttachToComponent(UsedMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false), FinalSocketName);
	NewHB->PrimaryComponentTick.bCanEverTick = true;
	NewHB->SetComponentTickEnabled(true);

	NewHB->Initialize
	(
		HalfHeight,
		Radius,
		bGrab,
		bForceGrab,
		GrabSock,
		HitParticle,
		HitSound,
		bBurst,
		Damage,
		HitStun,
		SelfHitLag,
		OpponentHitLag,
		BlockStun,
		PushFactor,
		Frz,
		FrzTw,
		bOTG,
		bOH,
		bAutoChild,
		bDontCancel,
		bUnblockable,
		bAntiArmor,
		bAntiProjectile,
		HitHeight,
		Attack,
		Launch,
		bInForceComboExtension,
		bResetComboExtensions,
		Direction,
		LaunchSpd,
		bForceTryPostLaunchJump
	);

	NewHB->SetHiddenInGame(!bDebugHitboxes);
	NewHB->bDebug = bDebugHitboxes;
	NewHB->DebugColor = HitboxDebugColor;
	NewHB->DebugThickness = HBDebugThickness;

	NewHB->SetRelativeLocation(Offset);
	NewHB->SetRelativeRotation(Rotation);

	NewHB->RegisterComponent();

	return NewHB;
}

class AOOSProjectile* AOOSPawn::AddProjectile(
	USkeletalMeshComponent* InMesh,
	TSubclassOf<AOOSProjectile> Class,
	int NumberOfHits,
	float HitTime,
	float Lifetime,
	FVector2D Speed,
	FVector Offset,
	FRotator Rotation,
	bool bGrab,
	bool bForceGrab,
	FName GrabSock,
	UParticleSystem* HitParticle,
	USoundBase* HitSound,
	UParticleSystem* DeathParticle,
	USoundBase* DeathSound,
	int Damage,
	float HitStun,
	float SelfHitLag,
	float OpponentHitLag,
	float BlockStun,
	float PushFactor,
	float Frz,
	bool FrzTw,
	bool bOTG,
	bool bOH,
	bool bAutoChild,
	bool bDontCancel,
	bool bUnblockable,
	bool bAntiArmor,
	bool bAntiProjectile,
	EOOSHitHeight HitHeight,
	EOOSInputAttack Attack,
	EOOSLaunchType Launch,
	bool bInForceComboExtension,
	bool bResetComboExtensions,
	EOOSDirectionMode Direction,
	FVector2D LaunchSpd,
	bool bForceTryPostLaunchJump
)
{
	UWorld* World = GetWorld();
	if (!World || !SkeletalMesh) return nullptr;

	USkeletalMeshComponent* UsedMesh = InMesh ? InMesh : SkeletalMesh;

	Offset.X = MovementComponent->bIsFacingRight ? Offset.X : -Offset.X;
	FTransform T = FTransform(Rotation, UsedMesh->GetComponentLocation() + Offset, FVector(1.f, 1.f, 1.f));
	AOOSProjectile* NewProj = World->SpawnActorDeferred<AOOSProjectile>(Class, T, this, this);
	if (!NewProj) return nullptr;

	NewProj->PawnOwner = this;
	NewProj->Initialize
	(
		NumberOfHits,
		HitTime,
		Lifetime,
		Speed,
		GetActorForwardVector(),
		bGrab,
		bForceGrab,
		GrabSock,
		HitParticle,
		HitSound,
		DeathParticle,
		DeathSound,
		false,
		Damage,
		HitStun,
		SelfHitLag,
		OpponentHitLag,
		BlockStun,
		PushFactor,
		Frz,
		FrzTw,
		bOTG,
		bOH,
		bAutoChild,
		bDontCancel,
		bUnblockable,
		bAntiArmor,
		bAntiProjectile,
		HitHeight,
		Attack,
		Launch,
		bInForceComboExtension,
		bResetComboExtensions,
		Direction,
		LaunchSpd,
		bForceTryPostLaunchJump
	);
	NewProj->OGSpeed = Speed;
	NewProj->Hitbox->SetHiddenInGame(!bDebugHitboxes);
	NewProj->Hitbox->bDebug = bDebugHitboxes;
	NewProj->Hitbox->DebugColor = HitboxDebugColor;
	NewProj->Hitbox->DebugThickness = 0.5f;
	NewProj->InitLocation = UsedMesh->GetComponentLocation() + Offset;
	if (NewProj->bIsBeam)
	{
		if (NewProj->bLockBeam)	NewProj->InitRelative = Offset;
		NewProj->BeamBox->SetHiddenInGame(!bDebugHitboxes);
		NewProj->BeamBox->bDebug = bDebugHitboxes;
		NewProj->BeamBox->DebugColor = HitboxDebugColor;
		NewProj->BeamBox->DebugThickness = 0.5f;
		NewProj->Hitbox->bEnabled = false;
	}
	else
	{
		NewProj->BeamBox->SetHiddenInGame(true);
	}


	NewProj->FinishSpawning(T);

	return NewProj;
}

void AOOSPawn::HitboxVisibility(bool Visible)
{
	TArray<USceneComponent*> InChildren = SkeletalMesh->GetAttachChildren();

	for (USceneComponent* Component : InChildren)
	{
		UOOSHurtbox* HurtBox = Cast<UOOSHurtbox>(Component);
		if (HurtBox)
			HurtBox->SetHiddenInGame(!Visible);
	}

	bDebugHitboxes = Visible;
	bDebugHurtboxes = Visible;
}

void AOOSPawn::OnLanded_Implementation()
{
	if (!MovementComponent || IsTransforming()) return;
	if (PawnState == EOOSPawnState::OOSPS_Floored)
	{
		MovementComponent->XSpeed = 0.f;
		return;
	}
	
	UOOSFighterInputs* Inputs = GetFighterInputs();

	// Reset  air flight/jumps/EXs/specials limiters.
	bCanFly = true;
	RemainingJumps = MaxAirJumps;
	AirEXs = 0;
	AirSpecials = 0;

	if (IsLaunched() || IsKO())
	{
		if (IsLaunched())
		{
			FVector ImpactPoint = GetActorLocation();
			ImpactPoint.Z -= Capsule->GetScaledCapsuleHalfHeight();
			OnThrowCollision.Broadcast(false, ImpactPoint);
		}

		if (AnimationSequence)
		{
			UAnimationAsset* CurrentAnim = AnimationSequence;
			if (IsKO())
			{
				if (!IgnoreDeathAnims())
				{
					if ((CurrentAnim == LaunchDownFaceDown) || (CurrentAnim == FloorBounceDownFar) || (CurrentAnim == HitWall))
					{
						PlayAnim(FloorBounceDownMedium);
					}
					else
					{
						PlayAnim(FloorBounceUpMedium);
					}
				}
			}
			else
			{
				EOOSInputDir Dir = EOOSInputDir::OOSID_None;
				if (Inputs) Dir = Inputs->DPadState;
				if (CurrentAnim == LaunchDownFaceDown)
				{					
					if (!bForceComboExtension && TimesLaunched >= 1)
					{
						WakeUpRecovery(true, true);
					}
					else if (bForceComboExtension && TimesLaunched >= 2)
					{
						WakeUpRecovery(true, true);
					}
					else
					{
						SetPawnState(EOOSPawnState::OOSPS_Launch);
						PlayAnim(FloorBounceDownFar);
						if (!bComboExtensionsReset) TimesLaunched++;
					}					
				}

				else if (CurrentAnim == FloorBounceDownFar)
				{
					if (!bForceComboExtension && TimesLaunched >= 2)
					{
						WakeUpRecovery(true, true);
					}
					else if (bForceComboExtension && TimesLaunched >= 3)
					{
						WakeUpRecovery(true, true);
					}
					else
					{
						SetPawnState(EOOSPawnState::OOSPS_Floored);
						PlayAnim(FloorBounceDownMedium);
					}					
				}

				else if (CurrentAnim == LaunchDownFaceUp)
				{					
					if (!bForceComboExtension && TimesLaunched >= 1)
					{
						WakeUpRecovery(true, true);
					}
					else if (bForceComboExtension && TimesLaunched >= 2)
					{
						WakeUpRecovery(true, true);
					}
					else
					{
						SetPawnState(EOOSPawnState::OOSPS_Launch);
						PlayAnim(FloorBounceUpFar);
						if (!bComboExtensionsReset) TimesLaunched++;
					}
				}

				else if (CurrentAnim == LaunchUpShort)
				{
					if (!bForceComboExtension && TimesFloored >= 2)
					{
						WakeUpRecovery(true, true);
					}
					else if (bForceComboExtension && TimesFloored >= 3)
					{
						WakeUpRecovery(true, true);
					}
					else
					{
						SetPawnState(EOOSPawnState::OOSPS_Floored);
						PlayAnim(FloorBounceUpMedium);
						if (!bComboExtensionsReset) TimesFloored++;
					}
				}

				else if (CurrentAnim == Air_Hit)
				{
					if (!bForceComboExtension && TimesFloored >= 2)
					{
						WakeUpRecovery(true, true);
					}
					else if (bForceComboExtension && TimesFloored >= 3)
					{
						WakeUpRecovery(true, true);
					}
					else
					{
						SetPawnState(EOOSPawnState::OOSPS_Floored);
						PlayAnim(FloorBounceUpMedium);
						if (!bComboExtensionsReset) TimesFloored++;
					}
				}

				else if (CurrentAnim == FloorBounceUpFar)
				{
					if (!bForceComboExtension && TimesLaunched >= 2)
					{
						WakeUpRecovery(true, true);
					}
					else if (bForceComboExtension && TimesLaunched >= 3)
					{
						WakeUpRecovery(true, true);
					}
					else
					{
						SetPawnState(EOOSPawnState::OOSPS_Floored);
						PlayAnim(FloorBounceUpMedium);
						MovementComponent->XSpeed = 0.f;
					}
				}

				else if (CurrentAnim == LaunchUpFar)
				{
					if (!bForceComboExtension && TimesLaunched >= 1)
					{
						WakeUpRecovery(true, true);
					}
					else if (bForceComboExtension && TimesLaunched >= 2)
					{
						WakeUpRecovery(true, true);
					}
					else
					{
						SetPawnState(EOOSPawnState::OOSPS_Floored);
						PlayAnim(FloorBounceUpMedium);
						if (!bComboExtensionsReset) TimesLaunched++;
					}
				}

				else if (CurrentAnim == Spiral)
				{
					switch (ResolveDirection(Dir))
					{
					case EOOSMoveDir::OOSMD_Forward: case EOOSMoveDir::OOSMD_DownForward: case EOOSMoveDir::OOSMD_UpForward:
						PlayAnim(QuickRecoverF);
						SetPawnState(EOOSPawnState::OOSPS_Idle, true);
						break;

					default:
						PlayAnim(QuickRecoverB);
						SetPawnState(EOOSPawnState::OOSPS_Idle, true);
						break;
					}
				}

				else if (CurrentAnim == Crumble)
				{
					SetPawnState(EOOSPawnState::OOSPS_Floored);
				}

				else if (CurrentAnim == LaunchBackShort)
				{
					PlayAnim(QuickRecoverB);
					SetPawnState(EOOSPawnState::OOSPS_Idle, true);
				}

				else if (CurrentAnim == LaunchBackFar)
				{	
					if (MovementComponent->YSpeed < 0.1f)
					{
						if (!bForceComboExtension && TimesLaunched >= 1)
						{
							WakeUpRecovery(true, true);
						}
						else if (bForceComboExtension && TimesLaunched >= 2)
						{
							WakeUpRecovery(true, true);
						}
						else
						{
							SetPawnState(EOOSPawnState::OOSPS_Launch);
							PlayAnim(FloorBounceUpFar);
							if (!bComboExtensionsReset) TimesLaunched++;
						}
					}
					else
						return;
				}

				else if (CurrentAnim == HitWall)
				{
					PlayAnim(FloorBounceDownMedium);
					SetPawnState(EOOSPawnState::OOSPS_Floored);
				}
				else if (IsTransforming())
				{
					return;
				}				
			}
		}
	}

	else if (IsStunned())
	{
		if (!bForceComboExtension && TimesFloored >= 2)
		{
			SetFlightModeEnabled(false);
			WakeUpRecovery(false, true);
		}
		else if (bForceComboExtension && TimesFloored >= 3)
		{
			SetFlightModeEnabled(false);
			WakeUpRecovery(false, true);
		}
		else
		{
			SetFlightModeEnabled(false);
			SetPawnState(EOOSPawnState::OOSPS_Floored);
			PlayAnim(FloorBounceUpMedium);				
			if (!bComboExtensionsReset) TimesFloored++;
		}
	}
	else if (IsChargingTransform())
	{
		PlayAnim(Charge_Ground, true, -1.f, 0.1f, 0.1f);
	}
	else if (IsTransforming())
	{
		return;
	}
	else if (IsPerformingMove() && CurrentNode && CurrentNode->Move.bDontBreakOnLanding)
	{
		// Skip breaking the combo/move.
		return;
	}
	else if (IsPerformingMove() && CurrentNode && CurrentNode->Move.LandingAnim)
	{
		// Play move landing animation but reset readied moves.
		PlayAnim(CurrentNode->Move.LandingAnim);
		ResetReadiedMoves();
		return;
	}
	else
	{
		// Let the player rapidly jump if holding up upon landing.
		if (Inputs && !IsPerformingMove() && !IsRecoveringOnGround() && !IsRecoveringInAir() &&
			((Inputs->DPadState == EOOSInputDir::OOSID_Up) || (Inputs->DPadState == EOOSInputDir::OOSID_UpRight) || (Inputs->DPadState == EOOSInputDir::OOSID_UpLeft)))
		{
			// Correct orientation.
			SetFacing(MovementComponent->bShouldFaceRight);

			// Set a timer for the actual jump, we want to play a couple landing frames for cosmetic reasons.
			UWorld* World = GetWorld();
			if (World)
			{
				FTimerHandle AutoJump;
				World->GetTimerManager().SetTimer(AutoJump, this, &AOOSPawn::AutoJump, 0.05f);
			}
		}

		if (IsFacingOpponent())
		{
			PlayAnim(Land);
			MovementComponent->XSpeed = 0.f;
		}
		else
		{
			SetFacing(MovementComponent->bShouldFaceRight);
			PlayAnim(LandTurn);
			MovementComponent->XSpeed = 0.f;
			// Show next anim frame now, so canceled attack anims don't flip around.
			SkeletalMesh->TickAnimation(0.f, false);
			//SkeletalMesh->RefreshBoneTransforms();
		}
		SetPawnState(EOOSPawnState::OOSPS_Idle, true);

	}

	// Break the combo.
	bIsPerformingMove = false;
}

void AOOSPawn::WakeUpRecovery(bool bFaceDown, bool bForwardRollOnNeutral)
{
	UOOSFighterInputs* Inputs = GetFighterInputs();

	EOOSInputDir Dir = EOOSInputDir::OOSID_None;
	if (Inputs) Dir = Inputs->DPadState;
	EOOSMoveDir InputDir = ResolveDirection(Dir, true);

	if(MovementComponent->bOnGround)
	{
		if (bFaceDown)
		{
			switch (InputDir)
			{
				// Forward roll
			case EOOSMoveDir::OOSMD_Forward: case EOOSMoveDir::OOSMD_DownForward: case EOOSMoveDir::OOSMD_UpForward:
				PlayAnim(LyingDownRollF);
				break;

				// Backward roll
			case EOOSMoveDir::OOSMD_Back: case EOOSMoveDir::OOSMD_DownBack: case EOOSMoveDir::OOSMD_UpBack:
				PlayAnim(LyingDownRollB);
				break;

			default:
				// Forced forward roll
				if (bForwardRollOnNeutral)
				{
					PlayAnim(LyingDownRollF);
				}
				// Neutral getup
				else
				{
					PlayAnim(LyingDownStand);
					SetFacing(MovementComponent->bShouldFaceRight);
				}
				break;
			}
		}
		else
		{
			switch (InputDir)
			{
				// Forward roll
			case EOOSMoveDir::OOSMD_Forward: case EOOSMoveDir::OOSMD_DownForward: case EOOSMoveDir::OOSMD_UpForward:
				PlayAnim(LyingUpRollF);
				break;

				// Backward roll
			case EOOSMoveDir::OOSMD_Back: case EOOSMoveDir::OOSMD_DownBack: case EOOSMoveDir::OOSMD_UpBack:
				PlayAnim(LyingUpRollB);
				break;

			default:
				// Forced forward roll
				if (bForwardRollOnNeutral)
				{
					PlayAnim(LyingUpRollF);
				}
				// Neutral getup
				else
				{
					PlayAnim(LyingUpStand);
					SetFacing(MovementComponent->bShouldFaceRight);
				}
				break;
			}
		}

		SetPawnState(EOOSPawnState::OOSPS_Idle, true);
	}	
}

void AOOSPawn::AutoJump()
{
	// Do the requested landing jump cancel.
	Jump();
}

void AOOSPawn::PushBlock()
{
	if (IsPushBlocking() || !MovementComponent || !Opponent || !Opponent->MovementComponent) return;

	if (IsBlocking())
	{
		if (MovementComponent->bOnGround)
		{
			PlayAnim(St_PushBlock);
		}
		else
		{
			PlayAnim(Air_PushBlock);
		}
	}
	else if (IsCrouchBlocking())
	{
		PlayAnim(Cr_PushBlock);
	}
	else return;

	// Cancel hitstop early
	HitStop = 0.f;
	TryHitStopOver();

	float Direction = MovementComponent->bShouldFaceRight ? 1.f : -1.f;

	Opponent->MovementComponent->ApplyPushImpulse(PushBlockAmt * Direction);
}

void AOOSPawn::OnTurnaround_Implementation()
{
	if (MovementComponent->bOnGround)
	{
		// Don't need to change the animation for blocking, it'd cancel the block anyway
		if (IsBlocking() || IsCrouchBlocking())
			return;

		if (IsCrouching())
		{
			PlayAnim(CrouchTurn);
		}
		else
		{
			PlayAnim(StandTurn);
		}
	}
}

void AOOSPawn::OnWallTouch()
{
	if (!AnimationSequence) return;

	UAnimationAsset* CurrentAnim = AnimationSequence;

	if (CurrentAnim == LaunchBackFar)
	{
		if (!bForceComboExtension && TimesWallBounced >= 1)
		{
			PlayAnim(LaunchBackShort);
		}
		else if (WallTechWindow <= 0.f)
		{			
			/* 
			* We bounce immediately, but we broadcast the event after the tech window 
			* to send the result.
			*/
			BounceOffWall();
			
			/* 
			* Can't walltech super/ultras, so complete the bounce.
			* @TODO Also if we don't have a PC, we're AI, but this is TERRIBLE. AI should be able to tech, but the 
			* immediate input is in the human controller... A quick idea would be to broadcast this to the AI controller
			* and make it roll odds for teching and return a result here again.
			*/
			if (!GetPlayerController() || Opponent->bIsPerformingSuper || Opponent->IsPerformingUltra())
			{
				OnWallHit(false);
			}
			else
			{
				LastWallBounce = FPlatformTime::Seconds();
				WallTechWindow = WALL_TECH_WINDOW;
			}
		}

	}

	if (IsLaunched())
	{
		FVector ImpactPoint = GetActorLocation();
		ImpactPoint += GetActorForwardVector() * Capsule->GetScaledCapsuleRadius() * (MovementComponent->bShouldFaceRight ? -1.f : 1.f);
		OnThrowCollision.Broadcast(false, ImpactPoint);
	}

}

void AOOSPawn::BounceOffWall()
{
	SetPawnState(EOOSPawnState::OOSPS_Launch);
	MovementComponent->XSpeed = (FMath::Sign(MovementComponent->XSpeed) * -1) * 1.5;
	MovementComponent->YSpeed = 8.f;
	PlayAnim(HitWall);
}

void AOOSPawn::OnWallHit_Implementation(bool bTech)
{
	if (!bComboExtensionsReset) TimesWallBounced++;
}

void AOOSPawn::OnAnimationEnd_Implementation(UAnimationAsset* AnimSequence)
{

}

void AOOSPawn::OnOOSAnimNotify_Implementation(UAnimationAsset* AnimSequence, FName NotifName)
{

}

void AOOSPawn::MoveHorizontal(float AxisValue)
{
	if (!MovementComponent || IsRecoveringOnGround() || IsRecoveringInAir() || IsLaunched() || IsStunned() || IsFloored() || IsCrouching() || IsDashing() || IsJumping() || IsPerformingMove() 
		|| IsChargingTransform() || IsTransforming() || IsBlocking() || IsCrouchBlocking()) return;

	if (IsSuperJumping())
	{
		if (AxisValue == 0.f) return;
		if (MovementComponent->bShouldFaceRight)
			// Forward
		{
			MovementComponent->AccelX((AxisValue > 0.f) ? SuperJumpXAccelFW : -SuperJumpXAccelBW);
		}
		else
			// Back
		{
			MovementComponent->AccelX((AxisValue > 0.f) ? SuperJumpXAccelBW : -SuperJumpXAccelFW);
		}
		return;
	}

	if (!FlyingCharacter && bIsPerformingMove)
	{
		if(MovementComponent->bOnGround)
		MovementComponent->XSpeed = 0.f;

		return;
	}

	if (MovementComponent->bFlightMode)
	{
		if (!AnimationSequence)
		{
			// If no anim is playing, this might be the first tick so force flying.
			PlayAnim(FlightBlendSpace, true);
		}

		BlendSpaceInput.X = (MovementComponent->bShouldFaceRight ? MovementComponent->XSpeed : -MovementComponent->XSpeed);

		if (AxisValue == 0.f) return;
		MovementComponent->AccelX((AxisValue > 0.f) ? FlightAccel : -FlightAccel);
		MovementComponent->bOnGround = false;

		return;
	}
	
	if (MovementComponent->bOnGround)
	{
		if (AnimationSequence)
		{
			UAnimationAsset* AS = AnimationSequence;

			if (AxisValue != 0.f)
			{
				if ((AxisValue > 0.f) == MovementComponent->bIsFacingRight)
				{
					if (AS != WalkFW)
					{
						PlayAnim(WalkFW, true);
					}
				}
				else
				{
					if (AS != WalkBW)
					{
						PlayAnim(WalkBW, true);
					}
				}
			}
			// If not touching any directions, don't override uncrouch or landing. Obviously don't replay if already idle.
			else if ((AS != Idle) && !IsLanding() && !IsTurning() && !IsRecoveringOnGround() && !IsRecoveringInAir() && (AS != CrouchToIdle) && (AS != CrouchLoop) && (AS != DashFW) && (AS != DashBW))
			{
				PlayAnim(Idle, true);
			}
		}
		else
		{
			// If no anim is playing, this might be the first tick so force idle.
			if (FlyingCharacter)
			{
				SetFlightModeEnabled(true);
				MovementComponent->bOnGround = false;
			}
			else
			PlayAnim(Idle, true);
		}
	}

	if (FMath::IsNearlyZero(AxisValue))
	{
		if (!MovementComponent->bUseXDecel)
		{
			MovementComponent->XSpeed = 0.f;
		}
	}
	else
	{
		float Speed = MovementComponent->bOnGround ? GroundMovementSpeed : AirMovementSpeed;
		MovementComponent->XSpeed = FMath::Sign(AxisValue) * Speed;
	}
}

void AOOSPawn::MoveVertical(float AxisValue)
{
	if (!MovementComponent || IsRecoveringOnGround() || IsRecoveringInAir() || IsLaunched() || IsStunned() || IsFloored() || IsCrouching() || IsDashing() || IsJumping() || IsPerformingMove() 
		|| IsChargingTransform() || IsTransforming() || IsBlocking() || IsCrouchBlocking()) return;

	if (MovementComponent->bFlightMode)
	{
		if (!AnimationSequence)
		{
			// If no anim is playing, this might be the first tick so force flying.
			PlayAnim(FlightBlendSpace, true);
		}
		
		BlendSpaceInput.Y = (MovementComponent->YSpeed);

		if (AxisValue == 0.f) return;
		MovementComponent->AccelY((AxisValue > 0.f) ? FlightAccel : -FlightAccel);
		
		return;
	}
}

bool AOOSPawn::Jump(bool bForce, bool bSkipMoveCheck)
{
	// Can't jump cancel a landing if custom landing from a move.
	bool bCanJumpCancel = CurrentNode && CurrentNode->Move.LandingAnim && AnimationSequence && (AnimationSequence == CurrentNode->Move.LandingAnim);

	if (!MovementComponent || !GetController()) return false;

	if (!bForce && (CanOnlyWalk() || IsRecoveringOnGround() || IsRecoveringInAir() || IsLaunched() || IsStunned() || IsChargingTransform() || IsTransforming() || IsFloored() || IsBlocking() || IsCrouchBlocking() || IsTurning()
		|| bCanJumpCancel || (!bSkipMoveCheck && IsPerformingMove() && (ReadiedJumpCancel == EOOSInputDir::OOSID_None)))) return false;

	UOOSFighterInputs* Inputs = GetFighterInputs();
	if (Inputs && Inputs->CanSuperJump() && MovementComponent->bOnGround && !IsPerformingMove())
	{
		return SuperJump();
	}

	if (MovementComponent->bOnGround)
	{
		if (!bForce && IsJumping() && !IsLanding()) return false;

		MovementComponent->bUseXDecel = false;
		MovementComponent->bUseYDecel = false;
		SetPawnState(EOOSPawnState::OOSPS_Jump);
		bIsPerformingMove = false;

		EOOSInputDir Dir = EOOSInputDir::OOSID_Up;
		if (Inputs) Dir = Inputs->DPadState;
		switch (Dir)
		{
		case EOOSInputDir::OOSID_Up: default:
			SetFacing(MovementComponent->bShouldFaceRight);
			MovementComponent->bOnGround = false;
			MovementComponent->XSpeed = 0.f;
			MovementComponent->YSpeed = JumpSpeed.Y;
			PlayAnim(SingleJump);
			break;

		case EOOSInputDir::OOSID_UpLeft:
			SetFacing(MovementComponent->bShouldFaceRight);
			MovementComponent->bOnGround = false;
			MovementComponent->XSpeed = -JumpSpeed.X;
			MovementComponent->YSpeed = JumpSpeed.Y;
			PlayAnim(MovementComponent->bIsFacingRight ? SingleJumpB : SingleJumpF);
			break;

		case EOOSInputDir::OOSID_UpRight:
			SetFacing(MovementComponent->bShouldFaceRight);
			MovementComponent->bOnGround = false;
			MovementComponent->XSpeed = JumpSpeed.X;
			MovementComponent->YSpeed = JumpSpeed.Y;
			PlayAnim(MovementComponent->bIsFacingRight ? SingleJumpF : SingleJumpB);
			break;
		}
		return true;
	}
	else
	{
		if (!MovementComponent->bFlightMode && (RemainingJumps > 0) && IsPawnHigherThan(HeightThresholdUp, HeightThresholdDown) && !IsDashing())
		{
			// If we don't have any air jump anims, just don't affect physics.
			if (!DoubleJump && !DoubleJumpB && !DoubleJumpF) return false;

			MovementComponent->bUseXDecel = false;
			MovementComponent->bUseYDecel = false;

			EOOSInputDir Dir = EOOSInputDir::OOSID_Up;
			if (ReadiedJumpCancel != EOOSInputDir::OOSID_None)
				Dir = ReadiedJumpCancel;
			else if (Inputs)
				Dir = Inputs->DPadState;
			switch (Dir)
			{
			case EOOSInputDir::OOSID_Up: default:
				MovementComponent->YSpeed = DoubleJumpSpeed.Y;
				MovementComponent->XSpeed = 0.f;
				PlayAnim(DoubleJump);
				break;

			case EOOSInputDir::OOSID_UpLeft:
				MovementComponent->XSpeed = -DoubleJumpSpeed.X;
				MovementComponent->YSpeed = DoubleJumpSpeed.Y;
				PlayAnim(MovementComponent->bIsFacingRight ? DoubleJumpB : DoubleJumpF);
				break;

			case EOOSInputDir::OOSID_UpRight:
				MovementComponent->XSpeed = DoubleJumpSpeed.X;
				MovementComponent->YSpeed = DoubleJumpSpeed.Y;
				PlayAnim(MovementComponent->bIsFacingRight ? DoubleJumpF : DoubleJumpB);
				break;
			}

			RemainingJumps--;
			SetPawnState(EOOSPawnState::OOSPS_Jump);
			bIsPerformingMove = false;
			return true;
		}
		else return false;
	}
}

bool AOOSPawn::SuperJump()
{
	if (!MovementComponent || !GetController() || CanOnlyWalk() || IsRecoveringOnGround() || IsRecoveringInAir() || IsLaunched() || IsStunned() || IsFloored() || IsPerformingMove() || IsChargingTransform()
		|| IsTransforming() || !MovementComponent->bOnGround || IsBlocking() || IsCrouchBlocking() || IsTurning() || !IsFacingOpponent()) return false;

	SetPawnState(EOOSPawnState::OOSPS_SuperJump);

	UOOSFighterInputs* Inputs = GetFighterInputs();
	EOOSInputDir Dir = EOOSInputDir::OOSID_Up;
	if (Inputs) Dir = Inputs->DPadState;
	switch (Dir)
	{
	case EOOSInputDir::OOSID_Up: default:
		SetFacing(MovementComponent->bShouldFaceRight);
		MovementComponent->bOnGround = false;
		MovementComponent->XSpeed = 0.f;
		MovementComponent->YSpeed = SuperJumpSpeed.Y;
		PlayAnim(SupJump);
		break;

	case EOOSInputDir::OOSID_UpLeft:
		SetFacing(MovementComponent->bShouldFaceRight);
		MovementComponent->bOnGround = false;
		MovementComponent->XSpeed = -SuperJumpSpeed.X;
		MovementComponent->YSpeed = SuperJumpSpeed.Y;
		PlayAnim(SupJump);
		break;

	case EOOSInputDir::OOSID_UpRight:
		SetFacing(MovementComponent->bShouldFaceRight);
		MovementComponent->bOnGround = false;
		MovementComponent->XSpeed = SuperJumpSpeed.X;
		MovementComponent->YSpeed = SuperJumpSpeed.Y;
		PlayAnim(SupJump);
		break;
	}

	return true;
}

bool AOOSPawn::Crouch()
{
	if (!MovementComponent || CanOnlyWalk() || IsCrouching() || IsRecoveringOnGround() || IsRecoveringInAir() || IsStunned() || IsLaunched() || IsFloored() || IsPerformingMove()
		|| IsChargingTransform() || IsTransforming() || !MovementComponent->bOnGround || IsBlocking() || IsCrouchBlocking())
	{
		return false;
	}

	MovementComponent->bUseXDecel = true;
	if (IsCrouching())
	{
		PlayAnim(CrouchLoop, true);
	}
	else
	{
		SetPawnState(EOOSPawnState::OOSPS_Crouch);
		PlayAnim(IdleToCrouch);
	}

	return true;
}

void AOOSPawn::Uncrouch()
{
	if (!MovementComponent || CanOnlyWalk() || !IsCrouching() || IsRecoveringOnGround() || IsRecoveringInAir() || IsStunned() || IsLanding() || IsLaunched() || IsFloored() || IsPerformingMove()
		|| IsChargingTransform() || IsTransforming() || !MovementComponent->bOnGround || IsBlocking() || IsCrouchBlocking() || IsTurning() || IsDashing())
	{
		return;
	}

	SetPawnState(EOOSPawnState::OOSPS_Idle, true);
	PlayAnim(CrouchToIdle);
}

void AOOSPawn::Dash(EOOSInputDir Direction)
{
	if (IsDashing() || !MovementComponent || IsCompletelyCrouched() || CanOnlyWalk() || IsRecoveringOnGround() || IsRecoveringInAir() || IsLaunched() || IsStunned() || IsFloored() ||
		(IsPerformingMove() && (ReadiedDashCancel == EOOSInputDir::OOSID_None)) || IsChargingTransform() || IsTransforming() || IsBlocking() || IsCrouchBlocking() || !IsFacingOpponent())
	{
		return;
	}

	bool OK = false;
	if (MovementComponent->bOnGround)
	{
		switch (Direction)
		{
		case EOOSInputDir::OOSID_None:
			OK = PlayAnim(DashFW);
			break;

		case EOOSInputDir::OOSID_Left: 
		case EOOSInputDir::OOSID_UpLeft: 
		case EOOSInputDir::OOSID_DownLeft:
			OK = PlayAnim(MovementComponent->bIsFacingRight ? DashBW : DashFW);
			break;

		case EOOSInputDir::OOSID_Right: 
		case EOOSInputDir::OOSID_UpRight: 
		case EOOSInputDir::OOSID_DownRight:
			OK = PlayAnim(MovementComponent->bIsFacingRight ? DashFW : DashBW);
			break;
		}
		if (!OK) return;

		SetPawnState(EOOSPawnState::OOSPS_Dash);
		bIsPerformingMove = false;
	}
	else
	{
		if (RemainingJumps > 0 && (IsPawnHigherThan(HeightThresholdUp, HeightThresholdDown) || MovementComponent->bFlightMode))
		{
			if (AirDashUp || AirDashDown || AirDashUpFW || AirDashUpBW || AirDashDownFW || AirDashDownBW)
			{
				AirDashEightWay(Direction);
			}
			else
			{
				AirDashTwoWay(Direction);
			}
		}
	}
}

void AOOSPawn::AirDashTwoWay(EOOSInputDir Direction)
{
	bool OK = false;
	switch (Direction)
	{
	case EOOSInputDir::OOSID_None:
		OK = PlayAnim(AirDashFW);
		break;

	case EOOSInputDir::OOSID_Left:
	case EOOSInputDir::OOSID_UpLeft:
	case EOOSInputDir::OOSID_DownLeft:
		OK = PlayAnim(MovementComponent->bIsFacingRight ? AirDashBW : AirDashFW);
		break;

	case EOOSInputDir::OOSID_Right:
	case EOOSInputDir::OOSID_UpRight:
	case EOOSInputDir::OOSID_DownRight:
		OK = PlayAnim(MovementComponent->bIsFacingRight ? AirDashFW : AirDashBW);
		break;
	}
	if (!OK) return;

	RemainingJumps--;
	SetPawnState(EOOSPawnState::OOSPS_Dash);
	bIsPerformingMove = false;
}

void AOOSPawn::AirDashEightWay(EOOSInputDir Direction)
{
	bool OK = false;
	switch (Direction)
	{
	case EOOSInputDir::OOSID_None:
		OK = PlayAnim(AirDashFW);
		break;
	case EOOSInputDir::OOSID_Right:
		OK = PlayAnim(MovementComponent->bIsFacingRight ? AirDashFW : AirDashBW);
		break;
	case EOOSInputDir::OOSID_UpRight:
		OK = PlayAnim(MovementComponent->bIsFacingRight ? AirDashUpFW : AirDashUpBW);
		break;
	case EOOSInputDir::OOSID_Up:
		OK = PlayAnim(AirDashUp);
		break;
	case EOOSInputDir::OOSID_UpLeft:
		OK = PlayAnim(MovementComponent->bIsFacingRight ? AirDashUpBW : AirDashUpFW);
		break;
	case EOOSInputDir::OOSID_Left:
		OK = PlayAnim(MovementComponent->bIsFacingRight ? AirDashBW : AirDashFW);
		break;
	case EOOSInputDir::OOSID_DownLeft:
		OK = PlayAnim(MovementComponent->bIsFacingRight ? AirDashDownBW : AirDashDownFW);
		break;
	case EOOSInputDir::OOSID_Down:
		OK = PlayAnim(AirDashDown);
		break;
	case EOOSInputDir::OOSID_DownRight:
		OK = PlayAnim(MovementComponent->bIsFacingRight ? AirDashDownFW : AirDashDownBW);
		break;
	}
	if (!OK) return;

	RemainingJumps--;
	SetPawnState(EOOSPawnState::OOSPS_Dash);
	bIsPerformingMove = false;
}

int AOOSPawn::GetHitStunRemaining() const
{
	if (IsStunned() || IsFloored())
	{
		return AnimationFramesRemaining();
	}
	else if (IsLaunched())
	{
		//@TODO: Launch recovery is based on velocity, so we'd have to do some ballistics
		//return 60;
		return AnimationFramesRemaining();
	}

	return 0;
}

int AOOSPawn::AnimationFramesRemaining() const
{
	UAnimSingleNodeInstance* SNI = Cast<UAnimSingleNodeInstance>(SkeletalMesh->GetAnimInstance());
	if (SNI)
	{
		float Rate = 1.f;
		UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(SNI->GetCurrentAsset());
		if (AnimSequence)
		{
			Rate = AnimSequence->RateScale;
		}

		float Duration = SNI->GetLength();
		float Position = SkeletalMesh->GetPosition();
		return floor((Duration - Position) / Rate * 60);
	}

	return 0;
}

int AOOSPawn::AnimationFramesElapsed() const
{
	UAnimSingleNodeInstance* SNI = Cast<UAnimSingleNodeInstance>(SkeletalMesh->GetAnimInstance());
	if (SNI)
	{
		float Rate = 1.f;
		UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(SNI->GetCurrentAsset());
		if (AnimSequence)
		{
			Rate = AnimSequence->RateScale;
		}

		float Position = SkeletalMesh->GetPosition();
		return floor(Position / Rate * 60);
	}

	return 0;
}

void AOOSPawn::ExternalMove_Begin(FVector2D Speed, bool bAir, bool bAllowGravity, bool bKeepMovement, bool bAddMovement, bool bUseXDecelerationAtStart, bool bUseYDecelerationAtStart, bool bDisablePush)
{
	if (!MovementComponent || !Opponent || !Opponent->MovementComponent || bIgnoreMovementNotifies) return;

	MovementComponent->bDoNotPush = bDisablePush;
	Opponent->MovementComponent->bDoNotPush = bDisablePush;
	MovementComponent->bUseXDecel = bUseXDecelerationAtStart;
	MovementComponent->bUseYDecel = bUseYDecelerationAtStart;
	MovementComponent->bUseGravity = bAllowGravity;

	if (bAir)
	{
		MovementComponent->bOnGround = false;
	}

	if (!bKeepMovement && !bAddMovement)
	{
		MovementComponent->XSpeed = MovementComponent->bIsFacingRight ? Speed.X : -Speed.X;
		MovementComponent->YSpeed = Speed.Y;
	}

	if (bAddMovement)
	{
		MovementComponent->XSpeed = MovementComponent->XSpeed + (MovementComponent->bIsFacingRight ? Speed.X : -Speed.X);
		MovementComponent->YSpeed = MovementComponent->YSpeed + Speed.Y;
	}
}

void AOOSPawn::ExternalMove_End(bool bUseXDecelerationAtEnd, bool bUseYDecelerationAtEnd, bool bStopAtEnd)
{
	if (!MovementComponent) return;

	MovementComponent->bDoNotPush = false;
	//Opponent->MovementComponent->bDoNotPush = false;
	MovementComponent->bUseXDecel = bUseXDecelerationAtEnd;
	MovementComponent->bUseYDecel = bUseYDecelerationAtEnd;

	if (!FlyingCharacter)
	{
		MovementComponent->bUseGravity = true;
	}

	if (bStopAtEnd)
	{
		MovementComponent->XSpeed = 0.f;

		MovementComponent->YSpeed = 0.f;
	}

}

void AOOSPawn::SetPawnState(EOOSPawnState NewState, bool bSkipAnims)
{
	if (!MovementComponent || !Opponent) return;

	ResetReadiedMoves();

	if (bDefeated)
	{
		if (bDefeatWaitForIdle)
		{
			if ((NewState == EOOSPawnState::OOSPS_Idle) && !bSkipAnims && MovementComponent->bOnGround)
			{
				bDefeatWaitForIdle = false;
				PlayAnim(TimeOverDefeat);
				return;
			}
		}
		else return;
	}

	UOOSFighterInputs* Inputs = GetFighterInputs();
	switch (NewState)
	{
	case EOOSPawnState::OOSPS_Idle:
		// If we're being set to idle and we're floored in training mode, refill HP and gauges and clear detransform cooldown.
		if ((PlayerIndex == 1) && MovementComponent->bOnGround && GameInstance && GameInstance->bTrainingMode && GameInstance->bTrainingAutoRefill)
		{
			TrainingRefill();
			Opponent->TrainingRefill();

			Opponent->LastDetransform = UGameplayStatics::GetTimeSeconds(GetWorld()) - DETRANSFORM_TIME_PENALTY;
		}

		// If coming back from any stun state, reset camera forced tracking.
		if (Camera && (PawnState == EOOSPawnState::OOSPS_Stun || PawnState == EOOSPawnState::OOSPS_Block || PawnState == EOOSPawnState::OOSPS_CrouchBlock
			|| PawnState == EOOSPawnState::OOSPS_Launch || PawnState == EOOSPawnState::OOSPS_Floored))
		{
			Camera->SetActorToFollow(nullptr);
		}

		Opponent->HitCount = 0;
		Opponent->DamageScale = 1.f;
		Opponent->HitStunScale = 1.f;
		//@TODO: Might be a better place to reset Opponent->bHasMadeContact
		ResetKnockdowns();
		DefensiveTransform = false;
		bIsPerformingMove = false;
		PawnState = NewState;

		if (!bSkipAnims)
		{
			if (FlyingCharacter)
			{
				MovementComponent->bFlightMode = true;
				MovementComponent->bOnGround = false;
				MovementComponent->bUseXDecel = false;
				MovementComponent->bUseXDecel = false;
			}

			if (MovementComponent->bOnGround)
			{

				EOOSInputDir Dir = EOOSInputDir::OOSID_None;
				if (Inputs) Dir = Inputs->DPadState;
				if ((Dir == EOOSInputDir::OOSID_Down) || (Dir == EOOSInputDir::OOSID_DownRight) || (Dir == EOOSInputDir::OOSID_DownLeft))
				{
					if (!Crouch())
					{
						PlayAnim(Idle, true);
					}
				}
				else if ((Dir == EOOSInputDir::OOSID_Up) || (Dir == EOOSInputDir::OOSID_UpRight) || (Dir == EOOSInputDir::OOSID_UpLeft))
				{
					if (!Jump())
					{
						PlayAnim(Idle, true);
					}
				}
				else
				{
					PlayAnim(Idle, true);
				}
			}
			else
			{
				if (MovementComponent->bFlightMode)
				{
					PlayAnim(FlightBlendSpace, true);
				}
				else if (MovementComponent->XSpeed == 0.f)
				{
					PlayAnim(Fall, true);
				}
				else if (MovementComponent->bIsFacingRight)
					// Fall forward
				{
					MovementComponent->XSpeed > 0.f ? PlayAnim(FallF) : PlayAnim(FallB);
				}
				else
					// Fall back
				{
					MovementComponent->XSpeed > 0.f ? PlayAnim(FallB) : PlayAnim(FallF);
				}
			}
		}
		break;

	case EOOSPawnState::OOSPS_ChargeTransform:
		PawnState = NewState;
		if (!bSkipAnims)
		{
			if (MovementComponent->bOnGround)
			{
				PlayAnim(Charge_Ground, true, -1.f, 0.1f, 0.1f);
			}
			else
			{
				PlayAnim(Charge_Air, true, -1.f, 0.1f, 0.1f);
			}
		}
		break;

	case EOOSPawnState::OOSPS_Defeat:
		bDefeated = true;
		if (GetController()) GetController()->UnPossess();
		MoveHorizontal(0.f);
		Opponent->MoveHorizontal(0.f);
		if (MovementComponent->bOnGround)
		{
			if (bDefeatWaitForIdle)
			{
				bDefeatWaitForIdle = false;
				PlayAnim(TimeOverDefeat);
			}
			else
			{
				if(!IgnoreDeathAnims())
					PlayAnim(Crumble);
			}
		}
		break;

	default:
		PawnState = NewState;
		break;
	}
}

void AOOSPawn::ResetKnockdowns()
{
	TimesLaunched = 0;
	TimesWallBounced = 0;
	TimesSpiralled = 0;
	TimesFloored = 0;

	bSpiralFallCounted = false;
	bForceComboExtension = false;
	bComboExtensionsReset = false;
}

bool AOOSPawn::IsPerformingMove() const
{
	return (PawnState == EOOSPawnState::OOSPS_Move) || (PawnState == EOOSPawnState::OOSPS_SpecialMove) || (PawnState == EOOSPawnState::OOSPS_HyperMove) || bIsPerformingMove;
}

bool AOOSPawn::IsPerformingUltra() const
{
	if (CurrentNode && IsPerformingMove())
	{
		if (CurrentNode->Move.MoveType == EOOSMoveType::OOSMT_Ultra) return true;
	}

	return false;
}

bool AOOSPawn::IsJumping()
{
	return PawnState == EOOSPawnState::OOSPS_Jump;
}

bool AOOSPawn::IsSuperJumping()
{
	return PawnState == EOOSPawnState::OOSPS_SuperJump;
}

bool AOOSPawn::IsDashing() const
{
	return PawnState == EOOSPawnState::OOSPS_Dash;
}

bool AOOSPawn::IsCrouching()
{
	return PawnState == EOOSPawnState::OOSPS_Crouch;
}

bool AOOSPawn::IsCompletelyCrouched()
{
	if (!AnimationSequence) return false;

	UAnimationAsset* AS = AnimationSequence;

	return (AS == CrouchLoop);
}

bool AOOSPawn::IsDoubleJumping()
{
	return IsJumping() && (RemainingJumps != MaxAirJumps);
}

bool AOOSPawn::IsFloored() const
{
	return PawnState == EOOSPawnState::OOSPS_Floored;
}

bool AOOSPawn::IsStunned() const
{
	return PawnState == EOOSPawnState::OOSPS_Stun;
}

bool AOOSPawn::IsLaunched() const
{
	return PawnState == EOOSPawnState::OOSPS_Launch;
}

bool AOOSPawn::IsIdle() const
{
	return PawnState == EOOSPawnState::OOSPS_Idle;
}

// Note that IsIdle ommits other states that cannot be represented in EOOSPawnState, such as IsRecoveringOnGround(), IsLanding(), etc.
bool AOOSPawn::IsNeutral() const
{
	if (!IsIdle())
	{
		return false;
	}
	else if (IsRecoveringOnGround() || IsRecoveringInAir() || IsLanding() || bIsPerformingMove)
	{
		return false;
	}
	else return true;
}

bool AOOSPawn::IsBlockInputAllowed() const
{
	return !IsStunned() && !IsFloored() && !IsLaunched() && !IsTransforming() && !bIsPerformingMove && !IsFrozen() && !IsRecoveringOnGround() && !IsRecoveringInAir() && !IsDashing();
}

bool AOOSPawn::IsInputAllowed() const
{
	return IsBlockInputAllowed() && !IsBlocking() && !IsCrouchBlocking();
}

bool AOOSPawn::IsInvincible()
{
	// Force invincible for the start of pushblocking
	//@TODO: If possible the invincible anim notify on frame 0 should trigger
	// instantly and handle this, instead of needing this special case
	if (IsPushBlocking())
	{
		if (AnimationFramesElapsed() == 0)
			return true;
	}

	return bInvincible || bFreezeInvincible || IsRecoveringOnGround() || IsRecoveringInAir();
}

bool AOOSPawn::IsFrozen() const
{
	return bFrozen;
}

bool AOOSPawn::IsAttackActive() const
{
	TArray<USceneComponent*> InChildren = SkeletalMesh->GetAttachChildren();

	for (USceneComponent* Component : InChildren)
	{
		UOOSHitbox* HitBox = Cast<UOOSHitbox>(Component);
		if (HitBox)
			return true;
	}
	return false;
}

bool AOOSPawn::IsChargingTransform()
{
	return PawnState == EOOSPawnState::OOSPS_ChargeTransform;
}

bool AOOSPawn::IsTransforming() const
{
	return PawnState == EOOSPawnState::OOSPS_Transform;
}

bool AOOSPawn::IsTransformCoolingDown() const
{
	return (UGameplayStatics::GetTimeSeconds(GetWorld()) - LastDetransform) <= DETRANSFORM_TIME_PENALTY;
}

void AOOSPawn::LockoutTransform()
{
	TransformLockout = TRANSFORM_LOCKOUT_TIME;
}

float AOOSPawn::LockoutTime() const
{
	return TRANSFORM_LOCKOUT_TIME;
}

bool AOOSPawn::IsTransformLockedOut() const
{
	return TransformLockout > 0;
}

bool AOOSPawn::IsBlocking() const
{
	return PawnState == EOOSPawnState::OOSPS_Block;
}

bool AOOSPawn::IsCrouchBlocking() const
{
	return PawnState == EOOSPawnState::OOSPS_CrouchBlock;
}

bool AOOSPawn::IsPushBlocking()
{
	if (!AnimationSequence) return false;

	UAnimationAsset* AS = AnimationSequence;

	return (AS == St_PushBlock) || (AS == Cr_PushBlock) || (AS == Air_PushBlock);
}

bool AOOSPawn::IsRecoveringOnGround() const
{
	if (!AnimationSequence) return false;

	UAnimationAsset* AS = AnimationSequence;

	return (AS == LyingDownStand) || (AS == LyingDownRollF) || (AS == LyingDownRollB) ||
		(AS == LyingUpStand) || (AS == LyingUpRollF) || (AS == LyingUpRollB) ||
		(AS == QuickRecoverB) || (AS == QuickRecoverF);
}

bool AOOSPawn::IsRecoveringInAir() const
{
	if (!AnimationSequence) return false;

	UAnimationAsset* AS = AnimationSequence;

	return (AS == AirComboBreak);
}

bool AOOSPawn::IsLanding() const
{
	if (!AnimationSequence) return false;

	if (AnimationSequence == Land || AnimationSequence == LandTurn)
	{
		return true;
	}
	else if (CurrentNode && CurrentNode->Move.LandingAnim)
	{
		bool bCustomLanding = AnimationSequence == CurrentNode->Move.LandingAnim;
		return bCustomLanding;
	}
	else return false;

}

bool AOOSPawn::IsFacingOpponent()
{
	return MovementComponent && (MovementComponent->bShouldFaceRight == MovementComponent->bIsFacingRight);
}

bool AOOSPawn::IsKO() const
{
	return bDefeated && !bDefeatWaitForIdle;
}

bool AOOSPawn::CanOnlyWalk() const
{
	return WalkOnly;
}

bool AOOSPawn::IsTurning()
{
	if (!AnimationSequence) return false;

	return (AnimationSequence == StandTurn) || (AnimationSequence == CrouchTurn);
}

bool AOOSPawn::IsTransformReady() const
{
	bool Amount = Transform >= (MAX_TRANSFORM_POINTS / 4);

	return !IsTransformCoolingDown() && !IsTransformLockedOut() && Amount;
}

UFUNCTION(BlueprintCallable) bool AOOSPawn::WasKilled() const
{
	return AnimationSequence && (AnimationSequence == InstantDeath);
}

bool AOOSPawn::IsCloseToOpponent(float Threshold) const
{
	return MovementComponent && MovementComponent->IsCloseToOpponent(Threshold) && MovementComponent->IsAtPushHeight();
}

bool AOOSPawn::CanPerformThrow() const
{
	return IsCloseToOpponent(40.f) && !Opponent->IsFloored() && !Opponent->IsRecoveringOnGround() && !Opponent->IsRecoveringInAir() && !Opponent->IsPerformingMove() && (MovementComponent->bOnGround == Opponent->MovementComponent->bOnGround);
}

void AOOSPawn::ResetTimeDilation()
{
	CustomTimeDilation = 1.f;
}

bool AOOSPawn::IsPausingMatchTimer() const
{
	return IsKO() || bIsPerformingSuper || IsPerformingUltra() || CanOnlyWalk();
}

// Called when XDecel Makes the Pawn come to a complete stop. Useful for resetting states.
void AOOSPawn::Stopped()
{
	if (IsDashing() && !IsPerformingMove())
	{
		//SetPawnState(EOOSPawnState::OOSPS_Idle);
	}
}

bool AOOSPawn::GetHit(
	EOOSHitHeight HitHeight,
	EOOSInputAttack AttackType,
	bool bUnblockable,
	bool bOH,
	EOOSLaunchType LaunchType,
	FVector2D LaunchSpeed,
	bool bForceExtension,
	bool bResetExtensions,
	bool bAntiArmor,
	int Damage,
	int Chip,
	float HitStun,
	float BlockStun,
	bool bIsProjectile,
	bool& bCounter,
	bool bIsSuper)
{
	if (!MovementComponent || !Opponent || !Opponent->MovementComponent) return false;


	UOOSFighterInputs* Inputs = GetFighterInputs();
	AOOSPlayerController* PlayerController = GetPlayerController();
	EOOSInputDir Dir = EOOSInputDir::OOSID_Up;
	if (Inputs) Dir = Inputs->DPadState;
	EOOSMoveDir DPad = ResolveDirection(Dir);

	// Check for blocking.
	if (!bUnblockable && IsBlockInputAllowed()) // && IsFacingOpponent()) && !IsTurning()) //@TODO: hotfix to prevent degenerate crossups, look into this more
	{
		bool bBlocked = false;
		bool bChipKO = Chip >= Health;

		if (!PlayerController)
		{
			AOOSAIController* AI = Cast<AOOSAIController>(GetController());
			if (AI)
			{
				DPad = AI->NotifyHit(HitHeight, bOH);
			}
		}

		// Ground
		if (MovementComponent->bOnGround)
		{
			// Already chain blocking Mid/High.
			if (IsBlocking())
			{
				// Low attack incoming.
				if (HitHeight == EOOSHitHeight::OOSHH_Low)
				{
					// Gotta block low, or else we get hit.
					// Already blocking, so any down is fine
					if ((DPad == EOOSMoveDir::OOSMD_DownBack) ||
						(DPad == EOOSMoveDir::OOSMD_DownForward) ||
						(DPad == EOOSMoveDir::OOSMD_Down))
					{
						bBlocked = true;
						PlayAnim_NoBlend(Cr_Block, false, BlockStun);
						SetPawnState(EOOSPawnState::OOSPS_CrouchBlock);
					}
				}
				// Mid/High are not necessary to block again.
				else
				{
					bBlocked = true;
					PlayAnim_NoBlend(St_Block, false, BlockStun);
				}
			}
			// Already chain blocking low.
			else if (IsCrouchBlocking())
			{
				// Overhead incoming.
				if (bOH)
				{
					// Gotta block high, else we get hit.
					// Already blocking, just don't be holding down
					if ((DPad != EOOSMoveDir::OOSMD_DownBack) &&
						(DPad != EOOSMoveDir::OOSMD_DownForward) &&
						(DPad != EOOSMoveDir::OOSMD_Down))
					{
						bBlocked = true;
						PlayAnim_NoBlend(St_Block, false, BlockStun);
						SetPawnState(EOOSPawnState::OOSPS_Block);
					}
				}
				// Any other height is not necessary to block.
				else
				{
					bBlocked = true;
					PlayAnim_NoBlend(Cr_Block, false, BlockStun);
				}
			}
			// Not in a blocking state.
			else
			{
				// Crouch blocking...
				if (DPad == EOOSMoveDir::OOSMD_DownBack)
				{
					// Blocks anything except Overheads.
					if (!bOH)
					{
						bBlocked = true;
						PlayAnim_NoBlend(Cr_Block, false, BlockStun);
						SetPawnState(EOOSPawnState::OOSPS_CrouchBlock);
					}
				}
				// Standing blocking...
				else if ((DPad == EOOSMoveDir::OOSMD_Back) || (DPad == EOOSMoveDir::OOSMD_UpBack))
				{
					// Blocks anything except lows.
					if (HitHeight != EOOSHitHeight::OOSHH_Low)
					{
						bBlocked = true;
						PlayAnim_NoBlend(St_Block, false, BlockStun);
						SetPawnState(EOOSPawnState::OOSPS_Block);
					}
				}
			}
		}
		// Air is easier, there aren't any height differentiations.
		else
		{
			if (IsBlocking())
			{
				bBlocked = true;
				PlayAnim_NoBlend(Air_Block, false, BlockStun);
			}
			else
			{
				if ((DPad == EOOSMoveDir::OOSMD_Back) || (DPad == EOOSMoveDir::OOSMD_UpBack) || (DPad == EOOSMoveDir::OOSMD_DownBack))
				{
					bBlocked = true;
					PlayAnim_NoBlend(Air_Block, false, BlockStun);
					SetPawnState(EOOSPawnState::OOSPS_Block);
				}
			}
		}

		OnPawnBlock.Broadcast();

		if (bBlocked && !bChipKO)
		{
			// Immediately start up the animation so it's shown during hitstop
			SkeletalMesh->TickAnimation(0.f, false);
			SkeletalMesh->RefreshBoneTransforms();

			// Reset Y velocity if blocking
			MovementComponent->YSpeed = 0.f;

			// Turn to face the right way if the opponent got behind
			SetFacing(MovementComponent->bShouldFaceRight);

			LastDamage = Chip;
			Health = FMath::Clamp(Health - Chip, 0, MaxHealth);
			Regen = FMath::Clamp(Regen - (Chip / 2), 0, MaxHealth);
			if (PlayerIndex == 1)
			{
				if (Opponent->HitCount == 0)
				{
					GameMode->TotalDamage = 0;
				}

				GameMode->LastDamage = Chip;
				GameMode->TotalDamage += Chip;
				if (GameMode->TotalDamage > GameMode->MaxDamage) GameMode->MaxDamage = GameMode->TotalDamage;
				GameMode->DamageScale = 0.3f; //@TODO: Use a constant for this
			}

			CamShake(AttackType, 0.5f);

			return false;
		}
	}

	if (MovementComponent->bOnGround && Counter != EOOSCounter::OOSCT_None)
	{
		if (!Opponent->IsTransforming())
		{
			if (!bIsProjectile)
			{
				switch (Counter)
				{
				case EOOSCounter::OOSCT_Low:
					switch (HitHeight)
					{
					case EOOSHitHeight::OOSHH_Low:
						Opponent->SetPawnState(EOOSPawnState::OOSPS_Stun);
						if (Opponent->MovementComponent->bOnGround)
						{
							Opponent->PlayAnim_NoBlend(Opponent->St_MidHeavy);
						}
						else
						{
							Opponent->PlayAnim_NoBlend(Opponent->Air_Hit);
							Opponent->MovementComponent->YSpeed = 4.f;
						}
						Opponent->Freeze(1.f, false, false);
						PrepareChildMove(CurrentNode);
						bCounter = true;
						return true;
						break;

					case EOOSHitHeight::OOSHH_Mid:
						break;

					case EOOSHitHeight::OOSHH_High:
						break;

					}
				case EOOSCounter::OOSCT_Mid:
					switch (HitHeight)
					{
					case EOOSHitHeight::OOSHH_Low:
						break;

					case EOOSHitHeight::OOSHH_Mid:
						Opponent->SetPawnState(EOOSPawnState::OOSPS_Stun);
						if (Opponent->MovementComponent->bOnGround)
						{
							Opponent->PlayAnim_NoBlend(Opponent->St_MidHeavy);
						}
						else
						{
							Opponent->PlayAnim_NoBlend(Opponent->Air_Hit);
							Opponent->MovementComponent->YSpeed = 4.f;
						}
						Opponent->Freeze(1.f, false, false);
						PrepareChildMove(CurrentNode);
						bCounter = true;
						return true;
						break;

					case EOOSHitHeight::OOSHH_High:
						break;

					}
					break;
				case EOOSCounter::OOSCT_High:
					switch (HitHeight)
					{
					case EOOSHitHeight::OOSHH_Low:
						break;

					case EOOSHitHeight::OOSHH_Mid:
						break;

					case EOOSHitHeight::OOSHH_High:
						Opponent->SetPawnState(EOOSPawnState::OOSPS_Stun);
						if (Opponent->MovementComponent->bOnGround)
						{
							Opponent->PlayAnim_NoBlend(Opponent->St_MidHeavy);
						}
						else
						{
							Opponent->PlayAnim_NoBlend(Opponent->Air_Hit);
							Opponent->MovementComponent->YSpeed = 4.f;
						}
						Opponent->Freeze(1.f, false, false);
						PrepareChildMove(CurrentNode);
						bCounter = true;
						return true;
						break;
					}
					break;
				case EOOSCounter::OOSCT_Super:
					switch (HitHeight)
					{
					case EOOSHitHeight::OOSHH_Low:
						Opponent->SetPawnState(EOOSPawnState::OOSPS_Stun);
						if (Opponent->MovementComponent->bOnGround)
						{
							Opponent->PlayAnim_NoBlend(Opponent->St_MidHeavy);
						}
						else
						{
							Opponent->PlayAnim_NoBlend(Opponent->Air_Hit);
							Opponent->MovementComponent->YSpeed = 4.f;
						}
						Opponent->Freeze(1.f, false, false);
						PrepareChildMove(CurrentNode);
						bCounter = true;
						return true;
						break;

					case EOOSHitHeight::OOSHH_Mid:
						Opponent->SetPawnState(EOOSPawnState::OOSPS_Stun);
						if (Opponent->MovementComponent->bOnGround)
						{
							Opponent->PlayAnim_NoBlend(Opponent->St_MidHeavy);
						}
						else
						{
							Opponent->PlayAnim_NoBlend(Opponent->Air_Hit);
							Opponent->MovementComponent->YSpeed = 4.f;
						}
						Opponent->Freeze(1.f, false, false);
						PrepareChildMove(CurrentNode);
						bCounter = true;
						return true;
						break;

					case EOOSHitHeight::OOSHH_High:
						Opponent->SetPawnState(EOOSPawnState::OOSPS_Stun);
						if (Opponent->MovementComponent->bOnGround)
						{
							Opponent->PlayAnim_NoBlend(Opponent->St_MidHeavy);
						}
						else
						{
							Opponent->PlayAnim_NoBlend(Opponent->Air_Hit);
							Opponent->MovementComponent->YSpeed = 4.f;
						}
						Opponent->Freeze(1.f, false, false);
						PrepareChildMove(CurrentNode);
						bCounter = true;
						return true;
						break;
					}
					break;
				}
			}
			else
			{
				switch (Counter)
				{
				case EOOSCounter::OOSCT_Low:
					break;

				case EOOSCounter::OOSCT_Mid:
					break;

				case EOOSCounter::OOSCT_High:
					break;

				case EOOSCounter::OOSCT_Projectile:
					if (!bIsSuper)
					{
						Opponent->SetPawnState(EOOSPawnState::OOSPS_Stun);
						if (Opponent->MovementComponent->bOnGround)
						{
							Opponent->PlayAnim_NoBlend(Opponent->St_MidHeavy);
						}
						else
						{
							Opponent->PlayAnim_NoBlend(Opponent->Air_Hit);
							Opponent->MovementComponent->YSpeed = 4.f;
						}
						Opponent->Freeze(1.f, false, false);
						PrepareChildMove(CurrentNode);
						bCounter = true;
						return true;
					}
					break;

				case EOOSCounter::OOSCT_SuperProjectile:
					Opponent->SetPawnState(EOOSPawnState::OOSPS_Stun);
					if (Opponent->MovementComponent->bOnGround)
					{
						Opponent->PlayAnim_NoBlend(Opponent->St_MidHeavy);
					}
					else
					{
						Opponent->PlayAnim_NoBlend(Opponent->Air_Hit);
						Opponent->MovementComponent->YSpeed = 4.f;
					}
					Opponent->Freeze(1.f, false, false);
					PrepareChildMove(CurrentNode);
					bCounter = true;
					return true;
					break;

				}
			}
		}
	}

	if (SuperArmor == 0 && HyperArmor == 0 || bAntiArmor)
	{
		SuperArmor = 0;
		HyperArmor = 0;
		SetFlightModeEnabled(false);
		bIsPerformingMove = false;
		bIsPerformingSuper = false;
		MovementComponent->bUseXDecel = false;
		MovementComponent->bUseYDecel = false;
		if (LaunchType == EOOSLaunchType::OOSLT_None)
		{
			if (HitStun > 0.f)
			{
				if (MovementComponent->bOnGround)
				{
					if (PawnState == EOOSPawnState::OOSPS_Crouch)
					{
						if (Opponent->MovementComponent->bOnGround)
						{
							switch (HitHeight)
							{
							case EOOSHitHeight::OOSHH_High: case EOOSHitHeight::OOSHH_Mid:
								switch (AttackType)
								{
								case EOOSInputAttack::OOSIA_Light:
									PlayAnim_NoBlend(Cr_HighLight, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Medium:
									PlayAnim_NoBlend(Cr_HighMedium, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Heavy:
									PlayAnim_NoBlend(Cr_HighHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Special:
									PlayAnim_NoBlend(Cr_HighHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_EX:
									PlayAnim_NoBlend(Cr_HighHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Super:
									PlayAnim_NoBlend(Cr_HighHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Ultra:
									PlayAnim_NoBlend(Cr_HighHeavy, false, HitStun);
									break;
								}
								break;

							case EOOSHitHeight::OOSHH_Low:
								switch (AttackType)
								{
								case EOOSInputAttack::OOSIA_Light:
									PlayAnim_NoBlend(Cr_LowLight, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Medium:
									PlayAnim_NoBlend(Cr_LowMedium, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Heavy:
									PlayAnim_NoBlend(Cr_LowHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Special:
									PlayAnim_NoBlend(Cr_LowHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_EX:
									PlayAnim_NoBlend(Cr_LowHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Super:
									PlayAnim_NoBlend(Cr_LowHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Ultra:
									PlayAnim_NoBlend(Cr_LowHeavy, false, HitStun);
									break;
								}
							}
						}
						else
						{
							switch (AttackType)
							{
							case EOOSInputAttack::OOSIA_Light:
								PlayAnim_NoBlend(Cr_HighLight, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Medium:
								PlayAnim_NoBlend(Cr_HighMedium, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Heavy:
								PlayAnim_NoBlend(Cr_HighHeavy, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Special:
								PlayAnim_NoBlend(Cr_HighHeavy, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_EX:
								PlayAnim_NoBlend(Cr_HighHeavy, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Super:
								PlayAnim_NoBlend(Cr_HighHeavy, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Ultra:
								PlayAnim_NoBlend(Cr_HighHeavy, false, HitStun);
								break;
							}
						}
					}
					else if (PawnState == EOOSPawnState::OOSPS_Floored)
					{
						if (AnimationSequence == FloorBounceUpFar || AnimationSequence == FloorBounceUpMedium || AnimationSequence == FloorBounceUpShort || AnimationSequence == LyingUp)
						{
							switch (AttackType)
							{
							case EOOSInputAttack::OOSIA_Light:
								PlayAnim_NoBlend(FloorBounceUpShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Medium:
								PlayAnim_NoBlend(FloorBounceUpShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Heavy:
								PlayAnim_NoBlend(FloorBounceUpShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Special:
								PlayAnim_NoBlend(FloorBounceUpShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_EX:
								PlayAnim_NoBlend(FloorBounceUpShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Super:
								PlayAnim_NoBlend(FloorBounceUpShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Ultra:
								PlayAnim_NoBlend(FloorBounceUpShort, false, HitStun);
								break;
							}
						}
						else if (AnimationSequence == FloorBounceDownFar || AnimationSequence == FloorBounceDownMedium || AnimationSequence == FloorBounceDownShort || AnimationSequence == LyingDown)
						{
							switch (AttackType)
							{
							case EOOSInputAttack::OOSIA_Light:
								PlayAnim_NoBlend(FloorBounceDownShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Medium:
								PlayAnim_NoBlend(FloorBounceDownShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Heavy:
								PlayAnim_NoBlend(FloorBounceDownShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Special:
								PlayAnim_NoBlend(FloorBounceDownShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_EX:
								PlayAnim_NoBlend(FloorBounceDownShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Super:
								PlayAnim_NoBlend(FloorBounceDownShort, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Ultra:
								PlayAnim_NoBlend(FloorBounceDownShort, false, HitStun);
								break;
							}
						}
					}
					else
					{
						if (Opponent->MovementComponent->bOnGround)
						{
							switch (HitHeight)
							{
							case EOOSHitHeight::OOSHH_High:
								switch (AttackType)
								{
								case EOOSInputAttack::OOSIA_Light:
									PlayAnim_NoBlend(St_HighLight, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Medium:
									PlayAnim_NoBlend(St_HighMedium, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Heavy:
									PlayAnim_NoBlend(St_HighHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Special:
									PlayAnim_NoBlend(St_HighHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_EX:
									PlayAnim_NoBlend(St_HighHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Super:
									PlayAnim_NoBlend(St_HighHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Ultra:
									PlayAnim_NoBlend(St_HighHeavy, false, HitStun);
									break;
								}
								break;

							case EOOSHitHeight::OOSHH_Mid:
								switch (AttackType)
								{
								case EOOSInputAttack::OOSIA_Light:
									PlayAnim_NoBlend(St_MidLight, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Medium:
									PlayAnim_NoBlend(St_MidMedium, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Heavy:
									PlayAnim_NoBlend(St_MidHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Special:
									PlayAnim_NoBlend(St_MidHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_EX:
									PlayAnim_NoBlend(St_MidHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Super:
									PlayAnim_NoBlend(St_MidHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Ultra:
									PlayAnim_NoBlend(St_MidHeavy, false, HitStun);
									break;
								}
								break;

							case EOOSHitHeight::OOSHH_Low:
								switch (AttackType)
								{
								case EOOSInputAttack::OOSIA_Light:
									PlayAnim_NoBlend(St_LowLight, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Medium:
									PlayAnim_NoBlend(St_LowMedium, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Heavy:
									PlayAnim_NoBlend(St_LowHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Special:
									PlayAnim_NoBlend(St_LowHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_EX:
									PlayAnim_NoBlend(St_LowHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Super:
									PlayAnim_NoBlend(St_LowHeavy, false, HitStun);
									break;

								case EOOSInputAttack::OOSIA_Ultra:
									PlayAnim_NoBlend(St_LowHeavy, false, HitStun);
									break;
								}
								break;
							}
						}
						else
						{
							switch (AttackType)
							{
							case EOOSInputAttack::OOSIA_Light:
								PlayAnim_NoBlend(St_HighLight, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Medium:
								PlayAnim_NoBlend(St_HighMedium, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Heavy:
								PlayAnim_NoBlend(St_HighHeavy, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Special:
								PlayAnim_NoBlend(St_HighHeavy, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_EX:
								PlayAnim_NoBlend(St_HighHeavy, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Super:
								PlayAnim_NoBlend(St_HighHeavy, false, HitStun);
								break;

							case EOOSInputAttack::OOSIA_Ultra:
								PlayAnim_NoBlend(St_HighHeavy, false, HitStun);
								break;
							}
						}
					}
				}
				else
				{
					switch (AttackType)
					{
					case EOOSInputAttack::OOSIA_Light:
						PlayAnim_NoBlend(Air_Hit, false, HitStun);
						if (Opponent->MovementComponent->YSpeed > -4)
							break;

					case EOOSInputAttack::OOSIA_Medium:
						PlayAnim_NoBlend(Air_Hit, false, HitStun);
						if (Opponent->MovementComponent->YSpeed > -4)
							break;

					case EOOSInputAttack::OOSIA_Heavy:
						PlayAnim_NoBlend(Air_Hit, false, HitStun);
						if (Opponent->MovementComponent->YSpeed > -4)
							break;

					case EOOSInputAttack::OOSIA_Special:
						PlayAnim_NoBlend(Air_Hit, false, HitStun);
						if (Opponent->MovementComponent->YSpeed > -4)
							break;

					case EOOSInputAttack::OOSIA_EX:
						PlayAnim_NoBlend(Air_Hit, false, HitStun);
						break;

					case EOOSInputAttack::OOSIA_Super:
						PlayAnim_NoBlend(Air_Hit, false, HitStun);
						break;

					case EOOSInputAttack::OOSIA_Ultra:
						PlayAnim_NoBlend(Air_Hit, false, HitStun);
						break;
					}

					if (Opponent->MovementComponent->YSpeed > -4)
					{
						MovementComponent->XSpeed = LaunchSpeed.X;
						MovementComponent->YSpeed = LaunchSpeed.Y + (Opponent->MovementComponent->YSpeed / 2);
					}
					else
					{
						MovementComponent->XSpeed = LaunchSpeed.X;
						MovementComponent->YSpeed = LaunchSpeed.Y;
					}
				}
				if (PawnState != EOOSPawnState::OOSPS_Floored)
					SetPawnState(EOOSPawnState::OOSPS_Stun);
			}
		}
		else
		{
			if (PawnState == EOOSPawnState::OOSPS_Floored)
			{
				if (AnimationSequence == FloorBounceUpFar || FloorBounceUpMedium || FloorBounceUpShort || LyingUp)
					switch (LaunchType)
					{
					case EOOSLaunchType::OOSLT_UpFar:
						Launch(LaunchSpeed, LaunchType);
						break;

					case EOOSLaunchType::OOSLT_UpShort:
						Launch(LaunchSpeed, LaunchType);
						break;

					case EOOSLaunchType::OOSLT_BackFar:
						Launch(LaunchSpeed, LaunchType);
						break;

					case EOOSLaunchType::OOSLT_BackShort:
						Launch(LaunchSpeed, LaunchType);
						break;

					case EOOSLaunchType::OOSLT_Crumble:
						Launch(LaunchSpeed, EOOSLaunchType::OOSLT_DownFaceUp);
						break;

					case EOOSLaunchType::OOSLT_DownFaceDown:
						Launch(LaunchSpeed, EOOSLaunchType::OOSLT_DownFaceUp);
						break;

					case EOOSLaunchType::OOSLT_DownFaceUp:
						Launch(LaunchSpeed, EOOSLaunchType::OOSLT_DownFaceUp);
						break;

					case EOOSLaunchType::OOSLT_ForcedDown:
						Launch(LaunchSpeed, EOOSLaunchType::OOSLT_DownFaceUp);
						break;

					case EOOSLaunchType::OOSLT_ForcedUp:
						Launch(LaunchSpeed, EOOSLaunchType::OOSLT_DownFaceUp);
						break;

					case EOOSLaunchType::OOSLT_Spiral:
						Launch(LaunchSpeed, LaunchType);
						break;
					}
				else if (AnimationSequence == FloorBounceDownFar || FloorBounceDownMedium || FloorBounceDownShort || LyingDown)
					switch (LaunchType)
					{
					case EOOSLaunchType::OOSLT_UpFar:
						Launch(LaunchSpeed, LaunchType);
						break;

					case EOOSLaunchType::OOSLT_UpShort:
						Launch(LaunchSpeed, LaunchType);
						break;

					case EOOSLaunchType::OOSLT_BackFar:
						Launch(LaunchSpeed, LaunchType);
						break;

					case EOOSLaunchType::OOSLT_BackShort:
						Launch(LaunchSpeed, LaunchType);
						break;

					case EOOSLaunchType::OOSLT_Crumble:
						Launch(LaunchSpeed, EOOSLaunchType::OOSLT_DownFaceDown);
						break;

					case EOOSLaunchType::OOSLT_DownFaceDown:
						Launch(LaunchSpeed, EOOSLaunchType::OOSLT_DownFaceDown);
						break;

					case EOOSLaunchType::OOSLT_DownFaceUp:
						Launch(LaunchSpeed, EOOSLaunchType::OOSLT_DownFaceDown);
						break;

					case EOOSLaunchType::OOSLT_ForcedDown:
						Launch(LaunchSpeed, EOOSLaunchType::OOSLT_DownFaceDown);
						break;

					case EOOSLaunchType::OOSLT_ForcedUp:
						Launch(LaunchSpeed, EOOSLaunchType::OOSLT_DownFaceDown);
						break;

					case EOOSLaunchType::OOSLT_Spiral:
						Launch(LaunchSpeed, LaunchType);
						break;
					}
			}
			else
			{
				Launch(LaunchSpeed, LaunchType);
				SetFlightModeEnabled(false);
				MovementComponent->bUseXDecel = false;
				MovementComponent->bUseYDecel = false;
				MovementComponent->bUseGravity = true;
			}
		}

		// Immediately start up the animation so it's shown during hitstop
		//@TODO: Ignore this on trades, so that during hitstop both fighters are
		// in their attack pose, and it's clear what is happening
		if (true)
		{
			SkeletalMesh->TickAnimation(0.f, false);
			SkeletalMesh->RefreshBoneTransforms();
			SecondaryMesh->TickAnimation(0.f, false);
			SecondaryMesh->RefreshBoneTransforms();
		}
	}
	else
	{
		SuperArmor--;
	}
	// Being hit forces orientation
	//SetFacing(MovementComponent->bShouldFaceRight);


	if (Rings == 0)
	{
		LastDamage = Damage * 2;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("My rings are at 0 and I am taking a hit"));
	}
	else
	{
		LastDamage = Damage;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("My rings are NOT at 0 and I am taking a hit"));
	}

	Health = FMath::Clamp(Health - LastDamage, 0, MaxHealth);
	Regen = FMath::Clamp(Regen - (LastDamage / 2), 0, MaxHealth);
	if (LastDamage > 0)
		Opponent->HitCount++;

	if (bResetExtensions)
	{
		ResetKnockdowns();
	}
	bComboExtensionsReset = bResetExtensions;
	bForceComboExtension = bForceExtension;
	bSpiralFallCounted = false;

	if (PlayerIndex == 1)
	{
		if (Opponent->HitCount == 1)
		{
			GameMode->TotalDamage = 0;
		}

		GameMode->LastDamage = LastDamage;
		GameMode->TotalDamage += LastDamage;
		if (GameMode->TotalDamage > GameMode->MaxDamage) GameMode->MaxDamage = GameMode->TotalDamage;
		if (Opponent->HitCount > GameMode->MaxHits) GameMode->MaxHits = Opponent->HitCount;
		GameMode->DamageScale = Opponent->DamageScale;
	}

	SendToBackDepthLayer();

	CamShake(AttackType);

	OnPawnHit.Broadcast();
	return true; // Not blocked.
}

void AOOSPawn::CamShake(EOOSInputAttack AttackType, float Scale /*= 1.f*/)
{
	float ShakeScale = 1.25f;
	switch (AttackType)
	{
	case EOOSInputAttack::OOSIA_Light:
		ShakeScale = 0.25f;
		break;
	case EOOSInputAttack::OOSIA_Medium:
		ShakeScale = 0.5f;
		break;
	case EOOSInputAttack::OOSIA_Heavy:
		ShakeScale = 0.75f;
		break;
	case EOOSInputAttack::OOSIA_Special:
		ShakeScale = 1.f;
		break;
	}
	Camera->HitShake(ShakeScale * Scale);
}

void AOOSPawn::BringToFrontDepthLayer()
{
	
	if (bIsInFrontDepthLayer) return;
		
	TArray<UPrimitiveComponent*> PrimComps;
	GetComponents<UPrimitiveComponent>(PrimComps);
	for (UPrimitiveComponent* Comp : PrimComps)
	{
		Comp->SetCustomPrimitiveDataFloat(3, DepthOffset);
	}	
	bIsInFrontDepthLayer = true;

	if (Transformed) Transformed->BringToFrontDepthLayer();

	if (Opponent) Opponent->SendToBackDepthLayer();
	
}

void AOOSPawn::SendToBackDepthLayer()
{
	
	if (!bIsInFrontDepthLayer) return;

	TArray<UPrimitiveComponent*> PrimComps;
	GetComponents<UPrimitiveComponent>(PrimComps);
	for (UPrimitiveComponent* Comp : PrimComps)
	{
		Comp->SetCustomPrimitiveDataFloat(3, 0);
	}
	bIsInFrontDepthLayer = false;

	if (Transformed) Transformed->SendToBackDepthLayer();

	if (Opponent) Opponent->BringToFrontDepthLayer();
	
}

void AOOSPawn::Freeze(float Duration, bool bTwitch, bool bInInvincible)
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearAllTimersForObject(this); // Clear any hitlag timer, not only previous freezes.

	if (Duration >= 0.f)
	{
		World->GetTimerManager().SetTimer(FreezeTimer, this, &AOOSPawn::Unfreeze, Duration);
	}

	bFrozen = true;
	bFreezeTwitch = bTwitch;
	FreezeLocation = GetActorLocation();
	CustomTimeDilation = 0.00001f;
	SkeletalMesh->SetComponentTickEnabled(false);

	if (bInInvincible)
	{
		bFreezeInvincible = true;
	}

	for (TObjectIterator<UParticleSystemComponent> Comp; Comp; ++Comp)
	{
		bool bOwnedSystem = (Comp->GetTypedOuter(AOOSPawn::StaticClass()) == this);
		if (Comp->IsRegistered() && bOwnedSystem)
		{
			Comp->CustomTimeDilation = 0.0001f;
		}
	}

	for (TObjectIterator<UNiagaraComponent> Comp; Comp; ++Comp)
	{
		if (Comp->IsRegistered() && (Comp->GetTypedOuter(AOOSPawn::StaticClass()) == this))
		{
			Comp->SetPaused(true);
		}
	}
}

void AOOSPawn::Unfreeze()
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(FreezeTimer);

	bFrozen = false;
	bFreezeTwitch = false;
	CustomTimeDilation = 1.f;
	SkeletalMesh->SetComponentTickEnabled(IsHitStopOver());
	SecondaryMesh->SetComponentTickEnabled(IsHitStopOver());

	for (TObjectIterator<UParticleSystemComponent> Comp; Comp; ++Comp)
	{
		bool bOwnedSystem = (Comp->GetTypedOuter(AOOSPawn::StaticClass()) == this);
		if (Comp->IsRegistered() && bOwnedSystem)
		{
			Comp->CustomTimeDilation = 1.f;
		}
	}

	for (TObjectIterator<UNiagaraComponent> Comp; Comp; ++Comp)
	{
		if (Comp->IsRegistered() && (Comp->GetTypedOuter(AOOSPawn::StaticClass()) == this))
		{
			Comp->SetPaused(false);
		}
	}

	bFreezeInvincible = false;
}

void AOOSPawn::SetHitStop(float Value)
{
	HitStop = Value;

	// Pause the animation
	SkeletalMesh->bPauseAnims = true;
	SkeletalMesh->SetComponentTickEnabled(false);
	SecondaryMesh->bPauseAnims = true;
	SecondaryMesh->SetComponentTickEnabled(false);
	MovementComponent->SetComponentTickEnabled(false);

	for (TObjectIterator<UParticleSystemComponent> Comp; Comp; ++Comp)
	{
		bool bOwnedSystem = (Comp->GetTypedOuter(AOOSPawn::StaticClass()) == this);
		if (Comp->IsRegistered() && bOwnedSystem)
		{
			Comp->CustomTimeDilation = 0.0001f;
		}
	}

	for (TObjectIterator<UNiagaraComponent> Comp; Comp; ++Comp)
	{
		if (Comp->IsRegistered() && (Comp->GetTypedOuter(AOOSPawn::StaticClass()) == this))
		{
			Comp->SetPaused(true);
		}
	}

	for (TObjectIterator<AOOSProjectile> Prj; Prj; ++Prj)
	{
		if (Prj->GetOwner() == this)
		{
			Prj->StopAnim(Value);
		}
	}

	TryHitStopOver();

}

void AOOSPawn::TryHitStopOver()
{
	// If HitStop is over
	if (HitStop <= 0)
	{
		// Resume Animation
		SkeletalMesh->bPauseAnims = false;
		SkeletalMesh->SetComponentTickEnabled(true);
		SecondaryMesh->bPauseAnims = false;
		SecondaryMesh->SetComponentTickEnabled(true);
		MovementComponent->SetComponentTickEnabled(true);

		for (TObjectIterator<UParticleSystemComponent> Comp; Comp; ++Comp)
		{
			bool bOwnedSystem = (Comp->GetTypedOuter(AOOSPawn::StaticClass()) == this);
			if (Comp->IsRegistered() && bOwnedSystem)
			{
				Comp->CustomTimeDilation = 1.f;
			}
		}

		for (TObjectIterator<UNiagaraComponent> Comp; Comp; ++Comp)
		{
			if (Comp->IsRegistered() && (Comp->GetTypedOuter(AOOSPawn::StaticClass()) == this))
			{
				Comp->SetPaused(false);
			}
		}

		for (TObjectIterator<AOOSProjectile> Prj; Prj; ++Prj)
		{
			if (Prj->GetOwner() == this)
			{
				Prj->ResumeAnim();
			}
		}
		HitStop = 0.f;
	}
}

bool AOOSPawn::IsHitStopOver()
{
	return HitStop <= 0;
}

void AOOSPawn::Launch(FVector2D LaunchSpeed, EOOSLaunchType LaunchType)
{
	if (!MovementComponent || !Opponent) return;

	SetPawnState(EOOSPawnState::OOSPS_Launch);
	MovementComponent->bUseXDecel = false;
	MovementComponent->bUseYDecel = false;

	switch (LaunchType)
	{
	case EOOSLaunchType::OOSLT_None:
		break;

	case EOOSLaunchType::OOSLT_UpShort:
		PlayAnim_NoBlend(LaunchUpShort);
		break;

	case EOOSLaunchType::OOSLT_UpFar:
		PlayAnim_NoBlend(LaunchUpFar);
		break;

	case EOOSLaunchType::OOSLT_DownFaceDown:
		PlayAnim_NoBlend(LaunchDownFaceDown);
		break;

	case EOOSLaunchType::OOSLT_DownFaceUp:
		PlayAnim_NoBlend(LaunchDownFaceUp);
		break;

	case EOOSLaunchType::OOSLT_BackShort:
		PlayAnim_NoBlend(LaunchBackShort);
		break;

	case EOOSLaunchType::OOSLT_BackFar:
		PlayAnim_NoBlend(LaunchBackFar);
		break;

	case EOOSLaunchType::OOSLT_Crumble:
		if (MovementComponent->bOnGround == true)
		{
			PlayAnim_NoBlend(Crumble);
			LaunchSpeed.X = 0.f;
			LaunchSpeed.Y = 0.f;
		}
		else
			PlayAnim_NoBlend(LaunchDownFaceUp);
		break;

	case EOOSLaunchType::OOSLT_ForcedDown:
		PlayAnim_NoBlend(LaunchDownFaceDown);
		ResetKnockdowns();
		break;

	case EOOSLaunchType::OOSLT_ForcedUp:
		PlayAnim_NoBlend(LaunchDownFaceUp);
		ResetKnockdowns();
		break;

	case EOOSLaunchType::OOSLT_Spiral:
		PlayAnim_NoBlend(Spiral);
		break;
	}

	MovementComponent->bOnGround = false;
	MovementComponent->XSpeed = LaunchSpeed.X;
	MovementComponent->YSpeed = LaunchSpeed.Y;
	SetActorToFollow(this);

}

void AOOSPawn::TryPostLaunchJump(bool bForce)
{
	if (!MovementComponent || !GetController()) return;
	if (bIsPerformingSuper) return;
	if (FlyingCharacter)return;

	if (CurrentNode)
	{
		// Special and super moves may use the same launches as normal S attacks, so prevent super jumping out of them.
		if ((CurrentNode->Move.MoveType != EOOSMoveType::OOSMT_Normal) && !bForce) return;
	}

	UOOSFighterInputs* Inputs = GetFighterInputs();
	EOOSInputDir Dir = EOOSInputDir::OOSID_Up;
	if (Inputs) Dir = Inputs->DPadState;
	EOOSInputAttack Attack = EOOSInputAttack::OOSIA_None;
	if (Inputs) Attack = Inputs->AttackState;

	bool U = Dir == EOOSInputDir::OOSID_Up;
	bool UL = Dir == EOOSInputDir::OOSID_UpLeft;
	bool UR = Dir == EOOSInputDir::OOSID_UpRight;
	bool S = ((uint8)Attack >> 3) & 1;
	// Holding up or launcher
	if (U || UL || UR || S)
	{
		if (FlyingCharacter && !MovementComponent->bOnGround) return;
		SetPawnState(EOOSPawnState::OOSPS_SuperJump);
		MovementComponent->bOnGround = false;
		bool FR = MovementComponent->bIsFacingRight;
		bool bJumpFW = S || (FR ? UR : UL);
		bool bJumpU = !bJumpFW && U;

		float Scale = Opponent->MovementComponent->OverallMovementScale;
		float Grv = Opponent->MovementComponent->Gravity;
		float OppV_X = Opponent->MovementComponent->XSpeed;
		float OppV_Y = Opponent->MovementComponent->YSpeed;
		FVector OwnLoc = GetActorLocation() / Scale;
		FVector OppLoc = Opponent->GetActorLocation() / Scale;

		// Aim for a bit higher than opponent by pretending they're launched faster vertically.
		OppV_Y += 0.5f;

		// TODO: Might be possible to simplify by calculating dT first, not sure if it's worth it though.
		/* Calculate Y coord at opponent peak, then calculate Y delta for owner and solve for target vertical speed. */
		float YMax = OppLoc.Z + FMath::Square(OppV_Y) / (2 * Grv);
		float dY = YMax - OwnLoc.Z;
		float TargetY = FMath::Sqrt(dY * 2 * Grv);

		/* Do the same for the X component, but slightly different since movement is linear */
		float XMax = OppLoc.X + (OppV_X * OppV_Y) / Grv;
		float dX = XMax - OwnLoc.X;
		float TargetX = (dX * Grv) / OppV_Y;

		MovementComponent->XSpeed = TargetX * (bJumpU ? 0 : (bJumpFW ? 1 : -1));
		MovementComponent->YSpeed = TargetY;
		PlayAnim(SupJump);

		bIsPerformingMove = false;
	}
}

void AOOSPawn::AddSuperPts(int Points)
{
	SuperPts = FMath::Min(SuperPts + FMath::FloorToInt(Points * SuperGain), MAX_SUPER_POINTS_PER_LV * MAX_SUPER_LEVELS);
}

void AOOSPawn::Kill()
{
}

bool AOOSPawn::IgnoreDeathAnims() const
{
	return false;
}

int AOOSPawn::GetSuperLevel() const
{
	return SuperPts / MAX_SUPER_POINTS_PER_LV;
}

int AOOSPawn::GetCurrentSuperPoints() const
{
	if (SuperPts != (MAX_SUPER_POINTS_PER_LV * MAX_SUPER_LEVELS))
	{
		return SuperPts % MAX_SUPER_POINTS_PER_LV;
	}
	else
	{
		return MAX_SUPER_POINTS_PER_LV;
	}
}

void AOOSPawn::TrainingRefill()
{
	Health = MaxHealth;
	Regen = MaxHealth;
	Rings = GameInstance->TrainingRingsRegen;


	SuperPts = MAX_SUPER_POINTS_PER_LV * MAX_SUPER_LEVELS;

	Transform = MAX_TRANSFORM_POINTS;

	TransformLockout = 0.f;
	OnTrainingRefill();
}

void AOOSPawn::OnTrainingRefill_Implementation()
{

}

void AOOSPawn::TransformPressed()
{
	if (IsLaunched() || IsFloored() /*|| IsStunned()*/ || IsTransforming() || IsKO() || CanOnlyWalk() || IsTransformCoolingDown() || IsPerformingMove() || Opponent->IsPerformingUltra() || (Health <= ChargeDrainAmount) || bBlockBurst) return;

	// If this is a defensive burst
	bool InRecovery = IsStunned() || IsChargingTransform() || IsTurning() ||
		IsRecoveringOnGround() || IsRecoveringInAir();
	if (InRecovery || IsBlocking() || IsCrouchBlocking())
	{
		DefensiveTransform = true;
	}

	SetPawnState(EOOSPawnState::OOSPS_ChargeTransform);
	TransformChargeDrainTimer = 0.f;

	ChargeSoundLoop->Activate();
}

void AOOSPawn::TransformReleased()
{
	if (IsLaunched() || IsFloored() || IsTransforming() || IsKO() || CanOnlyWalk()) return;

	if (IsChargingTransform())
	{
		SetPawnState(EOOSPawnState::OOSPS_Idle);
	}
	TransformChargeDrainTimer = 0.f;
	ChargeSoundLoop->Deactivate();

	if (IsTransformReady()) StartTransformation();
}

void AOOSPawn::StartTransformation()
{
	if (IsLaunched() || IsFloored() || IsKO() || CanOnlyWalk() || !IsTransformReady() || IsPerformingUltra() || bIsPerformingSuper || Opponent->IsPerformingUltra() || bBlockBurst) return;

	SetPawnState(EOOSPawnState::OOSPS_Transform);
	if (bHasTransformed)
	{
		PlayAnim(Transformation);
	}
	else
	{
		PlayAnim(Transformation_Cinematic);
		bHasTransformed = true;
	}
}

void AOOSPawn::FinishTransformation()
{
	if (IsLaunched() || IsFloored() || IsStunned() || IsKO() || CanOnlyWalk() || !IsTransformReady()) return;

	SwapPawns();
}

void AOOSPawn::SwapPawns()
{
	UWorld* World = GetWorld();
	if (!World) return;

	AOOSGameMode* InGameMode = Cast<AOOSGameMode>(UGameplayStatics::GetGameMode(World));

	if (!GetController() || !MovementComponent || !InGameMode) return;

	FVector ThisPos = GetActorLocation();
	FVector TransformPos = Transformed->GetActorLocation();

	EOOSInputDir DPad;
	UOOSFighterInputs* Inputs = GetFighterInputs();
	if (Inputs) DPad = Inputs->DPadState;
	AController* InController = GetController();
	InController->UnPossess();
	InController->Possess(Transformed);
	UOOSFighterInputs* TransformedInputs = Transformed->GetFighterInputs();
	if (TransformedInputs) TransformedInputs->DPadState = DPad;

	Transformed->SetActorLocation(ThisPos);

	Opponent->Opponent = Transformed;

	switch (PlayerIndex)
	{
	case 0:
		Camera->SetP1(Transformed);
		InGameMode->Fighter1 = Transformed;
		break;

	case 1:
		Camera->SetP2(Transformed);
		InGameMode->Fighter2 = Transformed;
		break;
	}

	Transformed->SetActorHiddenInGame(false);
	Transformed->SetActorTickEnabled(true);
	Transformed->MovementComponent->SetComponentTickEnabled(true);
	Transformed->Opponent = Opponent;
	Transformed->MaxHealth = MaxHealth;
	Transformed->Health = Health;
	Transformed->Regen = Regen;
	Transformed->SuperPts = SuperPts;
	Transformed->Transform = Transform;
	Transformed->HitCount = HitCount;
	Transformed->DamageScale = DamageScale;
	Transformed->HitStunScale = HitStunScale;
	Transformed->bDefeated = bDefeated;
	Transformed->bDefeatWaitForIdle = bDefeatWaitForIdle;
	Transformed->MovementComponent->bOnGround = MovementComponent->bOnGround;
	Transformed->SetFacing(MovementComponent->bIsFacingRight);
	Transformed->MovementComponent->bShouldFaceRight = MovementComponent->bShouldFaceRight;
	Transformed->MovementComponent->XSpeed = MovementComponent->XSpeed;
	Transformed->MovementComponent->YSpeed = MovementComponent->YSpeed;
	Transformed->SetPawnState(EOOSPawnState::OOSPS_Idle);

	// Send Pawn out of stage.
	SetActorLocation(TransformPos);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	MovementComponent->SetComponentTickEnabled(false);
}

void AOOSPawn::RefillTransform()
{
	Transform = MAX_TRANSFORM_POINTS;
}
