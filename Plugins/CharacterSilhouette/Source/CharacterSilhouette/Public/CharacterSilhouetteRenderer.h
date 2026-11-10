#pragma once

#include "CoreMinimal.h"
#include "ScreenPass.h"

struct FCharacterSilhouetteSettings
{
	// Outward width in ReferenceViewSize pixels, or Output.ViewRect pixels when no reference size is supplied.
	float WidthPixels = 3.f;
	// Optional final-image size for measuring width when Output is a differently sized internal scene target.
	FIntPoint ReferenceViewSize = FIntPoint::ZeroValue;
	// Only RGB is used. Full stroke coverage is opaque; alpha is reserved for geometric antialiasing.
	FLinearColor Color = FLinearColor::Black;
	// Stroke sampling only. A coarse source mask still has a coarse silhouette.
	int32 SamplesPerAxis = 2;
	// Overflow suppresses the whole stroke instead of drawing an incomplete contour.
	uint32 MaxSegments = 1048576;
	// 0: stroke; 1: source coverage; 2: stroke on both sides of the contour.
	int32 DebugMode = 0;
};

struct FCharacterSilhouetteOcclusion
{
	// Optional reversed-Z depth inputs describing the same camera region as Coverage and Output.
	FScreenPassTexture CharacterDepth;
	FScreenPassTexture SceneDepth;
	float DeviceDepthTolerance = 0.000001f;
};

struct FCharacterSilhouetteFeatures
{
	FScreenPassTexture Normals;
	FScreenPassTexture Identity;
	FScreenPassTexture Depth;
	FScreenPassTexture Positions;
	// Independent edge families can use independent stroke settings in separate invocations.
	bool bCharacterOverlaps = false;
	bool bSelfDepth = false;
	bool bNormalCreases = false;
	float DepthThreshold = 2.f;
	float CreaseAngle = 45.f;
	FMatrix44f ClipToTranslatedWorld = FMatrix44f::Identity;
};

// Optional RGBA LineMask receives visible coverage in alpha, unioned over successive calls. Clear it to transparent before the first call.
// Its view rect must match Output. Alpha is data and remains unaffected by post-process material exposure correction.
CHARACTERSILHOUETTE_API void AddCharacterSilhouettePass(FRDGBuilder& GraphBuilder, const FSceneView& View,
	FScreenPassTexture Coverage, FScreenPassRenderTarget Output, const FCharacterSilhouetteSettings& Settings,
	const FCharacterSilhouetteOcclusion* Occlusion = nullptr, const FCharacterSilhouetteFeatures* Features = nullptr,
	FScreenPassRenderTarget LineMask = {});
