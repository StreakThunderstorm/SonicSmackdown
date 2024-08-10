// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationMirrorTableEditorInterface.h"

void URMAMirrorAnimationMirrorTableEditorInterface::NativeConstruct()
{

	if (!OnAnimSelectionChangedDelegate.IsValid())
	{

		//Bind
		FContentBrowserModule& LContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		OnAnimSelectionChangedDelegate = LContentBrowserModule.GetOnAssetSelectionChanged().AddLambda(
			[&](const TArray<FAssetData>& Selection, bool IsPrimaryBrowser)
			{

				OnAnimSelectionChanged();

			});

	}

	Super::NativeConstruct();
	
}

void URMAMirrorAnimationMirrorTableEditorInterface::NativeDestruct()
{

	//Unbind
	FContentBrowserModule& LContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	LContentBrowserModule.GetOnAssetSelectionChanged().Remove(OnAnimSelectionChangedDelegate);
	OnAnimSelectionChangedDelegate.Reset();

	Super::NativeDestruct();

}

void URMAMirrorAnimationMirrorTableEditorInterface::SetMirrorTable(URMAMirrorAnimationMirrorTable* Value)
{

	MirrorTable = Value;

}
