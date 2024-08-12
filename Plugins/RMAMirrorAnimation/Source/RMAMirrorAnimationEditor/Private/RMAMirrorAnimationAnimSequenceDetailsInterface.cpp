// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationAnimSequenceDetailsInterface.h"
#include "RMAMirrorAnimationEditor.h"
#include "Components/PropertyViewBase.h"

void URMAMirrorAnimationAnimSequenceDetailsInterface::SetAnimSequence(UAnimSequence* Value)
{

	AnimSequence = Value;

}

URMAMirrorAnimationAnimSequenceCustomData* URMAMirrorAnimationAnimSequenceDetailsInterface::GetCustomData()
{

	if (!CustomData && GetAnimSequence())
	{

		CustomData = Cast<URMAMirrorAnimationAnimSequenceCustomData>(GetAnimSequence()->GetAssetUserDataOfClass(URMAMirrorAnimationAnimSequenceCustomData::StaticClass()));

		if (!CustomData)
		{

			CustomData = NewObject<URMAMirrorAnimationAnimSequenceCustomData>();
			CustomData->OnPropertyChangedDelegate.AddLambda([&](const FPropertyChangedEvent& Event, const FString& Name)
			{
				
				CustomData->OnPropertyChangedDelegate.RemoveAll(this);
				CustomData = DuplicateObject(CustomData, GetAnimSequence());
				GetAnimSequence()->AddAssetUserData(CustomData);
				GetAnimSequence()->MarkPackageDirty();

			});

		}

	}

	return CustomData;

}

void URMAMirrorAnimationAnimSequenceDetailsInterface::MirrorAnimation()
{

	if (GetCustomData())
	{

		if (GetCustomData()->MirrorTable)
		{

			//MirrorAnimation Editor Module
			FRMAMirrorAnimationEditor* LMirrorAnimationEditorModule = &FModuleManager::LoadModuleChecked<FRMAMirrorAnimationEditor>("RMAMirrorAnimationEditor");
			LMirrorAnimationEditorModule->MirrorAnimations({ AnimSequence }, *GetCustomData()->MirrorTable, true);

		}

	}

}

void URMAMirrorAnimationAnimSequenceDetailsInterface::ResetAnimation()
{

	if (GetCustomData())
	{

		//MirrorAnimation Editor Module
		FRMAMirrorAnimationEditor* LMirrorAnimationEditorModule = &FModuleManager::LoadModuleChecked<FRMAMirrorAnimationEditor>("RMAMirrorAnimationEditor");
		LMirrorAnimationEditorModule->ResetAnimation(AnimSequence);

	}

}

void URMAMirrorAnimationAnimSequenceDetailsInterface::SetViewObject(UPropertyViewBase* ViewBase, UObject* Object)
{
	if (ViewBase)
	{
		ViewBase->SetObject(Object);
	}
}
