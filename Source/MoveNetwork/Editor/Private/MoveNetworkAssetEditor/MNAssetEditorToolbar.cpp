#include "MNAssetEditorToolbar.h"

#include "EditorStyleSet.h"
#include "MNAssetEditor.h"
#include "MNEditorCommands.h"
#include "MNEditorStyle.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "AssetEditorToolbar_MoveNetwork"

void FMNAssetEditorToolbar::AddMoveNetworkToolbar(TSharedPtr<FExtender> Extender)
{
	check(MoveNetworkEditor.IsValid());
	TSharedPtr<FMNAssetEditor> MoveNetworkEditorPtr = MoveNetworkEditor.Pin();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, MoveNetworkEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP( this, &FMNAssetEditorToolbar::FillMoveNetworkToolbar ));
	MoveNetworkEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FMNAssetEditorToolbar::FillMoveNetworkToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(MoveNetworkEditor.IsValid());
	TSharedPtr<FMNAssetEditor> MoveNetworkEditorPtr = MoveNetworkEditor.Pin();

	ToolbarBuilder.BeginSection("Move Network");
	{
		ToolbarBuilder.AddToolBarButton(FMNEditorCommands::Get().GraphSettings,
			NAME_None,
			LOCTEXT("GraphSettings_Label", "Graph Settings"),
			LOCTEXT("GraphSettings_ToolTip", "Show the Graph Settings"),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.GameSettings"));
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Util");
	{
		ToolbarBuilder.AddToolBarButton(FMNEditorCommands::Get().AutoArrange,
			NAME_None,
			LOCTEXT("AutoArrange_Label", "Auto Arrange"),
			LOCTEXT("AutoArrange_ToolTip", "Auto arrange nodes, not perfect, but still handy"),
			FSlateIcon(FMNEditorStyle::GetStyleSetName(), "MoveNetworkEditor.AutoArrange"));
	}
	ToolbarBuilder.EndSection();

}


#undef LOCTEXT_NAMESPACE
