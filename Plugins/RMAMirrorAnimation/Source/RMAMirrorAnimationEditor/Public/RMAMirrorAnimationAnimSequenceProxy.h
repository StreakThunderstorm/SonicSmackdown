// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "Animation/AnimSequence.h"
#include "RMAMirrorAnimationAnimSequenceProxy.generated.h"

class URMAMirrorAnimationMirrorTable;

//AnimSequence Proxy
UCLASS()
class RMAMIRRORANIMATIONEDITOR_API URMAMirrorAnimationAnimSequenceProxy : public UAnimSequence
{

	GENERATED_BODY()

public:

	//Mirror Animation
	bool MirrorAnimation(URMAMirrorAnimationMirrorTable& MirrorTable);

	//Reset Animation
	void ResetAnimation();

};
