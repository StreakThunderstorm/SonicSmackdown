#include "MNEdGraph.h"
#include "../MoveNetworkEditorPCH.h"
#include "MoveNetwork.h"
#include "MNNode_EdNode.h"
#include "MNEdge_EdNode.h"
#include "EdGraph/EdGraphPin.h"

UMNEdGraph::UMNEdGraph()
{

}

UMNEdGraph::~UMNEdGraph()
{

}

void UMNEdGraph::RebuildMoveNetwork()
{
	LOG_INFO(TEXT("UMoveNetworkEdGraph::RebuildMoveNetwork has been called"));

	UMoveNetwork* Graph = GetMoveNetwork();

	Clear();

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UMNNode_EdNode* EdNode = Cast<UMNNode_EdNode>(Nodes[i]))
		{
			if (EdNode->MoveNetworkNode == nullptr)
				continue;

			UMNNode* MoveNetworkNode = EdNode->MoveNetworkNode;

			NodeMap.Add(MoveNetworkNode, EdNode);

			Graph->AllNodes.Add(MoveNetworkNode);

			for (int PinIdx = 0; PinIdx < EdNode->Pins.Num(); ++PinIdx)
			{
				UEdGraphPin* Pin = EdNode->Pins[PinIdx];

				if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
					continue;

				for (int LinkToIdx = 0; LinkToIdx < Pin->LinkedTo.Num(); ++LinkToIdx)
				{
					UMNNode* ChildNode = nullptr;
					if (UMNNode_EdNode* EdNode_Child = Cast<UMNNode_EdNode>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						ChildNode = EdNode_Child->MoveNetworkNode;
					}
					else if (UMNEdge_EdNode* EdNode_Edge = Cast<UMNEdge_EdNode>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						EdNode_Child = EdNode_Edge->GetEndNode();;
						if (EdNode_Child != nullptr)
						{
							ChildNode = EdNode_Child->MoveNetworkNode;
						}
					}

					if (ChildNode != nullptr)
					{
						MoveNetworkNode->ChildrenNodes.Add(ChildNode);

						ChildNode->ParentNodes.Add(MoveNetworkNode);
					}
					else
					{
						LOG_ERROR(TEXT("UMNEdGraph::RebuildMoveNetwork can't find child node"));
					}
				}
			}
		}
		else if (UMNEdge_EdNode* EdgeNode = Cast<UMNEdge_EdNode>(Nodes[i]))
		{
			UMNNode_EdNode* StartNode = EdgeNode->GetStartNode();
			UMNNode_EdNode* EndNode = EdgeNode->GetEndNode();
			UMNEdge* Edge = EdgeNode->MoveNetworkEdge;

			if (StartNode == nullptr || EndNode == nullptr || Edge == nullptr)
			{
				LOG_ERROR(TEXT("UMNEdGraph::RebuildMoveNetwork add edge failed."));
				continue;
			}

			EdgeMap.Add(Edge, EdgeNode);

			Edge->Graph = Graph;
			Edge->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
			Edge->StartNode = StartNode->MoveNetworkNode;
			Edge->EndNode = EndNode->MoveNetworkNode;
			Edge->StartNode->Edges.Add(Edge->EndNode, Edge);
		}
	}

	for (int i = 0; i < Graph->AllNodes.Num(); ++i)
	{
		UMNNode* Node = Graph->AllNodes[i];
		if (Node->ParentNodes.Num() == 0)
		{
			Graph->RootNodes.Add(Node);

			SortNodes(Node);
		}

		Node->Graph = Graph;
		Node->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
	}

	Graph->RootNodes.Sort([&](const UMNNode& L, const UMNNode& R)
	{
		UMNNode_EdNode* EdNode_LNode = NodeMap[&L];
		UMNNode_EdNode* EdNode_RNode = NodeMap[&R];
		return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
	});
}

UMoveNetwork* UMNEdGraph::GetMoveNetwork() const
{
	return CastChecked<UMoveNetwork>(GetOuter());
}

bool UMNEdGraph::Modify(bool bAlwaysMarkDirty /*= true*/)
{
	bool Rtn = Super::Modify(bAlwaysMarkDirty);

	GetMoveNetwork()->Modify();

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		Nodes[i]->Modify();
	}

	return Rtn;
}

void UMNEdGraph::Clear()
{
	UMoveNetwork* Graph = GetMoveNetwork();

	Graph->ClearGraph();
	NodeMap.Reset();
	EdgeMap.Reset();

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UMNNode_EdNode* EdNode = Cast<UMNNode_EdNode>(Nodes[i]))
		{
			UMNNode* MoveNetworkNode = EdNode->MoveNetworkNode;
			MoveNetworkNode->ParentNodes.Reset();
			MoveNetworkNode->ChildrenNodes.Reset();
			MoveNetworkNode->Edges.Reset();
		}
	}
}

void UMNEdGraph::SortNodes(UMNNode* RootNode)
{
	int Level = 0;
	TArray<UMNNode*> CurrLevelNodes = { RootNode };
	TArray<UMNNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		int32 LevelWidth = 0;
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UMNNode* Node = CurrLevelNodes[i];

			auto Comp = [&](const UMNNode& L, const UMNNode& R)
			{
				UMNNode_EdNode* EdNode_LNode = NodeMap[&L];
				UMNNode_EdNode* EdNode_RNode = NodeMap[&R];
				return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
			};

			Node->ChildrenNodes.Sort(Comp);
			Node->ParentNodes.Sort(Comp);

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

void UMNEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
}

