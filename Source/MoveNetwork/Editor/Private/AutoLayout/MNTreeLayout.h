#pragma once

#include "CoreMinimal.h"
#include "MNAutoLayout.h"
#include "MNTreeLayout.generated.h"

UCLASS()
class UMNTreeLayout : public UMNAutoLayout
{
	GENERATED_BODY()
public:
	UMNTreeLayout();
	virtual ~UMNTreeLayout();

	virtual void Layout(UEdGraph* EdGraph) override;

protected:
	void InitPass(UMNNode* RootNode, const FVector2D& Anchor);
	bool ResolveConflictPass(UMNNode* Node);

	bool ResolveConflict(UMNNode* LRoot, UMNNode* RRoot);

	void GetLeftContour(UMNNode* RootNode, int32 Level, TArray<UMNNode_EdNode*>& Contour);
	void GetRightContour(UMNNode* RootNode, int32 Level, TArray<UMNNode_EdNode*>& Contour);
	
	void ShiftSubTree(UMNNode* RootNode, const FVector2D& Offset);

	void UpdateParentNodePosition(UMNNode* RootNode);
};
