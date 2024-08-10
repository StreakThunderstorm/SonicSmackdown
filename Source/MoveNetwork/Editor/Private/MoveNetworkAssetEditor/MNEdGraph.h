#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "MNEdGraph.generated.h"

class UMoveNetwork;
class UMNNode;
class UMNEdge;
class UMNNode_EdNode;
class UMNEdge_EdNode;

UCLASS()
class UMNEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UMNEdGraph();
	virtual ~UMNEdGraph();

	virtual void RebuildMoveNetwork();

	UMoveNetwork* GetMoveNetwork() const;

	virtual bool Modify(bool bAlwaysMarkDirty = true) override;
	virtual void PostEditUndo() override;

	UPROPERTY(Transient)
	TMap<UMNNode*, UMNNode_EdNode*> NodeMap;

	UPROPERTY(Transient)
	TMap<UMNEdge*, UMNEdge_EdNode*> EdgeMap;

protected:
	void Clear();

	void SortNodes(UMNNode* RootNode);
};
