#pragma once

#include "ScreenPass.h"

class FViewInfo;
class FSceneInterface;

void UpdateCharacterSilhouetteOwners(FSceneInterface* Scene);

struct FCharacterSilhouetteSurface
{
	FScreenPassTexture Coverage;
	FScreenPassTexture Depth;
	FScreenPassTexture Normals;
	FScreenPassTexture Identity;
	FScreenPassTexture Positions;
};

FCharacterSilhouetteSurface AddCharacterSilhouetteSurfacePass(FRDGBuilder& GraphBuilder, const FViewInfo& View,
	int32 StencilMin, int32 StencilMax, int32 ResolutionScale);
