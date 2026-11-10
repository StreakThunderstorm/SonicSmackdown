// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDAnimNotify_Hitbox.h"
#include "SSDHitbox.h"
#include "Animation/AnimSequenceBase.h"
#include "Engine/World.h"
#include "SSDPawn.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"

USSDAnimNotify_Hitbox::USSDAnimNotify_Hitbox()
{
	HitboxType = USSDHitbox::StaticClass();

}

void USSDAnimNotify_Hitbox::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	SpawnHitbox(MeshComp);
}

void USSDAnimNotify_Hitbox::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	ASSDPawn* Owner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (Owner)
	{
		if (Owner->PlayerIndex == 0)
		{
			if (P1Hitbox)
			{
				P1Hitbox->DestroyHitbox();
				P1Hitbox = nullptr;
			}
		}
		else if (Owner->PlayerIndex == 1)
		{
			if (P2Hitbox)
			{
				P2Hitbox->DestroyHitbox();
				P2Hitbox = nullptr;
			}
		}		
	}
	else
	{
		if (P1Hitbox)
		{
			P1Hitbox->DestroyHitbox();
			P1Hitbox = nullptr;
		}
	}
}

void USSDAnimNotify_Hitbox::SpawnHitbox(USkeletalMeshComponent* MeshComp)
{	
	ASSDPawn* Owner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (Owner)
	{
		USSDHitbox* NewHB = Owner->AddHitbox
		(
			MeshComp,
			HitboxType, 
			Location, 
			Rotation, 
			HalfHeight, 
			Radius,
			bGrabMode,
			bForceGrab, 
			GrabSocket,
			HitParticle, 
			HitSound,
			bBurst, 
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
		, 
			SocketName
		);

		if (Owner->PlayerIndex == 0)
		{
			P1Hitbox = NewHB;
		}
		else if (Owner->PlayerIndex == 1)
		{
			P2Hitbox = NewHB;
		}
	}
	// If there's no Pawn, then we're working in the animation editor.
	else
	{
		USSDHitbox* NewHB = NewObject<USSDHitbox>(MeshComp, HitboxType);
		if (!NewHB)	return;

		NewHB->AttachToComponent(MeshComp, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false), SocketName);
		NewHB->PrimaryComponentTick.bCanEverTick = true;
		NewHB->SetVisibility(true);
		NewHB->bTickInEditor = true;

		NewHB->Initialize
		(
			HalfHeight, 
			Radius, 
			bGrabMode, 
			bForceGrab,
			GrabSocket,
			HitParticle, 
			HitSound, 
			bBurst,
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

		NewHB->bDebug = true;
		NewHB->DebugColor = FColor::Red;
		NewHB->DebugThickness = 0.5f;

		NewHB->RegisterComponent();

		NewHB->SetRelativeLocation(Location);
		NewHB->SetRelativeRotation(Rotation);

		// Use P1 stuff for editor
		P1Hitbox = NewHB;

	}
}
