// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationMirrorTableAssetTypeActions.h"
#include "RMAMirrorAnimationEditor.h"

void FRMAMirrorAnimationMirrorTableAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{

	//ToolkitMode
	EToolkitMode::Type LToolkitMode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	//MirrorAnimation Editor Module
	FRMAMirrorAnimationEditor* LMirrorAnimationEditorModule = &FModuleManager::LoadModuleChecked<FRMAMirrorAnimationEditor>("RMAMirrorAnimationEditor");

	for (auto LObjectIt = InObjects.CreateConstIterator(); LObjectIt; ++LObjectIt)
	{

		//MirrorTable
		URMAMirrorAnimationMirrorTable* LMirrorTable = Cast<URMAMirrorAnimationMirrorTable>(*LObjectIt);

		if (LMirrorTable)
		{

			LMirrorAnimationEditorModule->CreateMirrorTableEditor(LToolkitMode, EditWithinLevelEditor, LMirrorTable);

		}

	}

}
