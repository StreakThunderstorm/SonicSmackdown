// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationEditor.h"
#include "RMAMirrorAnimation.h"
#include "RMAMirrorAnimationMirrorTableEditorInterface.h"
#include "RMAMirrorAnimationMirrorTableAssetTypeActions.h"
#include "RMAMirrorAnimationMirrorTableThumbnailRenderer.h"
#include "RMAMirrorAnimationAnimSequenceProxy.h"
#include "RMAMirrorAnimationAnimSequenceDetails.h"
#include "RMAMirrorAnimationAnimSequenceCustomData.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "Styling/SlateStyleRegistry.h"
#include "PropertyEditorModule.h"
#include "Components/NativeWidgetHost.h"
#include "IContentBrowserSingleton.h"
#include "Blueprint/WidgetTree.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"

void FRMAMirrorAnimationEditor::StartupModule()
{

	//Register Asset Actions
	FAssetToolsModule& LAssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	LAssetToolsModule.Get().RegisterAssetTypeActions(MakeShareable(new FRMAMirrorAnimationMirrorTableAssetTypeActions));

	//Register Asset Thumbnail
	UThumbnailManager::Get().RegisterCustomRenderer(URMAMirrorAnimationMirrorTable::StaticClass(), URMAMirrorAnimationMirrorTableThumbnailRenderer::StaticClass());

	//Icons
	FStringAssetReference LPluginIconPath("/RMAMirrorAnimation/Icons/T_Plugin_02.T_Plugin_02");
	UTexture* LPluginIcon = Cast<UTexture>(LPluginIconPath.TryLoad());

	if (LPluginIcon)
	{

		SlateStyleSet->Set("MirrorTable", new FSlateImageBrush(LPluginIcon, FVector2D(64.0f, 64.0f)));

	}

	//Register Slate Style
	FSlateStyleRegistry::RegisterSlateStyle(SlateStyleSet.Get());

	//Bind
	FRMAMirrorAnimation& LMirrorAnimationModule = FModuleManager::Get().LoadModuleChecked<FRMAMirrorAnimation>("RMAMirrorAnimation");
	LMirrorAnimationModule.OnMirrorAnimationsDelegate.AddLambda(
		[&](const TArray<UAnimSequence*>& Animations, URMAMirrorAnimationMirrorTable& MirrorTable)
		{

			MirrorAnimations(Animations, MirrorTable, false);

		});

	//Bind
	FContentBrowserModule& LContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	LContentBrowserModule.GetOnAssetSelectionChanged().AddLambda(
		[&](const TArray<FAssetData>& Selection, bool IsPrimaryBrowser)
		{

			if (OnAnimSelectionChangedDelegate.IsBound())
			{

				OnAnimSelectionChangedDelegate.Execute(true);

			}

		});

	//Bind
	FCoreDelegates::OnPostEngineInit.AddLambda(
		[]() 
		{

			//Register Class Layout
			FPropertyEditorModule& LPropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
			LPropertyEditorModule.RegisterCustomClassLayout("AnimSequence", FOnGetDetailCustomizationInstance::CreateStatic(&FRMAMirrorAnimationAnimSequenceDetails::MakeInstance));
			LPropertyEditorModule.NotifyCustomizationModuleChanged();

		});

}

void FRMAMirrorAnimationEditor::ShutdownModule()
{



}

void FRMAMirrorAnimationEditor::CreateMirrorTableEditor(const EToolkitMode::Type ToolkitMode, const TSharedPtr<IToolkitHost>& InitToolkitHost, URMAMirrorAnimationMirrorTable* MirrorTable)
{

	if (MirrorTable)
	{

		//Cache
		MirrorTableCache = MirrorTable;

		//TabID
		const FName LTabID = TabIDAppendPrefix(*FString::FromInt(MirrorTableCache->GetUniqueID()), MirrorTableEditorTabIDPrefix);

		if (!FGlobalTabmanager::Get()->FindExistingLiveTab(LTabID).IsValid())
		{

			MirrorTableEditorTabIDs.AddUnique(LTabID);

			//Register Tab
			FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LTabID, FOnSpawnTab::CreateRaw(this, &FRMAMirrorAnimationEditor::BuildMirrorTableEditor))
				.SetDisplayName(FText::FromName(MirrorTableCache->GetFName()))
				.SetMenuType(ETabSpawnerMenuType::Hidden);

			//Invoke Tab
			TSharedRef<SDockTab> LTabRef = FGlobalTabmanager::Get()->TryInvokeTab(LTabID).ToSharedRef();

			//AssetRegistry Module
			FAssetRegistryModule& LAssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

			//Bind
			LAssetRegistryModule.Get().OnAssetRenamed().AddRaw(this, &FRMAMirrorAnimationEditor::OnAssetRenamed);
			FEditorDelegates::OnAssetsPreDelete.AddRaw(this, &FRMAMirrorAnimationEditor::OnAssetsPreDelete);

			//Unregister Tab
			FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LTabID);

		}

		//Clear Cache
		MirrorTableCache = NULL;

	}

}

void FRMAMirrorAnimationEditor::MirrorAnimations(const TArray<UAnimSequence*>& Animations, URMAMirrorAnimationMirrorTable& MirrorTable, bool InPlace /* = false */)
{

	if (Animations.Num() > 0)
	{

		for (int LIndex = 0; LIndex < Animations.Num(); LIndex++)
		{

			UAnimSequence* LSource = Animations[LIndex];

			if (LSource)
			{

				URMAMirrorAnimationAnimSequenceProxy* LProxy = NewObject<URMAMirrorAnimationAnimSequenceProxy>();

				if (LProxy)
				{

					CopyData(LSource, LProxy);

					if (LProxy->MirrorAnimation(MirrorTable))
					{

						UAnimSequence* LOutAnimation = LSource;

						if (!InPlace)
						{

							FString LOutPath;
							FString LOutName;
							FAssetToolsModule& LAssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

							LAssetToolsModule.Get().CreateUniqueAssetName(LSource->GetOutermost()->GetPathName(), "_MR", LOutPath, LOutName);
							LOutAnimation = Cast<UAnimSequence>(LAssetToolsModule.Get().DuplicateAsset(LOutName, FPaths::GetPath(LOutPath), LSource));

						}

						if (LOutAnimation)
						{

							LOutAnimation->CreateAnimation(LProxy);
							LOutAnimation->AddAssetUserData(LProxy->GetAssetUserDataOfClass(URMAMirrorAnimationAnimSequenceCustomData::StaticClass()));
							LOutAnimation->MarkPackageDirty();

						}

					}

				}

			}

		}

	}

}

void FRMAMirrorAnimationEditor::ResetAnimation(UAnimSequence* Animation)
{

	if (Animation)
	{

		URMAMirrorAnimationAnimSequenceProxy* LProxy = NewObject<URMAMirrorAnimationAnimSequenceProxy>();

		if (LProxy)
		{

			CopyData(Animation, LProxy);
			LProxy->ResetAnimation();
			Animation->CreateAnimation(LProxy);
			Animation->MarkPackageDirty();

		}

	}

}

void FRMAMirrorAnimationEditor::CopyData(UObject* Source, UObject* Target)
{

	if (Source && Target)
	{

		TArray<uint8> LData;
		FObjectWriter LWriter = FObjectWriter(LData);
		Source->Serialize(LWriter);
		FObjectReader(Target, LData);

	}

}

void FRMAMirrorAnimationEditor::OnAssetRenamed(const FAssetData& AssetData, const FString& OldName)
{

	//MirrorTable
	URMAMirrorAnimationMirrorTable* LMirrorTable = Cast<URMAMirrorAnimationMirrorTable>(AssetData.GetAsset());

	if (LMirrorTable)
	{

		//TabID
		const FName LTabID = TabIDAppendPrefix(*FString::FromInt(LMirrorTable->GetUniqueID()), MirrorTableEditorTabIDPrefix);

		//TabPtr
		TSharedPtr<SDockTab> LTabPtr = FGlobalTabmanager::Get()->FindExistingLiveTab(LTabID);

		if (LTabPtr.IsValid())
		{

			//Update Label
			LTabPtr->SetLabel(FText::FromName(LMirrorTable->GetFName()));

		}

	}

}

void FRMAMirrorAnimationEditor::OnAssetsPreDelete(const TArray<UObject*>& Assets)
{

	//AssetRegistry Module
	FAssetRegistryModule& LAssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	for (int LIndex = 0; LIndex < MirrorTableEditorTabIDs.Num(); LIndex++)
	{

		//TabPtr
		TSharedPtr<SDockTab> LTabPtr = FGlobalTabmanager::Get()->FindExistingLiveTab(MirrorTableEditorTabIDs[LIndex]);

		if (LTabPtr.IsValid())
		{

			const FString LMirrorTableID = TabIDRemovePrefix(MirrorTableEditorTabIDs[LIndex], MirrorTableEditorTabIDPrefix).ToString();
			URMAMirrorAnimationMirrorTable* LMirrorTable = Cast<URMAMirrorAnimationMirrorTable>(URMAMirrorAnimationFunctionLibrary::GetAssetByUniqueID(LMirrorTableID, URMAMirrorAnimationMirrorTable::StaticClass()));

			if (LMirrorTable)
			{

				if (Assets.Find(LMirrorTable) >= 0)
				{

					//Close Tab
					LTabPtr->RequestCloseTab();

					return;

				}

				else
				{

					//Dependencies
					TArray<FName> LDependencies;
					LAssetRegistryModule.Get().GetDependencies(LMirrorTable->GetOutermost()->FileName, LDependencies);

					for (int LAssetIndex = 0; LAssetIndex < Assets.Num(); LAssetIndex++)
					{

						if (Assets[LAssetIndex]->GetOuter())
						{

							if (LDependencies.Find(*Assets[LAssetIndex]->GetOuter()->GetPathName()) >= 0)
							{

								//Close Tab
								LTabPtr->RequestCloseTab();

								return;

							}

						}

					}

				}

			}

		}

	}

}

FName FRMAMirrorAnimationEditor::TabIDAppendPrefix(FName TabID, FName Prefix)
{

	return *Prefix.ToString().Append(TabID.ToString());

}

FName FRMAMirrorAnimationEditor::TabIDRemovePrefix(FName TabID, FName Prefix)
{

	FString LTabID = TabID.ToString();
	LTabID.RemoveAt(0, Prefix.ToString().Len());

	return *LTabID;

}

TSharedRef<class SDockTab> FRMAMirrorAnimationEditor::BuildMirrorTableEditor(const FSpawnTabArgs& SpawnArgs)
{

	//Interface
	URMAMirrorAnimationMirrorTableEditorInterface* LInterface = NULL;
	UClass* LInterfaceClass = FStringClassReference("/RMAMirrorAnimation/UMG/WBP_Interface.WBP_Interface_C").TryLoadClass<UUserWidget>();

	if (LInterfaceClass)
	{

		LInterface = NewObject<URMAMirrorAnimationMirrorTableEditorInterface>(GetTransientPackage(), LInterfaceClass);

	}

	if (LInterface && MirrorTableCache)
	{

		LInterface->Initialize();
		LInterface->SetMirrorTable(MirrorTableCache);

		UNativeWidgetHost* LAnimSelectionHost = Cast<UNativeWidgetHost>(LInterface->WidgetTree->FindWidget("AnimSelection_NWH"));

		if (LAnimSelectionHost)
		{

			FAssetPickerConfig LAssetPickerConfig = FAssetPickerConfig();

			LAssetPickerConfig.SelectionMode = ESelectionMode::None;
			LAssetPickerConfig.ThumbnailScale = 0.4f;
			LAssetPickerConfig.bShowBottomToolbar = false;
			LAssetPickerConfig.bAutohideSearchBar = true;
			LAssetPickerConfig.bAllowDragging = false;
			LAssetPickerConfig.bCanShowClasses = false;
			LAssetPickerConfig.bCanShowDevelopersFolder = false;
			LAssetPickerConfig.bPreloadAssetsForContextMenu = false;

			LAssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateLambda(
				[](const FAssetData& AssetData)
				{

					if (AssetData.GetClass()->IsChildOf(UAnimSequence::StaticClass()))
					{

						TArray<FAssetData> LSelection;
						GEditor->GetContentBrowserSelections(LSelection);

						return LSelection.Find(AssetData) < 0;

					}

					return true;

				});
			
			LAssetPickerConfig.RefreshAssetViewDelegates.Add(&OnAnimSelectionChangedDelegate);

			FContentBrowserModule& LContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			TSharedRef<SWidget> LAssetPickerWidget = LContentBrowserModule.Get().CreateAssetPicker(LAssetPickerConfig);
			LAnimSelectionHost->SetContent(LAssetPickerWidget);

		}

		return SNew(SDockTab)
			.TabRole(NomadTab)
			[

				SNew(SBox)
				.HAlign(EHorizontalAlignment::HAlign_Fill)
				.VAlign(EVerticalAlignment::VAlign_Fill)
				[

					LInterface->TakeWidget()

				]
			
			];

	}

	return SNew(SDockTab);

}

IMPLEMENT_MODULE(FRMAMirrorAnimationEditor, RMAMirrorAnimationEditor)
