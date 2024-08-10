#include "MNAssetTypeActions.h"
#include "MoveNetworkEditorPCH.h"
#include "MoveNetworkAssetEditor/MNAssetEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_MoveNetwork"

FMNAssetTypeActions::FMNAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FText FMNAssetTypeActions::GetName() const
{
	return LOCTEXT("FMoveNetworkAssetTypeActionsName", "Move Network");
}

FColor FMNAssetTypeActions::GetTypeColor() const
{
	return FColor(129, 196, 115);
}

UClass* FMNAssetTypeActions::GetSupportedClass() const
{
	return UMoveNetwork::StaticClass();
}

void FMNAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UMoveNetwork* Graph = Cast<UMoveNetwork>(*ObjIt))
		{
			TSharedRef<FMNAssetEditor> NewGraphEditor(new FMNAssetEditor());
			NewGraphEditor->InitMoveNetworkAssetEditor(Mode, EditWithinLevelEditor, Graph);
		}
	}
}

uint32 FMNAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Animation | MyAssetCategory;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE