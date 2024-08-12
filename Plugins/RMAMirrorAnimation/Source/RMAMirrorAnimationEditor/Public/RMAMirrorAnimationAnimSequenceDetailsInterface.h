// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "RMAMirrorAnimationMirrorTable.h"
#include "RMAMirrorAnimationAnimSequenceCustomData.h"
#include "Animation/AnimSequence.h"
#include "EditorUtilityWidget.h"
#include "RMAMirrorAnimationAnimSequenceDetailsInterface.generated.h"

class UPropertyViewBase;

//AnimSequence Details Interface
UCLASS()
class RMAMIRRORANIMATIONEDITOR_API URMAMirrorAnimationAnimSequenceDetailsInterface : public UEditorUtilityWidget
{

	GENERATED_BODY()

public:

	//Getter AnimSequence
	UFUNCTION(BlueprintPure, Category = "")
		UAnimSequence* GetAnimSequence()
	{

		return AnimSequence;

	}

	//Setter AnimSequence
	void SetAnimSequence(UAnimSequence* Value);

	//Getter CustomData
	UFUNCTION(BlueprintPure, Category = "")
		URMAMirrorAnimationAnimSequenceCustomData* GetCustomData();

	//Mirror Animation
	UFUNCTION(BlueprintCallable, Category = "")
		void MirrorAnimation();

	//Reset Animation
	UFUNCTION(BlueprintCallable, Category = "")
		void ResetAnimation();

	//Setter View Object
	UFUNCTION(BlueprintCallable, Category = "")
		void SetViewObject(UPropertyViewBase* ViewBase, UObject* Object);

protected:

	//AnimSequence
	UPROPERTY()
		UAnimSequence* AnimSequence;

	//CustomData
	UPROPERTY()
		URMAMirrorAnimationAnimSequenceCustomData* CustomData;

};
