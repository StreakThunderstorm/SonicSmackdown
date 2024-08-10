#pragma once

#include "CoreMinimal.h"
#include "MNNode.h"
#include "MNEdge.generated.h"

class UMoveNetwork;

UCLASS(Blueprintable)
class MOVENETWORKRUNTIME_API UMNEdge : public UObject
{
	GENERATED_BODY()

public:
	UMNEdge();
	virtual ~UMNEdge();

	UPROPERTY(VisibleAnywhere, Category = "MoveNetworkNode")
	UMoveNetwork* Graph;

	UPROPERTY(BlueprintReadOnly, Category = "MoveNetworkEdge")
	UMNNode* StartNode;

	UPROPERTY(BlueprintReadOnly, Category = "MoveNetworkEdge")
	UMNNode* EndNode;

	UFUNCTION(BlueprintPure, Category = "MoveNetworkEdge")
	UMoveNetwork* GetGraph() const;
};
