// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSAnimNotify_MultiHitbox.h"
#include "OOSHitbox.h"
#include "OOSPawn.h"
#include "Runtime/Engine/Classes/Animation/AnimSequenceBase.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Utils/OOSBlueprintFunctionLibrary.h"

void UOOSAnimNotify_MultiHitbox::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	if ((NumHitboxes == 0) || !MeshComp || !Animation) return;

	Period = TotalDuration / NumHitboxes;
	P1Remaining = NumHitboxes;
	P2Remaining = NumHitboxes;

	SpawnHB(MeshComp);
}

void UOOSAnimNotify_MultiHitbox::NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime)
{
	if ((NumHitboxes == 0) || !MeshComp || !Animation) return;

	AOOSPawn* Owner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
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

void UOOSAnimNotify_MultiHitbox::DestroyHB(USkeletalMeshComponent* MeshComp)
{
	AOOSPawn* Owner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
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

void UOOSAnimNotify_MultiHitbox::SpawnHB(USkeletalMeshComponent* MeshComp)
{

	AOOSPawn* Owner = UOOSBlueprintFunctionLibrary::GetOOSPawnOwner(MeshComp);
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

