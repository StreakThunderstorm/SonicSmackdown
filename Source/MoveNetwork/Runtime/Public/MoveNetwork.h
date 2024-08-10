#pragma once

#include "CoreMinimal.h"
#include "MNNode.h"
#include "MNEdge.h"
#include "GameplayTagContainer.h"
#include "MoveNetwork.generated.h"

UCLASS(BlueprintType)
class MOVENETWORKRUNTIME_API UMoveNetwork : public UObject
{
	GENERATED_BODY()

public:
	UMoveNetwork();
	virtual ~UMoveNetwork();

	UPROPERTY(EditDefaultsOnly, Category = "MoveNetwork")
	FString Name;

	UPROPERTY(EditDefaultsOnly, Category = "MoveNetwork")
	TSubclassOf<UMNNode> NodeType;

	UPROPERTY(EditDefaultsOnly, Category = "MoveNetwork")
	TSubclassOf<UMNEdge> EdgeType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MoveNetwork")
	FGameplayTagContainer GraphTags;

	UPROPERTY(BlueprintReadOnly, Category = "MoveNetwork")
	TArray<UMNNode*> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "MoveNetwork")
	TArray<UMNNode*> AllNodes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MoveNetwork")
	bool bEdgeEnabled;

	UFUNCTION(BlueprintCallable, Category = "MoveNetwork")
	void Print(bool ToConsole = true, bool ToScreen = true);

	UFUNCTION(BlueprintCallable, Category = "MoveNetwork")
	int GetLevelNum() const;

	UFUNCTION(BlueprintCallable, Category = "MoveNetwork")
	void GetNodesByLevel(int Level, TArray<UMNNode*>& Nodes);

	UFUNCTION(BlueprintCallable, Category = "MoveNetwork")
	void GetSortedNodes(TArray<UMNNode*>& Nodes);

	void ClearGraph();

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UEdGraph* EdGraph;

	UPROPERTY(EditDefaultsOnly, Category = "MoveNetwork_Editor")
	bool bCanRenameNode;
#endif
};
