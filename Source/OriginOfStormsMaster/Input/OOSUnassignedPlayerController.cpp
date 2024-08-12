// Fill out your copyright notice in the Description page of Project Settings.


#include "OOSUnassignedPlayerController.h"
#include "OriginOfStormsMaster/OOSGameMode.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

void AOOSUnassignedPlayerController::SetupInputComponent()
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

	InputComponent->BindAxis("HorizontalAxis", this, &AOOSUnassignedPlayerController::AxisPressed);
	InputComponent->BindAxis("VerticalAxis", this, &AOOSUnassignedPlayerController::AxisPressed);

}

void AOOSUnassignedPlayerController::KeyPressed(FKey Key)
{
	AlertGameMode();
}

void AOOSUnassignedPlayerController::AxisPressed(float Value)
{
	if (Value != 0.f)
		AlertGameMode();
}

void AOOSUnassignedPlayerController::AlertGameMode()
{
	const UWorld* World = GetWorld();
	if (!World) return;

	AOOSGameMode* GameMode = Cast<AOOSGameMode>(UGameplayStatics::GetGameMode(World));
	if (!GameMode) return;

	GameMode->UnassignedControllerInput(this);
}
