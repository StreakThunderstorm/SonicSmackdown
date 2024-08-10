#include "MNAssetTypeActions.h"
#include "MoveNetworkEditorPCH.h"
#include "EdGraphUtilities.h"
#include "MoveNetworkAssetEditor/MNNode_EdNode.h"
#include "MoveNetworkAssetEditor/MNEdge_EdNode.h"
#include "MoveNetworkAssetEditor/MNNode_SEdNode.h"
#include "MoveNetworkAssetEditor/MNEdge_SEdNode.h"
#include "MoveNetworkAssetEditor/MNEditorStyle.h"

DEFINE_LOG_CATEGORY(MoveNetworkEditor)

#define LOCTEXT_NAMESPACE "MoveNetworkEditor"

class FGraphPanelNodeFactory_MoveNetwork : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if (UMNNode_EdNode* EdNode_GraphNode = Cast<UMNNode_EdNode>(Node))
		{
			return SNew(MNNode_SEDNode, EdNode_GraphNode);
		}
		else if (UMNEdge_EdNode* EdNode_Edge = Cast<UMNEdge_EdNode>(Node))
		{
			return SNew(SMNEdge_EdNode, EdNode_Edge);
		}
		return nullptr;
	}
};

TSharedPtr<FGraphPanelNodeFactory> GraphPanelNodeFactory_MoveNetwork;

class FMoveNetworkEditor : public IMoveNetworkEditor
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);

private:
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

	EAssetTypeCategories::Type MoveNetworkAssetCategoryBit;
};

IMPLEMENT_MODULE( FMoveNetworkEditor, MoveNetworkEditor )

void FMoveNetworkEditor::StartupModule()
{
	FMNEditorStyle::Initialize();

	GraphPanelNodeFactory_MoveNetwork = MakeShareable(new FGraphPanelNodeFactory_MoveNetwork());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_MoveNetwork);

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	MoveNetworkAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("MoveNetwork")), LOCTEXT("MoveNetworkAssetCategory", "MoveNetwork"));

	RegisterAssetTypeAction(AssetTools, MakeShareable(new FMNAssetTypeActions(MoveNetworkAssetCategoryBit)));
}


void FMoveNetworkEditor::ShutdownModule()
{
	// Unregister all the asset types that we registered
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}

	if (GraphPanelNodeFactory_MoveNetwork.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_MoveNetwork);
		GraphPanelNodeFactory_MoveNetwork.Reset();
	}

	FMNEditorStyle::Shutdown();
}

void FMoveNetworkEditor::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE

