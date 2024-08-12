// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "IDetailCustomization.h"

//AnimSequence Details
class FRMAMirrorAnimationAnimSequenceDetails : public IDetailCustomization
{

public:

	//Make Instance For This Detail Class
	static TSharedRef<IDetailCustomization> MakeInstance();

	//Called When Details Should Be Customized
	virtual void CustomizeDetails(IDetailLayoutBuilder& LayoutBuilder) override;

};
