#pragma once

#include "Editor/UnrealEd/Public/SEditorViewport.h"
#include "Editor/UnrealEd/Public/EditorModeManager.h"
#include "UObject/GCObject.h"
#include "FDPreviewScene.h"
#include "Animation/AnimationAsset.h"

class IFrameDataEditor;

class SFDEditorViewport : public SEditorViewport, public FGCObject
{
public:

	SLATE_BEGIN_ARGS(SFDEditorViewport) {}
		SLATE_ARGUMENT(TWeakPtr<IFrameDataEditor>, FrameDataEditor)
		SLATE_ARGUMENT(class UFrameData*, FrameDataObject)
		SLATE_ARGUMENT(UAnimationAsset*, PreviewAnimation)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	SFDEditorViewport();

	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

	// FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	// End of FGCObject interface

	/** The parent tab where this viewport resides */
	TWeakPtr<SDockTab> ParentTab;

	void SetPreviewAnimation(UAnimationAsset* InAnimation);

	void TogglePlay();
	void Stop();
	void GoToFrame(uint32 Frame);
	void GoToFrame(float InPosition); // Version that takes frames mapped from 0.0 to SequenceDuration.

	bool IsPlaying() const;

private:

	bool bIsPlaying = false;
	bool bIsLooping = false;

	TObjectPtr<class UDebugSkelMeshComponent> PreviewMesh;
	TObjectPtr<class USkeletalMesh> PreviewMeshAsset;
	TObjectPtr<class UPostProcessComponent> PostProcess;
	TObjectPtr<class UFDAnimInstance> AnimInstance;
	TObjectPtr<class AOOSPawn> PreviewPawn;

	uint32 CurrentFrame = 0;
	float TimeElapsed = 0;

	/** Pointer back to the FrameData editor tool that owns us */
	TWeakPtr<IFrameDataEditor> FrameDataEditorPtr;

	/** Frame Data asset */
	TObjectPtr<UFrameData> FrameDataAsset;

	/** Editor viewport client */
	TSharedPtr<class FEditorViewportClient> EditorViewportClient;

	/** The scene for this viewport. */
	TSharedPtr<FFDPreviewScene> PreviewScene;
};