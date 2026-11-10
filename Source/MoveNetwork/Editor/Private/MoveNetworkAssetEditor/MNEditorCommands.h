#pragma once

#include "CoreMinimal.h"
#include "Styling/AppStyle.h"
#include "Framework/Commands/Commands.h"

class FMNEditorCommands : public TCommands<FMNEditorCommands>
{
public:
	/** Constructor */
	FMNEditorCommands()
		: TCommands<FMNEditorCommands>("MoveNetworkEditor", NSLOCTEXT("Contexts", "MoveNetworkEditor", "Move Network Editor"),
			NAME_None, FAppStyle::GetAppStyleSetName())
	{
	}
	
	TSharedPtr<FUICommandInfo> GraphSettings;
	TSharedPtr<FUICommandInfo> AutoArrange;

	virtual void RegisterCommands() override;
};
