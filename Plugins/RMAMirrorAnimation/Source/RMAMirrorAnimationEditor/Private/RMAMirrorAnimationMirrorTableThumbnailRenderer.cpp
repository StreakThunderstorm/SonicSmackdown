// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationMirrorTableThumbnailRenderer.h"
#include "CanvasItem.h"

void URMAMirrorAnimationMirrorTableThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily)
{

	//Path To The Texture
	FStringAssetReference LTexturePath("/RMAMirrorAnimation/Icons/T_Plugin_01.T_Plugin_01");

	//Try Load The Texture
	UTexture* LTexture = Cast<UTexture>(LTexturePath.TryLoad());

	if (LTexture)
	{
		
		FCanvasTileItem LTileItem(FVector2D(X, Y), LTexture->Resource, FVector2D(Width, Height), FLinearColor::White);

		//Draw Texture
		Canvas->DrawItem(LTileItem);

	}
	
}
