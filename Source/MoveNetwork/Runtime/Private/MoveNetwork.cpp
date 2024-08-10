#include "MoveNetwork.h"
#include "MoveNetworkRuntimePCH.h"
#include "MNNode_Move.h"
#include "OOSMove.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "MoveNetworkRuntime"

UMoveNetwork::UMoveNetwork()
{
	NodeType = UMNNode::StaticClass();
	EdgeType = UMNEdge::StaticClass();

	bEdgeEnabled = true;

#if WITH_EDITORONLY_DATA
	EdGraph = nullptr;

	bCanRenameNode = true;
#endif
}

UMoveNetwork::~UMoveNetwork()
{

}

void UMoveNetwork::Print(bool ToConsole /*= true*/, bool ToScreen /*= true*/)
{
	int Level = 0;
	TArray<UMNNode*> CurrLevelNodes = RootNodes;
	TArray<UMNNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UMNNode* Node = CurrLevelNodes[i];
			check(Node != nullptr);

			FString Message = FString::Printf(TEXT("%s, Level %d"), *Node->GetDescription().ToString(), Level);

			if (ToConsole)
			{
				LOG_INFO(TEXT("%s"), *Message);
			}

			if (ToScreen && GEngine != nullptr)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, Message);
			}

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

int UMoveNetwork::GetLevelNum() const
{
	int Level = 0;
	TArray<UMNNode*> CurrLevelNodes = RootNodes;
	TArray<UMNNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UMNNode* Node = CurrLevelNodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}

	return Level;
}

void UMoveNetwork::GetNodesByLevel(int Level, TArray<UMNNode*>& Nodes)
{
	int CurrLEvel = 0;
	TArray<UMNNode*> NextLevelNodes;

	Nodes = RootNodes;

	while (Nodes.Num() != 0)
	{
		if (CurrLEvel == Level)
			break;

		for (int i = 0; i < Nodes.Num(); ++i)
		{
			UMNNode* Node = Nodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		Nodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++CurrLEvel;
	}
}

void UMoveNetwork::GetSortedNodes(TArray<UMNNode*>& Nodes)
{
	Nodes.Empty();

	// Copy AllNodes
	for (UMNNode* Node : AllNodes)
	{
		Nodes.Add(Node);
	}

	Algo::Sort(Nodes, UMNNode_Move::Compare);
}

void UMoveNetwork::ClearGraph()
{
	for (int i = 0; i < AllNodes.Num(); ++i)
	{
		UMNNode* Node = AllNodes[i];

		Node->ParentNodes.Empty();
		Node->ChildrenNodes.Empty();
		Node->Edges.Empty();
	}

	AllNodes.Empty();
	RootNodes.Empty();
}

#undef LOCTEXT_NAMESPACE
