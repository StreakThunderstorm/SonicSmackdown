#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UMNNode_EdNode;

class MNNode_SEDNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(MNNode_SEDNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UMNNode_EdNode* InNode);

	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual bool IsNameReadOnly() const override;

	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);

	virtual FSlateColor GetBorderBackgroundColor() const;
	virtual FSlateColor GetBackgroundColor() const;

	virtual EVisibility GetDragOverMarkerVisibility() const;

	virtual const FSlateBrush* GetNameIcon() const;

protected:
	TSharedPtr<SBorder> NodeBody;
	TSharedPtr<SHorizontalBox> OutputPinBox;
};
