// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "OOSUnassignedPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSUnassignedPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

private:
	void KeyPressed(FKey Key);
	void AxisPressed(float Value);

	void AlertGameMode();

};
