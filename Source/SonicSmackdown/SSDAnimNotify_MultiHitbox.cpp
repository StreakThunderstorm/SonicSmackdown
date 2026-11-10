// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDAnimNotify_MultiHitbox.h"
#include "SSDHitbox.h"
#include "SSDPawn.h"
#include "Animation/AnimSequenceBase.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/SSDBlueprintFunctionLibrary.h"

void USSDAnimNotify_MultiHitbox::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	if ((NumHitboxes == 0) || !MeshComp || !Animation) return;

	Period = TotalDuration / NumHitboxes;
	P1Remaining = NumHitboxes;
	P2Remaining = NumHitboxes;

	SpawnHB(MeshComp);
}

void USSDAnimNotify_MultiHitbox::NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	if ((NumHitboxes == 0) || !MeshComp || !Animation) return;

	ASSDPawn* Owner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	float* LastSpawn;
	float* CurrentTime;
	int* Remaining;
	if (Owner)
	{
		if (Owner->PlayerIndex == 0)
		{
			LastSpawn = &P1LastSpawn;
			CurrentTime = &P1CurrentTime;
			Remaining = &P1Remaining;
		}
		else if (Owner->PlayerIndex == 1)
		{
			LastSpawn = &P2LastSpawn;
			CurrentTime = &P2CurrentTime;
			Remaining = &P2Remaining;
		}
		else return;
	}
	else
	{
		// Working in editor, use P1 stuff.
		LastSpawn = &P1LastSpawn;
		CurrentTime = &P1CurrentTime;
		Remaining = &P1Remaining;
	}

	*CurrentTime += UGameplayStatics::GetGlobalTimeDilation(MeshComp->GetWorld()) * FrameDeltaTime * Animation->RateScale;
	
	if (((*CurrentTime - *LastSpawn) >= Period) && (*Remaining > 0)) SpawnHB(MeshComp);
}

void USSDAnimNotify_MultiHitbox::DestroyHB(USkeletalMeshComponent* MeshComp)
{
	ASSDPawn* Owner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	if (Owner)
	{
		if (Owner->PlayerIndex == 0)
		{
			if (P1Hitbox && P1Hitbox->IsValidLowLevel())
			{
				P1Hitbox->DestroyHitbox();
				P1Hitbox = nullptr;
			}
		}
		else if (Owner->PlayerIndex == 1)
		{
			if (P2Hitbox && P2Hitbox->IsValidLowLevel())
			{
				P2Hitbox->DestroyHitbox();
				P2Hitbox = nullptr;
			}
		}
	}
	else
	{
		if (P1Hitbox && P1Hitbox->IsValidLowLevel())
		{
			P1Hitbox->DestroyHitbox();
			P1Hitbox = nullptr;
		}
	}
}

void USSDAnimNotify_MultiHitbox::SpawnHB(USkeletalMeshComponent* MeshComp)
{

	ASSDPawn* Owner = USSDBlueprintFunctionLibrary::GetSSDPawnOwner(MeshComp);
	float* LastSpawn;
	float* CurrentTime;
	int* Remaining;
	if (Owner)
	{
		if (Owner->PlayerIndex == 0)
		{
			LastSpawn = &P1LastSpawn;
			CurrentTime = &P1CurrentTime;
			Remaining = &P1Remaining;
		}
		else if (Owner->PlayerIndex == 1)
		{
			LastSpawn = &P2LastSpawn;
			CurrentTime = &P2CurrentTime;
			Remaining = &P2Remaining;
		}
		else return;
	}
	else
	{
		// Working in editor, use P1 stuff.
		LastSpawn = &P1LastSpawn;
		CurrentTime = &P1CurrentTime;
		Remaining = &P1Remaining;
	}

	DestroyHB(MeshComp);
	SpawnHitbox(MeshComp);

	*LastSpawn = *CurrentTime;
	*Remaining--;
}

