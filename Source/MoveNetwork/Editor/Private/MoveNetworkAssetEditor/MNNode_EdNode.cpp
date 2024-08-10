#include "MNNode_EdNode.h"
#include "MNEdGraph.h"
#include "Kismet2/Kismet2NameValidators.h"
#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "EdNode_MoveNetwork"

UMNNode_EdNode::UMNNode_EdNode()
{
	bCanRenameNode = true;
}

UMNNode_EdNode::~UMNNode_EdNode()
{

}

void UMNNode_EdNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNodes", FName(), TEXT("In"));
	CreatePin(EGPD_Output, "MultipleNodes", FName(), TEXT("Out"));
}

UMNEdGraph* UMNNode_EdNode::GetMoveNetworkEdGraph()
{
	return Cast<UMNEdGraph>(GetGraph());
}

FText UMNNode_EdNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (MoveNetworkNode == nullptr)
	{
		return Super::GetNodeTitle(TitleType);
	}
	else
	{
		return MoveNetworkNode->GetNodeTitle();
	}
}

void UMNNode_EdNode::PrepareForCopying()
{
	MoveNetworkNode->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
}

void UMNNode_EdNode::AutowireNewNode(UEdGraphPin* FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin != nullptr)
	{
		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}

void UMNNode_EdNode::SetMoveNetworkNode(UMNNode* InNode)
{
	MoveNetworkNode = InNode;
}

FLinearColor UMNNode_EdNode::GetBackgroundColor() const
{
	return MoveNetworkNode == nullptr ? FLinearColor::Black : MoveNetworkNode->GetBackgroundColor();
}

UEdGraphPin* UMNNode_EdNode::GetInputPin() const
{
	return Pins[0];
}

UEdGraphPin* UMNNode_EdNode::GetOutputPin() const
{
	return Pins[1];
}

void UMNNode_EdNode::PostEditUndo()
{
	UEdGraphNode::PostEditUndo();
}

#undef LOCTEXT_NAMESPACE
