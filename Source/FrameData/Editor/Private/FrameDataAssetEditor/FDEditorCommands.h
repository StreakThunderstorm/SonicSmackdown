#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Styling/AppStyle.h"

class FFDEditorCommands : public TCommands<FFDEditorCommands>
{
public:
	/** Constructor */
	FFDEditorCommands()
		: TCommands<FFDEditorCommands>("FrameDataEditor", NSLOCTEXT("Contexts", "FrameDataEditor", "Frame Data Editor"),
			NAME_None, FAppStyle::GetAppStyleSetName())
	{
	}
	
	TSharedPtr<FUICommandInfo> TogglePlay;
	TSharedPtr<FUICommandInfo> StepForward;
	TSharedPtr<FUICommandInfo> StepBack;

	virtual void RegisterCommands() override;
};
