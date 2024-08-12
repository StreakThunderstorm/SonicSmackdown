// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "RMAMirrorAnimationMirrorTable.h"
#include "EditorUtilityWidget.h"
#include "RMAMirrorAnimationMirrorTableEditorInterface.generated.h"

class UPropertyViewBase;

//MirrorTable Editor Interface
UCLASS()
class RMAMIRRORANIMATIONEDITOR_API URMAMirrorAnimationMirrorTableEditorInterface : public UEditorUtilityWidget
{

	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "")
		void OnAnimSelectionChanged();

	//Getter MirrorTable
	UFUNCTION(BlueprintPure, Category = "")
		URMAMirrorAnimationMirrorTable* GetMirrorTable()
	{

		return MirrorTable;

	};

	//Setter MirrorTable
	void SetMirrorTable(URMAMirrorAnimationMirrorTable* Value);
	
	//Setter View Object
	UFUNCTION(BlueprintCallable, Category = "")
		void SetViewObject(UPropertyViewBase* ViewBase, UObject* Object);

protected:

	//MirrorTable
	UPROPERTY()
		URMAMirrorAnimationMirrorTable* MirrorTable;

	//OnAnimSelectionChanged Delegate
	FDelegateHandle OnAnimSelectionChangedDelegate;

};
