#include "CharacterSilhouetteRenderer.h"
#include "CharacterSilhouetteShaders.h"
#include "CommonRenderResources.h"
#include "PixelShaderUtils.h"
#include "PipelineStateCache.h"
#include "RenderGraphUtils.h"

static bool SupportsCharacterStroke(EShaderPlatform Platform)
{
	return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
}

bool FCharacterMaskCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return SupportsCharacterStroke(Parameters.Platform);
}

bool FCharacterContourCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return SupportsCharacterStroke(Parameters.Platform);
}

bool FCharacterFeaturesCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return SupportsCharacterStroke(Parameters.Platform);
}

bool FCharacterArgumentsCS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return SupportsCharacterStroke(Parameters.Platform);
}

bool FCharacterStrokeVS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return SupportsCharacterStroke(Parameters.Platform);
}

bool FCharacterStrokePS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return SupportsCharacterStroke(Parameters.Platform);
}

bool FCharacterCompositePS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return SupportsCharacterStroke(Parameters.Platform);
}

bool FCharacterLineMaskPS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return SupportsCharacterStroke(Parameters.Platform);
}

IMPLEMENT_GLOBAL_SHADER(FCharacterMaskCS, "/Plugin/CharacterSilhouette/Private/CharacterSilhouette.usf", "ExtractMaskCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FCharacterContourCS, "/Plugin/CharacterSilhouette/Private/CharacterSilhouette.usf", "ExtractContourCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FCharacterFeaturesCS, "/Plugin/CharacterSilhouette/Private/CharacterFeatures.usf", "FeaturesCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FCharacterArgumentsCS, "/Plugin/CharacterSilhouette/Private/CharacterSilhouette.usf", "BuildArgumentsCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FCharacterStrokeVS, "/Plugin/CharacterSilhouette/Private/CharacterSilhouette.usf", "StrokeVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FCharacterStrokePS, "/Plugin/CharacterSilhouette/Private/CharacterSilhouette.usf", "StrokePS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FCharacterCompositePS, "/Plugin/CharacterSilhouette/Private/CharacterSilhouette.usf", "CompositePS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FCharacterLineMaskPS, "/Plugin/CharacterSilhouette/Private/CharacterSilhouette.usf", "LineMaskPS", SF_Pixel);

void AddCharacterSilhouettePass(FRDGBuilder& GraphBuilder, const FSceneView& View,
	FScreenPassTexture Coverage, FScreenPassRenderTarget Output, const FCharacterSilhouetteSettings& Settings,
	const FCharacterSilhouetteOcclusion* Occlusion, const FCharacterSilhouetteFeatures* Features, FScreenPassRenderTarget LineMask)
{
	check(IsInRenderingThread());
	if (!Coverage.IsValid() || !Output.IsValid() || View.GetFeatureLevel() < ERHIFeatureLevel::SM5)
		return;

	const FIntPoint CoverageSize = Coverage.ViewRect.Size();
	const FIntPoint OutputSize = Output.ViewRect.Size();
	if (CoverageSize.X <= 0 || CoverageSize.Y <= 0 || OutputSize.X <= 0 || OutputSize.Y <= 0 || Settings.WidthPixels <= 0.f)
		return;

	RDG_EVENT_SCOPE(GraphBuilder, "CharacterSilhouette");
	const FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(View.GetFeatureLevel());
	int32 Samples = Settings.SamplesPerAxis >= 4 ? 4 : Settings.SamplesPerAxis >= 2 ? 2 : 1;
	while (Samples > 1 && FMath::Max(OutputSize.X, OutputSize.Y) * Samples > GMaxTextureDimensions)
		Samples /= 2;

	const FIntPoint StrokeExtent(OutputSize.X * Samples, OutputSize.Y * Samples);
	const FVector2f StrokeSize(StrokeExtent.X, StrokeExtent.Y);
	const uint64 WorstCaseSegments = uint64(CoverageSize.X + 1) * uint64(CoverageSize.Y + 1) * 2;
	const uint32 Capacity = uint32(FMath::Min(WorstCaseSegments, uint64(FMath::Clamp(Settings.MaxSegments, 1u, 4194304u))));
	FRDGBufferRef Segments = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(FVector4f), Capacity),
		TEXT("CharacterSilhouette.Segments"));
	FRDGBufferRef Count = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateBufferDesc(sizeof(uint32), 1), TEXT("CharacterSilhouette.Count"));
	FRDGBufferUAVRef CountUAV = GraphBuilder.CreateUAV(Count, PF_R32_UINT);
	AddClearUAVPass(GraphBuilder, CountUAV, 0u);

	if (Features && Features->Normals.IsValid() && Features->Identity.IsValid() && Features->Depth.IsValid())
	{
		auto* Parameters = GraphBuilder.AllocParameters<FCharacterFeaturesCS::FParameters>();
		Parameters->SurfaceTexture = Features->Normals.Texture;
		Parameters->IdentityTexture = Features->Identity.Texture;
		Parameters->SurfaceDepth = Features->Depth.Texture;
		Parameters->SurfacePositions = Features->Positions.IsValid() ? Features->Positions.Texture : Features->Normals.Texture;
		Parameters->HasSurfacePositions = Features->Positions.IsValid();
		Parameters->SurfaceSize = Features->Normals.ViewRect.Size();
		Parameters->StrokeSize = StrokeSize;
		Parameters->ClipToTranslatedWorld = Features->ClipToTranslatedWorld;
		Parameters->DepthThreshold = FMath::Max(Features->DepthThreshold, 0.01f);
		Parameters->CreaseCosine = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(Features->CreaseAngle, 1.f, 179.f)));
		Parameters->EdgeFlags = uint32(Features->bCharacterOverlaps) | uint32(Features->bSelfDepth) * 2 | uint32(Features->bNormalCreases) * 4;
		Parameters->SegmentCapacity = Capacity;
		Parameters->SegmentOutput = GraphBuilder.CreateUAV(Segments);
		Parameters->SegmentCountOutput = CountUAV;
		TShaderMapRef<FCharacterFeaturesCS> Shader(ShaderMap);
		FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("ExtractCharacterFeatures"), Shader, Parameters,
			FComputeShaderUtils::GetGroupCount(Parameters->SurfaceSize, FIntPoint(8, 8)));
	}
	else
	{
		FCharacterContourCS::FParameters* Contour = GraphBuilder.AllocParameters<FCharacterContourCS::FParameters>();
		Contour->CoverageTexture = Coverage.Texture;
		Contour->CoverageMin = Coverage.ViewRect.Min;
		Contour->CoverageSize = CoverageSize;
		Contour->StrokeSize = StrokeSize;
		Contour->SegmentCapacity = Capacity;
		Contour->SegmentOutput = GraphBuilder.CreateUAV(Segments);
		Contour->SegmentCountOutput = CountUAV;
		TShaderMapRef<FCharacterContourCS> ContourShader(ShaderMap);
		FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("ExtractContour"), ContourShader, Contour,
			FComputeShaderUtils::GetGroupCount(CoverageSize + FIntPoint(1, 1), FIntPoint(8, 8)));
	}

	FRDGBufferRef Arguments = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateIndirectDesc<FRHIDrawIndirectParameters>(),
		TEXT("CharacterSilhouette.Arguments"));
	FCharacterArgumentsCS::FParameters* ArgumentParameters = GraphBuilder.AllocParameters<FCharacterArgumentsCS::FParameters>();
	ArgumentParameters->SegmentCapacity = Capacity;
	ArgumentParameters->SegmentCount = GraphBuilder.CreateSRV(Count, PF_R32_UINT);
	ArgumentParameters->DrawArgumentsOutput = GraphBuilder.CreateUAV(Arguments, PF_R32_UINT);
	TShaderMapRef<FCharacterArgumentsCS> ArgumentShader(ShaderMap);
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("BuildStrokeArguments"), ArgumentShader, ArgumentParameters, FIntVector(1, 1, 1));

	FRDGTextureRef Stroke = GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(StrokeExtent, PF_R16F, FClearValueBinding::Black,
		TexCreate_RenderTargetable | TexCreate_ShaderResource), TEXT("CharacterSilhouette.Stroke"));
	FCharacterStrokeParameters* Raster = GraphBuilder.AllocParameters<FCharacterStrokeParameters>();
	Raster->ContourSegments = GraphBuilder.CreateSRV(Segments);
	Raster->StrokeSize = StrokeSize;
	const FIntPoint ReferenceSize = Settings.ReferenceViewSize.X > 0 && Settings.ReferenceViewSize.Y > 0
		? Settings.ReferenceViewSize : OutputSize;
	Raster->StrokeToReferenceScale = FVector2f(float(ReferenceSize.X) / StrokeExtent.X, float(ReferenceSize.Y) / StrokeExtent.Y);
	Raster->StrokeFilterWidth = FMath::Max(Raster->StrokeToReferenceScale.X, Raster->StrokeToReferenceScale.Y);
	Raster->StrokeRadius = FMath::Clamp(Settings.WidthPixels, 0.25f, 64.f);
	Raster->CoverageTexture = Coverage.Texture;
	Raster->CoverageMin = Coverage.ViewRect.Min;
	Raster->CoverageSize = CoverageSize;
	const bool bHasOcclusion = Occlusion && Occlusion->CharacterDepth.IsValid() && Occlusion->SceneDepth.IsValid();
	const FScreenPassTexture CharacterDepth = bHasOcclusion ? Occlusion->CharacterDepth : Coverage;
	const FScreenPassTexture VisibleDepth = bHasOcclusion ? Occlusion->SceneDepth : Coverage;
	Raster->CharacterDepth = CharacterDepth.Texture;
	Raster->VisibleDepth = VisibleDepth.Texture;
	Raster->CharacterDepthMin = CharacterDepth.ViewRect.Min;
	Raster->CharacterDepthSize = CharacterDepth.ViewRect.Size();
	Raster->VisibleDepthMin = VisibleDepth.ViewRect.Min;
	Raster->VisibleDepthSize = VisibleDepth.ViewRect.Size();
	Raster->VisibleOnly = bHasOcclusion && !Features;
	Raster->DepthTolerance = bHasOcclusion ? FMath::Max(Occlusion->DeviceDepthTolerance, 0.f) : 0.f;
	Raster->DrawArguments = Arguments;
	Raster->RenderTargets[0] = FRenderTargetBinding(Stroke, ERenderTargetLoadAction::EClear);
	TShaderMapRef<FCharacterStrokeVS> VertexShader(ShaderMap);
	TShaderMapRef<FCharacterStrokePS> PixelShader(ShaderMap);
	GraphBuilder.AddPass(RDG_EVENT_NAME("RasterizeContourStroke"), Raster, ERDGPassFlags::Raster,
		[Raster, VertexShader, PixelShader, StrokeExtent](FRHICommandList& RHICmdList)
		{
			RHICmdList.SetViewport(0, 0, 0.f, StrokeExtent.X, StrokeExtent.Y, 1.f);
			FGraphicsPipelineStateInitializer Pipeline;
			RHICmdList.ApplyCachedRenderTargets(Pipeline);
			Pipeline.BoundShaderState.VertexDeclarationRHI = GEmptyVertexDeclaration.VertexDeclarationRHI;
			Pipeline.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			Pipeline.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			Pipeline.PrimitiveType = PT_TriangleList;
			Pipeline.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
			Pipeline.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			// Union capsule coverage. Shared endpoints never accumulate opacity or create dark joins.
			Pipeline.BlendState = TStaticBlendState<CW_RED, BO_Max, BF_One, BF_One>::GetRHI();
			SetGraphicsPipelineState(RHICmdList, Pipeline, 0);
			SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), *Raster);
			SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *Raster);
			RHICmdList.DrawPrimitiveIndirect(Raster->DrawArguments->GetIndirectRHICallBuffer(), 0);
		});

	FCharacterCompositePS::FParameters* Composite = GraphBuilder.AllocParameters<FCharacterCompositePS::FParameters>();
	Composite->StrokeTexture = Stroke;
	Composite->CoverageTexture = Coverage.Texture;
	Composite->CoverageSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp>::GetRHI();
	Composite->CoverageTextureSize = FVector2f(Coverage.Texture->Desc.Extent.X, Coverage.Texture->Desc.Extent.Y);
	Composite->CoverageMin = Coverage.ViewRect.Min;
	Composite->CoverageSize = CoverageSize;
	Composite->OutputMin = Output.ViewRect.Min;
	Composite->OutputSize = OutputSize;
	Composite->SamplesPerAxis = Samples;
	Composite->StrokeColor = FVector4f(Settings.Color.R, Settings.Color.G, Settings.Color.B, Settings.Color.A);
	Composite->DebugMode = FMath::Clamp(Settings.DebugMode, 0, 2);
	Composite->InteriorOnly = Features != nullptr;
	Composite->CharacterDepth = CharacterDepth.Texture;
	Composite->VisibleDepth = VisibleDepth.Texture;
	Composite->CharacterDepthMin = CharacterDepth.ViewRect.Min;
	Composite->CharacterDepthSize = CharacterDepth.ViewRect.Size();
	Composite->VisibleDepthMin = VisibleDepth.ViewRect.Min;
	Composite->VisibleDepthSize = VisibleDepth.ViewRect.Size();
	Composite->VisibleOnly = bHasOcclusion && Features;
	Composite->DepthTolerance = bHasOcclusion ? FMath::Max(Occlusion->DeviceDepthTolerance, 0.f) : 0.f;
	Composite->RenderTargets[0] = FRenderTargetBinding(Output.Texture, ERenderTargetLoadAction::ELoad);
	TShaderMapRef<FCharacterCompositePS> CompositeShader(ShaderMap);
	FPixelShaderUtils::AddFullscreenPass(GraphBuilder, ShaderMap, RDG_EVENT_NAME("CompositeSilhouette"), CompositeShader,
		Composite, Output.ViewRect, TStaticBlendState<CW_RGB, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha>::GetRHI());
	if (LineMask.IsValid() && Settings.DebugMode == 0)
	{
		check(LineMask.ViewRect == Output.ViewRect);
		auto* MaskParameters = GraphBuilder.AllocParameters<FCharacterLineMaskPS::FParameters>();
		*MaskParameters = *Composite;
		MaskParameters->RenderTargets[0] = FRenderTargetBinding(LineMask.Texture, ERenderTargetLoadAction::ELoad);
		TShaderMapRef<FCharacterLineMaskPS> MaskShader(ShaderMap);
		// Match successive opaque-color composites, including antialiased intersections between line families.
		FPixelShaderUtils::AddFullscreenPass(GraphBuilder, ShaderMap, RDG_EVENT_NAME("CharacterSilhouette.LineMask"), MaskShader,
			MaskParameters, LineMask.ViewRect,
			TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI());
	}
}

