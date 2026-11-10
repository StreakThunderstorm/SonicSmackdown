#pragma once

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

class FCharacterMaskCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCharacterMaskCS);
	SHADER_USE_PARAMETER_STRUCT(FCharacterMaskCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<uint2>, StencilTexture)
		SHADER_PARAMETER(FIntPoint, SourceMin)
		SHADER_PARAMETER(FIntPoint, MaskSize)
		SHADER_PARAMETER(uint32, StencilValue)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, MaskOutput)
	END_SHADER_PARAMETER_STRUCT()
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FCharacterContourCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCharacterContourCS);
	SHADER_USE_PARAMETER_STRUCT(FCharacterContourCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, CoverageTexture)
		SHADER_PARAMETER(FIntPoint, CoverageMin)
		SHADER_PARAMETER(FIntPoint, CoverageSize)
		SHADER_PARAMETER(FVector2f, StrokeSize)
		SHADER_PARAMETER(uint32, SegmentCapacity)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float4>, SegmentOutput)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<uint>, SegmentCountOutput)
	END_SHADER_PARAMETER_STRUCT()
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FCharacterArgumentsCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCharacterArgumentsCS);
	SHADER_USE_PARAMETER_STRUCT(FCharacterArgumentsCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(uint32, SegmentCapacity)
		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<uint>, SegmentCount)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<uint>, DrawArgumentsOutput)
	END_SHADER_PARAMETER_STRUCT()
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FCharacterFeaturesCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCharacterFeaturesCS);
	SHADER_USE_PARAMETER_STRUCT(FCharacterFeaturesCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SurfaceTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D<uint>, IdentityTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SurfaceDepth)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SurfacePositions)
		SHADER_PARAMETER(uint32, HasSurfacePositions)
		SHADER_PARAMETER(FIntPoint, SurfaceSize)
		SHADER_PARAMETER(FVector2f, StrokeSize)
		SHADER_PARAMETER(FMatrix44f, ClipToTranslatedWorld)
		SHADER_PARAMETER(float, DepthThreshold)
		SHADER_PARAMETER(float, CreaseCosine)
		SHADER_PARAMETER(uint32, EdgeFlags)
		SHADER_PARAMETER(uint32, SegmentCapacity)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float4>, SegmentOutput)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<uint>, SegmentCountOutput)
	END_SHADER_PARAMETER_STRUCT()
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

BEGIN_SHADER_PARAMETER_STRUCT(FCharacterStrokeParameters, )
	SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float4>, ContourSegments)
	SHADER_PARAMETER(FVector2f, StrokeSize)
	SHADER_PARAMETER(float, StrokeRadius)
	SHADER_PARAMETER(FVector2f, StrokeToReferenceScale)
	SHADER_PARAMETER(float, StrokeFilterWidth)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, CoverageTexture)
	SHADER_PARAMETER(FIntPoint, CoverageMin)
	SHADER_PARAMETER(FIntPoint, CoverageSize)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, CharacterDepth)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, VisibleDepth)
	SHADER_PARAMETER(FIntPoint, CharacterDepthMin)
	SHADER_PARAMETER(FIntPoint, CharacterDepthSize)
	SHADER_PARAMETER(FIntPoint, VisibleDepthMin)
	SHADER_PARAMETER(FIntPoint, VisibleDepthSize)
	SHADER_PARAMETER(uint32, VisibleOnly)
	SHADER_PARAMETER(float, DepthTolerance)
	RDG_BUFFER_ACCESS(DrawArguments, ERHIAccess::IndirectArgs)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

class FCharacterStrokeVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCharacterStrokeVS);
	SHADER_USE_PARAMETER_STRUCT(FCharacterStrokeVS, FGlobalShader);
	using FParameters = FCharacterStrokeParameters;
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FCharacterStrokePS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCharacterStrokePS);
	SHADER_USE_PARAMETER_STRUCT(FCharacterStrokePS, FGlobalShader);
	using FParameters = FCharacterStrokeParameters;
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FCharacterCompositePS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCharacterCompositePS);
	SHADER_USE_PARAMETER_STRUCT(FCharacterCompositePS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, StrokeTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, CoverageTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, CoverageSampler)
		SHADER_PARAMETER(FVector2f, CoverageTextureSize)
		SHADER_PARAMETER(FIntPoint, CoverageMin)
		SHADER_PARAMETER(FIntPoint, CoverageSize)
		SHADER_PARAMETER(FIntPoint, OutputMin)
		SHADER_PARAMETER(FIntPoint, OutputSize)
		SHADER_PARAMETER(int32, SamplesPerAxis)
		SHADER_PARAMETER(FVector4f, StrokeColor)
		SHADER_PARAMETER(int32, DebugMode)
		SHADER_PARAMETER(uint32, InteriorOnly)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, CharacterDepth)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, VisibleDepth)
		SHADER_PARAMETER(FIntPoint, CharacterDepthMin)
		SHADER_PARAMETER(FIntPoint, CharacterDepthSize)
		SHADER_PARAMETER(FIntPoint, VisibleDepthMin)
		SHADER_PARAMETER(FIntPoint, VisibleDepthSize)
		SHADER_PARAMETER(uint32, VisibleOnly)
		SHADER_PARAMETER(float, DepthTolerance)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FCharacterLineMaskPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCharacterLineMaskPS);
	SHADER_USE_PARAMETER_STRUCT(FCharacterLineMaskPS, FGlobalShader);
	using FParameters = FCharacterCompositePS::FParameters;
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

