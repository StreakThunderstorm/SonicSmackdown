#include "CharacterSilhouetteRenderer.h"
#include "CharacterSilhouetteShaders.h"
#include "CharacterSilhouetteProjectSettings.h"
#include "CharacterSilhouetteSurface.h"
#include "HAL/IConsoleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ScopeExit.h"
#include "Modules/ModuleManager.h"
#include "SceneRendering.h"
#include "SceneViewExtension.h"
#include "RendererInterface.h"
#include "EngineModule.h"
#include "ShaderCore.h"
#include "SystemTextures.h"

static TAutoConsoleVariable<int32> CVarSilhouetteEnabled(TEXT("r.CharacterSilhouette.Enable"), 0,
	TEXT("Enable the opt-in custom-stencil silhouette adapter. 0: off, 1: on."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarSilhouetteStencil(TEXT("r.CharacterSilhouette.Stencil"), 1,
	TEXT("Character custom-stencil value (1-255). Mesh selection remains controlled by the project."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> CVarSilhouetteWidth(TEXT("r.CharacterSilhouette.Width"), 3.f,
	TEXT("Outward stroke width at ReferenceHeight, or output pixels if ReferenceHeight is zero."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> CVarSilhouetteReferenceHeight(TEXT("r.CharacterSilhouette.ReferenceHeight"), 1080.f,
	TEXT("Reference output height for visual weight. 0 uses a fixed output-pixel width."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarSilhouetteSamples(TEXT("r.CharacterSilhouette.Samples"), 2,
	TEXT("Stroke samples per axis: 1, 2, or 4. Does not supersample the source geometry."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> CVarSilhouetteRed(TEXT("r.CharacterSilhouette.ColorR"), 0.f, TEXT("Stroke red."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> CVarSilhouetteGreen(TEXT("r.CharacterSilhouette.ColorG"), 0.f, TEXT("Stroke green."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> CVarSilhouetteBlue(TEXT("r.CharacterSilhouette.ColorB"), 0.f, TEXT("Stroke blue."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarSilhouetteVisibleOnly(TEXT("r.CharacterSilhouette.VisibleOnly"), 1,
	TEXT("1 depth-tests stroke pixels against the scene; 0 draws through foreground geometry. Neither changes the contour mask."),
	ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> CVarSilhouetteDepthTolerance(TEXT("r.CharacterSilhouette.DepthTolerance"), 0.000001f,
	TEXT("Reversed-device-depth tolerance for the optional visible-only mask adapter."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarSilhouetteDebug(TEXT("r.CharacterSilhouette.Debug"), 0,
	TEXT("0: exterior stroke, 1: white source mask, 2: stroke on both sides of the contour."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarSilhouetteGeometry(TEXT("r.CharacterSilhouette.Geometry"), 1,
	TEXT("Use independent high-resolution character geometry instead of the scene stencil mask."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarSilhouetteGeometryScale(TEXT("r.CharacterSilhouette.GeometryScale"), 2,
	TEXT("Character geometry resolution relative to final output: 1-4."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarSilhouetteStencilEnd(TEXT("r.CharacterSilhouette.StencilEnd"), 0,
	TEXT("Inclusive upper selected stencil value. 0 selects only Stencil."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarSilhouetteOverlaps(TEXT("r.CharacterSilhouette.Overlaps"), 1,
	TEXT("Draw visible boundaries between character actors in the geometry pass."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarSilhouetteSelfDepth(TEXT("r.CharacterSilhouette.SelfDepth"), 1,
	TEXT("Draw self-occlusion depth discontinuities in the geometry pass."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarSilhouetteInnerEdges(TEXT("r.CharacterSilhouette.InnerEdges"), 1,
	TEXT("Draw geometric normal creases in the geometry pass."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> CVarSilhouetteInnerWidth(TEXT("r.CharacterSilhouette.InnerWidth"), 1.25f,
	TEXT("Full self-depth line thickness at ReferenceHeight."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> CVarSilhouetteInnerEdgeWidth(TEXT("r.CharacterSilhouette.InnerEdgeWidth"), 1.25f,
	TEXT("Full inner edge line thickness at ReferenceHeight."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> CVarSilhouetteSelfDepthThreshold(TEXT("r.CharacterSilhouette.SelfDepthThreshold"), 2.f,
	TEXT("Self-depth separation threshold in world units (normally cm)."), ECVF_RenderThreadSafe);
static TAutoConsoleVariable<float> CVarSilhouetteInnerEdgeAngle(TEXT("r.CharacterSilhouette.InnerEdgeAngle"), 45.f,
	TEXT("Geometric normal crease angle in degrees."), ECVF_RenderThreadSafe);

static const FName SilhouetteDepthOrderTag(TEXT("CharacterSilhouette"));
static bool bRequestedEarlyCustomDepth = false;

static void PublishEmptyCharacterLineMask(FRDGBuilder& GraphBuilder, const FViewInfo& View)
{
	const FName TextureName(TEXT("CharacterSilhouetteLines"));
	const FSceneTextures& Textures = View.GetSceneTextures();
	if (Textures.UserSceneTextures.Contains(TextureName))
		return;
	FTransientUserSceneTexture Texture = {};
	Texture.Texture = GSystemTextures.GetBlackDummy(GraphBuilder);
	Texture.ResolutionDivisor = Textures.Config.Extent;
	for (const FSceneView* FamilyView : View.Family->Views)
		Texture.ResolutionDivisor = Texture.ResolutionDivisor.ComponentMax(FamilyView->UnconstrainedViewRect.Max);
	Texture.ResolutionDivisor = Texture.ResolutionDivisor.ComponentMax(FIntPoint(1, 1));
	Texture.bUsed = true;
	Textures.UserSceneTextures.Add(TextureName, { Texture });
}

static void UpdateSilhouetteRenderPrerequisites()
{
	const bool bEnabled = CVarSilhouetteEnabled.GetValueOnGameThread() != 0;
	if (bEnabled == bRequestedEarlyCustomDepth)
		return;
	if (IConsoleVariable* Order = IConsoleManager::Get().FindConsoleVariable(TEXT("r.CustomDepth.Order")))
	{
		// Tagged overrides restore the previous value when outlines are disabled, without writing project config.
		if (bEnabled)
			Order->Set(0, ECVF_SetByCode, SilhouetteDepthOrderTag);
		else
			Order->Unset(ECVF_SetByCode, SilhouetteDepthOrderTag);
		bRequestedEarlyCustomDepth = bEnabled;
	}
}

class FCharacterSilhouetteViewExtension : public FSceneViewExtensionBase
{
public:
	FCharacterSilhouetteViewExtension(const FAutoRegister& AutoRegister) : FSceneViewExtensionBase(AutoRegister) {}
	virtual void SetupViewFamily(FSceneViewFamily& ViewFamily) override
	{
		UpdateSilhouetteRenderPrerequisites();
		if (CVarSilhouetteGeometry.GetValueOnGameThread())
			UpdateCharacterSilhouetteOwners(ViewFamily.Scene);
	}
	virtual void SetupView(FSceneViewFamily& ViewFamily, FSceneView& View) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& ViewFamily) override {}
	void RenderPostOpaque(FPostOpaqueRenderParameters& Parameters)
	{
		if (!Parameters.View || !Parameters.GraphBuilder)
			return;
		const FViewInfo& View = *Parameters.View;
		FRDGBuilder& GraphBuilder = *Parameters.GraphBuilder;
		const auto SceneTextures = Parameters.SceneTexturesUniformParams;
		if (View.GetFeatureLevel() < ERHIFeatureLevel::SM5 || !View.bIsViewInfo)
			return;
		ON_SCOPE_EXIT
		{
			// Optional consumers must always see a valid texture, without triggering UE's missing-input debug overlay.
			PublishEmptyCharacterLineMask(GraphBuilder, View);
		};
		if (!CVarSilhouetteEnabled.GetValueOnRenderThread())
			return;
		// Draw after opaque-scene fog and clouds, while translucent Niagara still follows this stage.
		if (!IsForwardShadingEnabled(View.GetShaderPlatform()))
		{
			if (!bReportedUnsupportedRenderer)
				UE_LOG(LogTemp, Warning, TEXT("CharacterSilhouette's pre-translucency stage requires desktop forward shading."));
			bReportedUnsupportedRenderer = true;
			return;
		}
		if (!SceneTextures || !Parameters.ColorTexture)
			return;

		const FSceneTextureUniformParameters& Textures = *SceneTextures->GetContents();
		const bool bGeometry = CVarSilhouetteGeometry.GetValueOnRenderThread() != 0;
		if (!bGeometry && (!Textures.CustomStencilTexture || !Textures.CustomDepthTexture || !Textures.CustomDepthTexture->HasBeenProduced()))
		{
			if (!bReportedMissingMask)
				UE_LOG(LogTemp, Warning,
					TEXT("CharacterSilhouette needs custom depth before the base pass. Check r.CustomDepth.Order and mesh stencil selection."));
			bReportedMissingMask = true;
			return;
		}

		const FIntRect SourceRect = static_cast<const FViewInfo&>(View).ViewRect;
		const FIntPoint Size = SourceRect.Size();
		if (Size.X <= 0 || Size.Y <= 0)
			return;

		FCharacterSilhouetteSurface Surface;
		FScreenPassTexture Coverage;
		if (bGeometry)
		{
			const int32 FirstStencil = FMath::Clamp(CVarSilhouetteStencil.GetValueOnRenderThread(), 1, 255);
			const int32 LastStencil = FMath::Clamp(CVarSilhouetteStencilEnd.GetValueOnRenderThread(), FirstStencil, 255);
			Surface = AddCharacterSilhouetteSurfacePass(GraphBuilder, static_cast<const FViewInfo&>(View), FirstStencil, LastStencil,
				CVarSilhouetteGeometryScale.GetValueOnRenderThread());
			Coverage = Surface.Coverage;
		}
		else
		{
			FRDGTextureRef Mask = GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(Size, PF_R16F, FClearValueBinding::None,
				TexCreate_UAV | TexCreate_ShaderResource), TEXT("CharacterSilhouette.StencilMask"));
			FCharacterMaskCS::FParameters* MaskParameters = GraphBuilder.AllocParameters<FCharacterMaskCS::FParameters>();
			MaskParameters->StencilTexture = Textures.CustomStencilTexture;
			MaskParameters->SourceMin = SourceRect.Min;
			MaskParameters->MaskSize = Size;
			MaskParameters->StencilValue = FMath::Clamp(CVarSilhouetteStencil.GetValueOnRenderThread(), 1, 255);
			MaskParameters->MaskOutput = GraphBuilder.CreateUAV(Mask);
			TShaderMapRef<FCharacterMaskCS> Shader(GetGlobalShaderMap(View.GetFeatureLevel()));
			FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("CharacterSilhouette.ExtractStencilMask"), Shader, MaskParameters,
				FComputeShaderUtils::GetGroupCount(Size, FIntPoint(8, 8)));
			Coverage = FScreenPassTexture(Mask);
		}
		if (!Coverage.IsValid())
			return;
		FCharacterSilhouetteOcclusion Occlusion;
		Occlusion.CharacterDepth = bGeometry ? Surface.Depth : FScreenPassTexture(Textures.CustomDepthTexture, SourceRect);
		Occlusion.SceneDepth = FScreenPassTexture(Textures.SceneDepthTexture, SourceRect);
		Occlusion.DeviceDepthTolerance = FMath::Max(CVarSilhouetteDepthTolerance.GetValueOnRenderThread(), 0.f);

		FCharacterSilhouetteSettings Settings;
		Settings.ReferenceViewSize = View.UnscaledViewRect.Size();
		const float ReferenceHeight = CVarSilhouetteReferenceHeight.GetValueOnRenderThread();
		Settings.WidthPixels = CVarSilhouetteWidth.GetValueOnRenderThread();
		if (ReferenceHeight > 0.f)
			Settings.WidthPixels *= Settings.ReferenceViewSize.Y / ReferenceHeight;
		Settings.SamplesPerAxis = CVarSilhouetteSamples.GetValueOnRenderThread();
		Settings.Color = FLinearColor(CVarSilhouetteRed.GetValueOnRenderThread(), CVarSilhouetteGreen.GetValueOnRenderThread(),
			CVarSilhouetteBlue.GetValueOnRenderThread(), 1.f);
		// Scene color is HDR and pre-exposed here; colored lines must use the same exposure convention.
		const float PreExposure = static_cast<const FViewInfo&>(View).PreExposure;
		Settings.Color.R *= PreExposure;
		Settings.Color.G *= PreExposure;
		Settings.Color.B *= PreExposure;
		Settings.DebugMode = CVarSilhouetteDebug.GetValueOnRenderThread();
		const auto Output = FScreenPassRenderTarget(Parameters.ColorTexture, SourceRect, ERenderTargetLoadAction::ELoad);
		const auto* Visibility = CVarSilhouetteVisibleOnly.GetValueOnRenderThread() != 0 ? &Occlusion : nullptr;
		const auto LineMask = FScreenPassRenderTarget(GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(
			Parameters.ColorTexture->Desc.Extent, PF_FloatRGBA, FClearValueBinding::Transparent, TexCreate_RenderTargetable | TexCreate_ShaderResource),
			TEXT("CharacterSilhouette.VisibleLines")), SourceRect, ERenderTargetLoadAction::ELoad);
		AddClearRenderTargetPass(GraphBuilder, LineMask.Texture, FLinearColor::Transparent);
		auto PublishLineMask = [&]()
		{
			// Native user scene textures belong to this render graph and view family, including independent scene captures.
			// Publish only coverage; no scene color is captured or restored and unrelated post-process materials are untouched.
			const FSceneTextures& TexturesForFamily = View.GetSceneTextures();
			const FName TextureName(TEXT("CharacterSilhouetteLines"));
			TArray<FTransientUserSceneTexture>& Published = TexturesForFamily.UserSceneTextures.FindOrAdd(TextureName);
			if (Published.IsEmpty())
			{
				FIntPoint Extent = TexturesForFamily.Config.Extent;
				for (const FSceneView* FamilyView : View.Family->Views)
					Extent = Extent.ComponentMax(FamilyView->UnconstrainedViewRect.Max);
				FTransientUserSceneTexture Texture = {};
				Texture.Texture = GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(Extent, PF_FloatRGBA, FClearValueBinding::Transparent,
					TexCreate_RenderTargetable | TexCreate_ShaderResource), TEXT("UST.CharacterSilhouetteLines"));
				Texture.ResolutionDivisor = FIntPoint(1, 1);
				// Fog is optional; an unconsumed mask is intentional and must not trigger UE's unused-output debug overlay.
				Texture.bUsed = true;
				Published.Add(Texture);
				AddClearRenderTargetPass(GraphBuilder, Texture.Texture, FLinearColor::Transparent);
			}
			const int32 ViewIndex = View.Family->Views.IndexOfByKey(static_cast<const FSceneView*>(&View));
			if (ViewIndex >= 0 && ViewIndex < 32)
				Published[0].ViewMask |= 1u << ViewIndex;
			const FIntRect MaskRect = GetDownscaledViewRect(View.UnconstrainedViewRect, SourceRect.Max, FIntPoint(1, 1));
			AddDrawTexturePass(GraphBuilder, View, FScreenPassTexture(LineMask.Texture, SourceRect),
				FScreenPassRenderTarget(Published[0].Texture, MaskRect, ERenderTargetLoadAction::ELoad));
		};
		AddCharacterSilhouettePass(GraphBuilder, View, Coverage, Output, Settings, Visibility, nullptr, LineMask);
		if (!bGeometry || Settings.DebugMode != 0)
		{
			PublishLineMask();
			return;
		}
		FCharacterSilhouetteFeatures Features;
		Features.Normals = Surface.Normals;
		Features.Identity = Surface.Identity;
		Features.Depth = Surface.Depth;
		Features.Positions = Surface.Positions;
		Features.ClipToTranslatedWorld = FMatrix44f(View.ViewMatrices.GetInvTranslatedViewProjectionMatrix());
		if (CVarSilhouetteOverlaps.GetValueOnRenderThread())
		{
			Features.bCharacterOverlaps = true;
			FCharacterSilhouetteSettings OverlapSettings = Settings;
			OverlapSettings.WidthPixels *= 0.5f;
			AddCharacterSilhouettePass(GraphBuilder, View, Coverage, Output, OverlapSettings, Visibility, &Features, LineMask);
		}
		Features.bCharacterOverlaps = false;
		Features.DepthThreshold = CVarSilhouetteSelfDepthThreshold.GetValueOnRenderThread();
		Features.CreaseAngle = CVarSilhouetteInnerEdgeAngle.GetValueOnRenderThread();
		auto DrawInternalLines = [&](float Thickness)
		{
			Settings.WidthPixels = Thickness * 0.5f;
			if (ReferenceHeight > 0.f)
				Settings.WidthPixels *= Settings.ReferenceViewSize.Y / ReferenceHeight;
			AddCharacterSilhouettePass(GraphBuilder, View, Coverage, Output, Settings, Visibility, &Features, LineMask);
		};
		if (CVarSilhouetteSelfDepth.GetValueOnRenderThread())
		{
			Features.bSelfDepth = true;
			DrawInternalLines(CVarSilhouetteInnerWidth.GetValueOnRenderThread());
			Features.bSelfDepth = false;
		}
		if (CVarSilhouetteInnerEdges.GetValueOnRenderThread())
		{
			Features.bNormalCreases = true;
			DrawInternalLines(CVarSilhouetteInnerEdgeWidth.GetValueOnRenderThread());
		}
		PublishLineMask();
	}

protected:
	virtual bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override
	{
		return CVarSilhouetteEnabled.GetValueOnAnyThread() != 0;
	}

private:
	bool bReportedUnsupportedRenderer = false;
	bool bReportedMissingMask = false;
};

class FCharacterSilhouetteModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		const FString ShaderDirectory = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("CharacterSilhouette"))->GetBaseDir(), TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/CharacterSilhouette"), ShaderDirectory);
		PostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddRaw(this, &FCharacterSilhouetteModule::CreateViewExtension);
		ConsoleSinkHandle = IConsoleManager::Get().RegisterConsoleVariableSink_Handle(
			FConsoleCommandDelegate::CreateStatic(&UpdateSilhouetteRenderPrerequisites));
	}

	virtual void ShutdownModule() override
	{
		if (PostOpaqueHandle.IsValid())
			GetRendererModule().RemovePostOpaqueRenderDelegate(PostOpaqueHandle);
		FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
		IConsoleManager::Get().UnregisterConsoleVariableSink_Handle(ConsoleSinkHandle);
		if (bRequestedEarlyCustomDepth)
		{
			if (IConsoleVariable* Order = IConsoleManager::Get().FindConsoleVariable(TEXT("r.CustomDepth.Order")))
				Order->Unset(ECVF_SetByCode, SilhouetteDepthOrderTag);
			bRequestedEarlyCustomDepth = false;
		}
		ViewExtension.Reset();
	}

private:
	void CreateViewExtension()
	{
		GetDefault<UCharacterSilhouetteProjectSettings>()->ApplySettings();
		UpdateSilhouetteRenderPrerequisites();
		ViewExtension = FSceneViewExtensions::NewExtension<FCharacterSilhouetteViewExtension>();
		PostOpaqueHandle = GetRendererModule().RegisterPostOpaqueRenderDelegate(
			FPostOpaqueRenderDelegate::CreateSP(ViewExtension.ToSharedRef(), &FCharacterSilhouetteViewExtension::RenderPostOpaque));
	}

	FDelegateHandle PostEngineInitHandle;
	FDelegateHandle PostOpaqueHandle;
	FConsoleVariableSinkHandle ConsoleSinkHandle;
	TSharedPtr<FCharacterSilhouetteViewExtension, ESPMode::ThreadSafe> ViewExtension;
};

IMPLEMENT_MODULE(FCharacterSilhouetteModule, CharacterSilhouette)
