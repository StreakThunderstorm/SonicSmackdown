// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "RMAMirrorAnimationMirrorTable.h"
#include "Blutility/Classes/EditorUtilityWidget.h"
#include "RMAMirrorAnimationMirrorTableEditorInterface.generated.h"

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

protected:

	//MirrorTable
	UPROPERTY()
		URMAMirrorAnimationMirrorTable* MirrorTable;

	//OnAnimSelectionChanged Delegate
	FDelegateHandle OnAnimSelectionChangedDelegate;

};
