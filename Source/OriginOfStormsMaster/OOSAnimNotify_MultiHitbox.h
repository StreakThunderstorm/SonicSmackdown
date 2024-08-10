// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OOSAnimNotify_Hitbox.h"
#include "Components/SkeletalMeshComponent.h"
#include "OOSAnimNotify_MultiHitbox.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSAnimNotify_MultiHitbox : public UOOSAnimNotify_Hitbox
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime) override;
	
	UPROPERTY(Category = Effects, EditAnywhere) int NumHitboxes = 4.f;

protected:

	void DestroyHB(USkeletalMeshComponent* MeshComp);
	void SpawnHB(USkeletalMeshComponent* MeshComp);

private:

	float Period = 0.f;

	float P1LastSpawn = 0.f;
	float P1CurrentTime = 0.f;
	int P1Remaining = 0;

	float P2LastSpawn = 0.f;
	float P2CurrentTime = 0.f;
	int P2Remaining = 0;

};
