#pragma once

#include "CoreMinimal.h"
#include "MNAutoLayout.h"
#include "MNForceDirectedLayout.generated.h"

UCLASS()
class UMNForceDirectedLayout : public UMNAutoLayout
{
	GENERATED_BODY()
public:
	UMNForceDirectedLayout();
	virtual ~UMNForceDirectedLayout();

	virtual void Layout(UEdGraph* EdGraph) override;

protected:
	virtual FBox2D LayoutOneTree(UMNNode* RootNode, const FBox2D& PreTreeBound);

protected:
	bool bRandomInit;
	float InitTemperature;
	float CoolDownRate;
};
