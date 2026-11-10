// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "SSDUnassignedPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API ASSDUnassignedPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

private:
	void KeyPressed(FKey Key);
	void AxisPressed(float Value);

	void AlertGameMode();

};
