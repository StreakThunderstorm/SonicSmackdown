// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationAnimSequenceDetails.h"
#include "RMAMirrorAnimationAnimSequenceDetailsInterface.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"

TSharedRef<IDetailCustomization> FRMAMirrorAnimationAnimSequenceDetails::MakeInstance()
{

	return MakeShareable(new FRMAMirrorAnimationAnimSequenceDetails);

}

void FRMAMirrorAnimationAnimSequenceDetails::CustomizeDetails(IDetailLayoutBuilder& LayoutBuilder)
{
	
	IDetailCategoryBuilder& LCategoryBuilder = LayoutBuilder.EditCategory("MirrorAnimation");
	
	//Interface
	URMAMirrorAnimationAnimSequenceDetailsInterface* LInterface = NULL;
	UClass* LInterfaceClass = FStringClassReference("/RMAMirrorAnimation/UMG/AnimSequence/WBP_DetailsInterface.WBP_DetailsInterface_C").TryLoadClass<UUserWidget>();
	
	if (LInterfaceClass)
	{

		LInterface = NewObject<URMAMirrorAnimationAnimSequenceDetailsInterface>(GetTransientPackage(), LInterfaceClass);

	}

	if (LInterface)
	{

		TArray<TWeakObjectPtr<UObject>> LObjects;
		LayoutBuilder.GetObjectsBeingCustomized(LObjects);

		if (LObjects.Num() > 0)
		{

			LInterface->Initialize();
			LInterface->SetAnimSequence(Cast<UAnimSequence>(LObjects[0]));

			LCategoryBuilder
				.AddCustomRow(FText::GetEmpty())
				.WholeRowContent()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[

					LInterface->TakeWidget()

				];

		}

	}

}
