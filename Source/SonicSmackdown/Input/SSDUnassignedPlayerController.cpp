// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDUnassignedPlayerController.h"
#include "SSDGameMode.h"
#include "SSDPlayerController.h"
#include "Kismet/GameplayStatics.h"

void ASSDUnassignedPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// Any key pressed.
	FInputKeyBinding KBP(FInputChord(EKeys::AnyKey, false, false, false, false), EInputEvent::IE_Pressed);
	KBP.bConsumeInput = false;
	KBP.bExecuteWhenPaused = false;
	KBP.KeyDelegate.GetDelegateWithKeyForManualSet().BindLambda([this](const FKey& Key) {
		this->KeyPressed(Key);
		});
	InputComponent->KeyBindings.Add(KBP);

	InputComponent->BindAxis("HorizontalAxis", this, &ASSDUnassignedPlayerController::AxisPressed);
	InputComponent->BindAxis("VerticalAxis", this, &ASSDUnassignedPlayerController::AxisPressed);

}

void ASSDUnassignedPlayerController::KeyPressed(FKey Key)
{
	AlertGameMode();
}

void ASSDUnassignedPlayerController::AxisPressed(float Value)
{
	if (Value != 0.f)
		AlertGameMode();
}

void ASSDUnassignedPlayerController::AlertGameMode()
{
	const UWorld* World = GetWorld();
	if (!World) return;

	ASSDGameMode* GameMode = Cast<ASSDGameMode>(UGameplayStatics::GetGameMode(World));
	if (!GameMode) return;

	GameMode->UnassignedControllerInput(this);
}
