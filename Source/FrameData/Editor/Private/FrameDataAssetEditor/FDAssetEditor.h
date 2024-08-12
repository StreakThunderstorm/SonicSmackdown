#pragma once

#include "CoreMinimal.h"
#include "FrameData.h"
#include "IFrameDataEditor.h"
#include "Editor/EditorWidgets/Public/ITransportControl.h"
#include "Editor/UnrealEd/Public/Toolkits/AssetEditorToolkit.h"
#include "Misc/NotifyHook.h"

class FGGAssetEditorToolbar;

class FFDAssetEditor : public IFrameDataEditor, public FAssetEditorToolkit, public FGCObject, public FNotifyHook
{
public:
	FFDAssetEditor();
	virtual ~FFDAssetEditor();

	void InitFrameDataAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UFrameData* Asset);

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	virtual void SaveAsset_Execute() override;
	// End of FAssetEditorToolkit

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~ End FGCObject Interface


private:
	void RefreshAnimAsset(UAnimationAsset* InAnimation);

	UFrameData* EditingFrameData;
	TObjectPtr<UAnimationAsset> PreviewAnimationAsset;

	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Properties(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_SocketManager(const FSpawnTabArgs& Args);

	/** Preview Viewport widget */
	TSharedPtr<class SFDEditorViewport> Viewport;
	/** Property View */
	TSharedPtr<class IDetailsView> DetailsView;
	/** Triggers panel */
	TSharedPtr<class SFDTriggerPanel> TriggersPanel;
	/** Timeline */
	TSharedPtr<class SFDScrubPanel> ScrubControl;

	FReply StepForward();
	FReply StepBack();
	FReply TogglePlay();
	void TogglePlay_KeyboardCommand() { TogglePlay(); };
	void StepForward_KeyboardCommand() { StepForward(); };
	void StepBack_KeyboardCommand() { StepBack(); };
	FReply GoToStart();
	FReply GoToEnd();
	EPlaybackMode::Type GetPlaybackMode();
	uint32 GetNumFrames() const;
	float GetAnimationLength() const;
	float GetPlaybackPosition() const;
	uint32 GetCurrentFrame() const;
	void SetPlaybackPosition(float InPosition);
	void OnClickSlider();
	void OnReleaseSlider(float InPosition);
	bool bIsDragging = false;
};


