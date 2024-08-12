// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"
#include "RMAMirrorAnimationMirrorTableThumbnailRenderer.generated.h"

//MirrorTable Thumbnail
UCLASS()
class RMAMIRRORANIMATIONEDITOR_API URMAMirrorAnimationMirrorTableThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{

	GENERATED_BODY()
	
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily) override;

};
