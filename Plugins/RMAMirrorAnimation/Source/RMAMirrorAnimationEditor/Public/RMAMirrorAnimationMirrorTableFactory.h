// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "RMAMirrorAnimationMirrorTable.h"
#include "RMAMirrorAnimationMirrorTableFactory.generated.h"

//MirrorTable Factory
UCLASS()
class RMAMIRRORANIMATIONEDITOR_API URMAMirrorAnimationMirrorTableFactory : public UFactory
{

	GENERATED_BODY()
		
public:

	URMAMirrorAnimationMirrorTableFactory();

	//Create New Asset
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	//Override Thumbnail To It Asset
	virtual FName GetNewAssetThumbnailOverride() const override;

	//Return The Name Of The Factory For Menus
	virtual FText GetDisplayName() const override;

	//Return The Tooltip Text Description Of This Factory
	virtual FText GetToolTip() const override;

	//Category For This Factory, BitFlag Mask Using EAssetTypeCategories
	virtual uint32 GetMenuCategories() const override;

};
