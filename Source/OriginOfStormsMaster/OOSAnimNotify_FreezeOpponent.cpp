// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSAnimNotify_FreezeOpponent.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "OOSPawn.h"
#include "OOSProjectile.h"
#include "OOSSonicRing.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"


void UOOSAnimNotify_FreezeOpponent::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
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
			if (Class && (Cast<UOOSAnimNotify_FreezeOpponent>(Class) == this))
			{
				Duration = Animation->Notifies[i].GetDuration();
			}
		}
	}

	// Now Pawn freezing is responsible for freezing owned particle systems.
	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->Opponent->Freeze(Duration, false, true);
	}

	// Projectile freezing as well takes care of its particle systems.
	for (TObjectIterator<AOOSProjectile> Proj; Proj; ++Proj)
	{
		Proj->Freeze(Duration);
	}

	TArray<AActor*> Rings;
	UGameplayStatics::GetAllActorsOfClass(World, AOOSSonicRing::StaticClass(), Rings);
	for (int i = 0; i < Rings.Num(); ++i)
	{
		AOOSSonicRing* Ring = Cast<AOOSSonicRing>(Rings[i]);
		if (Ring)
		{
			Ring->Freeze(Duration);
		}
	}
}

void UOOSAnimNotify_FreezeOpponent::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	AOOSPawn* FPOwner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
	if (FPOwner)
	{
		FPOwner->Opponent->Unfreeze();
	}

	for (TObjectIterator<AOOSProjectile> Proj; Proj; ++Proj)
	{
		Proj->Unfreeze();
	}

	TArray<AActor*> Rings;
	UGameplayStatics::GetAllActorsOfClass(World, AOOSSonicRing::StaticClass(), Rings);
	for (int i = 0; i < Rings.Num(); ++i)
	{
		AOOSSonicRing* Ring = Cast<AOOSSonicRing>(Rings[i]);
		if (Ring)
		{
			Ring->Unfreeze();
		}
	}
}
