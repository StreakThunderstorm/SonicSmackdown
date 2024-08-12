// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "AssetTypeActions_Base.h"
#include "RMAMirrorAnimationMirrorTable.h"

//MirrorTable AssetType Actions
class FRMAMirrorAnimationMirrorTableAssetTypeActions : public FAssetTypeActions_Base
{

public:

	//Return The Name Of This Type
	virtual FText GetName() const override
	{	

		return FText::FromString(FString("MirrorTable"));

	}

	//Return The Color Associated With This Type
	virtual FColor GetTypeColor() const override
	{
		
		return FColor::FromHex("454589FF");

	}

	//Checks To See If The Specified Object Is Handled By This Type
	virtual UClass* GetSupportedClass() const override
	{

		return URMAMirrorAnimationMirrorTable::StaticClass();

	}

	//Opens The Asset Editor For The Specified Objects. If EditWithinLevelEditor Is Valid, The World-Centric Editor Will Be Used
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

	//Return The Categories That This Asset Type. The Return Value Is One Or More Flags From EAssetTypeCategories
	virtual uint32 GetCategories() override
	{

		return EAssetTypeCategories::Basic;

	}

};
