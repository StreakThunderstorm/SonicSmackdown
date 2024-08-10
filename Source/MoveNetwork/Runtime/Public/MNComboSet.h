// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MNComboSet.generated.h"

// Forward declarations
class UMoveNetwork;
class UMNNode;
class UMNNode_Move;

// Data structure for a set of move cancels.
USTRUCT(BlueprintType)
struct FOOSMoveCancels
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UMNNode_Move*> Nodes;

	FOOSMoveCancels()
	{
		Nodes = TArray<UMNNode_Move*>();
	};

	FOOSMoveCancels(TArray<UMNNode_Move*> ComboNodes)
	{
		Nodes = ComboNodes;
	};
};

/**
 * 
 */
UCLASS()
class MOVENETWORKRUNTIME_API UMNComboSet : public UObject
{
	GENERATED_BODY()

private:
	UMNComboSet();

public:
	void SetMoveNetwork(UMoveNetwork* MoveNetwork);
	void Reset();

	TArray<UMNNode_Move*> ComboNodes(UMNNode* Node);

private:
	TMap<UMNNode*, FOOSMoveCancels> MoveCancels;

	void AddChildren(
		TArray<UMNNode*> ChildrenNodes,
		TArray<UMNNode_Move*>* ComboNodes,
		TArray<UMNNode*>* AlreadyTested);
	
};
