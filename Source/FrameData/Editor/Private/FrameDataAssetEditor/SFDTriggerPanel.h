// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SScrubControlPanel.h"

class UFrameData;

class FRAMEDATAEDITOR_API SFDTriggerPanel : public SScrubControlPanel
{
public:

	SLATE_BEGIN_ARGS(SFDTriggerPanel)
		: _FrameDataAsset()
		, _Frame(0)
	{}
		SLATE_ARGUMENT(UFrameData*, FrameDataAsset)
		SLATE_ATTRIBUTE(uint32, Frame)
	SLATE_END_ARGS()


	/**
	 * Construct the widget
	 * 
	 * @param InArgs   A declaration from which to construct the widget
	 */
	void Construct( const FArguments& InArgs );

	void Update(bool bForce = false);

private:

	FReply OnAddTriggerClicked(FVector2D CursorPos);
	TSharedPtr<SWidget> SummonContextMenu(FVector2D Position);
	template<typename TriggerTypeClass>
	void MakeNewTriggerPicker(class FMenuBuilder& MenuBuilder);
	void AddNewTrigger(UClass* InClass);

	UFrameData* FrameDataAsset;

	TAttribute<uint32> FrameAttribute;
	uint32 CurrentFrame;

	FVector2D LastClickPosition;

};

