#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "../../../Runtime/Public/MoveNetwork.h"
#include "../MoveNetworkAssetEditor/MNEdGraph.h"
#include "../MoveNetworkAssetEditor/MNNode_EdNode.h"
#include "../MoveNetworkAssetEditor/MNEdge_EdNode.h"
#include "../MoveNetworkAssetEditor/MNEditorSettings.h"
#include "MNAutoLayout.generated.h"

UCLASS(abstract)
class UMNAutoLayout : public UObject
{
	GENERATED_BODY()
public:
	UMNAutoLayout();
	virtual ~UMNAutoLayout();

	virtual void Layout(UEdGraph* InEdGraph) {};

	class UMNEditorSettings* Settings;

protected:
	int32 GetNodeWidth(UMNNode_EdNode* EdNode);

	int32 GetNodeHeight(UMNNode_EdNode* EdNode);

	FBox2D GetNodeBound(UEdGraphNode* EdNode);

	FBox2D GetActualBounds(UMNNode* RootNode);

	virtual void RandomLayoutOneTree(UMNNode* RootNode, const FBox2D& Bound);

protected:
	UMoveNetwork* Graph;
	UMNEdGraph* EdGraph;
	int32 MaxIteration;
	int32 OptimalDistance;
};
