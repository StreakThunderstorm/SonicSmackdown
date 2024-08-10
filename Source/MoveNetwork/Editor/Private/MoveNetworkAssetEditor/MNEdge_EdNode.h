#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "MNEdge_EdNode.generated.h"

class UMNNode;
class UMNEdge;
class UMNNode_EdNode;

UCLASS(MinimalAPI)
class UMNEdge_EdNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY()
	class UEdGraph* Graph;

	UPROPERTY(VisibleAnywhere, Instanced, Category = "MoveNetwork")
	UMNEdge* MoveNetworkEdge;

	void SetEdge(UMNEdge* Edge);

	virtual void AllocateDefaultPins() override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	virtual void PrepareForCopying() override;

	virtual UEdGraphPin* GetInputPin() const { return Pins[0]; }
	virtual UEdGraphPin* GetOutputPin() const { return Pins[1]; }

	void CreateConnections(UMNNode_EdNode* Start, UMNNode_EdNode* End);

	UMNNode_EdNode* GetStartNode();
	UMNNode_EdNode* GetEndNode();
};
