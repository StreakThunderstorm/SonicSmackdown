#include "MNNode.h"
#include "MoveNetwork.h"

#define LOCTEXT_NAMESPACE "MoveNetworkNode"

UMNNode::UMNNode()
{
#if WITH_EDITORONLY_DATA
	CompatibleGraphType = UMoveNetwork::StaticClass();

	BackgroundColor = FLinearColor::Black;
#endif
}

UMNNode::~UMNNode()
{

}

UMNEdge* UMNNode::GetEdge(UMNNode* ChildNode)
{
	return Edges.Contains(ChildNode) ? Edges.FindChecked(ChildNode) : nullptr;
}

FText UMNNode::GetDescription_Implementation() const
{
	return LOCTEXT("NodeDesc", "Move Network Node");
}

#if WITH_EDITOR

FLinearColor UMNNode::GetBackgroundColor() const
{
	return BackgroundColor;
}

FText UMNNode::GetNodeTitle() const
{
	return NodeTitle.IsEmpty() ? GetDescription() : NodeTitle;
}

void UMNNode::SetNodeTitle(const FText& NewTitle)
{
	NodeTitle = NewTitle;
}

bool UMNNode::CanCreateConnection(UMNNode* Other, FText& ErrorMessage)
{
	return true;
}

#endif

bool UMNNode::IsLeafNode() const
{
	return ChildrenNodes.Num() == 0;
}

UMoveNetwork* UMNNode::GetGraph() const
{
	return Graph;
}

#undef LOCTEXT_NAMESPACE
