#pragma once

#include "CoreMinimal.h"
#include "EditorStyleSet.h"
#include "Framework/Commands/Commands.h"

class FMNEditorCommands : public TCommands<FMNEditorCommands>
{
public:
	/** Constructor */
	FMNEditorCommands()
		: TCommands<FMNEditorCommands>("MoveNetworkEditor", NSLOCTEXT("Contexts", "MoveNetworkEditor", "Move Network Editor"), NAME_None, FEditorStyle::GetStyleSetName())
	{
	}
	
	TSharedPtr<FUICommandInfo> GraphSettings;
	TSharedPtr<FUICommandInfo> AutoArrange;

	virtual void RegisterCommands() override;
};
