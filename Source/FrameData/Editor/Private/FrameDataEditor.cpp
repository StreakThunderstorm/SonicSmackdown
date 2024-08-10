#include "FDAssetTypeActions.h"
#include "FrameDataEditorPCH.h"
#include "EdGraphUtilities.h"

DEFINE_LOG_CATEGORY(FrameDataEditor)

#define LOCTEXT_NAMESPACE "FrameDataEditor"

class FFrameDataEditor : public IFrameDataEditor
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);

private:
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

	EAssetTypeCategories::Type FrameDataAssetCategoryBit;
};

IMPLEMENT_MODULE( FFrameDataEditor, FrameDataEditor )

void FFrameDataEditor::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	FrameDataAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("FrameData")), LOCTEXT("FrameDataAssetCategory", "FrameData"));

	RegisterAssetTypeAction(AssetTools, MakeShareable(new FFDAssetTypeActions(FrameDataAssetCategoryBit)));
}


void FFrameDataEditor::ShutdownModule()
{
	// Unregister all the asset types that we registered
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}
}

void FFrameDataEditor::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE

