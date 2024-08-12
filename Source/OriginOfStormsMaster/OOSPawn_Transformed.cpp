// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSPawn_Transformed.h"
#include "OOSGameMode.h"
#include "OOSMovementComponent.h"
#include "OOSCamera.h"
#include "OOSAnimNotify_Movement.h"
#include "Engine/World.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

void AOOSPawn_Transformed::Tick(float DeltaTime)
{	
	if (!IsTransforming())
	{
		// Transform pts draining and HP recover.
		if (TransformDrainTimer < TransformDrainInterval)
		{
			TransformDrainTimer += DeltaTime;
		}
		else
		{
			TransformDrainTimer = 0.f;
			Health += HealthRegenAmount;
			Health = FMath::Clamp(Health, 0, Regen);
		}

		if (Transform <= 0)
		{
			Transform = 0;
			QuickTransformation();
		}
	}

	AirSpecials = 0;
	AirEXs = 0;

	// Call parent after checking xform gauge so we can detransform before dying (post-GuardianCon stream fix).
	Super::Tick(DeltaTime);

}


// Override transform button methods from parent.
void AOOSPawn_Transformed::TransformPressed()
{
	if (IsLaunched() || IsFloored() || IsStunned() || IsTransforming() || IsKO() || CanOnlyWalk()) return;
}

void AOOSPawn_Transformed::TransformReleased()
{
	if (IsLaunched() || IsFloored() || IsStunned() || IsTransforming() || IsKO() || CanOnlyWalk() || IsPerformingUltra() || bIsPerformingSuper || Opponent->IsPerformingUltra()) return;

	QuickTransformation(true);
}

void AOOSPawn_Transformed::StartTransformation()
{
	if (IsLaunched() || IsFloored() || IsStunned() || IsKO() || CanOnlyWalk()) return;

	PawnState = EOOSPawnState::OOSPS_Transform;
}

void AOOSPawn_Transformed::FinishTransformation()
{
	if (IsLaunched() || IsFloored() || IsStunned() || IsKO() || CanOnlyWalk()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AOOSGameMode* InGameMode = Cast<AOOSGameMode>(UGameplayStatics::GetGameMode(World));


	if (!GetController() || !InGameMode) return;

	// @Jaime play transform FX here.

	FVector ThisPos = GetActorLocation();
	FVector NormalPos = Normal->GetActorLocation();

	UOOSFighterInputs* Inputs = GetFighterInputs();
	EOOSInputDir DPad;
	if (Inputs) DPad = Inputs->DPadState;
	AController* InController = GetController();

	InController->UnPossess();
	InController->Possess(Normal);
	Normal->SetActorLocation(ThisPos);

	UOOSFighterInputs* NormalInputs = Normal->GetFighterInputs();
	if (NormalInputs) NormalInputs->DPadState = DPad;

	Opponent->Opponent = Normal;

	switch (PlayerIndex)
	{
	case 0:
		Camera->SetP1(Normal);
		InGameMode->Fighter1 = Normal;
		break;

	case 1:
		Camera->SetP2(Normal);
		InGameMode->Fighter2 = Normal;
		break;
	}

	Normal->SetActorHiddenInGame(false);
	Normal->SetActorTickEnabled(true);
	Normal->MovementComponent->SetComponentTickEnabled(true);
	Normal->Opponent = Opponent;
	Normal->MaxHealth = MaxHealth;
	Normal->Health = Health;
	Normal->Regen = Regen;
	Normal->SuperPts = SuperPts;
	Normal->Transform = Transform;
	Normal->MovementComponent->bOnGround = MovementComponent->bOnGround;
	Normal->SetFacing(MovementComponent->bIsFacingRight);
	Normal->MovementComponent->bShouldFaceRight = MovementComponent->bShouldFaceRight;
	Normal->MovementComponent->XSpeed = MovementComponent->XSpeed;
	Normal->MovementComponent->YSpeed = MovementComponent->YSpeed;
	Normal->bIsPerformingSuper = false;
	Normal->SetPawnState(EOOSPawnState::OOSPS_Idle);

	// Send transformed out of stage	
	SetActorLocation(NormalPos);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	MovementComponent->SetComponentTickEnabled(false);
	
}

void AOOSPawn_Transformed::QuickTransformation(bool bManual)
{
	UWorld* World = GetWorld();
	if (!World) return;

	AOOSGameMode* InGameMode = Cast<AOOSGameMode>(UGameplayStatics::GetGameMode(World));

	if (!GetController() || !InGameMode) return;

	if (bManual)
	{
		Transform = FMath::Max(Transform - (MAX_TRANSFORM_POINTS / 4), 0);
	}

	if (DetransformParticle)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DetransformParticle, GetActorLocation(), GetActorRotation(), true);

	if (DetransformSound)
		UGameplayStatics::PlaySound2D(GetWorld(), DetransformSound);

	FVector ThisPos = GetActorLocation();
	FVector NormalPos = Normal->GetActorLocation();

	UOOSFighterInputs* Inputs = GetFighterInputs();
	EOOSInputDir DPad;
	if (Inputs) DPad = Inputs->DPadState;
	AController* In = GetController();
	In->UnPossess();
	In->Possess(Normal);
	UOOSFighterInputs* NormalInputs = Normal->GetFighterInputs();
	if (NormalInputs) NormalInputs->DPadState = DPad;

	float HeightDiff = Capsule->GetScaledCapsuleHalfHeight() - Normal->Capsule->GetScaledCapsuleHalfHeight();
	Normal->SetActorLocation(ThisPos + (-HeightDiff * FVector::UpVector));

	Opponent->Opponent = Normal;

	switch (PlayerIndex)
	{
	case 0:
		Camera->SetP1(Normal);
		InGameMode->Fighter1 = Normal;
		break;

	case 1:
		Camera->SetP2(Normal);
		InGameMode->Fighter2 = Normal;
		break;
	}

	Normal->SetActorHiddenInGame(false);
	Normal->MovementComponent->SetComponentTickEnabled(true);
	Normal->SetActorTickEnabled(true);
	Normal->Opponent = Opponent;
	Normal->MaxHealth = MaxHealth;
	Normal->Health = Health;
	Normal->Regen = Regen;
	Normal->SuperPts = SuperPts;
	Normal->Transform = Transform;
	Normal->HitCount = HitCount;
	Normal->DamageScale = DamageScale;
	Normal->HitStunScale = HitStunScale;
	Normal->MovementComponent->bOnGround = MovementComponent->bOnGround;
	Normal->SetFacing(MovementComponent->bIsFacingRight);
	Normal->MovementComponent->bShouldFaceRight = MovementComponent->bShouldFaceRight;
	Normal->MovementComponent->XSpeed = MovementComponent->XSpeed;
	Normal->MovementComponent->YSpeed = MovementComponent->YSpeed;
	Normal->TransitionSequence = AnimationSequence;
	if (bManual)
	{
		// Stop this transformed pawn.
		if (Inputs) Inputs->MoveBuffer.Empty();
		SetPawnState(EOOSPawnState::OOSPS_Idle);

		// Stop the normal pawn.
		if(NormalInputs) NormalInputs->MoveBuffer.Empty();
		Normal->SetPawnState(EOOSPawnState::OOSPS_Idle);

		Normal->LastDetransform = UGameplayStatics::GetTimeSeconds(World);
	}
	else
	{
		Normal->SetPawnState(PawnState);
		if (AnimationSequence)
		{
			Normal->PlayAnim(Normal->TransitionSequence);
		}
	}

	// Send transformed out of stage	
	SetActorLocation(NormalPos);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	MovementComponent->SetComponentTickEnabled(false);
}


