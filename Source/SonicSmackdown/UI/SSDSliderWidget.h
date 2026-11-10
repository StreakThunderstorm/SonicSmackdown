// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SSDSliderWidget.generated.h"

/**
 * Native base for BP_Slider.
 *
 * BP_Slider lays out its notches and handle in PreConstruct from the size of the
 * canvas slot it sits in. Since UE5, the parent canvas exposes its Slate slot to the
 * UCanvasPanelSlot before this widget is built, but only pushes the layout data into
 * it afterwards. During PreConstruct the slot therefore reports the Slate default
 * size (1x1) instead of the designed one, and everything ends up misplaced.
 * Pushing the layout data early makes the slot report its real size.
 */
UCLASS(Abstract)
class SONICSMACKDOWN_API USSDSliderWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
};
