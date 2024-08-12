// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
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
	void ClearDeprecatedData(UAnimSequence* AnimSequence);

	//MirrorTable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MirrorAnimation")
		URMAMirrorAnimationMirrorTable* MirrorTable;

	//AnimationData
	UPROPERTY()
		TArray<FBoneAnimationTrack> AnimationData;

	//OnPropertyChanged Delegate
	FOnPropertyChanged OnPropertyChangedDelegate;

};
