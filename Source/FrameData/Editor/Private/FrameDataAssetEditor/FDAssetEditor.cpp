#include "FDAssetEditor.h"
#include "SDockTab.h"
#include "../FrameDataEditorPCH.h"
#include "AssetToolsModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "GenericCommands.h"
#include "FDEditorCommands.h"
#include "GraphEditorActions.h"
#include "IDetailsView.h"
#include "IStructureDetailsView.h"
#include "PropertyEditorModule.h"
#include "Editor/UnrealEd/Public/Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "SFDScrubPanel.h"
#include "EdGraphUtilities.h"
#include "SFDEditorViewport.h"
#include "SFDTriggerPanel.h"
#include "SEditorViewport.h"
#include "Runtime/Engine/Classes/Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"

#define LOCTEXT_NAMESPACE "FDAssetEditor"

const FName FrameDataEditorAppName = FName(TEXT("FrameDataEditorApp"));

struct FFDAssetEditorTabs
{
	// Tab identifiers
	static const FName FrameDataPropertyID;
	static const FName ViewportID;
	static const FName FrameDataEditorSettingsID;
};

//////////////////////////////////////////////////////////////////////////

const FName FFDAssetEditorTabs::FrameDataPropertyID(TEXT("FrameDataProperty"));
const FName FFDAssetEditorTabs::ViewportID(TEXT("Viewport"));
const FName FFDAssetEditorTabs::FrameDataEditorSettingsID(TEXT("FrameDataEditorSettings"));

//////////////////////////////////////////////////////////////////////////

FFDAssetEditor::FFDAssetEditor()
{
	
}

FFDAssetEditor::~FFDAssetEditor()
{
	
}

void FFDAssetEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_StaticMeshEditor", "Static Mesh Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FFDAssetEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FFDAssetEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "FrameDataEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FFDAssetEditorTabs::FrameDataPropertyID, FOnSpawnTab::CreateSP(this, &FFDAssetEditor::SpawnTab_Properties))
		.SetDisplayName(LOCTEXT("PropertiesTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "FrameDataEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FFDAssetEditorTabs::FrameDataEditorSettingsID, FOnSpawnTab::CreateSP(this, &FFDAssetEditor::SpawnTab_SocketManager))
		.SetDisplayName(LOCTEXT("SocketManagerTab", "Socket Manager"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "FrameDataEditor.Tabs.SocketManager"));
}


void FFDAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FFDAssetEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FFDAssetEditorTabs::FrameDataPropertyID);
	InTabManager->UnregisterTabSpawner(FFDAssetEditorTabs::FrameDataEditorSettingsID);
}

void FFDAssetEditor::InitFrameDataAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UFrameData* Asset)
{
	EditingFrameData = Asset;
	if (EditingFrameData)
	{
		PreviewAnimationAsset = EditingFrameData->Animation;
		EditingFrameData->RefreshFrameData();
	}

	FGenericCommands::Register();
	FGraphEditorCommands::Register();
	FFDEditorCommands::Register();

	// Bind keyboard commands.
	TSharedPtr<FUICommandList> ToolkitCommands = GetToolkitCommands();
	ToolkitCommands->MapAction(FFDEditorCommands::Get().TogglePlay,
		FExecuteAction::CreateSP(this, &FFDAssetEditor::TogglePlay_KeyboardCommand));
	ToolkitCommands->MapAction(FFDEditorCommands::Get().StepForward,
		FExecuteAction::CreateSP(this, &FFDAssetEditor::StepForward_KeyboardCommand), EUIActionRepeatMode::RepeatEnabled);
	ToolkitCommands->MapAction(FFDEditorCommands::Get().StepBack,
		FExecuteAction::CreateSP(this, &FFDAssetEditor::StepBack_KeyboardCommand), EUIActionRepeatMode::RepeatEnabled);

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	// Tab widgets
	Viewport = SNew(SFDEditorViewport)
		.FrameDataEditor(SharedThis(this))
		.FrameDataObject(EditingFrameData)
		.PreviewAnimation(PreviewAnimationAsset);

	TriggersPanel = SNew(SFDTriggerPanel)
		.FrameDataAsset(EditingFrameData)
		.Frame(this, &FFDAssetEditor::GetCurrentFrame);

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	DetailsView = PropertyModule.CreateDetailView(Args);
	DetailsView->SetObject(EditingFrameData);
	DetailsView->OnFinishedChangingProperties().AddSP(this, &FFDAssetEditor::OnFinishedChangingProperties);
	
	// Layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_FrameDataEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)->SetHideTabWell(true)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.8f)
					->AddTab(FFDAssetEditorTabs::ViewportID, ETabState::OpenedTab)->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.2f)
						->AddTab(FFDAssetEditorTabs::FrameDataPropertyID, ETabState::OpenedTab)->SetHideTabWell(true)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.8f)
						->AddTab(FFDAssetEditorTabs::FrameDataEditorSettingsID, ETabState::OpenedTab)
					)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, FrameDataEditorAppName, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, EditingFrameData, false);

}

TSharedRef<SDockTab> FFDAssetEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FFDAssetEditorTabs::ViewportID);

	ScrubControl = SNew(SFDScrubPanel)
		.FrameDataAsset(EditingFrameData)
		.IsEnabled(true)
		.Value(this, &FFDAssetEditor::GetPlaybackPosition)
		.NumOfKeys(this, &FFDAssetEditor::GetNumFrames)
		.SequenceLength(this, &FFDAssetEditor::GetAnimationLength)
		.ViewInputMin(0.f)
		.ViewInputMax(this, &FFDAssetEditor::GetAnimationLength)
		.DisplayDrag(true)
		.bAllowZoom(true)
		.IsRealtimeStreamingMode(false)
		.OnClickedForwardPlay(this, &FFDAssetEditor::TogglePlay)
		.OnClickedForwardStep(this, &FFDAssetEditor::StepForward)
		.OnClickedBackwardStep(this, &FFDAssetEditor::StepBack)
		.OnClickedForwardEnd(this, &FFDAssetEditor::GoToEnd)
		.OnClickedBackwardEnd(this, &FFDAssetEditor::GoToStart)
		.OnGetPlaybackMode(this, &FFDAssetEditor::GetPlaybackMode)
		.OnValueChanged(this, &FFDAssetEditor::SetPlaybackPosition)
		.OnEndSliderMovement(this, &FFDAssetEditor::OnReleaseSlider)
		.OnBeginSliderMovement(this, &FFDAssetEditor::OnClickSlider);


	//		.OnClickedBackwardPlay(this, &FFlipbookEditor::OnClick_Backward)
	//		.OnClickedToggleLoop(this, &FFlipbookEditor::OnClick_ToggleLoop)
	//		.OnGetLooping(this, &FFlipbookEditor::IsLooping)
	//		.ViewInputMin(this, &FFlipbookEditor::GetViewRangeMin)
	//		.ViewInputMax(this, &FFlipbookEditor::GetViewRangeMax)
	//		.OnSetInputViewRange(this, &FFlipbookEditor::SetViewRange)

	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		.Label(LOCTEXT("FrameDataViewport_TabTitle", "Viewport"))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				Viewport.ToSharedRef()
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Bottom)
			[
				ScrubControl.ToSharedRef()
			]
		];

	Viewport->ParentTab = SpawnedTab;

	return SpawnedTab;
}

TSharedRef<SDockTab> FFDAssetEditor::SpawnTab_Properties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FFDAssetEditorTabs::FrameDataPropertyID);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("FrameDataEditor.Tabs.Properties"))
		.Label(LOCTEXT("FrameDataProperties_TabTitle", "Details"))
		[
			DetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FFDAssetEditor::SpawnTab_SocketManager(const FSpawnTabArgs& InArgs)
{
	check(InArgs.GetTabId() == FFDAssetEditorTabs::FrameDataEditorSettingsID);	

	return SNew(SDockTab)
		.Label(LOCTEXT("FrameDataSettings_TabTitle", "Details"))
		[
			TriggersPanel.ToSharedRef()
		];
}

FReply FFDAssetEditor::StepForward()
{
	if (!Viewport.IsValid() || !EditingFrameData) return FReply::Handled();

	Viewport->Stop();
	EditingFrameData->StepForward(false);
	Viewport->GoToFrame(EditingFrameData->GetCurrentFrame());
	TriggersPanel->Update();

	return FReply::Handled();
}

FReply FFDAssetEditor::StepBack()
{
	if (!Viewport.IsValid() || !EditingFrameData) return FReply::Handled();

	Viewport->Stop();
	EditingFrameData->StepBack(false);
	Viewport->GoToFrame(EditingFrameData->GetCurrentFrame());
	TriggersPanel->Update();

	return FReply::Handled();
}

FReply FFDAssetEditor::TogglePlay()
{
	if (!Viewport.IsValid()) return FReply::Handled();

	Viewport->TogglePlay();
	if (!Viewport->IsPlaying())
		TriggersPanel->Update();

	return FReply::Handled();
}

FReply FFDAssetEditor::GoToStart()
{
	Viewport->Stop();
	EditingFrameData->GoToStart();
	Viewport->GoToFrame(EditingFrameData->GetCurrentFrame());
	TriggersPanel->Update();

	return FReply::Handled();
}

FReply FFDAssetEditor::GoToEnd()
{
	Viewport->Stop();
	EditingFrameData->GoToEnd();
	Viewport->GoToFrame(EditingFrameData->GetCurrentFrame());
	TriggersPanel->Update();

	return FReply::Handled();
}

EPlaybackMode::Type FFDAssetEditor::GetPlaybackMode()
{
	return Viewport->IsPlaying() ? EPlaybackMode::Type::PlayingForward : EPlaybackMode::Type::Stopped;
}

uint32 FFDAssetEditor::GetNumFrames() const
{
	return EditingFrameData->FrameData.Num();
}

float FFDAssetEditor::GetAnimationLength() const
{
	return EditingFrameData->GetAnimationLength();
}

float FFDAssetEditor::GetPlaybackPosition() const
{
	return EditingFrameData->GetCurrentFrame_Pos();
}

uint32 FFDAssetEditor::GetCurrentFrame() const
{
	if (!EditingFrameData) return 0;

	return EditingFrameData->GetCurrentFrame();
}

void FFDAssetEditor::SetPlaybackPosition(float InPosition)
{
	// Clamp the gathered timeline position so we can't drag beyond timeline widget.
	// Also apply an offset of half the duration of a frame forwards so clicking instead of grabbing is nicer.
	float PosInRange = FMath::Clamp(InPosition, 0.f, EditingFrameData->GetAnimationLength());
	PosInRange += EditingFrameData->GetHalfFrameLength(); 

	EditingFrameData->GoToFrame(PosInRange);
	Viewport->GoToFrame(PosInRange);
	if (!bIsDragging)
		TriggersPanel->Update();
}

void FFDAssetEditor::OnClickSlider()
{
	Viewport->Stop();
	bIsDragging = true;
}

void FFDAssetEditor::OnReleaseSlider(float InPosition)
{
	bIsDragging = false;
	TriggersPanel->Update();
}

void FFDAssetEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!EditingFrameData) return;

	const FName ChangedProperty = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (ChangedProperty == GET_MEMBER_NAME_CHECKED(UFrameData, Animation))
	{
		RefreshAnimAsset(EditingFrameData->Animation);
		EditingFrameData->RefreshFrameData();
	}


}

void FFDAssetEditor::RefreshAnimAsset(UAnimationAsset* InAnimation)
{
	if (Viewport.IsValid()) Viewport->SetPreviewAnimation(InAnimation);
}

void FFDAssetEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PreviewAnimationAsset);
}


FName FFDAssetEditor::GetToolkitFName() const
{
	return FName("FFrameDataEditor");
}

FText FFDAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("FrameDataEditorAppLabel", "Frame Data Editor");
}

FText FFDAssetEditor::GetToolkitName() const
{
	const bool bDirtyState = EditingFrameData->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("FrameDataName"), FText::FromString(EditingFrameData->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("FrameDataEditorToolkitName", "{FrameDataName}{DirtyState}"), Args);
}

FText FFDAssetEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(EditingFrameData);
}

FLinearColor FFDAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FFDAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("FrameDataEditor");
}

FString FFDAssetEditor::GetDocumentationLink() const
{
	return TEXT("");
}

void FFDAssetEditor::SaveAsset_Execute()
{
	FAssetEditorToolkit::SaveAsset_Execute();
}


#undef LOCTEXT_NAMESPACE

