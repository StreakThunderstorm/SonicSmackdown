// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/SSDSliderWidget.h"
#include "Components/CanvasPanelSlot.h"

void USSDSliderWidget::NativePreConstruct()
{
	// Make the parent canvas slot report its designed size before the Blueprint
	// PreConstruct reads it (see class comment).
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		// Only the layout is pushed: the Z order has to be left for the canvas to
		// apply once the slot is attached, or the slot would not be sorted.
		const FAnchorData Layout = CanvasSlot->GetLayout();
		CanvasSlot->SetOffsets(Layout.Offsets);
		CanvasSlot->SetAnchors(Layout.Anchors);
		CanvasSlot->SetAlignment(Layout.Alignment);
	}

	Super::NativePreConstruct();
}
