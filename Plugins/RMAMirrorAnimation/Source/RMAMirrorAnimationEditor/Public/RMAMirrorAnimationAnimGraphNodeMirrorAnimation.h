// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "AnimGraphNode_AssetPlayerBase.h"
#include "RMAMirrorAnimationAnimNodeMirrorAnimation.h"
#include "RMAMirrorAnimationAnimGraphNodeMirrorAnimation.generated.h"

//AnimNode (Editor) Used To Mirror Animation In Blueprint Animation.
UCLASS(ClassGroup = (RMAMirrorAnimation))
class RMAMIRRORANIMATIONEDITOR_API URMAMirrorAnimationAnimGraphNodeMirrorAnimation : public UAnimGraphNode_AssetPlayerBase
{
	
	GENERATED_BODY()

	//Return The Color Of The Node
	virtual FLinearColor GetNodeTitleColor() const override
	{

		return FLinearColor::FromSRGBColor(FColor::FromHex("454589FF"));

	};

	//Return The Category Of The Node
	virtual FString GetNodeCategory() const override
	{

		return FString("RMAMirrorAnimation");

	};

	//Show Warning Message
	virtual void ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog) override;

	//Process This Node's Data During Compilation
	virtual void OnProcessDuringCompilation(IAnimBlueprintCompilationContext& InCompilationContext, IAnimBlueprintGeneratedClassCompiledData& OutCompiledData) override {};

	//Return The Name Of The Node
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
	{

		return FText::AsCultureInvariant("Mirror Animation");

	};

	//Load Dependencies
	virtual void PreloadRequiredAssets() override;

public:

	//Node (Runtime)
	UPROPERTY(EditAnywhere, Category = "Mirror Animation")
		FRMAMirrorAnimationAnimNodeMirrorAnimation Node;

};
