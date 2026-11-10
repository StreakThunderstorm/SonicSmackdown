#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "SSDGameViewportClient.generated.h"

UCLASS()
class SONICSMACKDOWN_API USSDGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	virtual void RemapControllerInput(FInputKeyEventArgs& EventArgs) override;
};
