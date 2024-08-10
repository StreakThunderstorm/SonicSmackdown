// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "OOSHUD.generated.h"

/**
*
*/
UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSHUD : public AHUD
{
	GENERATED_BODY()

public:
	AOOSHUD();

	virtual void DrawHUD() override;
	virtual void BeginPlay() override;

	UPROPERTY(Category = InputBufferDir, EditAnywhere, meta = (AllowPrivateAccess = "true")) UTexture* InputDebugTexture;
	UPROPERTY(Category = DebugInputSize, EditAnywhere, meta = (AllowPrivateAccess = "true")) uint8 YOffset = 0;
	UPROPERTY(Category = DebugInputSize, EditAnywhere, meta = (AllowPrivateAccess = "true")) uint8 IconSize = 50;
	UPROPERTY(Category = DebugInputSize, EditAnywhere, meta = (AllowPrivateAccess = "true")) uint8 IconClearance = 5;
	UPROPERTY(Category = DebugInputSize, EditAnywhere, meta = (ClampMin = "0", ClampMax = "32", UIMin = "0", UIMax = "32", AllowPrivateAccess = "true")) uint8 InputsToDisplay = 32;

	class AOOSPlayerController* OwningPC;

protected:

	float BorderSpacing;

private:

	float GlobalScale = 1.f;

	int ArrayOffset;
	float YPosition;

	void DrawLight(float XPos, float YPos);
	void DrawMedium(float XPos, float YPos);
	void DrawHeavy(float XPos, float YPos);
	void DrawSpecial(float XPos, float YPos);
};
