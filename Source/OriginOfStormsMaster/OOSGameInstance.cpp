// Fill out your copyright notice in the Description page of Project Settings.


#include "OOSGameInstance.h"
#include "InputCoreTypes.h"
#include "OOSArcadeLadder.h"

UOOSGameInstance::UOOSGameInstance()
{
	Bindings.Add(FOOSPlayerBindings::KeyboardDefaults());
	Bindings.Add(FOOSPlayerBindings::GamepadDefaults());
	Bindings.Add(FOOSPlayerBindings::GamepadDefaults());
	Bindings.Add(FOOSPlayerBindings::GamepadDefaults());

	AILevel = 1.f;

}

void UOOSGameInstance::ResetModeData()
{
	bTrainingMode = false;
	bVersusCPU = false;
	bCPUVersusCPU = false;
	bArcadeMode = false;
}

bool UOOSGameInstance::IsSinglePlayerMode() const
{
	return bTrainingMode || bVersusCPU || bCPUVersusCPU;
}

void UOOSGameInstance::RestoreBindingDefaults(int Index)
{
	if (Index < 0 || Index >= Bindings.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to restore defaults for a controller outside the range"));
		return;
	}

	// Keyboard
	if (Index == 0)
	{
		Bindings[Index] = FOOSPlayerBindings::KeyboardDefaults();
	}
	// Gamepad
	else
	{
		Bindings[Index] = FOOSPlayerBindings::GamepadDefaults();
	}
}

void UOOSGameInstance::SetNullBindingsToDefaults()
{
	for (int i = 0; i < Bindings.Num(); i++)
	{
		if (Bindings[i].Directions.Num() == 0 && Bindings[i].Attacks.Num() == 0)
		{
			RestoreBindingDefaults(i);
		}
	}
}