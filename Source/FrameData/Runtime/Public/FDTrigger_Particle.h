// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FDTrigger.h"
#include "FDTrigger_Particle.generated.h"

/**
 * 
 */
UCLASS()
class FRAMEDATARUNTIME_API UFDTrigger_Particle : public UFDTrigger
{
	GENERATED_BODY()

public:

	UFDTrigger_Particle();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Particle)
	UParticleSystem* HitParticle;
	
};
