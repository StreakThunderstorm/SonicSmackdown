#include "SSDGameViewportClient.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameMapsSettings.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "HAL/IConsoleManager.h"
#include "InputKeyEventArgs.h"

void USSDGameViewportClient::RemapControllerInput(FInputKeyEventArgs& EventArgs)
{
	UWorld* CurrentWorld = GetWorld();
	UGameInstance* CurrentGameInstance = CurrentWorld ? CurrentWorld->GetGameInstance() : nullptr;
	const IConsoleVariable* RemapDeviceIds = IConsoleManager::Get().FindConsoleVariable(TEXT("input.bRemapDeviceIdForOffsetPlayerGamepadIds"));
	if (!CurrentGameInstance || CurrentGameInstance->GetNumLocalPlayers() <= 1 || !EventArgs.Key.IsGamepadKey()
		|| !GetDefault<UGameMapsSettings>()->bOffsetPlayerGamepadIds || !RemapDeviceIds || !RemapDeviceIds->GetBool())
	{
		Super::RemapControllerInput(EventArgs);
		return;
	}

	// Matches create the selected fighter first, so local-player indices need not match controller IDs.
	// Keep keyboard at ID 0 and route offset gamepads by the same IDs used by selection and saved bindings.
	ULocalPlayer* TargetPlayer = GEngine->GetLocalPlayerFromControllerId(this, EventArgs.ControllerId + 1);
	if (!TargetPlayer || !TargetPlayer->GetPlatformUserId().IsValid())
	{
		Super::RemapControllerInput(EventArgs);
		return;
	}

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	const FPlatformUserId TargetUser = TargetPlayer->GetPlatformUserId();
	FInputDeviceId TargetDevice = DeviceMapper.GetPrimaryInputDeviceForUser(TargetUser);
	if (!TargetDevice.IsValid())
	{
		TargetDevice = DeviceMapper.AllocateNewInputDeviceId();
		DeviceMapper.Internal_MapInputDeviceToUser(TargetDevice, TargetUser, EInputDeviceConnectionState::Connected);
	}

	EventArgs.InputDevice = TargetDevice;
	++EventArgs.ControllerId;
}
