// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationAnimSequenceDetailsInterface.h"
#include "RMAMirrorAnimationEditor.h"

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

					GetAnimSequence()->AddAssetUserData(CustomData);
					GetAnimSequence()->MarkPackageDirty();
					CustomData->OnPropertyChangedDelegate.RemoveAll(this);

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
