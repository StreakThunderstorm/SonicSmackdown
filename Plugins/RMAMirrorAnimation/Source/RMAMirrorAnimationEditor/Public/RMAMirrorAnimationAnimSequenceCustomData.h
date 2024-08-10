// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "Engine/AssetUserData.h"
#include "RMAMirrorAnimationMirrorTable.h"
#include "RMAMirrorAnimationAnimSequenceCustomData.generated.h"

//AnimSequence CustomData
UCLASS()
class RMAMIRRORANIMATIONEDITOR_API URMAMirrorAnimationAnimSequenceCustomData : public UAssetUserData
{

	GENERATED_BODY()

public:

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	//MirrorTable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MirrorAnimation")
		URMAMirrorAnimationMirrorTable* MirrorTable;

	//RawAnimationData
	TArray<FRawAnimSequenceTrack> RawAnimationData;

	//OnPropertyChanged Delegate
	FOnPropertyChanged OnPropertyChangedDelegate;

};
