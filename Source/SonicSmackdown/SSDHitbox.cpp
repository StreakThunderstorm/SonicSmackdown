// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDHitbox.h"
#include "SSDPawn_Transformed.h"
#include "SSDProjectile.h"
#include "SSDMovementComponent.h"
#include "SSDAnimNotify_Hitbox.h"
#include "SSDPawn_Burst.h"
#include "Kismet/KismetMathLibrary.h"
#include "SkeletalAnimation/SSDSkeletalMeshComponent.h"
#include "PawnRenderLayerEffectComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine.h"

USSDHitbox::USSDHitbox()
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

void USSDHitbox::Initialize
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
	enum ESSDHitHeight Height, 
	enum ESSDInputAttack Att, 
	enum ESSDLaunchType Launch,
	bool bForceExtension,
	bool bResetExtensions,
	enum ESSDDirectionMode Direction, 
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

	// Get the ASSDPawn owning this Hitbox. If it fails (meaning that the Hitbox is attached to an external Actor), try and get its owner again.
	AActor* Owner = GetOwner();
	if (Owner)
	{
		FPOwner = Cast<ASSDPawn>(Owner);
		if (!FPOwner)
		{
			// If owner of the hitbox is not a SSDPawn, try SSDProjectile
			ASSDProjectile* POwner = Cast<ASSDProjectile>(Owner);
			if (POwner)
			{
				FPOwner = Cast<ASSDPawn>(POwner->PawnOwner);
				bIsProjectile = true;
			}
			else
			{
				// Try luck one more time, we could be attached to other type of minion actor that can spawn hitboxes.
				FPOwner = Cast<ASSDPawn>(Owner->GetOwner());
			}
		}
	}
}

void USSDHitbox::BeginPlay()
{
	Super::BeginPlay();

}

void USSDHitbox::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
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
		ASSDProjectile* Prj = Cast<ASSDProjectile>(GetOwner());
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

	// A contact that only happened between ticks is handled as a single tick of contact.
	FSSDOverlapInfo SweptOverlap;
	if (UpdateSweep(SweptOverlap))
	{
		OverlapInfo = SweptOverlap;
		bIsMakingContact = true;
		MainLoop(DeltaTime);
		bIsMakingContact = HuBList.Num() > 0;
	}
	else
	{
		MainLoop(DeltaTime);
	}
	
}

bool USSDHitbox::CanSweepForHits() const
{
	if (!bSweepEnabled || bGrabMode) return false;
	if (DirectionMode == ESSDDirectionMode::SSDDM_Gravitational && Push < 0.f) return false;

	return true;
}

bool USSDHitbox::UpdateSweep(FSSDOverlapInfo& OutOverlap)
{
	const FTransform HitCur(GetComponentQuat(), GetComponentLocation());
	const FTransform HitPrev = PrevSweepTransform;
	const bool bHadHistory = bHasSweepHistory;
	const TMap<TWeakObjectPtr<USSDHurtbox>, FTransform> PrevHurtboxes = MoveTemp(PrevHurtboxTransforms);

	PrevSweepTransform = HitCur;
	bHasSweepHistory = true;
	PrevHurtboxTransforms.Reset();

	ASSDPawn* Opponent = FPOwner ? FPOwner->Opponent : nullptr;
	if (!Opponent) return false;

	// Only look for contacts the overlap events can't see: nothing touching now, nothing hit yet.
	const bool bSweep = bHadHistory && bEnabled && !bIsMakingContact && !bHasMadeContact && CanSweepForHits()
		&& GetCollisionEnabled() != ECollisionEnabled::NoCollision;

	const float ThisRadius = GetScaledCapsuleRadius();
	const FVector ThisHalf(0.f, 0.f, GetScaledCapsuleHalfHeight_WithoutHemisphere());

	bool bFound = false;
	float BestAlpha = 0.f;

	TInlineComponentArray<USSDHurtbox*> Hurtboxes(Opponent);
	for (USSDHurtbox* Hurtbox : Hurtboxes)
	{
		// Same filter as the overlap handlers: plain hurtboxes only, not the opponent's hitboxes.
		if (!Hurtbox || Hurtbox->GetClass() != USSDHurtbox::StaticClass() || !Hurtbox->IsRegistered()) continue;

		const FTransform HurtCur(Hurtbox->GetComponentQuat(), Hurtbox->GetComponentLocation());
		PrevHurtboxTransforms.Add(Hurtbox, HurtCur);

		if (!bSweep || Hurtbox->GetCollisionEnabled() == ECollisionEnabled::NoCollision) continue;

		const FTransform* HurtPrev = PrevHurtboxes.Find(TWeakObjectPtr<USSDHurtbox>(Hurtbox));
		if (!HurtPrev) continue;

		// Hitbox pose as seen from the hurtbox, before and now.
		const FTransform Rel0 = HitPrev.GetRelativeTransform(*HurtPrev);
		const FTransform Rel1 = HitCur.GetRelativeTransform(HurtCur);

		const float Travel = FMath::Max(
			FVector::Dist(Rel0.TransformPositionNoScale(ThisHalf), Rel1.TransformPositionNoScale(ThisHalf)),
			FVector::Dist(Rel0.TransformPositionNoScale(-ThisHalf), Rel1.TransformPositionNoScale(-ThisHalf)));
		if (Travel > MaxSweepDistance) continue;

		// Step finely enough that the capsules can't skip past each other between two samples.
		const float OtherRadius = Hurtbox->GetScaledCapsuleRadius();
		const float RadiusSum = ThisRadius + OtherRadius;
		const int32 NumSteps = FMath::Clamp(FMath::CeilToInt(Travel / FMath::Max(1.f, RadiusSum * 0.5f)), 1, 32);

		const FVector OtherHalf = Hurtbox->GetUpVector() * Hurtbox->GetScaledCapsuleHalfHeight_WithoutHemisphere();
		const FVector OtherA = HurtCur.GetLocation() + OtherHalf;
		const FVector OtherB = HurtCur.GetLocation() - OtherHalf;

		// The final pose is left to the regular overlap events, so only the in-between ones are sampled.
		for (int32 Step = 1; Step < NumSteps; ++Step)
		{
			const float Alpha = (float)Step / (float)NumSteps;
			if (bFound && Alpha >= BestAlpha) break;

			const FTransform Rel(FQuat::Slerp(Rel0.GetRotation(), Rel1.GetRotation(), Alpha), FMath::Lerp(Rel0.GetLocation(), Rel1.GetLocation(), Alpha));
			const FTransform Pose = Rel * HurtCur;
			const FVector ThisA = Pose.TransformPositionNoScale(ThisHalf);
			const FVector ThisB = Pose.TransformPositionNoScale(-ThisHalf);

			FVector ThisP, OtherP;
			FMath::SegmentDistToSegmentSafe(ThisA, ThisB, OtherA, OtherB, ThisP, OtherP);

			const float Distance = FVector::Dist(ThisP, OtherP);
			if (Distance <= RadiusSum)
			{
				bFound = true;
				BestAlpha = Alpha;

				OutOverlap.ThisHB = this;
				OutOverlap.OtherActor = Opponent;
				OutOverlap.OtherHB = Hurtbox;
				OutOverlap.ImpactPoint = OtherP + ((ThisP - OtherP).GetSafeNormal() * OtherRadius);
				OutOverlap.Penetration = RadiusSum - Distance;
				break;
			}
		}
	}

	return bFound;
}

void USSDHitbox::MainLoop(float DeltaTime)
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
			if (DirectionMode == ESSDDirectionMode::SSDDM_Gravitational && (Push < 0.f))
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
			if (DirectionMode == ESSDDirectionMode::SSDDM_Gravitational && (Push < 0.f))
			{

				FPOwner->MovementComponent->bDoNotPush = false;
				FPOwner->Opponent->MovementComponent->bDoNotPush = false;
				FPOwner->Opponent->MovementComponent->bUseGravity = true;
				FPOwner->Opponent->MovementComponent->bUseXDecel = true;

			}
		}
	}
}

float USSDHitbox::GetDirection()
{
	float Direction = 0.f;

	switch (DirectionMode)
	{
	case ESSDDirectionMode::SSDDM_Directional:
		Direction = FPOwner->MovementComponent->bIsFacingRight ? 1.f : -1.f;
		break;

	case ESSDDirectionMode::SSDDM_TwoSided:
		Direction = FMath::Sign((FPOwner->Opponent->GetActorLocation() - GetComponentLocation()).GetSafeNormal() | FPOwner->GetActorForwardVector());
		break;

	case ESSDDirectionMode::SSDDM_Gravitational:
		Direction = 0.f;
		break;
	}

	return Direction;
}

void USSDHitbox::TryHit(FSSDOverlapInfo HitOverlapInfo)
{
	if (FPOwner->Opponent->IsTransforming() || (Damage == 0 && bAntiProjectile && !bBurst && !bAutoPerformChildMove) || (FPOwner->GrabType == ESSDGrab::SSDGR_Air && FPOwner->Opponent->MovementComponent->bOnGround) || (FPOwner->GrabType == ESSDGrab::SSDGR_Ground && !FPOwner->Opponent->MovementComponent->bOnGround))
	{
		return;
	}

	bool AlreadyMadeContact = FPOwner->HitCount > 0 && FPOwner->bHasMadeContact; // If the owning Pawn already hit once this move
	bHasMadeContact = true;
	FPOwner->bHasMadeContact = true; // Inform the owning Pawn about the hit for frame data
	FPOwner->bHasLandedAttack = !bDontCancel; // Inform the owning Pawn about the hit to enable cancel

	// Hit routine
	FVector Dir = (FPOwner->Opponent->GetActorLocation() - GetComponentLocation()).GetSafeNormal();
	(void)FPOwner->GetActorRotation().UnrotateVector(Dir);
	GravitationalDirection.X = Dir.X;
	GravitationalDirection.Y = Dir.Z;

	float Direction = GetDirection();

	bool FaceRight;
	if (DirectionMode == ESSDDirectionMode::SSDDM_Directional)
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
			UPawnRenderLayerEffectComponent::BindEffect(UGameplayStatics::SpawnEmitterAtLocation(
				FPOwner->GetWorld(), FPOwner->Opponent->TransformAbsorbParticle, HitOverlapInfo.ImpactPoint, FRotator::ZeroRotator, true), FPOwner->Opponent);

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
			case ESSDInputAttack::SSDIA_Super:
			case ESSDInputAttack::SSDIA_Ultra:
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
			case ESSDInputAttack::SSDIA_Light:
				DamageScaleDegradation = 0.9f;
				HitStunScaleDegradation = 0.9f;
				break;

			case ESSDInputAttack::SSDIA_Medium:
				DamageScaleDegradation = 0.92f;
				HitStunScaleDegradation = 0.92f;
				break;

			case ESSDInputAttack::SSDIA_Heavy:
				DamageScaleDegradation = 0.94f;
				HitStunScaleDegradation = 0.94f;
				break;

			case ESSDInputAttack::SSDIA_Special:
				DamageScaleDegradation = 0.95f;
				HitStunScaleDegradation = 0.95f;
				break;

			case ESSDInputAttack::SSDIA_EX:
				DamageScaleDegradation = 0.96f;
				HitStunScaleDegradation = 0.96f;
				break;

			case ESSDInputAttack::SSDIA_Super:
				DamageScaleDegradation = 0.98f;
				HitStunScaleDegradation = 0.98f;
				break;

			case ESSDInputAttack::SSDIA_Ultra:
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

		if (Attack == ESSDInputAttack::SSDIA_Ultra)
		{
			const float Alpha = 1.f;
			CurrentHitDamageScale = FMath::Lerp(CurrentHitDamageScale, 1.f, Alpha);
			CurrentHitStunScale = 1.f; // Use unscaled hitstun for ultras so they don't drop randomly
		}

		FVector2D LaunchSpd = LaunchSpeed;
		LaunchSpd.X *= Direction;
		ESSDLaunchType Launch = LaunchType;
		if ((DirectionMode == ESSDDirectionMode::SSDDM_Gravitational) && (Push > 0.f))
		{
			if (Push > 0.f)
			{
				LaunchSpd = GravitationalDirection * Push;
			}
			else
			{
				Launch = ESSDLaunchType::SSDLT_None;
			}
		}

		float HitDamage = Damage * CurrentHitDamageScale;
		float ChipDamage = Damage * 0.3f;

		bool bCounter = false;
		bool bIsSuper = false;

		ASSDProjectile* Prj = Cast<ASSDProjectile>(GetOwner());
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
					ASSDPawn_Burst* PB = Cast<ASSDPawn_Burst>(FPOwner->Opponent);
					if (PB)
					{
						int RingsToDrop = PB->Rings;
						RingsToDrop = FMath::Clamp(RingsToDrop, 0, 10);
						PB->ScatterRings(RingsToDrop);
						PB->Rings = PB->Rings - RingsToDrop;
					}

					FPOwner->Opponent->Launch(FVector2D(FPOwner->MovementComponent->bIsFacingRight ? 2.f : -2.f,4.f), ESSDLaunchType::SSDLT_BackShort);
					FPOwner->Opponent->bIsPerformingMove = false;
					FPOwner->Opponent->bGrabbed = false;
					FPOwner->Opponent->Unfreeze();
				}
			}

			// Super/transform earn and pay.
			if ((Attack != ESSDInputAttack::SSDIA_EX) && (Attack != ESSDInputAttack::SSDIA_Super) && (Attack != ESSDInputAttack::SSDIA_Ultra) && (!FPOwner->bMeterBlocked))
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
					UPawnRenderLayerEffectComponent::BindEffect(UGameplayStatics::SpawnEmitterAtLocation(
						FPOwner->GetWorld(), FPOwner->TechParticle, HitOverlapInfo.ImpactPoint, FRotator::ZeroRotator, true), FPOwner);

				if(FPOwner->TechSound)
					UGameplayStatics::PlaySound2D(FPOwner->GetWorld(), FPOwner->TechSound);
			}
			else
			{
				// Non blocked hit FX
				if (HitEffect)
					UPawnRenderLayerEffectComponent::BindEffect(UGameplayStatics::SpawnEmitterAtLocation(
						FPOwner->GetWorld(), HitEffect, HitOverlapInfo.ImpactPoint, FRotator::ZeroRotator, true), FPOwner);

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
			if ((Attack != ESSDInputAttack::SSDIA_EX) && (Attack != ESSDInputAttack::SSDIA_Super) && (Attack != ESSDInputAttack::SSDIA_Ultra))
			{
				FPOwner->SuperPts = FMath::Min(FPOwner->SuperPts + FMath::FloorToInt(Damage * 0.75f * SuperGain_Own), MAX_SUPER_POINTS_PER_LV * 5);
			}
			FPOwner->Opponent->SuperPts = FMath::Min(FPOwner->Opponent->SuperPts + FMath::FloorToInt(Damage * 0.5f * SuperGain_Opp), MAX_SUPER_POINTS_PER_LV * 5);

			SetTransformOnHit(true, Damage, TransformGain_Own, TransformGain_Opp);

			// Blocked hit FX
			if (FPOwner->BlockParticle)
				UPawnRenderLayerEffectComponent::BindEffect(UGameplayStatics::SpawnEmitterAtLocation(FPOwner->GetWorld(), FPOwner->BlockParticle,
					HitOverlapInfo.ImpactPoint, UKismetMathLibrary::MakeRotFromX(HitOverlapInfo.ImpactPoint - FPOwner->GetActorLocation()), true), FPOwner->Opponent);

			if (FPOwner->BlockSound)
				UGameplayStatics::PlaySound2D(FPOwner->GetWorld(), FPOwner->BlockSound);

			// Blocked hitlag.
			{
				HitLag();

			}
		}
	}

	OnOpponentPostHit.Broadcast(HitOverlapInfo);

}

void USSDHitbox::SetTransformOnHit(bool bBlocked, int HitDamage, float OwnMult, float OppMult)
{
	// Convert the damage to a percentage of max hp
	float DamagePercent = HitDamage / (float)FPOwner->Opponent->MaxHealth;

	// Hit
	if (!bBlocked)
	{
		// Attacker
		// Don't get transform energy from hitting if transformed.
		if (!FPOwner->IsA<ASSDPawn_Transformed>())
		{
			FPOwner->Transform = FMath::Min(FPOwner->Transform + FMath::FloorToInt(HitDamage * 0.25f * OwnMult), MAX_TRANSFORM_POINTS);
		}

		// Defender
		if (FPOwner->Opponent->IsA<ASSDPawn_Transformed>())
		{
			// If transformed opponent, remove transform energy
			FPOwner->Opponent->Transform = FMath::Max(FPOwner->Opponent->Transform - FMath::FloorToInt(HitDamage * 0.25f * OppMult), 0);
		}
		else
		{
			// * 0.6667f
			FPOwner->Opponent->Transform = FMath::Min(FPOwner->Opponent->Transform + FMath::FloorToInt(HitDamage * 0.5f * OppMult), MAX_TRANSFORM_POINTS);
		}
	}
	// Blocked
	else
	{
		// Attacker
		// Don't get transform energy from hitting if transformed.
		if (!FPOwner->IsA<ASSDPawn_Transformed>())
		{
			FPOwner->Transform = FMath::Min(FPOwner->Transform + FMath::FloorToInt(HitDamage * 0.125f * OwnMult), MAX_TRANSFORM_POINTS);
		}

		// Defender
		if (FPOwner->Opponent->IsA<ASSDPawn_Transformed>())
		{
			// If transformed opponent, remove transform energy
			FPOwner->Opponent->Transform = FMath::Max(FPOwner->Opponent->Transform - FMath::FloorToInt(HitDamage * 0.16f * OppMult), 0);
		}
		else
		{
			FPOwner->Opponent->Transform = FMath::Min(FPOwner->Opponent->Transform + FMath::FloorToInt(HitDamage * 0.25f * OppMult), MAX_TRANSFORM_POINTS);
		}
	}
}

FHitResult USSDHitbox::GrabOpponent(bool bSweep, bool bDrag, float dTime, float Drag)
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
	if (LaunchType == ESSDLaunchType::SSDLT_None || bSocketValid)
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

void USSDHitbox::ResetHitbox()
{
	bHasMadeContact = false;
	bIsMakingContact = false;
	bHitConfirm = false;
	FPOwner->Opponent->bGrabbed = false;
	ClearComponentOverlaps(true, false);
	UpdateOverlaps();
}

void USSDHitbox::ReceiveHBBeginOverlaps(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (!OtherActor) return; //This check is for components that begin play already overlapping. It seems that the overlap event is fired before they are assigned an owner.
	if (!OtherActor->IsInA(ASSDPawn::StaticClass()) && !OtherActor->GetOwner()) return; //If not a pawn and doesn't have owner.
	if (FPOwner == OtherActor) return; // If the Hurtbox belongs to the pawn who spawned the Hitbox, there's nothing to do.
	
	// For collision with HURTBOXES.
	if (OtherComp->GetClass() == USSDHurtbox::StaticClass())
	{
		FSSDOverlapInfo NewOverlap;
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

void USSDHitbox::ReceiveHBEndOverlaps(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (!OtherActor) return; //This check is for components that begin play already overlapping. It seems that the overlap event is fired before they are assigned an owner.
	if (!OtherActor->IsInA(ASSDPawn::StaticClass()) && !OtherActor->GetOwner()) return; //If not a pawn and doesn't have owner.
	if (FPOwner == OtherActor) return; // If the Hurtbox belongs to the pawn who spawned the Hitbox, there's nothing to do.

	// For leaving overlap with HURTBOXES.
	if (OtherComp->GetClass() == USSDHurtbox::StaticClass())
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

bool USSDHitbox::FindOverlapInfo(UPrimitiveComponent *ThisComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FSSDOverlapInfo& NewOverlap)
{
	// First check if we've got a valid owner.
	if (!FPOwner) return false;

	// Then check if the incoming Hurtbox is valid.
	ASSDPawn* OtherPawn = Cast<ASSDPawn>(OtherActor);
	if (OtherPawn != FPOwner->Opponent) return false;

	// Also check if we're not overlapping with non-hitbox comps.
	USSDHurtbox *ThisHitbox = Cast<USSDHurtbox>(ThisComp);
	USSDHurtbox *OtherHitbox = Cast<USSDHurtbox>(OtherComp);
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

void USSDHitbox::DestroyHitbox()
{
	if (FPOwner)
	{
		if (bHasMadeContact)
		{
			if (LaunchType == ESSDLaunchType::SSDLT_UpFar && bHitConfirm && !FPOwner->Opponent->IsChargingTransform())
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

			if (DirectionMode == ESSDDirectionMode::SSDDM_Gravitational)
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
void USSDHitbox::GetHurtboxesBounds(FVector& Origin, FVector& BoxExtent)
{
	FBox Box(EForceInit::ForceInitToZero);

	for (int i = 0; i < HuBList.Num(); i++)
	{
		Box += HuBList[i].OtherHB->Bounds.GetBox();
	}

	Box.GetCenterAndExtents(Origin, BoxExtent);
}

void USSDHitbox::HitLag()
{
	if (!FPOwner) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FPOwner->SetHitStop(SelfHitLag);
	FPOwner->Opponent->SetHitStop(OpponentHitLag);
}
