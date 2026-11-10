// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDGameInstance.h"
#include "InputCoreTypes.h"
#include "SSDArcadeLadder.h"

USSDGameInstance::USSDGameInstance()
{
	Bindings.Add(FSSDPlayerBindings::KeyboardDefaults());
	Bindings.Add(FSSDPlayerBindings::GamepadDefaults());
	Bindings.Add(FSSDPlayerBindings::GamepadDefaults());
	Bindings.Add(FSSDPlayerBindings::GamepadDefaults());

	AILevel = 1.f;

}

void USSDGameInstance::ResetModeData()
{
	bTrainingMode = false;
	bVersusCPU = false;
	bCPUVersusCPU = false;
	bArcadeMode = false;
}

bool USSDGameInstance::IsSinglePlayerMode() const
{
	return bTrainingMode || bVersusCPU || bCPUVersusCPU;
}

void USSDGameInstance::RestoreBindingDefaults(int Index)
{
	if (Index < 0 || Index >= Bindings.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to restore defaults for a controller outside the range"));
		return;
	}

	// Keyboard
	if (Index == 0)
	{
		Bindings[Index] = FSSDPlayerBindings::KeyboardDefaults();
	}
	// Gamepad
	else
	{
		Bindings[Index] = FSSDPlayerBindings::GamepadDefaults();
	}
}

void USSDGameInstance::SetNullBindingsToDefaults()
{
	for (int i = 0; i < Bindings.Num(); i++)
	{
		if (Bindings[i].Directions.Num() == 0 && Bindings[i].Attacks.Num() == 0)
		{
			RestoreBindingDefaults(i);
		}
	}
}