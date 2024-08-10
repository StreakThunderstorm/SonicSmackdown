#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "MNNode.h"
#include "MNNode_EdNode.generated.h"

class UMNEdge_EdNode;
class UMNEdGraph;
class MNNode_SEDNode;

UCLASS(MinimalAPI)
class UMNNode_EdNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UMNNode_EdNode();
	virtual ~UMNNode_EdNode();

	UPROPERTY(VisibleAnywhere, Instanced, Category = "MoveNetwork")
	UMNNode* MoveNetworkNode;

	void SetMoveNetworkNode(UMNNode* InNode);
	UMNEdGraph* GetMoveNetworkEdGraph();

	MNNode_SEDNode* SEdNode;

	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void PrepareForCopying() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;

	virtual FLinearColor GetBackgroundColor() const;
	virtual UEdGraphPin* GetInputPin() const;
	virtual UEdGraphPin* GetOutputPin() const;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif

};
