#include "MNAutoLayout.h"
#include "Kismet/KismetMathLibrary.h"
#include "../MoveNetworkAssetEditor/MNNode_EdNode.h"
#include "../MoveNetworkAssetEditor/MNNode_SEdNode.h"

UMNAutoLayout::UMNAutoLayout()
{
	Settings = nullptr;
	MaxIteration = 50;
	OptimalDistance = 150;
}

UMNAutoLayout::~UMNAutoLayout()
{

}

FBox2D UMNAutoLayout::GetNodeBound(UEdGraphNode* EdNode)
{
	int32 NodeWidth = GetNodeWidth(Cast<UMNNode_EdNode>(EdNode));
	int32 NodeHeight = GetNodeHeight(Cast<UMNNode_EdNode>(EdNode));
	FVector2D Min(EdNode->NodePosX, EdNode->NodePosY);
	FVector2D Max(EdNode->NodePosX + NodeWidth, EdNode->NodePosY + NodeHeight);
	return FBox2D(Min, Max);
}

FBox2D UMNAutoLayout::GetActualBounds(UMNNode* RootNode)
{
	int Level = 0;
	TArray<UMNNode*> CurrLevelNodes = { RootNode };
	TArray<UMNNode*> NextLevelNodes;

	FBox2D Rtn = GetNodeBound(EdGraph->NodeMap[RootNode]);

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UMNNode* Node = CurrLevelNodes[i];
			check(Node != nullptr);

			Rtn += GetNodeBound(EdGraph->NodeMap[Node]);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}
	return Rtn;
}

void UMNAutoLayout::RandomLayoutOneTree(UMNNode* RootNode, const FBox2D& Bound)
{
	int Level = 0;
	TArray<UMNNode*> CurrLevelNodes = { RootNode };
	TArray<UMNNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UMNNode* Node = CurrLevelNodes[i];
			check(Node != nullptr);

			UMNNode_EdNode* EdNode_Node = EdGraph->NodeMap[Node];

			EdNode_Node->NodePosX = UKismetMathLibrary::RandomFloatInRange(Bound.Min.X, Bound.Max.X);
			EdNode_Node->NodePosY = UKismetMathLibrary::RandomFloatInRange(Bound.Min.Y, Bound.Max.Y);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}
}

int32 UMNAutoLayout::GetNodeWidth(UMNNode_EdNode* EdNode)
{
	return EdNode->SEdNode->GetCachedGeometry().GetLocalSize().X;
}

int32 UMNAutoLayout::GetNodeHeight(UMNNode_EdNode* EdNode)
{
	return EdNode->SEdNode->GetCachedGeometry().GetLocalSize().Y;
}

