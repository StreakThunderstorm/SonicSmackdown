// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSProjectile.h"
#include "OOSPawn.h"
#include "OOSMovementComponent.h"
#include "SkeletalAnimation/OOSSkeletalMeshComponent.h"
#include "Engine.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"

// Sets default values
AOOSProjectile::AOOSProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Hitbox = CreateDefaultSubobject<UOOSHitbox>("Hitbox");
	RootComponent = Hitbox;
	BeamBox = CreateDefaultSubobject<UOOSHitbox>("BeamBox");
	BeamBox->SetupAttachment(RootComponent);
}

void AOOSProjectile::Initialize
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
	enum EOOSHitHeight Height, 
	enum EOOSInputAttack Att, 
	enum EOOSLaunchType Launch,
	bool bForceExtension,
	bool bResetExtensions,
	enum EOOSDirectionMode Direction, 
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
		FPOwner = Cast<AOOSPawn>(PawnOwner);
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
			World->GetTimerManager().SetTimer(Death, this, &AOOSProjectile::ProjectileTimeOver, 1.f, false, Lifetime);
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

AOOSProjectile* AOOSProjectile::SpawnChild
(
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
	bool bForceExtension,
	bool bResetExtensions,
	EOOSDirectionMode Direction, 
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
	AOOSProjectile* NewProj = World->SpawnActorDeferred<AOOSProjectile>(Class, T, this);
	if (!NewProj) return nullptr;

	NewProj->PawnOwner = PawnOwner;
	NewProj->Initialize
	(
		NumberOfHits, 
		HitTime, 
		Lifetime, 
		Speed, 
		PawnOwner->GetActorForwardVector(), 
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
void AOOSProjectile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AOOSProjectile::Tick(float DeltaTime)
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

void AOOSProjectile::Freeze(float Duration)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FTimerManager& TimerManager = World->GetTimerManager();
	if(TimerManager.IsTimerActive(NewHit)) TimerManager.PauseTimer(NewHit);
	if (TimerManager.IsTimerActive(Death)) TimerManager.PauseTimer(Death);

	if (Duration >= 0.f)
	{
		World->GetTimerManager().SetTimer(FreezeTimer, this, &AOOSProjectile::Unfreeze, Duration);
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

void AOOSProjectile::Unfreeze()
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

void AOOSProjectile::StopAnim(float Duration)
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

void AOOSProjectile::ResumeAnim()
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

void AOOSProjectile::OnContact(FOOSOverlapInfo OverlapInfo)
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
		World->GetTimerManager().SetTimer(NewHit, this, &AOOSProjectile::ResetProjectile, 1.f, false, TimeBetweenHits);
		RemainingHits--;
		TimesHit++;
	}

}

void AOOSProjectile::ProjectileBeginOverlap(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (!OtherActor) return;																		//This check is for components that begin play already overlapping. It seems that the overlap event is fired before they are assigned an owner.
	if (!OtherActor->IsInA(AOOSProjectile::StaticClass()) && !OtherActor->GetOwner() && !OtherActor->IsInA(AOOSPawn::StaticClass()))
	{
		return;		//If not a Projectile and doesn't have owner.
	}
	AOOSPawn* OtherOwner = Cast<AOOSPawn>(OtherActor->GetOwner());									//Get Projectile's owner pawn.
	if (FPOwner == OtherOwner) return;		
	
	FTimerManager& TimerManager = World->GetTimerManager();
	
	//If Projectile's owner pawn is the same, do nothing.


	if (OtherActor->IsInA(AOOSProjectile::StaticClass()))
	{
		AOOSProjectile* OtherProj = Cast<AOOSProjectile>(OtherActor);
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
					UGameplayStatics::SpawnEmitterAtLocation(World, HitFX, GetActorLocation());		

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

			TimerManager.SetTimer(NewHit, this, &AOOSProjectile::ResetProjectile, 1.f, false, TimeBetweenHits);
			if(!bProtected)
			RemainingHits--;	
			TimesHit++;
		}
	}
	else if (OtherComp->IsInA(UOOSHitbox::StaticClass()))
	{
		UOOSHitbox* Hitbox = Cast<UOOSHitbox>(OtherComp);

		if (Hitbox->GetOwner() == FPOwner)
			return;

		if (Hitbox->bAntiProjectile)
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

				TimerManager.SetTimer(NewHit, this, &AOOSProjectile::ResetProjectile, 1.f, false, TimeBetweenHits);
				if(!bProtected)
				RemainingHits--;
				TimesHit++;
			}
		}
	}
}

void AOOSProjectile::Destroy_Effects()
{
	DestroyWithEffects();
}

void AOOSProjectile::DestroyWithEffects()
{
	UWorld* World = GetWorld();
	if (!World) return;

	ProjectileDeath();

	if(DeathFX)
		UGameplayStatics::SpawnEmitterAtLocation(World, DeathFX, GetActorLocation());
	if(DeathSFX)
		UGameplayStatics::PlaySound2D(World, DeathSFX);

	DestroyProjectile();
}

void AOOSProjectile::ResetProjectile()
{
	Hitbox->ResetHitbox();
	if(bIsBeam)
	BeamBox->ResetHitbox();
}

void AOOSProjectile::ProjectileTimeOver()
{
	OnTimeOut(); //Call BP event.
	Destroy_Effects();
}

void AOOSProjectile::OnTimeOut_Implementation()
{

}

void AOOSProjectile::ProjectileDeath_Implementation()
{

}

void AOOSProjectile::OnOpponentHit_Implementation(FOOSOverlapInfo OverlapInfo)
{

}
void AOOSProjectile::DestroyProjectile()
{
	Hitbox->DestroyHitbox();
	if(bIsBeam)
	BeamBox->DestroyHitbox();
	Destroy();
}

void AOOSProjectile::ResetLifetime(float NewLifetime)
{
	UWorld* World = GetWorld();

	if (World && (NewLifetime > 0.f))
	{
		World->GetTimerManager().SetTimer(Death, this, &AOOSProjectile::ProjectileTimeOver, 1.f, false, NewLifetime);
	}
}