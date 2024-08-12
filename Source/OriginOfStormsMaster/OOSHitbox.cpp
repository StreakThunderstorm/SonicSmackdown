// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSHitbox.h"
#include "OOSPawn_Transformed.h"
#include "OOSProjectile.h"
#include "OOSMovementComponent.h"
#include "OOSAnimNotify_Hitbox.h"
#include "OOSPawn_Burst.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "SkeletalAnimation/OOSSkeletalMeshComponent.h"
#include "Engine.h"

UOOSHitbox::UOOSHitbox()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(true);

	SetCollisionObjectType(HITBOX_OBJ);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SetCollisionResponseToChannel(HITBOX_OBJ, ECollisionResponse::ECR_Overlap);
	SetCollisionResponseToChannel(PROJECTILE_OBJ, ECollisionResponse::ECR_Overlap);

	HuBList.Empty();

	FScriptDelegate BeginDelegate;
	FScriptDelegate EndDelegate;
	BeginDelegate.BindUFunction(this, TEXT("ReceiveHBBeginOverlaps"));
	EndDelegate.BindUFunction(this, TEXT("ReceiveHBEndOverlaps"));
	OnComponentBeginOverlap.Add(BeginDelegate);
	OnComponentEndOverlap.Add(EndDelegate);

}

void UOOSHitbox::Initialize
(
	float HalfHeight,
	float Radius,
	bool bGrab,
	bool bForceGrb,
	FName GrabSock,
	UParticleSystem* HitParticle,
	USoundBase* HitSFX,
	bool bBrst,
	int Dmg,
	float HStun,
	float SelfHL,
	float OpponentHL,
	float BStun,
	float PushFactor,
	float Frz,
	bool FrzTw,
	bool bOTG,
	bool bOH,
	bool bAutoChild,
	bool bNoCancel,
	bool bUnBlck,
	bool bAntiA,
	bool bAntiPrj,
	enum EOOSHitHeight Height, 
	enum EOOSInputAttack Att, 
	enum EOOSLaunchType Launch,
	bool bForceExtension,
	bool bResetExtensions,
	enum EOOSDirectionMode Direction, 
	FVector2D LaunchSpd, 
	bool bForceJmp
)
{
	// Pass half height and radius below zero to keep initial hitbox dimensions.
	if (HalfHeight >= 0.f) SetCapsuleHalfHeight(HalfHeight);
	if (Radius >= 0.f) SetCapsuleRadius(Radius);

	bGrabMode = bGrab;
	bForceGrab = bForceGrb;
	GrabSocket = GrabSock;

	HitEffect = HitParticle;
	HitSound = HitSFX;

	bBurst = bBrst;
	Damage = Dmg;
	HitStun = HStun;
	BlockStun = BStun;
	SelfHitLag = SelfHL;
	OpponentHitLag = OpponentHL;
	Push = PushFactor;

	Freeze = Frz;
	FreezeTwitch = FrzTw;
	bOffTheGround = bOTG;
	bOverhead = bOH;
	bAutoPerformChildMove = bAutoChild;
	bDontCancel = bNoCancel;
	bUnblockable = bUnBlck;
	bAntiArmor = bAntiA;
	bAntiProjectile = bAntiPrj;
	HitHeight = Height;
	Attack = Att;

	LaunchType = Launch;
	bForceComboExtension = bForceExtension;
	bResetComboExtensions = bResetExtensions;
	DirectionMode = Direction;
	LaunchSpeed = LaunchSpd;	
	bForceTryPostLaunchJump = bForceJmp;

	// Get the AOOSPawn owning this Hitbox. If it fails (meaning that the Hitbox is attached to an external Actor), try and get its owner again.
	AActor* Owner = GetOwner();
	if (Owner)
	{
		FPOwner = Cast<AOOSPawn>(Owner);
		if (!FPOwner)
		{
			// If owner of the hitbox is not a OOSPawn, try OOSProjectile
			AOOSProjectile* POwner = Cast<AOOSProjectile>(Owner);
			if (POwner)
			{
				FPOwner = Cast<AOOSPawn>(POwner->PawnOwner);
				bIsProjectile = true;
			}
			else
			{
				// Try luck one more time, we could be attached to other type of minion actor that can spawn hitboxes.
				FPOwner = Cast<AOOSPawn>(Owner->GetOwner());
			}
		}
	}
}

void UOOSHitbox::BeginPlay()
{
	Super::BeginPlay();

}

void UOOSHitbox::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Debug
	/*if (bDebug && FPOwner)
	{
		for (int i = 0; i < HuBList.Num(); i++)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FString(HuBList[i].OtherHB->GetName()));
			DrawDebugPoint(GetWorld(), HuBList[i].ImpactPoint, 5.f, FColor::Red);
		}
		GEngine->AddOnScreenDebugMessage(-1, 0, bIsMakingContact ? FColor::Red : FColor::Magenta, FString("Hitbox").Append(GetName()).Append(" owner: Player ").Append(FString::FromInt(FPOwner->PlayerIndex)));
	}*/
	if (bIsProjectile)
	{
		AOOSProjectile* Prj = Cast<AOOSProjectile>(GetOwner());
		if (Prj->bPawnVelocityLaunch && Prj->GetOwner() == FPOwner)
		{
			LaunchSpeed = (FPOwner->MovementComponent->bIsFacingRight ? FVector2D(FPOwner->MovementComponent->XSpeed, FPOwner->MovementComponent->YSpeed) : FVector2D(FPOwner->MovementComponent->XSpeed * -1.f, FPOwner->MovementComponent->YSpeed));
		}

		else if (Prj->bPrjXLaunch && !Prj->bPrjYLaunch && Prj->GetOwner() == FPOwner)
		{
			LaunchSpeed = FVector2D((Prj->Speed.X/100.f) * Prj->PrjXLaunchMultiplier, Prj->Hitbox->LaunchSpeed.Y);
			Push *= FMath::Sign(Prj->Speed.X);
		}

		else if (Prj->bPrjYLaunch && !Prj->bPrjXLaunch && Prj->GetOwner() == FPOwner)
		{
			LaunchSpeed = FVector2D(Prj->Hitbox->LaunchSpeed.X, (Prj->Speed.Y/100.f)* Prj->PrjYLaunchMultiplier);
		}

		else if (Prj->bPrjXLaunch && Prj->bPrjYLaunch && Prj->GetOwner() == FPOwner)
		{
			LaunchSpeed = FVector2D((Prj->Speed.X/100.f)*Prj->PrjXLaunchMultiplier, (Prj->Speed.Y/100.f)*Prj->PrjYLaunchMultiplier);
			Push *= FMath::Sign(Prj->Speed.X);
		}
	}

	MainLoop(DeltaTime);
	
}

void UOOSHitbox::MainLoop(float DeltaTime)
{
	if (bIsMakingContact && bEnabled)
	{
		if (!FPOwner || !FPOwner->Opponent || FPOwner->Opponent->IsInvincible() || FPOwner->Opponent->WasKilled() || (!bOffTheGround && FPOwner->Opponent->IsFloored()))
		{
			return;
		}

		if (!bHasMadeContact)
		{
			TryHit(OverlapInfo);
		}

		if (bHitConfirm || bForceGrab)
		{
			if (bGrabMode)
			{
				if (FPOwner->Opponent->DefensiveTransform)
				{
					FPOwner->Opponent->bGrabbed = false;
					DestroyHitbox();
					return;
				}
				FPOwner->MovementComponent->bDoNotPush = true;
				FPOwner->Opponent->MovementComponent->bDoNotPush = true;
				FPOwner->Opponent->bGrabbed = true;
				GrabOpponent();
				return; // Skip gravitational pull.
			}
		}

		if (bHitConfirm)
		{
			if (DirectionMode == EOOSDirectionMode::OOSDM_Gravitational && (Push < 0.f))
			{
				if (FPOwner->Opponent->DefensiveTransform)
				{
					FPOwner->Opponent->bGrabbed = false;
					DestroyHitbox();
					return;
				}
				FPOwner->MovementComponent->bDoNotPush = true;
				FPOwner->Opponent->MovementComponent->bDoNotPush = true;
				FPOwner->Opponent->MovementComponent->bOnGround = false;
				FPOwner->Opponent->MovementComponent->bUseGravity = false;
				FPOwner->Opponent->bGrabbed = true;
				GrabOpponent(true, true, DeltaTime, FMath::Abs(Push));
			}
		}
	}
	else
	{
		if (bHitConfirm)
		{
			if (DirectionMode == EOOSDirectionMode::OOSDM_Gravitational && (Push < 0.f))
			{

				FPOwner->MovementComponent->bDoNotPush = false;
				FPOwner->Opponent->MovementComponent->bDoNotPush = false;
				FPOwner->Opponent->MovementComponent->bUseGravity = true;
				FPOwner->Opponent->MovementComponent->bUseXDecel = true;

			}
		}
	}
}

float UOOSHitbox::GetDirection()
{
	float Direction = 0.f;

	switch (DirectionMode)
	{
	case EOOSDirectionMode::OOSDM_Directional:
		Direction = FPOwner->MovementComponent->bIsFacingRight ? 1.f : -1.f;
		break;

	case EOOSDirectionMode::OOSDM_TwoSided:
		Direction = FMath::Sign((FPOwner->Opponent->GetActorLocation() - GetComponentLocation()).GetSafeNormal() | FPOwner->GetActorForwardVector());
		break;

	case EOOSDirectionMode::OOSDM_Gravitational:
		Direction = 0.f;
		break;
	}

	return Direction;
}

void UOOSHitbox::TryHit(FOOSOverlapInfo InOverlapInfo)
{
	if (FPOwner->Opponent->IsTransforming() || (Damage == 0 && bAntiProjectile && !bBurst && !bAutoPerformChildMove) || (FPOwner->GrabType == EOOSGrab::OOSGR_Air && FPOwner->Opponent->MovementComponent->bOnGround) || (FPOwner->GrabType == EOOSGrab::OOSGR_Ground && !FPOwner->Opponent->MovementComponent->bOnGround))
	{
		return;
	}

	bool AlreadyMadeContact = FPOwner->HitCount > 0 && FPOwner->bHasMadeContact; // If the owning Pawn already hit once this move
	bHasMadeContact = true;
	FPOwner->bHasMadeContact = true; // Inform the owning Pawn about the hit for frame data
	FPOwner->bHasLandedAttack = !bDontCancel; // Inform the owning Pawn about the hit to enable cancel

	// Hit routine
	FVector Dir = (FPOwner->Opponent->GetActorLocation() - GetComponentLocation()).GetSafeNormal();
	FPOwner->GetActorRotation().UnrotateVector(Dir);
	GravitationalDirection.X = Dir.X;
	GravitationalDirection.Y = Dir.Z;

	float Direction = GetDirection();

	bool FaceRight;
	if (DirectionMode == EOOSDirectionMode::OOSDM_Directional)
	{
		// Being hit forces orientation
		FaceRight = FPOwner->Opponent->MovementComponent->bIsFacingRight;
		if (FPOwner->MovementComponent->bIsFacingRight && LaunchSpeed.X > 0)
			FaceRight = false;
		else if (FPOwner->MovementComponent->bIsFacingRight && LaunchSpeed.X < 0)
			FaceRight = true;
		else if (!FPOwner->MovementComponent->bIsFacingRight && LaunchSpeed.X < 0)
			FaceRight = false;
		else if (!FPOwner->MovementComponent->bIsFacingRight && LaunchSpeed.X > 0)
			FaceRight = true;
		FPOwner->Opponent->SetFacing(FaceRight);
	}

	// Push opponent
	if (!bGrabMode)
	{
		if (!FPOwner->Opponent->MovementComponent->ApplyPushImpulse(Push * Direction) && !bIsProjectile)
		{
			FPOwner->MovementComponent->ApplyPushImpulse(Push * -Direction);
		}
	}

	// Get transform pts multiplier
	// At the time of implementation, it should be the same on both pawns, 
	// but we'll prepare for future differential scaling just in case.
	float TransformGain_Own = FPOwner->TransformGain;
	float TransformGain_Opp = FPOwner->Opponent->TransformGain;
	// Also we can disable gaining burst, but never super points!
	float SuperGain_Own = FPOwner->SuperGain;
	float SuperGain_Opp = FPOwner->Opponent->SuperGain;

	if (FPOwner->Opponent->IsChargingTransform())
	{
		// Super/transform earn and pay.
		FPOwner->SuperPts = FMath::Min(FPOwner->SuperPts + FMath::FloorToInt(Damage * 2 * SuperGain_Own), MAX_SUPER_POINTS_PER_LV * 5);
		FPOwner->Opponent->SuperPts = FMath::Min(FPOwner->Opponent->SuperPts + FMath::FloorToInt(Damage * 1.4f * SuperGain_Opp), MAX_SUPER_POINTS_PER_LV * 5);

		FPOwner->Transform = FMath::Min(FPOwner->Transform + FMath::FloorToInt(Damage * 0.125f * TransformGain_Own), MAX_TRANSFORM_POINTS);
		FPOwner->Opponent->Transform = FMath::Min(FPOwner->Opponent->Transform + FMath::FloorToInt(Damage * 0.25f * TransformGain_Opp), MAX_TRANSFORM_POINTS);

		HitLag();

		// Charging hit FX.
		if (FPOwner->Opponent->TransformAbsorbParticle)
			UGameplayStatics::SpawnEmitterAtLocation(FPOwner->GetWorld(), FPOwner->Opponent->TransformAbsorbParticle, InOverlapInfo.ImpactPoint, FRotator::ZeroRotator, true);

		if (FPOwner->Opponent->TransformAbsorbSound)
			UGameplayStatics::PlaySound2D(FPOwner->GetWorld(), FPOwner->Opponent->TransformAbsorbSound);
	}
	else
	{
		if (bBurst)
		{
			//@TODO: reduce damage scale on defensive bursts?
			if (FPOwner->DefensiveTransform)
			{
				FPOwner->Opponent->bGrabbed = false;
			}
		}
		else if (FPOwner->HitCount <= 0)
		{
			switch (Attack)
			{
			case EOOSInputAttack::OOSIA_Super:
			case EOOSInputAttack::OOSIA_Ultra:
				FPOwner->DamageScale = 1.f;
				break;

				// Increased damage for the first hit for non-supers/ultras
			default:
				FPOwner->DamageScale = COMBO_FIRST_HIT_DAMAGE;
				break;
			}

			FPOwner->HitStunScale = 1.f;
		}
		else
		{
			// Lower damage back to baseline after first hit
			FPOwner->DamageScale = FMath::Min(1.f, FPOwner->DamageScale);

			float DamageScaleDegradation = 1.f;
			float HitStunScaleDegradation = 1.f;

			switch (Attack)
			{
			case EOOSInputAttack::OOSIA_Light:
				DamageScaleDegradation = 0.9f;
				HitStunScaleDegradation = 0.9f;
				break;

			case EOOSInputAttack::OOSIA_Medium:
				DamageScaleDegradation = 0.92f;
				HitStunScaleDegradation = 0.92f;
				break;

			case EOOSInputAttack::OOSIA_Heavy:
				DamageScaleDegradation = 0.94f;
				HitStunScaleDegradation = 0.94f;
				break;

			case EOOSInputAttack::OOSIA_Special:
				DamageScaleDegradation = 0.95f;
				HitStunScaleDegradation = 0.95f;
				break;

			case EOOSInputAttack::OOSIA_EX:
				DamageScaleDegradation = 0.96f;
				HitStunScaleDegradation = 0.96f;
				break;

			case EOOSInputAttack::OOSIA_Super:
				DamageScaleDegradation = 0.98f;
				HitStunScaleDegradation = 0.98f;
				break;

			case EOOSInputAttack::OOSIA_Ultra:
				DamageScaleDegradation = 1.f;
				HitStunScaleDegradation = 1.f;
				break;
			}

			// Halve hitstun scale effects
			HitStunScaleDegradation = FMath::Lerp(HitStunScaleDegradation, 1.f, 0.5f);
			// If this isn't the first hit of this move, reduce the effect more
			if (AlreadyMadeContact)
				HitStunScaleDegradation = FMath::Lerp(HitStunScaleDegradation, 1.f, 0.67f);

			FPOwner->DamageScale = FMath::Max(MINIMUM_DAMAGE_SCALE, FPOwner->DamageScale * DamageScaleDegradation);
			FPOwner->HitStunScale = FMath::Max(MINIMUM_HITSTUN_SCALE, FPOwner->HitStunScale * HitStunScaleDegradation);
		}

		float CurrentHitDamageScale = FPOwner->DamageScale;
		//CurrentHitDamageScale = MINIMUM_DAMAGE_SCALE; // for testing damage during long combos
		float CurrentHitStunScale = FPOwner->HitStunScale;

		if (Attack == EOOSInputAttack::OOSIA_Ultra)
		{
			const float Alpha = 1.f;
			CurrentHitDamageScale = FMath::Lerp(CurrentHitDamageScale, 1.f, Alpha);
			CurrentHitStunScale = 1.f; // Use unscaled hitstun for ultras so they don't drop randomly
		}

		FVector2D LaunchSpd = LaunchSpeed;
		LaunchSpd.X *= Direction;
		EOOSLaunchType Launch = LaunchType;
		if ((DirectionMode == EOOSDirectionMode::OOSDM_Gravitational) && (Push > 0.f))
		{
			if (Push > 0.f)
			{
				LaunchSpd = GravitationalDirection * Push;
			}
			else
			{
				Launch = EOOSLaunchType::OOSLT_None;
			}
		}

		float HitDamage = Damage * CurrentHitDamageScale;
		float ChipDamage = Damage * 0.3f;

		bool bCounter = false;
		bool bIsSuper = false;

		AOOSProjectile* Prj = Cast<AOOSProjectile>(GetOwner());
		if(Prj)
		{
			if (Prj->bIsSuper)
				bIsSuper = true;
		}
		bHitConfirm = FPOwner->Opponent->GetHit
		(
			HitHeight,
			Attack,
			bUnblockable,
			bOverhead,
			Launch,
			LaunchSpd,
			bForceComboExtension,
			bResetComboExtensions,
			bAntiArmor,
			HitDamage,
			ChipDamage,
			HitStun * CurrentHitStunScale,
			BlockStun,
			bIsProjectile,
			bCounter,
			bIsSuper
		);

		if (bHitConfirm)
		{
			if (bBurst)
			{
				if (!FPOwner->DefensiveTransform)
				{
					// Reset hit stun scale halfway
					FPOwner->HitStunScale = FMath::Lerp(FPOwner->HitStunScale, 1.f, 0.5f);
					// Prevent opponent from bursting for a short time
					FPOwner->Opponent->LockoutTransform();
					FPOwner->Opponent->bIsPerformingMove = false;
					FPOwner->Opponent->bGrabbed = false;
					FPOwner->Opponent->Unfreeze();
				}
				else		
				{
					AOOSPawn_Burst* PB = Cast<AOOSPawn_Burst>(FPOwner->Opponent);
					if (PB)
					{
						int RingsToDrop = PB->Rings;
						RingsToDrop = FMath::Clamp(RingsToDrop, 0, 10);
						PB->ScatterRings(RingsToDrop);
						PB->Rings = PB->Rings - RingsToDrop;
					}

					FPOwner->Opponent->Launch(FVector2D(FPOwner->MovementComponent->bIsFacingRight ? 2.f : -2.f,4.f), EOOSLaunchType::OOSLT_BackShort);
					FPOwner->Opponent->bIsPerformingMove = false;
					FPOwner->Opponent->bGrabbed = false;
					FPOwner->Opponent->Unfreeze();
				}
			}

			// Super/transform earn and pay.
			if ((Attack != EOOSInputAttack::OOSIA_EX) && (Attack != EOOSInputAttack::OOSIA_Super) && (Attack != EOOSInputAttack::OOSIA_Ultra) && (!FPOwner->bMeterBlocked))
			{
				FPOwner->SuperPts = FMath::Min(FPOwner->SuperPts + FMath::FloorToInt(Damage * 1.5 * SuperGain_Own), MAX_SUPER_POINTS_PER_LV * 5);
			}

			FPOwner->Opponent->SuperPts = FMath::Min(FPOwner->Opponent->SuperPts + FMath::FloorToInt(Damage * 1.25f * SuperGain_Opp), MAX_SUPER_POINTS_PER_LV * 5);

			SetTransformOnHit(false, Damage, TransformGain_Own, TransformGain_Opp);

			if (FPOwner->Opponent->IsFrozen())
			{
				FPOwner->Opponent->Unfreeze();
			}
			// Freeze hit.
			if (Freeze > 0.f)
			{
				FPOwner->Opponent->Freeze(Freeze, FreezeTwitch);
			}
			else
				// Regular hitlag.
			{
				HitLag();
			}

			if (bCounter)
			{
				if(FPOwner->TechParticle)
					UGameplayStatics::SpawnEmitterAtLocation(FPOwner->GetWorld(), FPOwner->TechParticle, InOverlapInfo.ImpactPoint, FRotator::ZeroRotator, true);

				if(FPOwner->TechSound)
					UGameplayStatics::PlaySound2D(FPOwner->GetWorld(), FPOwner->TechSound);
			}
			else
			{
				// Non blocked hit FX
				if (HitEffect)
					UGameplayStatics::SpawnEmitterAtLocation(FPOwner->GetWorld(), HitEffect, InOverlapInfo.ImpactPoint, FRotator::ZeroRotator, true);

				if (HitSound)
					UGameplayStatics::PlaySound2D(FPOwner->GetWorld(), HitSound);
			}

			// Perform child move.
			if (bAutoPerformChildMove && FPOwner->CurrentNode)
			{
				FPOwner->PrepareChildMove(FPOwner->CurrentNode);
			}

		}
		else
		{

			// Super/transform earn and pay.
			if ((Attack != EOOSInputAttack::OOSIA_EX) && (Attack != EOOSInputAttack::OOSIA_Super) && (Attack != EOOSInputAttack::OOSIA_Ultra))
			{
				FPOwner->SuperPts = FMath::Min(FPOwner->SuperPts + FMath::FloorToInt(Damage * 0.75f * SuperGain_Own), MAX_SUPER_POINTS_PER_LV * 5);
			}
			FPOwner->Opponent->SuperPts = FMath::Min(FPOwner->Opponent->SuperPts + FMath::FloorToInt(Damage * 0.5f * SuperGain_Opp), MAX_SUPER_POINTS_PER_LV * 5);

			SetTransformOnHit(true, Damage, TransformGain_Own, TransformGain_Opp);

			// Blocked hit FX
			if (FPOwner->BlockParticle)
				UGameplayStatics::SpawnEmitterAtLocation(FPOwner->GetWorld(), FPOwner->BlockParticle, InOverlapInfo.ImpactPoint, UKismetMathLibrary::MakeRotFromX((InOverlapInfo.ImpactPoint - FPOwner->GetActorLocation())), true);

			if (FPOwner->BlockSound)
				UGameplayStatics::PlaySound2D(FPOwner->GetWorld(), FPOwner->BlockSound);

			// Blocked hitlag.
			{
				HitLag();

			}
		}
	}

	OnOpponentPostHit.Broadcast(InOverlapInfo);

}

void UOOSHitbox::SetTransformOnHit(bool bBlocked, int InDamage, float OwnMult, float OppMult)
{
	// Convert the damage to a percentage of max hp
	float DamagePercent = InDamage / (float)FPOwner->Opponent->MaxHealth;

	// Hit
	if (!bBlocked)
	{
		// Attacker
		// Don't get transform energy from hitting if transformed.
		if (!FPOwner->IsA<AOOSPawn_Transformed>())
		{
			FPOwner->Transform = FMath::Min(FPOwner->Transform + FMath::FloorToInt(InDamage * 0.25f * OwnMult), MAX_TRANSFORM_POINTS);
		}

		// Defender
		if (FPOwner->Opponent->IsA<AOOSPawn_Transformed>())
		{
			// If transformed opponent, remove transform energy
			FPOwner->Opponent->Transform = FMath::Max(FPOwner->Opponent->Transform - FMath::FloorToInt(InDamage * 0.25f * OppMult), 0);
		}
		else
		{
			// * 0.6667f
			FPOwner->Opponent->Transform = FMath::Min(FPOwner->Opponent->Transform + FMath::FloorToInt(InDamage * 0.5f * OppMult), MAX_TRANSFORM_POINTS);
		}
	}
	// Blocked
	else
	{
		// Attacker
		// Don't get transform energy from hitting if transformed.
		if (!FPOwner->IsA<AOOSPawn_Transformed>())
		{
			FPOwner->Transform = FMath::Min(FPOwner->Transform + FMath::FloorToInt(InDamage * 0.125f * OwnMult), MAX_TRANSFORM_POINTS);
		}

		// Defender
		if (FPOwner->Opponent->IsA<AOOSPawn_Transformed>())
		{
			// If transformed opponent, remove transform energy
			FPOwner->Opponent->Transform = FMath::Max(FPOwner->Opponent->Transform - FMath::FloorToInt(InDamage * 0.16f * OppMult), 0);
		}
		else
		{
			FPOwner->Opponent->Transform = FMath::Min(FPOwner->Opponent->Transform + FMath::FloorToInt(InDamage * 0.25f * OppMult), MAX_TRANSFORM_POINTS);
		}
	}
}

FHitResult UOOSHitbox::GrabOpponent(bool bSweep, bool bDrag, float dTime, float Drag)
{
	bool bSocketValid = FPOwner->Opponent->SkeletalMesh->DoesSocketExist(GrabSocket);
	FVector SocketOffset = FVector::ZeroVector;
	if (bSocketValid)
	{
		SocketOffset = FPOwner->Opponent->GetActorLocation() - FPOwner->Opponent->SkeletalMesh->GetSocketLocation(GrabSocket);
		SocketOffset.Y = 0.f;
	}

	FVector NewLoc = GetComponentLocation();
	float Diff = 0.f;
	if (LaunchType == EOOSLaunchType::OOSLT_None || bSocketValid)
	{
		NewLoc = FVector(GetComponentLocation().X, 0.f, GetComponentLocation().Z);
		Diff = 0.f;
	}
	else
	{
		Diff = (FPOwner->Opponent->Capsule->GetScaledCapsuleHalfHeight());
	}

	NewLoc += FVector::UpVector * FMath::Max(0.f, Diff); // Correct Z.
	//NewLoc += (FPOwner->GetActorLocation() - NewLoc).ProjectOnTo(FPOwner->GetActorRightVector()); // Stay in 2D plane.
	FVector FinalLoc;
	if (bDrag)
	{
		FinalLoc = FMath::VInterpTo(FPOwner->Opponent->GetActorLocation(), NewLoc, dTime, Drag);
	}
	else
	{
		FinalLoc = NewLoc;
	}

	FHitResult Hit;
	FPOwner->Opponent->SetActorLocation(FinalLoc + SocketOffset, bSweep, &Hit);
	FPOwner->Opponent->bGrabbed = true;

	return Hit;
}

void UOOSHitbox::ResetHitbox()
{
	bHasMadeContact = false;
	bIsMakingContact = false;
	bHitConfirm = false;
	FPOwner->Opponent->bGrabbed = false;
	ClearComponentOverlaps(true, false);
	UpdateOverlaps();
}

void UOOSHitbox::ReceiveHBBeginOverlaps(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (!OtherActor) return; //This check is for components that begin play already overlapping. It seems that the overlap event is fired before they are assigned an owner.
	if (!OtherActor->IsInA(AOOSPawn::StaticClass()) && !OtherActor->GetOwner()) return; //If not a pawn and doesn't have owner.
	if (FPOwner == OtherActor) return; // If the Hurtbox belongs to the pawn who spawned the Hitbox, there's nothing to do.
	
	// For collision with HURTBOXES.
	if (OtherComp->GetClass() == UOOSHurtbox::StaticClass())
	{
		FOOSOverlapInfo NewOverlap;
		if (FindOverlapInfo(ThisComp, OtherActor, OtherComp, NewOverlap))
		{
			HuBList.Add(NewOverlap);
			
			if (!bIsMakingContact)
			{
				OverlapInfo = NewOverlap;
				bIsMakingContact = true;
			}
		}
	}
}

void UOOSHitbox::ReceiveHBEndOverlaps(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (!OtherActor) return; //This check is for components that begin play already overlapping. It seems that the overlap event is fired before they are assigned an owner.
	if (!OtherActor->IsInA(AOOSPawn::StaticClass()) && !OtherActor->GetOwner()) return; //If not a pawn and doesn't have owner.
	if (FPOwner == OtherActor) return; // If the Hurtbox belongs to the pawn who spawned the Hitbox, there's nothing to do.

	// For leaving overlap with HURTBOXES.
	if (OtherComp->GetClass() == UOOSHurtbox::StaticClass())
	{
		int NumEntries = HuBList.Num();
		if (NumEntries == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Overlap with a Hurtbox ended with the list empty."));
		}
		else
		{
			for (int i = 0; i < NumEntries; i++)
			{
				if (HuBList[i].OtherHB == OtherComp)
				{
					HuBList.RemoveAt(i);
					break;
				}
			}
		}

		if (HuBList.Num() == 0)
		{
			bIsMakingContact = false;
		}
	}
}

bool UOOSHitbox::FindOverlapInfo(UPrimitiveComponent *ThisComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FOOSOverlapInfo& NewOverlap)
{
	// First check if we've got a valid owner.
	if (!FPOwner) return false;

	// Then check if the incoming Hurtbox is valid.
	AOOSPawn* OtherPawn = Cast<AOOSPawn>(OtherActor);
	if (OtherPawn != FPOwner->Opponent) return false;

	// Also check if we're not overlapping with non-hitbox comps.
	UOOSHurtbox *ThisHitbox = Cast<UOOSHurtbox>(ThisComp);
	UOOSHurtbox *OtherHitbox = Cast<UOOSHurtbox>(OtherComp);
	if (!ThisHitbox || !OtherHitbox) return false;

	NewOverlap.ThisHB = ThisComp;
	NewOverlap.OtherActor = OtherActor;
	NewOverlap.OtherHB = OtherComp;

	FVector ThisHalfHeightNoRadius = ThisHitbox->GetUpVector() * ThisHitbox->GetScaledCapsuleHalfHeight_WithoutHemisphere();
	FVector ThisA = ThisHitbox->GetComponentLocation() + ThisHalfHeightNoRadius;
	FVector ThisB = ThisHitbox->GetComponentLocation() - ThisHalfHeightNoRadius;
	FVector ThisP;

	FVector OtherHalfHeightNoRadius = OtherHitbox->GetUpVector() * OtherHitbox->GetScaledCapsuleHalfHeight_WithoutHemisphere();
	FVector OtherA = OtherHitbox->GetComponentLocation() + OtherHalfHeightNoRadius;
	FVector OtherB = OtherHitbox->GetComponentLocation() - OtherHalfHeightNoRadius;
	FVector OtherP;

	FMath::SegmentDistToSegmentSafe(ThisA, ThisB, OtherA, OtherB, ThisP, OtherP);

	NewOverlap.ImpactPoint = OtherP + ((ThisP - OtherP).GetSafeNormal() * OtherHitbox->GetScaledCapsuleRadius());
	NewOverlap.Penetration = (ThisHitbox->GetScaledCapsuleRadius() + OtherHitbox->GetScaledCapsuleRadius()) - (OtherP - ThisP).Size();

	return true;
}

void UOOSHitbox::DestroyHitbox()
{
	if (FPOwner)
	{
		if (bHasMadeContact)
		{
			if (LaunchType == EOOSLaunchType::OOSLT_UpFar && bHitConfirm && !FPOwner->Opponent->IsChargingTransform())
			{
				FPOwner->TryPostLaunchJump(bForceTryPostLaunchJump);
			}
		}
		
		if (bHitConfirm)
		{
			if (bGrabMode)
			{
				FPOwner->MovementComponent->bDoNotPush = false;
				FPOwner->Opponent->MovementComponent->bDoNotPush = false;
				FPOwner->Opponent->bGrabbed = false;
			}

			if (DirectionMode == EOOSDirectionMode::OOSDM_Gravitational)
			{
				FPOwner->MovementComponent->bDoNotPush = false;
				FPOwner->Opponent->MovementComponent->bDoNotPush = false;
				FPOwner->Opponent->MovementComponent->bUseGravity = true;
				FPOwner->Opponent->MovementComponent->bUseXDecel = true;
			}
		}
		FPOwner->Opponent->bGrabbed = false;
	}

	// We have to do this since at least one more Tick() is fired after destroying.
	bHasMadeContact = false;
	bIsMakingContact = false;
	bHitConfirm = false;

	DestroyComponent();
}

// Gets the extent of the group of currently overlapping Hurtboxes.
void UOOSHitbox::GetHurtboxesBounds(FVector& Origin, FVector& BoxExtent)
{
	FBox Box(EForceInit::ForceInitToZero);

	for (int i = 0; i < HuBList.Num(); i++)
	{
		Box += HuBList[i].OtherHB->Bounds.GetBox();
	}

	Box.GetCenterAndExtents(Origin, BoxExtent);
}

void UOOSHitbox::HitLag()
{
	if (!FPOwner) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FPOwner->SetHitStop(SelfHitLag);
	FPOwner->Opponent->SetHitStop(OpponentHitLag);
}