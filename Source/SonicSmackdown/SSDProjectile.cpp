// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDProjectile.h"
#include "SSDPawn.h"
#include "SSDMovementComponent.h"
#include "SkeletalAnimation/SSDSkeletalMeshComponent.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"
#include "PawnRenderLayerEffectComponent.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ASSDProjectile::ASSDProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Hitbox = CreateDefaultSubobject<USSDHitbox>("Hitbox");
	RootComponent = Hitbox;
	BeamBox = CreateDefaultSubobject<USSDHitbox>("BeamBox");
	BeamBox->SetupAttachment(RootComponent);
}

void ASSDProjectile::Initialize
(
	int NHits,
	float HitTime,
	float Lifetime,
	FVector2D Spd,
	FVector FW,
	bool bGrab,
	bool bForceGrab,
	FName GrabSock,
	UParticleSystem* HitParticle, 
	USoundBase* HitSFX, 
	UParticleSystem* DeathParticle, 
	USoundBase* DeathSound,
	bool bBrst,
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
	enum ESSDHitHeight Height, 
	enum ESSDInputAttack Att, 
	enum ESSDLaunchType Launch,
	bool bForceExtension,
	bool bResetExtensions,
	enum ESSDDirectionMode Direction, 
	FVector2D LaunchSpd, 
	bool bForceTryPostLaunchJump
)
{
	// Get owning Actor.
	Speed = Spd;
	OGSpeed = Spd;

	//Check if its a beam and add secondary Hitbox
	if (!bIsBeam)
		BeamBox->bEnabled = false;

	// Try to get the runtime pawn and determine the movement direction.
	if (PawnOwner)
	{
		FPOwner = Cast<ASSDPawn>(PawnOwner);
		if (FPOwner)
		{			
			ForwardVector = FPOwner->MovementComponent->bIsFacingRight ? FW : -FW;
		}
		else
		{
			ForwardVector = FW;
		}
		
		UWorld* World = GetWorld();
		
		if (World && (Lifetime > 0.f))
		{
			World->GetTimerManager().SetTimer(Death, this, &ASSDProjectile::ProjectileTimeOver, 1.f, false, Lifetime);
		}
	}

	// Logically, any value below 1 should make it have infinite hits until Lifetime ends, but we're making 0 make it harmless in order to test collision against StageBounds or other stuff.
	if (NHits == 0)
	{
		Hitbox->SetCollisionResponseToChannel(HITBOX_OBJ, ECollisionResponse::ECR_Ignore);
		return;
	}

	Hitbox->Initialize
	(
		-1.f,
		-1.f,
		bGrab,
		bForceGrab,
		GrabSock,
		HitParticle,
		HitSFX,
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
		Height, 
		Att, 
		Launch,
		bForceExtension,
		bResetExtensions,
		Direction, 
		LaunchSpd, 
		bForceTryPostLaunchJump
	);
	BeamBox->Initialize
	(
		-1.f,
		-1.f,
		bGrab,
		bForceGrab,
		GrabSock,
		HitParticle,
		HitSFX,
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
		Height,
		Att,
		Launch,
		bForceExtension,
		bResetExtensions,
		Direction,
		LaunchSpd,
		bForceTryPostLaunchJump
	);

	// Bind the hitbox's hit delegate to OnContact method.
	FScriptDelegate HurtDelegate;
	HurtDelegate.BindUFunction(this, TEXT("OnContact"));
	Hitbox->OnOpponentPostHit.AddUnique(HurtDelegate);
	BeamBox->OnOpponentPostHit.AddUnique(HurtDelegate);

	// Assign function for collision against stage bounds.
	FScriptDelegate OverlapDelegate;
	OverlapDelegate.BindUFunction(this, TEXT("ProjectileBeginOverlap"));
	Hitbox->OnComponentBeginOverlap.AddUnique(OverlapDelegate);
	BeamBox->OnOpponentPostHit.AddUnique(HurtDelegate);

	FScriptDelegate StageHit;
	StageHit.BindUFunction(this, TEXT("DestroyWithEffects"));
	OnActorHit.AddUnique(StageHit);

	RemainingHits = NHits;
	if (HitTime > KINDA_SMALL_NUMBER) TimeBetweenHits = HitTime;
	DeathFX = DeathParticle;
	DeathSFX = DeathSound;

	HitFX = HitParticle;
	HitSound = HitSFX;

	Hitbox->SetCollisionObjectType(PROJECTILE_OBJ);
	Hitbox->SetCollisionResponseToChannel(HITBOX_OBJ, ECollisionResponse::ECR_Overlap);
	Hitbox->SetCollisionResponseToChannel(PROJECTILE_OBJ, ECollisionResponse::ECR_Overlap);
	BeamBox->SetCollisionObjectType(PROJECTILE_OBJ);
	BeamBox->SetCollisionResponseToChannel(HITBOX_OBJ, ECollisionResponse::ECR_Overlap);
	BeamBox->SetCollisionResponseToChannel(PROJECTILE_OBJ, ECollisionResponse::ECR_Overlap);
}

ASSDProjectile* ASSDProjectile::SpawnChild
(
	TSubclassOf<ASSDProjectile> Class, 
	int NumberOfHits, 
	float HitTime, 
	float Lifetime, 
	FVector2D ChildSpeed,
	FVector Offset, 
	FRotator Rotation, 
	bool bGrab, 
	bool bForceGrab,
	FName GrabSock,
	UParticleSystem* HitParticle, 
	USoundBase* ChildHitSound,
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
	ESSDHitHeight HitHeight, 
	ESSDInputAttack Attack, 
	ESSDLaunchType Launch,
	bool bForceExtension,
	bool bResetExtensions,
	ESSDDirectionMode Direction, 
	FVector2D LaunchSpd, 
	bool bForceTryPostLaunchJump
)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	// Check if we're in anim editor or in game.
	if (FPOwner)
	{
		Offset.X = FPOwner->MovementComponent->bIsFacingRight ? -Offset.X : Offset.X;
	}
	else
	{
		// Flip X and Y coords for editor.
		Offset = FVector(Offset.Y, Offset.X, Offset.Z);
	}
	
	FTransform T = FTransform(Rotation, GetActorLocation() + Offset, FVector(1.f, 1.f, 1.f));
	ASSDProjectile* NewProj = World->SpawnActorDeferred<ASSDProjectile>(Class, T, this);
	if (!NewProj) return nullptr;

	NewProj->PawnOwner = PawnOwner;
	NewProj->Initialize
	(
		NumberOfHits, 
		HitTime, 
		Lifetime, 
		ChildSpeed,
		PawnOwner->GetActorForwardVector(), 
		bGrab, 
		bForceGrab,
		GrabSock,
		HitParticle, 
		ChildHitSound,
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
		bForceExtension,
		bResetExtensions,
		Direction, 
		LaunchSpd, 
		bForceTryPostLaunchJump
	);
	NewProj->Hitbox->SetHiddenInGame(!Hitbox->bDebug);
	NewProj->Hitbox->bDebug = Hitbox->bDebug;
	NewProj->Hitbox->DebugColor = Hitbox->DebugColor;
	NewProj->Hitbox->DebugThickness = 0.5f;
	NewProj->InitLocation = GetActorLocation() + Offset;
	NewProj->InitRelative = FVector::ZeroVector;

	if (NewProj->bIsBeam)
	{
		NewProj->BeamBox->SetHiddenInGame(!FPOwner->bDebugHitboxes);
		NewProj->BeamBox->bDebug = FPOwner->bDebugHitboxes;
		NewProj->BeamBox->DebugColor = FPOwner->HitboxDebugColor;
		NewProj->BeamBox->DebugThickness = 0.5f;
		NewProj->Hitbox->bEnabled = false;
	}
	else
	{
		NewProj->BeamBox->SetHiddenInGame(true);
	}	

	NewProj->BeamRoot = NewProj->InitLocation + NewProj->InitRelative;
	NewProj->FinishSpawning(T);

	return NewProj;
}

// Called when the game starts or when spawned
void ASSDProjectile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASSDProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (!World) return;

	if (bFrozen) return;

	FVector ThisFrameOffset;
	ThisFrameOffset = ForwardVector * Speed.X;
	ThisFrameOffset += FVector::UpVector * Speed.Y;
	ThisFrameOffset *= DeltaTime;

	AddActorWorldOffset(ThisFrameOffset, true);

	FTimerManager& TimerManager = World->GetTimerManager();
	if (!TimerManager.IsTimerActive(NewHit) && !bFrozen && !bDeceleration)
	{
		Speed = OGSpeed;
	}

	if (bIsBeam)
	{
		if (FPOwner)
		{	
			if (bLockBeam)
			{
				BeamRoot = FPOwner->SkeletalMesh->GetComponentLocation() + InitRelative;
			}
			else
			{
				BeamRoot = InitLocation + InitRelative;
			}

			BeamBox->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ((GetActorLocation() - BeamRoot).GetSafeNormal()));
			BeamBox->SetCapsuleHalfHeight(FPOwner->MovementComponent->bIsFacingRight ? (GetActorLocation() - BeamRoot).Size() / 2.f : (BeamRoot - GetActorLocation()).Size() / 2.f);
		}
		else
		{			
			BeamRoot = InitLocation;
			BeamBox->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ((GetActorLocation() - BeamRoot).GetSafeNormal()));
			BeamBox->SetCapsuleHalfHeight((GetActorLocation() - BeamRoot).Size() / 2.f);
		}
		BeamBox->SetWorldLocation( (GetActorLocation() + BeamRoot) / 2.f);
	}
}

void ASSDProjectile::Freeze(float Duration)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FTimerManager& TimerManager = World->GetTimerManager();
	if(TimerManager.IsTimerActive(NewHit)) TimerManager.PauseTimer(NewHit);
	if (TimerManager.IsTimerActive(Death)) TimerManager.PauseTimer(Death);

	if (Duration >= 0.f)
	{
		World->GetTimerManager().SetTimer(FreezeTimer, this, &ASSDProjectile::Unfreeze, Duration);
	}

	TArray<UParticleSystemComponent*> ParticleSystems;
	GetComponents<UParticleSystemComponent>(ParticleSystems);
	for (UParticleSystemComponent* System : ParticleSystems)
	{
		if (System)
		{
			System->CustomTimeDilation = 0.0001f;
		}
	}

	TArray<UNiagaraComponent*> Niagaras;
	GetComponents<UNiagaraComponent>(Niagaras);
	for (UNiagaraComponent* Niagara : Niagaras)
	{
		if (Niagara)
		{
			Niagara->SetPaused(true);
		}
	}

	bFrozen = true;
	CustomTimeDilation = 0.0001f;
}

void ASSDProjectile::Unfreeze()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FTimerManager& TimerManager = World->GetTimerManager();
	if (TimerManager.IsTimerPaused(NewHit)) TimerManager.UnPauseTimer(NewHit);
	if (TimerManager.IsTimerPaused(Death)) TimerManager.UnPauseTimer(Death);

	World->GetTimerManager().ClearTimer(FreezeTimer);

	TArray<UParticleSystemComponent*> ParticleSystems;
	GetComponents<UParticleSystemComponent>(ParticleSystems);
	for (UParticleSystemComponent* System : ParticleSystems)
	{
		if (System)
		{
			System->CustomTimeDilation = 1.f;
		}
	}
	
	TArray<UNiagaraComponent*> Niagaras;
	GetComponents<UNiagaraComponent>(Niagaras);
	for (UNiagaraComponent* Niagara : Niagaras)
	{
		if (Niagara)
		{
			Niagara->SetPaused(false);
		}
	}

	bFrozen = false;	
	CustomTimeDilation = 1.f;
}

void ASSDProjectile::StopAnim(float Duration)
{
	TArray<USkeletalMeshComponent*> SkelMeshes;
	GetComponents<USkeletalMeshComponent>(SkelMeshes);
	for (USkeletalMeshComponent* Mesh : SkelMeshes)
	{
		if (Mesh)
		{
			Mesh->bPauseAnims = true;
		}
	}
	CustomTimeDilation = 0.0001f;
}

void ASSDProjectile::ResumeAnim()
{
	TArray<USkeletalMeshComponent*> SkelMeshes;
	GetComponents<USkeletalMeshComponent>(SkelMeshes);
	for (USkeletalMeshComponent* Mesh : SkelMeshes)
	{
		if (Mesh)
		{
			Mesh->bPauseAnims = false;
		}
	}
	CustomTimeDilation = 1.f;
}

void ASSDProjectile::OnContact(FSSDOverlapInfo OverlapInfo)
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	OnOpponentHit(OverlapInfo);

	if (RemainingHits == 1)
	{
		TimesHit++;
		DestroyWithEffects();
	}
	else
	{
		World->GetTimerManager().SetTimer(NewHit, this, &ASSDProjectile::ResetProjectile, 1.f, false, TimeBetweenHits);
		RemainingHits--;
		TimesHit++;
	}

}

void ASSDProjectile::ProjectileBeginOverlap(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (!OtherActor) return;																		//This check is for components that begin play already overlapping. It seems that the overlap event is fired before they are assigned an owner.
	if (!OtherActor->IsInA(ASSDProjectile::StaticClass()) && !OtherActor->GetOwner() && !OtherActor->IsInA(ASSDPawn::StaticClass()))
	{
		return;		//If not a Projectile and doesn't have owner.
	}
	ASSDPawn* OtherOwner = Cast<ASSDPawn>(OtherActor->GetOwner());									//Get Projectile's owner pawn.
	if (FPOwner == OtherOwner) return;		
	
	FTimerManager& TimerManager = World->GetTimerManager();
	
	//If Projectile's owner pawn is the same, do nothing.


	if (OtherActor->IsInA(ASSDProjectile::StaticClass()))
	{
		ASSDProjectile* OtherProj = Cast<ASSDProjectile>(OtherActor);
		if (OtherProj->PawnOwner == PawnOwner) return;

		if (RemainingHits == 1 && !bProtected)
		{
			DestroyWithEffects();
		}
		else if (bOneHitKO && !bProtected)
		{
			DestroyWithEffects();
		}		
		else
		{			
			if (HitSound)
				UGameplayStatics::PlaySound2D(World, HitSound);
			if (bIsBeam)
				if (HitFX)
					UPawnRenderLayerEffectComponent::BindEffect(UGameplayStatics::SpawnEmitterAtLocation(World, HitFX, GetActorLocation()), this);

			if (!bIsSuper)
			{
				Speed.X = 0.f;
				Speed.Y = 0.f;
			}
			else if (bIsSuper && OtherProj->bIsSuper)
			{
				if (!Hitbox->bAntiProjectile)
				{
					Speed.X = 0.f;
					Speed.Y = 0.f;
				}
				else if(Hitbox->bAntiProjectile && !OtherProj->Hitbox->bAntiProjectile)
				{
					OtherProj->Hitbox->bEnabled = false;
					OtherProj->BeamBox->bEnabled = false;
				}
				else if (Hitbox->bAntiProjectile && OtherProj->Hitbox->bAntiProjectile)
				{
					Speed.X = 0.f;
					Speed.Y = 0.f;
				}

			}

			TimerManager.SetTimer(NewHit, this, &ASSDProjectile::ResetProjectile, 1.f, false, TimeBetweenHits);
			if(!bProtected)
			RemainingHits--;	
			TimesHit++;
		}
	}
	else if (OtherComp->IsInA(USSDHitbox::StaticClass()))
	{
		USSDHitbox* OtherHitbox = Cast<USSDHitbox>(OtherComp);

		if (OtherHitbox->GetOwner() == FPOwner)
			return;

		if (OtherHitbox->bAntiProjectile)
		{
			if (RemainingHits == 1 && !bProtected)
			{
				DestroyWithEffects();
			}
			else if (bOneHitKO && !bProtected)
			{
				DestroyWithEffects();
			}

			else
			{
				if (HitSound)
					UGameplayStatics::PlaySound2D(World, HitSound);

				TimerManager.SetTimer(NewHit, this, &ASSDProjectile::ResetProjectile, 1.f, false, TimeBetweenHits);
				if(!bProtected)
				RemainingHits--;
				TimesHit++;
			}
		}
	}
}

void ASSDProjectile::Destroy_Effects()
{
	DestroyWithEffects();
}

void ASSDProjectile::DestroyWithEffects()
{
	UWorld* World = GetWorld();
	if (!World) return;

	ProjectileDeath();

	if(DeathFX)
		UPawnRenderLayerEffectComponent::BindEffect(UGameplayStatics::SpawnEmitterAtLocation(World, DeathFX, GetActorLocation()), this);
	if(DeathSFX)
		UGameplayStatics::PlaySound2D(World, DeathSFX);

	DestroyProjectile();
}

void ASSDProjectile::ResetProjectile()
{
	Hitbox->ResetHitbox();
	if(bIsBeam)
	BeamBox->ResetHitbox();
}

void ASSDProjectile::ProjectileTimeOver()
{
	OnTimeOut(); //Call BP event.
	Destroy_Effects();
}

void ASSDProjectile::OnTimeOut_Implementation()
{

}

void ASSDProjectile::ProjectileDeath_Implementation()
{

}

void ASSDProjectile::OnOpponentHit_Implementation(FSSDOverlapInfo OverlapInfo)
{

}
void ASSDProjectile::DestroyProjectile()
{
	Hitbox->DestroyHitbox();
	if(bIsBeam)
	BeamBox->DestroyHitbox();
	Destroy();
}

void ASSDProjectile::ResetLifetime(float NewLifetime)
{
	UWorld* World = GetWorld();

	if (World && (NewLifetime > 0.f))
	{
		World->GetTimerManager().SetTimer(Death, this, &ASSDProjectile::ProjectileTimeOver, 1.f, false, NewLifetime);
	}
}
