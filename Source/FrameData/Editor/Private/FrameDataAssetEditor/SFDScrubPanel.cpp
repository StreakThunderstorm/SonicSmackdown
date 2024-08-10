// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SFDScrubPanel.h"
#include "FrameData.h"
#include "FDTrigger.h"
#include "Engine.h"
#include "Editor\ClassViewer\Public\ClassViewerModule.h"
#include "Editor\ClassViewer\Public\ClassViewerFilter.h"
#include "Runtime\CoreUObject\Public\UObject\UObjectIterator.h"
#include "Runtime\Slate\Public\Framework\Application\SlateApplication.h"
#include "Runtime\Slate\Public\Framework\MultiBox\MultiBoxBuilder.h"
#include "Runtime\SlateCore\Public\Layout\WidgetPath.h"

#define LOCTEXT_NAMESPACE "FDScrubPanel"

void SFDScrubPanel::Construct( const FArguments& InArgs )
{
	FrameDataAsset = InArgs._FrameDataAsset;

	this->ChildSlot
	.Padding(FMargin(0.0f, 0.0f))
	[
		SNew(SScrubControlPanel)
		.Value(InArgs._Value)
		.NumOfKeys(InArgs._NumOfKeys)
		.SequenceLength(InArgs._SequenceLength)
		.DisplayDrag(InArgs._DisplayDrag)
		.OnValueChanged(InArgs._OnValueChanged)
		.OnBeginSliderMovement(InArgs._OnBeginSliderMovement)
		.OnEndSliderMovement(InArgs._OnEndSliderMovement)
		.OnClickedForwardPlay(InArgs._OnClickedForwardPlay)
		.OnClickedRecord(InArgs._OnClickedRecord)
		.OnClickedBackwardPlay(InArgs._OnClickedBackwardPlay)
		.OnClickedForwardStep(InArgs._OnClickedForwardStep)
		.OnClickedBackwardStep(InArgs._OnClickedBackwardStep)
		.OnClickedForwardEnd(InArgs._OnClickedForwardEnd)
		.OnClickedBackwardEnd(InArgs._OnClickedBackwardEnd)
		.OnClickedToggleLoop(InArgs._OnClickedToggleLoop)
		.OnGetLooping(InArgs._OnGetLooping)
		.OnGetPlaybackMode(InArgs._OnGetPlaybackMode)
		.OnGetRecording(InArgs._OnGetRecording)
		.ViewInputMin(InArgs._ViewInputMin)
		.ViewInputMax(InArgs._ViewInputMax)
		.OnSetInputViewRange(InArgs._OnSetInputViewRange)
		.OnCropAnimSequence(InArgs._OnCropAnimSequence)
		.OnTickPlayback(InArgs._OnTickPlayback)
	];
}

#undef LOCTEXT_NAMESPACE
