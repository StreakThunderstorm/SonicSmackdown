// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSCapsuleComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "GameFramework/HUD.h"
#include "Engine.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include <VertexFactory.h>
#include "DynamicMeshBuilder.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UOOSCapsuleComponent::UOOSCapsuleComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DebugColor = FLinearColor(0, 1, 0, 0.5f);
}

void UOOSCapsuleComponent::PostLoad()
{
	Super::PostLoad();

}

void UOOSCapsuleComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	if(FaceMat->IsValidLowLevel()) OutMaterials.Add(FaceMat);
	if(WireMat->IsValidLowLevel()) OutMaterials.Add(WireMat);
}

FPrimitiveSceneProxy* UOOSCapsuleComponent::CreateSceneProxy()
{
	class FOOSCapsuleComponentSceneProxy : public FPrimitiveSceneProxy
	{
	public:
		FOOSCapsuleComponentSceneProxy(UOOSCapsuleComponent* Component)
			:FPrimitiveSceneProxy(Component),
			Material(Component->FaceMat),
			LineMaterial(Component->WireMat),
			Color(Component->DebugColor)
		{
			if (!Material || !Material->IsValidLowLevel()) return;

			FMaterialRelevance Result;
			Result |= Material->GetRelevance_Concurrent(GetScene().GetFeatureLevel());
			Result |= LineMaterial->GetRelevance_Concurrent(GetScene().GetFeatureLevel());

			MaterialRelevance = Result;

			IndexBuffer.Indices = Component->Indices;
			LineBuffer.Indices.Empty();

			// Copy the vertex buffer to a dynamic one to generate the staticmesh VB.
			TArray<FDynamicMeshVertex> Vertices;
			for (int i = 0; i < Component->Vertices.Num(); ++i)
			{
				FDynamicMeshVertex Vert = FDynamicMeshVertex(FVector3f(Component->Vertices[i]));
				Vert.TangentZ = Component->Normals[i];
				Vertices.Add(Vert);

				// Build the line buffer for the outline.
				if ((i > 1) && (i < (Component->Vertices.Num() - 1)))
				{
					LineBuffer.Indices.Add(i);
					LineBuffer.Indices.Add(i + 1);
				}
			}
			// Closing line.
			LineBuffer.Indices.Add(Component->Vertices.Num() - 1);
			LineBuffer.Indices.Add(2);

			VertexBuffers.InitFromDynamicVertex(&VertexFactory, Vertices);

			BeginInitResource(&VertexBuffers.PositionVertexBuffer);
			BeginInitResource(&VertexBuffers.StaticMeshVertexBuffer);
			BeginInitResource(&VertexBuffers.ColorVertexBuffer);
			BeginInitResource(&IndexBuffer);
			BeginInitResource(&LineBuffer);
			BeginInitResource(&VertexFactory);
		};

		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			if (!Material || !Material->IsValidLowLevel()) return;

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				// Construct mesh and add to the collector for rendering.
				FMeshBatch& Mesh = Collector.AllocateMesh();
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &IndexBuffer;
				Mesh.VertexFactory = &VertexFactory;
				Mesh.MaterialRenderProxy = Material->GetRenderProxy();

				bool bHasPrecomputedVolumetricLightmap_Mesh;
				FMatrix PreviousLocalToWorld_Mesh;
				int32 SingleCaptureIndex_Mesh;
				bool bOutputsVelocity_Mesh;
				GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap_Mesh, PreviousLocalToWorld_Mesh, SingleCaptureIndex_Mesh, bOutputsVelocity_Mesh);

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer_Mesh = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();

				DynamicPrimitiveUniformBuffer_Mesh.Set(GetLocalToWorld(), PreviousLocalToWorld_Mesh, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap_Mesh, bOutputsVelocity_Mesh);
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer_Mesh.UniformBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = true;
				Collector.AddMesh(ViewIndex, Mesh);

				// Construct outline and add to the collector for rendering
				// in order to display a PROPER wireframe on top of any geo
				// which Epic haven't been able to provide us with for 4+ years...
				/*FMeshBatch& Outline = Collector.AllocateMesh();
				FMeshBatchElement& LineBatchElement = Outline.Elements[0];
				LineBatchElement.IndexBuffer = &LineBuffer;
				Outline.VertexFactory = &VertexFactory;
				Outline.MaterialRenderProxy = LineMaterial->GetRenderProxy();				
				
				bool bHasPrecomputedVolumetricLightmap_Line;
				FMatrix PreviousLocalToWorld_Line;
				int32 SingleCaptureIndex_Line;
				GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap_Line, PreviousLocalToWorld_Line, SingleCaptureIndex_Line);

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer_Line = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				DynamicPrimitiveUniformBuffer_Line.Set(GetLocalToWorld(), PreviousLocalToWorld_Line, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap_Line, UseEditorDepthTest());
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer_Line.UniformBuffer;

				LineBatchElement.FirstIndex = 0;
				LineBatchElement.NumPrimitives = LineBuffer.Indices.Num() / 2;
				LineBatchElement.MinVertexIndex = 2;
				LineBatchElement.MaxVertexIndex = VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
				Outline.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Outline.Type = PT_LineList;
				Outline.DepthPriorityGroup = SDPG_World;
				Outline.bCanApplyViewModeOverrides = true;
				Collector.AddMesh(ViewIndex, Outline);*/
			}
		}

		// Draw a worldspace line that PROPERLY renders on top of any geo, since Epic haven't been able to do so for 4 years.
		// Start and End in local space
		void DrawLine(FVector Start, FVector End, FMeshElementCollector& Collector, int32 ViewIndex, FLinearColor InColor, float Thickness)
		{
			TArray<FDynamicMeshVertex> Vertices;
			FStaticMeshVertexBuffers LineVB;
			FDynamicMeshIndexBuffer32 LineIB;
			FLocalVertexFactory LineVF = FLocalVertexFactory(GetScene().GetFeatureLevel(), "FOOSCapsuleComponentSceneProxy_Outlines");
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const
		{
			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = IsShown(View);
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bDynamicRelevance = true;
			Result.bRenderInMainPass = ShouldRenderInMainPass();
			Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
			Result.bRenderCustomDepth = ShouldRenderCustomDepth();
			Result.bEditorNoDepthTestPrimitiveRelevance = true;
			MaterialRelevance.SetPrimitiveViewRelevance(Result);
			return Result;
		}

		virtual bool CanBeOccluded() const override
		{
			return !MaterialRelevance.bDisableDepthTest;
		}

		virtual uint32 GetMemoryFootprint(void) const override
		{
			return sizeof *this + GetAllocatedSize();
		}

		uint32 GetAllocatedSize(void) const
		{
			return FPrimitiveSceneProxy::GetAllocatedSize();
		}

		virtual ~FOOSCapsuleComponentSceneProxy()
		{
			VertexBuffers.PositionVertexBuffer.ReleaseResource();
			VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
			VertexBuffers.ColorVertexBuffer.ReleaseResource();
			IndexBuffer.ReleaseResource();
			LineBuffer.ReleaseResource();
			VertexFactory.ReleaseResource();
		}

	private:

		FStaticMeshVertexBuffers VertexBuffers;
		FDynamicMeshIndexBuffer32 IndexBuffer;
		FDynamicMeshIndexBuffer32 LineBuffer;
		FLocalVertexFactory VertexFactory = FLocalVertexFactory(GetScene().GetFeatureLevel(), "FOOSCapsuleComponentSceneProxy");
		UMaterialInterface* Material;
		UMaterialInterface* LineMaterial;
		FLinearColor Color;
		FMaterialRelevance MaterialRelevance;
	};

	CreateDebugMesh();
	return new FOOSCapsuleComponentSceneProxy(this);
}

void UOOSCapsuleComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport /*= ETeleportType::None*/)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);

	/*AActor* Owner = GetOwner();
	USceneComponent* Parent = GetAttachParent();
	if (Owner && Parent)
	{
		// Disable calling this event again.
		bWantsOnUpdateTransform = false;

		// Direction of the camera.
		FVector Y = Owner->GetActorRightVector();

		// Parent transform. Get parent's if not attached to any socket.
		FQuat ParentRot;
		FVector ParentLoc;
		FVector ParentScale;
		FName SocketName = GetAttachSocketName();
		if (Parent->DoesSocketExist(SocketName))
		{
			ParentRot = Parent->GetSocketQuaternion(SocketName);
			ParentLoc = Parent->GetSocketLocation(SocketName);
			ParentScale = Parent->GetSocketTransform(SocketName).GetScale3D();
		}
		else
		{
			ParentRot = Parent->GetComponentRotation().Quaternion();
			ParentLoc = Parent->GetComponentLocation();
			ParentScale = Parent->GetSocketTransform(SocketName).GetScale3D();
		}

		// Rotate towards camera, but try to follow limbs.
		FVector X = FVector::VectorPlaneProject(ParentRot.GetForwardVector(), Y);
		FVector Z = FVector::VectorPlaneProject(ParentRot.GetUpVector(), Y);
		FRotator NewRot = UKismetMathLibrary::MakeRotFromYZ(Y, Z);

		SetWorldRotation(NewRot);
		
		// Snap to Actor Owner's XZ plane.
		FVector ToOwnerXZPlane = (Owner->GetActorLocation() - ParentLoc).ProjectOnTo(Y);

		SetWorldLocation(ParentLoc + ToOwnerXZPlane);

		// Calculate Z scale depending on parent (socket) rotation. Calculate incidence of the capsule's radius on the scale
		// since we want the capsule to turn into a sphere when we fully shrink it vertically 
		float RadiusScale = CapsuleRadius / CapsuleHalfHeight;

		FVector Scale3D = ParentScale;
		Scale3D.Z = (Scale3D.Z - RadiusScale) * Z.Size();
		Scale3D.Z += RadiusScale;
		SetWorldScale3D(Scale3D);

		// Reenable calling this event.
		bWantsOnUpdateTransform = true;
	}*/
}

void UOOSCapsuleComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

// Called when the game starts
void UOOSCapsuleComponent::BeginPlay()
{
	Super::BeginPlay();
	

}

#if WITH_EDITOR
void UOOSCapsuleComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	MarkRenderStateDirty();

	Super::PostEditChangeProperty(PropertyChangedEvent);	
}
#endif	// WITH_EDITOR

void UOOSCapsuleComponent::CreateDebugMesh()
{
	Vertices.Empty();
	Indices.Empty();

	float HalfHeight_WithoutHemisphere = GetUnscaledCapsuleHalfHeight_WithoutHemisphere();
	float Radius = GetUnscaledCapsuleRadius();
	int SectorCount = 16;
	int StackCount = 17;

	// Vertex positions and normals.
	float X, Y, Z, XY;
	float NX, NY, NZ, LengthInv = 1.0f / Radius;

	float SectorStepAngle = 2 * PI / SectorCount;
	float StackStepAngle = PI / StackCount;
	float SectorAngle, StackAngle;

	// Write vertex buffer
	for (int i = 0; i <= StackCount; ++i)
	{
		bool bIsTopHemisphere = i <= (StackCount / 2);
		float HemisphereOffset = bIsTopHemisphere ? HalfHeight_WithoutHemisphere : -HalfHeight_WithoutHemisphere;
		StackAngle = PI / 2 - i * StackStepAngle;    
		XY = Radius * FMath::Cos(StackAngle);           
		Z = Radius * FMath::Sin(StackAngle);
		Z += HemisphereOffset;

		for (int j = 0; j <= SectorCount; ++j)
		{
			SectorAngle = j * SectorStepAngle;

			X = XY * FMath::Cos(SectorAngle);
			Y = XY * FMath::Sin(SectorAngle);
			Vertices.Add(FVector(X, Y, Z));

			NX = X * LengthInv;
			NY = Y * LengthInv;
			NZ = Z * LengthInv;
			Normals.Add(FVector(NX, NY, NZ));
		}
	}

	// Write index buffer
	int K1, K2;
	for (int i = 0; i < StackCount; ++i)
	{
		K1 = i * (SectorCount + 1);
		K2 = K1 + SectorCount + 1;

		for (int j = 0; j < SectorCount; ++j, ++K1, ++K2)
		{
			// Top and bottom stacks have only 1 triangle per sector.
			if (i != 0)
			{
				Indices.Add(K2);
				Indices.Add(K1);
				Indices.Add(K1 + 1);
			}

			if (i != (StackCount - 1))
			{
				Indices.Add(K2);
				Indices.Add(K1 + 1);
				Indices.Add(K2 + 1);
			}
		}
	}

	// Get mats and set params.
	FSoftObjectPath Path("Material'/Game/UI/Debug/M_SimpleTranslucent.M_SimpleTranslucent'");
	FaceMat = UMaterialInstanceDynamic::Create(Cast<UMaterialInterface>(Path.TryLoad()), this);
	WireMat = UMaterialInstanceDynamic::Create(Cast<UMaterialInterface>(Path.TryLoad()), this);

	UMaterialInstanceDynamic* Mesh = Cast<UMaterialInstanceDynamic>(FaceMat);
	UMaterialInstanceDynamic* Outline = Cast<UMaterialInstanceDynamic>(WireMat);
	if (Mesh)
	{
		Mesh->SetVectorParameterValue("Color", FVector(DebugColor.R, DebugColor.G, DebugColor.B));
		Mesh->SetScalarParameterValue("Opacity", 0.4f);
	}
	if (Outline)
	{
		Outline->SetVectorParameterValue("Color", FVector(DebugColor.R, DebugColor.G, DebugColor.B));
		Outline->SetScalarParameterValue("Opacity", 1.f);
	}
}


