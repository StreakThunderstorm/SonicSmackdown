#include "MNAssetEditor.h"
#include "../MoveNetworkEditorPCH.h"
#include "MNAssetEditorToolbar.h"
#include "MNAssetGraphSchema.h"
#include "MNEditorCommands.h"
#include "MNEdGraph.h"
#include "AssetToolsModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "Editor/UnrealEd/Public/Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EdGraphUtilities.h"
#include "EditorStyleSet.h"
#include "MNEdGraph.h"
#include "MNNode_EdNode.h"
#include "MNEdge_EdNode.h"
#include "../AutoLayout/MNTreeLayout.h"
#include "../AutoLayout/MNForceDirectedLayout.h"

#define LOCTEXT_NAMESPACE "AssetEditor_MoveNetwork"

const FName MoveNetworkEditorAppName = FName(TEXT("MoveNetworkEditorApp"));

struct FMNAssetEditorTabs
{
	// Tab identifiers
	static const FName MoveNetworkPropertyID;
	static const FName ViewportID;
	static const FName MoveNetworkEditorSettingsID;
};

//////////////////////////////////////////////////////////////////////////

const FName FMNAssetEditorTabs::MoveNetworkPropertyID(TEXT("MoveNetworkProperty"));
const FName FMNAssetEditorTabs::ViewportID(TEXT("Viewport"));
const FName FMNAssetEditorTabs::MoveNetworkEditorSettingsID(TEXT("MoveNetworkEditorSettings"));

//////////////////////////////////////////////////////////////////////////

FMNAssetEditor::FMNAssetEditor()
{
	EditingGraph = nullptr;

	MoveNetworkEditorSettings = NewObject<UMNEditorSettings>(UMNEditorSettings::StaticClass());

	OnPackageSavedDelegateHandle = UPackage::PackageSavedEvent.AddRaw(this, &FMNAssetEditor::OnPackageSaved);
}

FMNAssetEditor::~FMNAssetEditor()
{
	UPackage::PackageSavedEvent.Remove(OnPackageSavedDelegateHandle);
}

void FMNAssetEditor::InitMoveNetworkAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UMoveNetwork* Graph)
{
	EditingGraph = Graph;
	CreateEdGraph();

	FGenericCommands::Register();
	FGraphEditorCommands::Register();
	FMNEditorCommands::Register();

	if (!ToolbarBuilder.IsValid())
	{
		ToolbarBuilder = MakeShareable(new FMNAssetEditorToolbar(SharedThis(this)));
	}

	BindCommands();

	CreateInternalWidgets();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarBuilder->AddMoveNetworkToolbar(ToolbarExtender);

	// Layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_MoveNetworkEditor_Layout_v1")
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
					->SetSizeCoefficient(0.65f)
					->AddTab(FMNAssetEditorTabs::ViewportID, ETabState::OpenedTab)->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.7f)
						->AddTab(FMNAssetEditorTabs::MoveNetworkPropertyID, ETabState::OpenedTab)->SetHideTabWell(true)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.3f)
						->AddTab(FMNAssetEditorTabs::MoveNetworkEditorSettingsID, ETabState::OpenedTab)
					)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, MoveNetworkEditorAppName, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, EditingGraph, false);

	RegenerateMenusAndToolbars();
}

void FMNAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_MoveNetworkEditor", "Move Network Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FMNAssetEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FMNAssetEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FMNAssetEditorTabs::MoveNetworkPropertyID, FOnSpawnTab::CreateSP(this, &FMNAssetEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Property"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FMNAssetEditorTabs::MoveNetworkEditorSettingsID, FOnSpawnTab::CreateSP(this, &FMNAssetEditor::SpawnTab_EditorSettings))
		.SetDisplayName(LOCTEXT("EditorSettingsTab", "Move Network Editor Setttings"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FMNAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FMNAssetEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FMNAssetEditorTabs::MoveNetworkPropertyID);
	InTabManager->UnregisterTabSpawner(FMNAssetEditorTabs::MoveNetworkEditorSettingsID);
}

FName FMNAssetEditor::GetToolkitFName() const
{
	return FName("FMoveNetworkEditor");
}

FText FMNAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("MoveNetworkEditorAppLabel", "Move Network Editor");
}

FText FMNAssetEditor::GetToolkitName() const
{
	const bool bDirtyState = EditingGraph->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("MoveNetworkName"), FText::FromString(EditingGraph->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("MoveNetworkEditorToolkitName", "{MoveNetworkName}{DirtyState}"), Args);
}

FText FMNAssetEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(EditingGraph);
}

FLinearColor FMNAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FMNAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("MoveNetworkEditor");
}

FString FMNAssetEditor::GetDocumentationLink() const
{
	return TEXT("");
}

void FMNAssetEditor::SaveAsset_Execute()
{
	if (EditingGraph != nullptr)
	{
		RebuildMoveNetwork();
	}

	FAssetEditorToolkit::SaveAsset_Execute();
}

void FMNAssetEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(EditingGraph);
	Collector.AddReferencedObject(EditingGraph->EdGraph);
}

FString FMNAssetEditor::GetReferencerName() const
{
	return "MNAssetEditor";
}

UMNEditorSettings* FMNAssetEditor::GetSettings() const
{
	return MoveNetworkEditorSettings;
}

TSharedRef<SDockTab> FMNAssetEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMNAssetEditorTabs::ViewportID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"));

	if (ViewportWidget.IsValid())
	{
		SpawnedTab->SetContent(ViewportWidget.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FMNAssetEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMNAssetEditorTabs::MoveNetworkPropertyID);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("Details_Title", "Property"))
		[
			PropertyWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FMNAssetEditor::SpawnTab_EditorSettings(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FMNAssetEditorTabs::MoveNetworkEditorSettingsID);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("EditorSettings_Title", "Move Network Editor Setttings"))
		[
			EditorSettingsWidget.ToSharedRef()
		];
}

void FMNAssetEditor::CreateInternalWidgets()
{
	ViewportWidget = CreateViewportWidget();

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyWidget = PropertyModule.CreateDetailView(Args);
	PropertyWidget->SetObject(EditingGraph);
	PropertyWidget->OnFinishedChangingProperties().AddSP(this, &FMNAssetEditor::OnFinishedChangingProperties);

	EditorSettingsWidget = PropertyModule.CreateDetailView(Args);
	EditorSettingsWidget->SetObject(MoveNetworkEditorSettings);
}

TSharedRef<SGraphEditor> FMNAssetEditor::CreateViewportWidget()
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_MoveNetwork", "Move Network");

	CreateCommandList();

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FMNAssetEditor::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FMNAssetEditor::OnNodeDoubleClicked);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(EditingGraph->EdGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}

void FMNAssetEditor::BindCommands()
{
	ToolkitCommands->MapAction(FMNEditorCommands::Get().GraphSettings,
		FExecuteAction::CreateSP(this, &FMNAssetEditor::GraphSettings),
		FCanExecuteAction::CreateSP(this, &FMNAssetEditor::CanGraphSettings)
	);

	ToolkitCommands->MapAction(FMNEditorCommands::Get().AutoArrange,
		FExecuteAction::CreateSP(this, &FMNAssetEditor::AutoArrange),
		FCanExecuteAction::CreateSP(this, &FMNAssetEditor::CanAutoArrange)
	);
}

void FMNAssetEditor::CreateEdGraph()
{
	if (EditingGraph->EdGraph == nullptr)
	{
		EditingGraph->EdGraph = CastChecked<UMNEdGraph>(FBlueprintEditorUtils::CreateNewGraph(EditingGraph, NAME_None, UMNEdGraph::StaticClass(), UMNAssetGraphSchema::StaticClass()));
		EditingGraph->EdGraph->bAllowDeletion = false;

		// Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema* Schema = EditingGraph->EdGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*EditingGraph->EdGraph);
	}
}

void FMNAssetEditor::CreateCommandList()
{
	if (GraphEditorCommands.IsValid())
	{
		return;
	}

	GraphEditorCommands = MakeShareable(new FUICommandList);

	// Can't use CreateSP here because derived editor are already implementing TSharedFromThis<FAssetEditorToolkit>
	// however it should be safe, since commands are being used only within this editor
	// if it ever crashes, this function will have to go away and be reimplemented in each derived class

	GraphEditorCommands->MapAction(FMNEditorCommands::Get().GraphSettings,
		FExecuteAction::CreateRaw(this, &FMNAssetEditor::GraphSettings),
		FCanExecuteAction::CreateRaw(this, &FMNAssetEditor::CanGraphSettings));

	GraphEditorCommands->MapAction(FMNEditorCommands::Get().AutoArrange,
		FExecuteAction::CreateRaw(this, &FMNAssetEditor::AutoArrange),
		FCanExecuteAction::CreateRaw(this, &FMNAssetEditor::CanAutoArrange));

	GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FMNAssetEditor::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FMNAssetEditor::CanSelectAllNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FMNAssetEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMNAssetEditor::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FMNAssetEditor::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMNAssetEditor::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FMNAssetEditor::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMNAssetEditor::CanCutNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FMNAssetEditor::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FMNAssetEditor::CanPasteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FMNAssetEditor::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FMNAssetEditor::CanDuplicateNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &FMNAssetEditor::OnRenameNode),
		FCanExecuteAction::CreateSP(this, &FMNAssetEditor::CanRenameNodes)
	);
}

TSharedPtr<SGraphEditor> FMNAssetEditor::GetCurrGraphEditor() const
{
	return ViewportWidget;
}

FGraphPanelSelectionSet FMNAssetEditor::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	TSharedPtr<SGraphEditor> FocusedGraphEd = GetCurrGraphEditor();
	if (FocusedGraphEd.IsValid())
	{
		CurrentSelection = FocusedGraphEd->GetSelectedNodes();
	}

	return CurrentSelection;
}

void FMNAssetEditor::RebuildMoveNetwork()
{
	if (EditingGraph == nullptr)
	{
		LOG_WARNING(TEXT("FMoveNetworkAssetEditor::RebuildMoveNetwork EditingGraph is nullptr"));
		return;
	}

	UMNEdGraph* EdGraph = Cast<UMNEdGraph>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	EdGraph->RebuildMoveNetwork();
}

void FMNAssetEditor::SelectAllNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		CurrentGraphEditor->SelectAllNodes();
	}
}

bool FMNAssetEditor::CanSelectAllNodes()
{
	return true;
}

void FMNAssetEditor::DeleteSelectedNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());

	CurrentGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* EdNode = Cast<UEdGraphNode>(*NodeIt);
		if (EdNode == nullptr || !EdNode->CanUserDeleteNode())
			continue;;

		if (UMNNode_EdNode* EdNode_Node = Cast<UMNNode_EdNode>(EdNode))
		{
			EdNode_Node->Modify();

			const UEdGraphSchema* Schema = EdNode_Node->GetSchema();
			if (Schema != nullptr)
			{
				Schema->BreakNodeLinks(*EdNode_Node);
			}

			EdNode_Node->DestroyNode();
		}
		else
		{
			EdNode->Modify();
			EdNode->DestroyNode();
		}
	}
}

bool FMNAssetEditor::CanDeleteNodes()
{
	// If any of the nodes can be deleted then we should allow deleting
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node != nullptr && Node->CanUserDeleteNode())
		{
			return true;
		}
	}

	return false;
}

void FMNAssetEditor::DeleteSelectedDuplicatableNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FGraphPanelSelectionSet OldSelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}
}

void FMNAssetEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
}

bool FMNAssetEditor::CanCutNodes()
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FMNAssetEditor::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;

	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node == nullptr)
		{
			SelectedIter.RemoveCurrent();
			continue;
		}

		if (UMNEdge_EdNode* EdNode_Edge = Cast<UMNEdge_EdNode>(*SelectedIter))
		{
			UMNNode_EdNode* StartNode = EdNode_Edge->GetStartNode();
			UMNNode_EdNode* EndNode = EdNode_Edge->GetEndNode();

			if (!SelectedNodes.Contains(StartNode) || !SelectedNodes.Contains(EndNode))
			{
				SelectedIter.RemoveCurrent();
				continue;
			}
		}

		Node->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

bool FMNAssetEditor::CanCopyNodes()
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}

void FMNAssetEditor::PasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		PasteNodesHere(CurrentGraphEditor->GetPasteLocation());
	}
}

void FMNAssetEditor::PasteNodesHere(const FVector2D& Location)
{
	// Find the graph editor with focus
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}
	// Select the newly pasted stuff
	UEdGraph* EdGraph = CurrentGraphEditor->GetCurrentGraph();

	{
		const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
		EdGraph->Modify();

		// Clear the selection set (newly pasted stuff will be selected)
		CurrentGraphEditor->ClearSelectionSet();

		// Grab the text to paste from the clipboard.
		FString TextToImport;
		FPlatformApplicationMisc::ClipboardPaste(TextToImport);

		// Import the nodes
		TSet<UEdGraphNode*> PastedNodes;
		FEdGraphUtilities::ImportNodesFromText(EdGraph, TextToImport, PastedNodes);

		//Average position of nodes so we can move them while still maintaining relative distances to each other
		FVector2D AvgNodePosition(0.0f, 0.0f);

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* Node = *It;
			AvgNodePosition.X += Node->NodePosX;
			AvgNodePosition.Y += Node->NodePosY;
		}

		float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* Node = *It;
			CurrentGraphEditor->SetNodeSelection(Node, true);

			Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
			Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

			Node->SnapToGrid(16);

			// Give new node a different Guid from the old one
			Node->CreateNewGuid();
		}
	}

	// Update UI
	CurrentGraphEditor->NotifyGraphChanged();

	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}
}

bool FMNAssetEditor::CanPasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return false;
	}

	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(CurrentGraphEditor->GetCurrentGraph(), ClipboardContent);
}

void FMNAssetEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FMNAssetEditor::CanDuplicateNodes()
{
	return CanCopyNodes();
}

void FMNAssetEditor::GraphSettings()
{
	PropertyWidget->SetObject(EditingGraph);
}

bool FMNAssetEditor::CanGraphSettings() const
{
	return true;
}

void FMNAssetEditor::AutoArrange()
{
	UMNEdGraph* EdGraph = Cast<UMNEdGraph>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	const FScopedTransaction Transaction(LOCTEXT("MoveNetworkEditorAutoArrange", "Move Network Editor: Auto Arrange"));

	EdGraph->Modify();

	UMNAutoLayout* LayoutStrategy = nullptr;
	switch (MoveNetworkEditorSettings->AutoLayoutStrategy)
	{
	case EMNAutoLayout::Tree:
		LayoutStrategy = NewObject<UMNAutoLayout>(EdGraph, UMNTreeLayout::StaticClass());
		break;
	case EMNAutoLayout::ForceDirected:
		LayoutStrategy = NewObject<UMNAutoLayout>(EdGraph, UMNForceDirectedLayout::StaticClass());
		break;
	default:
		break;
	}

	if (LayoutStrategy != nullptr)
	{
		LayoutStrategy->Settings = MoveNetworkEditorSettings;
		LayoutStrategy->Layout(EdGraph);
		LayoutStrategy->ConditionalBeginDestroy();
	}
	else
	{
		LOG_ERROR(TEXT("FMNAssetEditor::AutoArrange LayoutStrategy is null."));
	}
}

bool FMNAssetEditor::CanAutoArrange() const
{
	return EditingGraph != nullptr && Cast<UMNEdGraph>(EditingGraph->EdGraph) != nullptr;
}

void FMNAssetEditor::OnRenameNode()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UEdGraphNode* SelectedNode = Cast<UEdGraphNode>(*NodeIt);
			if (SelectedNode != NULL && SelectedNode->bCanRenameNode)
			{
				CurrentGraphEditor->IsNodeTitleVisible(SelectedNode, true);
				break;
			}
		}
	}
}

bool FMNAssetEditor::CanRenameNodes() const
{
	UMNEdGraph* EdGraph = Cast<UMNEdGraph>(EditingGraph->EdGraph);
	check(EdGraph != nullptr);

	UMoveNetwork* Graph = EdGraph->GetMoveNetwork();
	check(Graph != nullptr)

	return Graph->bCanRenameNode && GetSelectedNodes().Num() == 1;
}

void FMNAssetEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection;

	for (UObject* SelectionEntry : NewSelection)
	{
		Selection.Add(SelectionEntry);
	}

	if (Selection.Num() == 0) 
	{
		PropertyWidget->SetObject(EditingGraph);

	}
	else
	{
		PropertyWidget->SetObjects(Selection);
	}
}

void FMNAssetEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
	
}

void FMNAssetEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (EditingGraph == nullptr)
		return;

	EditingGraph->EdGraph->GetSchema()->ForceVisualizationCacheClear();
}

void FMNAssetEditor::OnPackageSaved(const FString& PackageFileName, UObject* Outer)
{
	RebuildMoveNetwork();
}

void FMNAssetEditor::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager) 
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}


#undef LOCTEXT_NAMESPACE

