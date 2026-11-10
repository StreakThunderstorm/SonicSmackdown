#include "FDAssetTypeActions.h"
#include "AssetToolsModule.h"
#include "FrameDataEditorPCH.h"
#include "EdGraphUtilities.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY(FrameDataEditor)

#define LOCTEXT_NAMESPACE "FrameDataEditor"

class FFrameDataEditor : public IFrameDataEditor
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 8
	void SynchronizeAssetTypesBeforeGarbageCollection();
#endif

private:
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

	EAssetTypeCategories::Type FrameDataAssetCategoryBit;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 8
	FDelegateHandle PreGarbageCollectHandle;
	bool bSynchronizingAssetTypes = false;
#endif
};

IMPLEMENT_MODULE( FFrameDataEditor, FrameDataEditor )

void FFrameDataEditor::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 8
	if (!IsRunningCommandlet())
		PreGarbageCollectHandle = FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddRaw(
			this, &FFrameDataEditor::SynchronizeAssetTypesBeforeGarbageCollection);
#endif

	FrameDataAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("FrameData")), LOCTEXT("FrameDataAssetCategory", "FrameData"));

	RegisterAssetTypeAction(AssetTools, MakeShareable(new FFDAssetTypeActions(FrameDataAssetCategoryBit)));
}


void FFrameDataEditor::ShutdownModule()
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 8
	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().Remove(PreGarbageCollectHandle);
#endif
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

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 8
void FFrameDataEditor::SynchronizeAssetTypesBeforeGarbageCollection()
{
	if (!IsInGameThread() || IsRunningCommandlet() || bSynchronizingAssetTypes)
		return;

	TGuardValue<bool> Guard(bSynchronizingAssetTypes, true);
	if (FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools"))
	{
		// UE5.8 replaces the dynamic Resonance reverb definition with a static definition after engine initialization.
		// Refresh AssetTools' weak adapters before GC destroys the retired definition; otherwise Content Browser startup dereferences it.
		TArray<TWeakPtr<IAssetTypeActions>> AssetTypeActions;
		AssetToolsModule->Get().GetAssetTypeActionsList(AssetTypeActions);
	}
}
#endif

#undef LOCTEXT_NAMESPACE

