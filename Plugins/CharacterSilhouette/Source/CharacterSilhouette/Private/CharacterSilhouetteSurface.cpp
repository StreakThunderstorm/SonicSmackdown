#include "CharacterSilhouetteSurface.h"
#include "MeshMaterialShader.h"
#include "MeshPassProcessor.inl"
#include "PrimitiveSceneProxy.h"
#include "ScenePrivate.h"
#include "SceneRendering.h"
#include "Materials/Material.h"
#include "MaterialShared.h"
#include "Interfaces/IPluginManager.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "UObject/UObjectIterator.h"

static TMap<FPrimitiveComponentId, FName> CharacterOwners;

void UpdateCharacterSilhouetteOwners(FSceneInterface* Scene)
{
	TMap<FPrimitiveComponentId, FName> Owners;
	for (TObjectIterator<UPrimitiveComponent> Iterator; Iterator; ++Iterator)
	{
		UPrimitiveComponent* Component = *Iterator;
		if (!Component->IsRegistered() || !Component->bRenderCustomDepth || !Component->GetWorld() || Component->GetWorld()->Scene != Scene)
			continue;
		AActor* Owner = Component->GetOwner();
		if (!Owner)
			continue;
		AActor* Root = Owner;
		// Attached armor and pawn-owned weapon actors share their character's overlap identity.
		for (int32 Step = 0; Root && !Root->IsA<APawn>() && Step < 32; ++Step)
			Root = Root->GetAttachParentActor() ? Root->GetAttachParentActor() : Root->GetOwner();
		if (!Root || !Root->IsA<APawn>())
			Root = Owner;
		Owners.Add(Component->GetPrimitiveSceneId(), Root->GetFName());
	}
	ENQUEUE_RENDER_COMMAND(UpdateSilhouetteOwners)(
		[Owners = MoveTemp(Owners)](FRHICommandListImmediate& RHICmdList) mutable
	{
		CharacterOwners = MoveTemp(Owners);
	});
}

class FCharacterSurfaceVS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FCharacterSurfaceVS, MeshMaterial);

public:
	FCharacterSurfaceVS() {}
	FCharacterSurfaceVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FMeshMaterialShader(Initializer) {}
	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5)
			&& !IsTranslucentBlendMode(Parameters.MaterialParameters) && !Parameters.VertexFactoryType->SupportsNaniteRendering();
	}
	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& Environment)
	{
		FMeshMaterialShader::ModifyCompilationEnvironment(Parameters, Environment);
		Environment.SetDefine(TEXT("SCENE_TEXTURES_DISABLED"), 1);
		const auto ShadingPlugin = IPluginManager::Get().FindPlugin(TEXT("PrExtendedShadingPro"));
		const auto LayerPlugin = IPluginManager::Get().FindPlugin(TEXT("PawnRenderLayers"));
		Environment.SetDefine(TEXT("SILHOUETTE_PR_DEPTH_BIAS"), ShadingPlugin && ShadingPlugin->IsEnabled());
		Environment.SetDefine(TEXT("SILHOUETTE_PAWN_LAYERS"), LayerPlugin && LayerPlugin->IsEnabled());
	}
};

class FCharacterSurfaceElementData : public FMeshMaterialShaderElementData
{
public:
	uint32 Identity = 0;
};

class FCharacterSurfacePS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FCharacterSurfacePS, MeshMaterial);

public:
	FCharacterSurfacePS() {}
	FCharacterSurfacePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FMeshMaterialShader(Initializer)
	{
		CharacterIdentity.Bind(Initializer.ParameterMap, TEXT("CharacterIdentity"));
	}
	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return FCharacterSurfaceVS::ShouldCompilePermutation(Parameters);
	}
	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& Environment)
	{
		FCharacterSurfaceVS::ModifyCompilationEnvironment(Parameters, Environment);
	}
	void GetShaderBindings(const FScene* Scene, ERHIFeatureLevel::Type FeatureLevel, const FPrimitiveSceneProxy* Proxy,
		const FMaterialRenderProxy& MaterialProxy, const FMaterial& Material, const FCharacterSurfaceElementData& ElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, Proxy, MaterialProxy, Material, ElementData, ShaderBindings);
		ShaderBindings.Add(CharacterIdentity, ElementData.Identity);
	}

private:
	LAYOUT_FIELD(FShaderParameter, CharacterIdentity);
};

IMPLEMENT_MATERIAL_SHADER_TYPE(, FCharacterSurfaceVS, TEXT("/Plugin/CharacterSilhouette/Private/CharacterSurface.usf"), TEXT("SurfaceVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FCharacterSurfacePS, TEXT("/Plugin/CharacterSilhouette/Private/CharacterSurface.usf"), TEXT("SurfacePS"), SF_Pixel);

class FCharacterSurfaceProcessor : public FMeshPassProcessor
{
public:
	FCharacterSurfaceProcessor(const FViewInfo& View, const FMeshPassProcessorRenderState& State, FMeshPassDrawListContext* Context)
		: FMeshPassProcessor(TEXT("CharacterSilhouetteSurface"), View.Family->Scene->GetRenderScene(), View.GetFeatureLevel(), &View, Context),
		RenderState(State) {}
	virtual void AddMeshBatch(const FMeshBatch& Mesh, uint64 Elements, const FPrimitiveSceneProxy* Proxy, int32 StaticMeshId = -1) override
	{
		const FMaterialRenderProxy* MaterialProxy = Mesh.MaterialRenderProxy;
		while (MaterialProxy)
		{
			const FMaterial* Material = MaterialProxy->GetMaterialNoFallback(FeatureLevel);
			if (Material && !IsTranslucentBlendMode(Material->GetBlendMode()))
			{
				FMaterialShaderTypes Types;
				Types.AddShaderType<FCharacterSurfaceVS>();
				Types.AddShaderType<FCharacterSurfacePS>();
				FMaterialShaders Shaders;
				if (Material->TryGetShaders(Types, Mesh.VertexFactory->GetType(), Shaders))
				{
					TMeshProcessorShaders<FCharacterSurfaceVS, FCharacterSurfacePS> PassShaders;
					Shaders.TryGetVertexShader(PassShaders.VertexShader);
					Shaders.TryGetPixelShader(PassShaders.PixelShader);
					FCharacterSurfaceElementData Data;
					Data.InitializeMeshMaterialData(ViewIfDynamicMeshCommand, Proxy, Mesh, StaticMeshId, false);
					Data.Identity = CharacterIdentity;
					const FMeshDrawingPolicyOverrideSettings Overrides = ComputeMeshOverrideSettings(Mesh);
					BuildMeshDrawCommands(Mesh, Elements, Proxy, *MaterialProxy, *Material, RenderState, PassShaders,
						ComputeMeshFillMode(*Material, Overrides), ComputeMeshCullMode(*Material, Overrides),
						CalculateMeshStaticSortKey(PassShaders.VertexShader, PassShaders.PixelShader), EMeshPassFeatures::Default, Data);
					return;
				}
			}
			MaterialProxy = MaterialProxy->GetFallback(FeatureLevel);
		}
	}
	void SetCharacterIdentity(uint32 Identity) { CharacterIdentity = Identity; }

private:
	FMeshPassProcessorRenderState RenderState;
	uint32 CharacterIdentity = 0;
};

class FCharacterSurfaceMaskCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCharacterSurfaceMaskCS);
	SHADER_USE_PARAMETER_STRUCT(FCharacterSurfaceMaskCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SurfaceTexture)
		SHADER_PARAMETER(FIntPoint, SurfaceSize)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, SurfaceMask)
	END_SHADER_PARAMETER_STRUCT()
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};

IMPLEMENT_GLOBAL_SHADER(FCharacterSurfaceMaskCS, "/Plugin/CharacterSilhouette/Private/CharacterFeatures.usf", "SurfaceMaskCS", SF_Compute);

BEGIN_SHADER_PARAMETER_STRUCT(FCharacterSurfacePassParameters, )
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
	SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneUniformParameters, Scene)
	SHADER_PARAMETER_STRUCT_INCLUDE(FInstanceCullingDrawParams, InstanceCullingDrawParams)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

FCharacterSilhouetteSurface AddCharacterSilhouetteSurfacePass(FRDGBuilder& GraphBuilder, const FViewInfo& View,
	int32 StencilMin, int32 StencilMax, int32 ResolutionScale)
{
	FIntPoint Size = View.UnscaledViewRect.Size() * FMath::Clamp(ResolutionScale, 1, 4);
	if (Size.X <= 0 || Size.Y <= 0 || !View.CachedViewUniformShaderParameters)
		return {};
	while (FMath::Max(Size.X, Size.Y) > GMaxTextureDimensions || int64(Size.X) * Size.Y > 67108864)
		Size /= 2;
	FCharacterSilhouetteSurface Surface;
	Surface.Normals = FScreenPassTexture(GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(Size, PF_FloatRGBA,
		FClearValueBinding::Transparent, TexCreate_RenderTargetable | TexCreate_ShaderResource), TEXT("CharacterSilhouette.SurfaceNormals")));
	Surface.Identity = FScreenPassTexture(GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(Size, PF_R32_UINT,
		FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource), TEXT("CharacterSilhouette.SurfaceIdentity")));
	Surface.Depth = FScreenPassTexture(GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(Size, PF_DepthStencil,
		FClearValueBinding::DepthFar, TexCreate_DepthStencilTargetable | TexCreate_ShaderResource), TEXT("CharacterSilhouette.SurfaceDepth")));
	Surface.Positions = FScreenPassTexture(GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(Size, PF_A32B32G32R32F,
		FClearValueBinding::Transparent, TexCreate_RenderTargetable | TexCreate_ShaderResource), TEXT("CharacterSilhouette.SurfacePositions")));
	FViewUniformShaderParameters Uniforms = *View.CachedViewUniformShaderParameters;
	Uniforms.ViewRectMin = FVector4f(0, 0, 0, 0);
	Uniforms.ViewSizeAndInvSize = FVector4f(Size.X, Size.Y, 1.f / Size.X, 1.f / Size.Y);
	Uniforms.BufferSizeAndInvSize = Uniforms.ViewSizeAndInvSize;
	Uniforms.ScreenPositionScaleBias = FVector4f(0.5f, -0.5f, 0.5f, 0.5f);
	const auto ViewBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(Uniforms, UniformBuffer_SingleFrame);
	FMeshPassProcessorRenderState State;
	State.SetBlendState(TStaticBlendState<>::GetRHI());
	State.SetDepthStencilState(TStaticDepthStencilState<true, CF_DepthNearOrEqual>::GetRHI());
	State.SetDepthStencilAccess(FExclusiveDepthStencil::DepthWrite_StencilNop);
	FCharacterSurfacePassParameters* Parameters = GraphBuilder.AllocParameters<FCharacterSurfacePassParameters>();
	Parameters->View = ViewBuffer;
	Parameters->Scene = GetSceneUniformBufferRef(GraphBuilder, View);
	Parameters->InstanceCullingDrawParams.InstanceCulling = FInstanceCullingContext::CreateDummyInstanceCullingUniformBuffer(GraphBuilder);
	Parameters->RenderTargets[0] = FRenderTargetBinding(Surface.Normals.Texture, ERenderTargetLoadAction::EClear);
	Parameters->RenderTargets[1] = FRenderTargetBinding(Surface.Identity.Texture, ERenderTargetLoadAction::EClear);
	Parameters->RenderTargets[2] = FRenderTargetBinding(Surface.Positions.Texture, ERenderTargetLoadAction::EClear);
	Parameters->RenderTargets.DepthStencil = FDepthStencilBinding(Surface.Depth.Texture, ERenderTargetLoadAction::EClear,
		ERenderTargetLoadAction::ENoAction, FExclusiveDepthStencil::DepthWrite_StencilNop);
	GraphBuilder.AddPass(RDG_EVENT_NAME("CharacterSilhouette.HighResolutionGeometry"), Parameters, ERDGPassFlags::Raster,
		[&View, Size, State, StencilMin, StencilMax, Owners = CharacterOwners](FRHICommandList& RHICmdList)
		{
			RHICmdList.SetViewport(0, 0, 0.f, Size.X, Size.Y, 1.f);
			DrawDynamicMeshPass(View, RHICmdList, [&](FDynamicPassMeshDrawListContext* Context)
			{
				FCharacterSurfaceProcessor Processor(View, State, Context);
				TMap<FName, uint32> Identities;
				auto Select = [&](const FPrimitiveSceneProxy* Proxy)
				{
					if (!Proxy || !Proxy->IsShown(&View) || !Proxy->ShouldRenderInMainPass() || !Proxy->ShouldRenderCustomDepth()
						|| Proxy->GetCustomDepthStencilValue() < StencilMin
						|| Proxy->GetCustomDepthStencilValue() > StencilMax)
						return false;
					const FName* CharacterOwner = Owners.Find(Proxy->GetPrimitiveComponentId());
					const FName Owner = CharacterOwner ? *CharacterOwner : Proxy->GetOwnerName();
					uint32& Identity = Identities.FindOrAdd(Owner);
					if (!Identity)
						Identity = Identities.Num();
					Processor.SetCharacterIdentity(Identity);
					return true;
				};
				for (const FMeshBatchAndRelevance& Element : View.DynamicMeshElements)
					if (Select(Element.PrimitiveSceneProxy) && Element.Mesh->bUseForMaterial)
						Processor.AddMeshBatch(*Element.Mesh, ~0ull, Element.PrimitiveSceneProxy);
				const FScene* Scene = View.Family->Scene->GetRenderScene();
				for (int32 Index = 0; Index < Scene->Primitives.Num(); ++Index)
					if (View.PrimitiveVisibilityMap[Index] && Select(Scene->Primitives[Index]->Proxy))
						for (const FStaticMeshBatch& Mesh : Scene->Primitives[Index]->StaticMeshes)
							if (View.StaticMeshVisibilityMap[Mesh.Id] && Mesh.bUseForMaterial)
								Processor.AddMeshBatch(Mesh, ~0ull, Scene->Primitives[Index]->Proxy, Mesh.Id);
				}, true);
		});
	Surface.Coverage = FScreenPassTexture(GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(Size, PF_R16F,
		FClearValueBinding::None, TexCreate_UAV | TexCreate_ShaderResource), TEXT("CharacterSilhouette.HighResolutionCoverage")));
	auto* MaskParameters = GraphBuilder.AllocParameters<FCharacterSurfaceMaskCS::FParameters>();
	MaskParameters->SurfaceTexture = Surface.Normals.Texture;
	MaskParameters->SurfaceSize = Size;
	MaskParameters->SurfaceMask = GraphBuilder.CreateUAV(Surface.Coverage.Texture);
	TShaderMapRef<FCharacterSurfaceMaskCS> MaskShader(GetGlobalShaderMap(View.GetFeatureLevel()));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("CharacterSilhouette.SurfaceCoverage"), MaskShader, MaskParameters,
		FComputeShaderUtils::GetGroupCount(Size, FIntPoint(8, 8)));
	return Surface;
}
