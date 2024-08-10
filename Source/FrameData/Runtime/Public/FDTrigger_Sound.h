// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FDTrigger.h"
#include "FDTrigger_Sound.generated.h"

/**
 * 
 */
UCLASS()
class FRAMEDATARUNTIME_API UFDTrigger_Sound : public UFDTrigger
{
	GENERATED_BODY()

public:

	UFDTrigger_Sound();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	USoundBase* SoundAsset;
	
};
