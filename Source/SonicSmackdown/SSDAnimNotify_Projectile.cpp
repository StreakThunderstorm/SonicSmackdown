// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDAnimNotify_Projectile.h"
#include "Animation/AnimSequenceBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/BlockingVolume.h"
#include "Engine/World.h"
#include "SSDPawn.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"

void USSDAnimNotify_Projectile::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	SpawnProjectile(MeshComp);
}

void USSDAnimNotify_Projectile::SpawnProjectile(USkeletalMeshComponent* MeshComp)
{
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->AddProjectile
		(
			MeshComp, 
			ProjectileClass, 
			NumberOfHits, 
			TimeBetweenHits, 
			Lifetime, 
			Speed, 
			Location, 
			Rotation, 
			bGrabMode,
			bForceGrab, 
			GrabSocket,
			HitParticle, 
			HitSound, 
			DeathParticle, 
			DeathSound, 
			Damage, 
			HitStun, 
			SelfHitLag, 
			OpponentHitLag, 
			BlockStun, 
			PushFactor, 
			Freeze, 
			FreezeTwitch, 
			bOffTheGround, 
			bOverhead, 
			bAutoPerformChildMove,
			bDontCancel,
			bUnblockable,
			bAntiArmor,
			bAntiProjectile, 
			HitHeight, 
			AttackType,
			Launch,
			bForceComboExtension,
			bResetComboExtensions,
			DirectionMode, 
			LaunchSpeed
		, 
			bForceTryPostLaunchJump
		);
	}
	// If there's no Pawn, then we're working in the animation editor.
	else
	{
		UWorld* World = MeshComp->GetWorld();
		if (!World) return;

		AActor* Owner = MeshComp->GetOwner();
		if (!Owner) return;

		// Kill the old projectiles
		TArray<AActor*> OldProjectiles;
		UGameplayStatics::GetAllActorsOfClass(World, ASSDProjectile::StaticClass(), OldProjectiles);
		for (int i = 0; i < OldProjectiles.Num(); i++)
		{
			ASSDProjectile* PJ = Cast<ASSDProjectile>(OldProjectiles[i]);
			if (PJ) PJ->Destroy_Effects(); // Call destroy handler that plays effects in editor.
			else OldProjectiles[i]->Destroy();
		}

		FTransform T = FTransform(Rotation, Owner->GetActorLocation() + FVector(Location.Y, Location.X, Location.Z), FVector(1.f, 1.f, 1.f));
		ASSDProjectile* NewProj = World->SpawnActorDeferred<ASSDProjectile>(ProjectileClass, T, Owner);
		if (!NewProj) return;

		NewProj->PawnOwner = Owner;
		NewProj->Initialize
		(
			NumberOfHits, 
			TimeBetweenHits, 
			Lifetime, 
			Speed, 
			FVector::RightVector, 
			bGrabMode, 
			bForceGrab,
			GrabSocket,
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
			Freeze, 
			FreezeTwitch, 
			bOffTheGround, 
			bOverhead, 
			bAutoPerformChildMove, 
			bDontCancel,
			bUnblockable,
			bAntiArmor,
			bAntiProjectile,
			HitHeight, 
			AttackType, 
			Launch,
			bForceComboExtension,
			bResetComboExtensions,
			DirectionMode,
			LaunchSpeed, 
			bForceTryPostLaunchJump
		);
		NewProj->Hitbox->bDebug = true;
		NewProj->Hitbox->DebugColor = FColor::Red;
		NewProj->Hitbox->DebugThickness = 0.5f;
		NewProj->InitLocation = Owner->GetActorLocation() + FVector(Location.Y, Location.X, Location.Z);
		NewProj->InitRelative = FVector(Location.Y, Location.X, Location.Z);

		NewProj->FinishSpawning(T);
	}
}
