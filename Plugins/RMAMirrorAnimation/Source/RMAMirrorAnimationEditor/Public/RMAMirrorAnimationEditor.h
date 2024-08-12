// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "ContentBrowserDelegates.h"

class URMAMirrorAnimationMirrorTable;

//Module Editor
class FRMAMirrorAnimationEditor : public IModuleInterface
{

public:

	//Return The Module Instance
	static inline FRMAMirrorAnimationEditor& Get()
	{

		return FModuleManager::LoadModuleChecked<FRMAMirrorAnimationEditor>("RMAMirrorAnimationEditor");

	}

	//Return The Module Was Loaded
	static inline bool IsAvailable()
	{

		return FModuleManager::Get().IsModuleLoaded("RMAMirrorAnimationEditor");

	}

	//Called After The Module Has Been Loaded
	virtual void StartupModule() override;

	//Called Before The Module Is Unloaded
	virtual void ShutdownModule() override;

public:

	//Create MirrorTable Editor
	void CreateMirrorTableEditor(const EToolkitMode::Type ToolkitMode, const TSharedPtr<IToolkitHost>& InitToolkitHost, URMAMirrorAnimationMirrorTable* MirrorTable);

	//Mirror Animations
	void MirrorAnimations(const TArray<UAnimSequence*>& Animations, URMAMirrorAnimationMirrorTable& MirrorTable, bool InPlace = false);

	//Reset Animation
	void ResetAnimation(UAnimSequence* Animation);

private:

	//Copy Animation
	void CopyAnimation(UAnimSequence* Source, UAnimSequence* Target);

	//On Asset Renamed
	void OnAssetRenamed(const FAssetData& AssetData, const FString& OldName);

	//On Assets PreDelete
	void OnAssetsPreDelete(const TArray<UObject*>& Assets);

	//TabID Append Prefix
	FName TabIDAppendPrefix(FName TabID, FName Prefix);

	//TabID Remove Prefix
	FName TabIDRemovePrefix(FName TabID, FName Prefix);

	//Build MirrorTable Editor
	TSharedRef<class SDockTab> BuildMirrorTableEditor(const FSpawnTabArgs& SpawnArgs);

protected:

	//Slate Style Set
	TSharedRef<FSlateStyleSet> SlateStyleSet = MakeShareable(new FSlateStyleSet("RMAMirrorAnimation"));

	//MirrorTable Cache
	UPROPERTY()
		URMAMirrorAnimationMirrorTable* MirrorTableCache;

	//MirrorTable Editor TabIDs
	TArray<FName> MirrorTableEditorTabIDs;

	//MirrorTable Editor TabID Prefix
	const FName MirrorTableEditorTabIDPrefix = "MirrorTableEditor";

	//OnAnimSelectionChanged Delegate
	FRefreshAssetViewDelegate OnAnimSelectionChangedDelegate;

};
