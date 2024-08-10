// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationAnimSequenceCustomData.h"

void URMAMirrorAnimationAnimSequenceCustomData::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{

	Super::PostEditChangeProperty(PropertyChangedEvent);

	OnPropertyChangedDelegate.Broadcast(PropertyChangedEvent, PropertyChangedEvent.GetPropertyName().ToString());

}
