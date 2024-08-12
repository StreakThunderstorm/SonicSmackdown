#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorStyle.h"
#include "EditorStyleSet.h"

class FFDEditorCommands : public TCommands<FFDEditorCommands>
{
public:
	/** Constructor */
	FFDEditorCommands()
		: TCommands("FrameDataEditor", NSLOCTEXT("Contexts", "FrameDataEditor", "Frame Data Editor"), NAME_None, FEditorStyle::GetStyleSetName())
	{
	}
	
	TSharedPtr<FUICommandInfo> TogglePlay;
	TSharedPtr<FUICommandInfo> StepForward;
	TSharedPtr<FUICommandInfo> StepBack;

	virtual void RegisterCommands() override;
};
