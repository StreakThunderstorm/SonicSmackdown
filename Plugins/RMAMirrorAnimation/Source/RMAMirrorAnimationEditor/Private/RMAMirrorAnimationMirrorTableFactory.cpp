// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationMirrorTableFactory.h"
#include "AssetTypeCategories.h"

URMAMirrorAnimationMirrorTableFactory::URMAMirrorAnimationMirrorTableFactory()
{

	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = URMAMirrorAnimationMirrorTable::StaticClass();

}

UObject* URMAMirrorAnimationMirrorTableFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{

	URMAMirrorAnimationMirrorTable* LMirrorTable = NewObject<URMAMirrorAnimationMirrorTable>(InParent, Name, Flags | RF_Transactional);

	return LMirrorTable;

}

FName URMAMirrorAnimationMirrorTableFactory::GetNewAssetThumbnailOverride() const
{

	return TEXT("MirrorTable");

}

FText URMAMirrorAnimationMirrorTableFactory::GetDisplayName() const
{

	return FText::FromString(FString("Mirror Table"));

}

FText URMAMirrorAnimationMirrorTableFactory::GetToolTip() const
{

	return FText::FromString(FString("Mirror Table"));

}

uint32 URMAMirrorAnimationMirrorTableFactory::GetMenuCategories() const
{

	return EAssetTypeCategories::Basic;

}
