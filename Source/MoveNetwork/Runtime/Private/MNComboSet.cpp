// Fill out your copyright notice in the Description page of Project Settings.


#include "MNComboSet.h"
#include "MoveNetwork.h"
#include "MNNode_Move.h"

UMNComboSet::UMNComboSet()
{

}

void UMNComboSet::SetMoveNetwork(UMoveNetwork* MoveNetwork)
{
	MoveCancels.Reset();

	if (MoveNetwork)
	{
		for (size_t i = 0; i < MoveNetwork->AllNodes.Num(); i++)
		{
			UMNNode* Node = MoveNetwork->AllNodes[i];
			TArray<UMNNode_Move*> ComboNodes = TArray<UMNNode_Move*>();

			// Skip non-moves
			if (Cast<UMNNode_Move>(Node))
			{
				TArray<UMNNode*> AlreadyTested = TArray<UMNNode*>();
				AddChildren(Node->ChildrenNodes, &ComboNodes, &AlreadyTested);

				Algo::Sort(ComboNodes, UMNNode_Move::Compare);
			}

			MoveCancels.Add(Node, FOOSMoveCancels(ComboNodes));
		}
	}
}

void UMNComboSet::Reset()
{
	MoveCancels.Reset();
}

TArray<UMNNode_Move*> UMNComboSet::ComboNodes(UMNNode* Node)
{
	if (Node && MoveCancels.Contains(Node))
	{
		return MoveCancels[Node].Nodes;
	}

	return TArray<UMNNode_Move*>();
}

void UMNComboSet::AddChildren(
	TArray<UMNNode*> ChildrenNodes,
	TArray<UMNNode_Move*>* ComboNodes,
	TArray<UMNNode*>* AlreadyTested)
{
	for (UMNNode* Node : ChildrenNodes)
	{
		if (!AlreadyTested->Contains(Node))
		{
			AlreadyTested->Add(Node);

			// Add moves
			UMNNode_Move* Move = Cast<UMNNode_Move>(Node);
			if (Move)
			{
				ComboNodes->Add(Move);
			}

			// Continue iterating
			AddChildren(Node->ChildrenNodes, ComboNodes, AlreadyTested);
		}
	}
}