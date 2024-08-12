
#include "SFDEditorViewport.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "FrameData.h"
#include "Engine/TextureCube.h"
#include "AnimPreviewInstance.h"
#include "OriginOfStormsMaster/OOSPawn.h"
#include "FDAnimInstance.h"
#include "Editor/UnrealEd/Public/ComponentAssetBroker.h"

#define LOCTEXT_NAMESPACE "SFDEditorViewport"

SFDEditorViewport::SFDEditorViewport()
{
	PreviewScene = MakeShareable(new FFDPreviewScene());
}

void SFDEditorViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{

	if (EditorViewportClient.IsValid())
	{
		UWorld* World = EditorViewportClient->GetWorld();
		if (World)
		{
			World->Tick(ELevelTick::LEVELTICK_ViewportsOnly, InDeltaTime);
		}

		if (bIsPlaying)
		{
			TimeElapsed += InDeltaTime;
			uint32 FramesElapsed = FMath::FloorToInt(TimeElapsed / (1.f / 60));

			if (CurrentFrame != FramesElapsed)
			{
				FrameDataAsset->StepForward(bIsLooping);
				GoToFrame(FrameDataAsset->GetCurrentFrame());

				CurrentFrame = FramesElapsed;
			}
		}
		
	}
}

void SFDEditorViewport::Construct(const FArguments& InArgs)
{
	FrameDataEditorPtr = InArgs._FrameDataEditor;
	FrameDataAsset = InArgs._FrameDataObject;
	SetPreviewAnimation(InArgs._PreviewAnimation);

	if (PreviewScene.IsValid())
	{
		PostProcess = NewObject<UPostProcessComponent>();
		if (PostProcess)
		{
			FSoftObjectPath Path("Material'/Game/UI/FrameDataEditor/Materials/M_Test_TrainingGrid.M_Test_TrainingGrid'");
			UMaterialInterface* TrainingGridMaterial = Cast<UMaterialInterface>(Path.TryLoad());
			if (TrainingGridMaterial)
			{
				PostProcess->Settings.AddBlendable(TScriptInterface<IBlendableInterface>(TrainingGridMaterial), 1.f);
			}
			Path = FSoftObjectPath("MaterialInstanceConstant'/Game/PostProcess/MI_PP_Outlines.MI_PP_Outlines'");
			UMaterialInterface* OutlinesMaterial = Cast<UMaterialInterface>(Path.TryLoad());
			if (OutlinesMaterial)
			{
				PostProcess->Settings.AddBlendable(TScriptInterface<IBlendableInterface>(OutlinesMaterial), 1.f);
			}

			PostProcess->bUnbound = true;

			PreviewScene->AddComponent(PostProcess, FTransform());
		}

		FSoftObjectPath Path("TextureCube'/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap'");
		UTextureCube* SkyLightCubemap = Cast<UTextureCube>(Path.TryLoad());
		if (SkyLightCubemap)
		{
			PreviewScene->SetSkyCubemap(SkyLightCubemap);
			PreviewScene->UpdateCaptureContents();
			PreviewScene->SetSkyBrightness(1.f);
		}

		PreviewScene->SetLightBrightness(1.75f);
	}

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedRef<FEditorViewportClient> SFDEditorViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FEditorViewportClient(nullptr, PreviewScene.Get(), SharedThis(this)));

	EditorViewportClient->bSetListenerPosition = false;

	EditorViewportClient->SetRealtime(true);
	EditorViewportClient->SetShowGrid();

	FVector X = FVector::ForwardVector;
	FVector Zoom = -X * 400.f;
	FVector Height = FVector(0.f,0.f, 75.f);
	EditorViewportClient->SetCameraSetup(FVector::ZeroVector, FRotator::ZeroRotator, FVector::ZeroVector, FVector::ZeroVector , Zoom + Height, X.Rotation());
	EditorViewportClient->SetViewLocationForOrbiting(Height, 400.f);

	EditorViewportClient->ViewFOV = 30.f;

	return EditorViewportClient.ToSharedRef();
}

void SFDEditorViewport::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(FrameDataAsset);
	Collector.AddReferencedObject(PreviewMeshAsset);
	Collector.AddReferencedObject(PostProcess);
	Collector.AddReferencedObject(AnimInstance);
	Collector.AddReferencedObject(PreviewPawn);
}

FString SFDEditorViewport::GetReferencerName() const
{
	return "FDEditorViewport";
}

void SFDEditorViewport::SetPreviewAnimation(UAnimationAsset* InAnimation)
{
	if (!PreviewScene.IsValid()) return;

	UWorld* World = PreviewScene->GetWorld();
	if (!World) return;

	if (PreviewPawn)
	{
		PreviewPawn->Destroy();
		PreviewPawn = nullptr;
	}
	
	// Get relevant assets and rebuild the scene.
	USkeletalMesh* Mesh = nullptr;		

	if (InAnimation)
	{
		/*USkeleton* Skeleton = InAnimation->GetSkeleton();
		Mesh = InAnimation->GetPreviewMesh();
		if (Skeleton)
		{
			if (Mesh == nullptr)
			{
				Mesh = Skeleton->GetPreviewMesh();
			}
			if (Mesh == nullptr)
			{
				Mesh = Skeleton->FindCompatibleMesh();
			}

			if (Mesh)
			{
				FVector MeshExtent = Mesh->GetImportedBounds().BoxExtent;
				float HalfHeight = MeshExtent.Z / 2.f;
				float Radius = FMath::Max(MeshExtent.X, MeshExtent.Y);

				PreviewPawn = World->SpawnActor<AOOSPawn>(FVector(0.f,0.f,HalfHeight), FRotator::ZeroRotator);
				PreviewPawn->Capsule->SetCapsuleHalfHeight(HalfHeight);
				PreviewPawn->Capsule->SetVisibility(true); // REMOVE LATER

				PreviewPawn->SkeletalMesh->SetRelativeLocation(FVector(0.f, 0.f, -HalfHeight));
				PreviewPawn->SkeletalMesh->SetSkeletalMesh(Mesh);

				for (int i = 0; i < Skeleton->PreviewAttachedAssetContainer.Num(); ++i)
				{
					TSubclassOf<UActorComponent> ComponentClass = FComponentAssetBrokerage::GetPrimaryComponentForAsset(Skeleton->PreviewAttachedAssetContainer[i].GetAttachedObject()->GetClass());
					if (*ComponentClass && ComponentClass->IsChildOf(USceneComponent::StaticClass()))
					{
						USceneComponent* SceneComponent = NewObject<USceneComponent>(PreviewPawn->SkeletalMesh, ComponentClass);
						if (SceneComponent)
						{
							FComponentAssetBrokerage::AssignAssetToComponent(SceneComponent, Skeleton->PreviewAttachedAssetContainer[i].GetAttachedObject());

							// Attach component to the preview component
							SceneComponent->SetupAttachment(PreviewPawn->SkeletalMesh, Skeleton->PreviewAttachedAssetContainer[i].AttachedTo);
							SceneComponent->RegisterComponent();
						}
					}
				}

				PreviewPawn->SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationCustomMode);
				AnimInstance = NewObject<UFDAnimInstance>(PreviewPawn->SkeletalMesh);
				PreviewPawn->SkeletalMesh->AnimScriptInstance = AnimInstance;
				PreviewPawn->SkeletalMesh->InitializeAnimScriptInstance();
				
				if (!AnimInstance) return;
				AnimInstance->SetAnimationAsset(InAnimation);
				AnimInstance->SetPlaying(false);
				bIsLooping = true;

				PreviewMeshAsset = Mesh;
			}
		}*/
	}	
}

void SFDEditorViewport::TogglePlay()
{
	bIsPlaying = !bIsPlaying;
}

void SFDEditorViewport::Stop()
{
	bIsPlaying = false;
}

void SFDEditorViewport::GoToFrame(uint32 Frame)
{
	float Length = FrameDataAsset->GetAnimationLength();
	uint32 NumFrames = FrameDataAsset->FrameData.Num() - 1;

	AnimInstance->SetPosition((Length / NumFrames) * Frame, false);
}

void SFDEditorViewport::GoToFrame(float InPosition)
{
	float Length = FrameDataAsset->GetAnimationLength();
	uint32 Frame = uint8((InPosition / Length) * (FrameDataAsset->FrameData.Num() - 1));
	GoToFrame(Frame);
}

bool SFDEditorViewport::IsPlaying() const
{
	return bIsPlaying;
}

#undef LOCTEXT_NAMESPACE