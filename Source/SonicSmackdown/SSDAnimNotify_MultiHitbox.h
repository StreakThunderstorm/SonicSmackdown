// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SSDAnimNotify_Hitbox.h"
#include "Components/SkeletalMeshComponent.h"
#include "SSDAnimNotify_MultiHitbox.generated.h"

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API USSDAnimNotify_MultiHitbox : public USSDAnimNotify_Hitbox
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;
	
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
