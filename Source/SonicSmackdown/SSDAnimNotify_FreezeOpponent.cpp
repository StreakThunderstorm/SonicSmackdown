// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDAnimNotify_FreezeOpponent.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Kismet/GameplayStatics.h"
#include "SSDPawn.h"
#include "SSDProjectile.h"
#include "SSDSonicRing.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"


void USSDAnimNotify_FreezeOpponent::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	float Duration = -1.f;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	UAnimSingleNodeInstance* SNI = MeshComp->GetSingleNodeInstance();
	if (SNI)
	{
		for (int i = 0; i < Animation->Notifies.Num(); ++i)
		{
			UAnimNotifyState* Class = Animation->Notifies[i].NotifyStateClass;
			if (Class && (Cast<USSDAnimNotify_FreezeOpponent>(Class) == this))
			{
				Duration = Animation->Notifies[i].GetDuration();
			}
		}
	}

	// Now Pawn freezing is responsible for freezing owned particle systems.
	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->Opponent->Freeze(Duration, false, true);
	}

	// Projectile freezing as well takes care of its particle systems.
	for (TObjectIterator<ASSDProjectile> Proj; Proj; ++Proj)
	{
		Proj->Freeze(Duration);
	}

	TArray<AActor*> Rings;
	UGameplayStatics::GetAllActorsOfClass(World, ASSDSonicRing::StaticClass(), Rings);
	for (int i = 0; i < Rings.Num(); ++i)
	{
		ASSDSonicRing* Ring = Cast<ASSDSonicRing>(Rings[i]);
		if (Ring)
		{
			Ring->Freeze(Duration);
		}
	}
}

void USSDAnimNotify_FreezeOpponent::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation,
	const FAnimNotifyEventReference& EventReference)
{
	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	ASSDPawn* FPOwner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->Opponent->Unfreeze();
	}

	for (TObjectIterator<ASSDProjectile> Proj; Proj; ++Proj)
	{
		Proj->Unfreeze();
	}

	TArray<AActor*> Rings;
	UGameplayStatics::GetAllActorsOfClass(World, ASSDSonicRing::StaticClass(), Rings);
	for (int i = 0; i < Rings.Num(); ++i)
	{
		ASSDSonicRing* Ring = Cast<ASSDSonicRing>(Rings[i]);
		if (Ring)
		{
			Ring->Unfreeze();
		}
	}
}
