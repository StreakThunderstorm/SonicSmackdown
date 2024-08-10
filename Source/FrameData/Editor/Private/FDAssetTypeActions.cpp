#include "FDAssetTypeActions.h"
#include "FrameDataEditorPCH.h"
#include "FrameDataAssetEditor/FDAssetEditor.h"

#define LOCTEXT_NAMESPACE "FDAssetTypeActions"

FFDAssetTypeActions::FFDAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FText FFDAssetTypeActions::GetName() const
{
	return LOCTEXT("FFrameDataAssetTypeActionsName", "Frame Data");
}

FColor FFDAssetTypeActions::GetTypeColor() const
{
	return FColor(129, 196, 115);
}

UClass* FFDAssetTypeActions::GetSupportedClass() const
{
	return UFrameData::StaticClass();
}

void FFDAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UFrameData* FD = Cast<UFrameData>(*ObjIt))
		{
			TSharedRef<FFDAssetEditor> NewEditor(new FFDAssetEditor());
			NewEditor->InitFrameDataAssetEditor(Mode, EditWithinLevelEditor, FD);
		}
	}
}

uint32 FFDAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Animation | MyAssetCategory;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE