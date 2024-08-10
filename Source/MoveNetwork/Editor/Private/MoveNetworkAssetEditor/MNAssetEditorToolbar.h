
#pragma once

#include "CoreMinimal.h"

class FMNAssetEditor;
class FExtender;
class FToolBarBuilder;

class FMNAssetEditorToolbar : public TSharedFromThis<FMNAssetEditorToolbar>
{
public:
	FMNAssetEditorToolbar(TSharedPtr<FMNAssetEditor> InMoveNetworkEditor)
		: MoveNetworkEditor(InMoveNetworkEditor) {}

	void AddMoveNetworkToolbar(TSharedPtr<FExtender> Extender);

private:
	void FillMoveNetworkToolbar(FToolBarBuilder& ToolbarBuilder);

protected:
	/** Pointer back to the blueprint editor tool that owns us */
	TWeakPtr<FMNAssetEditor> MoveNetworkEditor;
};
