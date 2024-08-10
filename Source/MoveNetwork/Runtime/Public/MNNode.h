#pragma once

#include "CoreMinimal.h"
#include "SubclassOf.h"
#include "TextProperty.h"
#include "OOSMove.h"
#include "MNNode.generated.h"

class UMoveNetwork;
class UMNEdge;

UCLASS(BlueprintType)
class MOVENETWORKRUNTIME_API UMNNode : public UObject
{
	GENERATED_BODY()

public:
	UMNNode();
	virtual ~UMNNode();

	UPROPERTY(VisibleDefaultsOnly, Category = "MoveNetworkNode")
	UMoveNetwork* Graph;

	UPROPERTY(BlueprintReadOnly, Category = "MoveNetworkNode")
	TArray<UMNNode*> ParentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "MoveNetworkNode")
	TArray<UMNNode*> ChildrenNodes;

	UPROPERTY(BlueprintReadOnly, Category = "MoveNetworkNode")
	TMap<UMNNode*, UMNEdge*> Edges;

	UFUNCTION(BlueprintCallable, Category = "MoveNetworkNode")
	virtual UMNEdge* GetEdge(UMNNode* ChildNode);

	UFUNCTION(BlueprintCallable, Category = "MoveNetworkNode")
	bool IsLeafNode() const;

	UFUNCTION(BlueprintCallable, Category = "MoveNetworkNode")
	UMoveNetwork* GetGraph() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MissionNode")
	FText GetDescription() const;
	virtual FText GetDescription_Implementation() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MoveNetworkNode")
	FText NodeTitle;

	//////////////////////////////////////////////////////////////////////////
#if WITH_EDITORONLY_DATA

	UPROPERTY(VisibleDefaultsOnly, Category = "MoveNetworkNode_Editor")
	TSubclassOf<UMoveNetwork> CompatibleGraphType;

	UPROPERTY(EditDefaultsOnly, Category = "MoveNetworkNode_Editor")
	FLinearColor BackgroundColor;

	UPROPERTY(EditDefaultsOnly, Category = "MoveNetworkNode_Editor")
	FText ContextMenuName;
#endif

#if WITH_EDITOR
	virtual FLinearColor GetBackgroundColor() const;

	virtual FText GetNodeTitle() const;

	virtual void SetNodeTitle(const FText& NewTitle);

	virtual bool CanCreateConnection(UMNNode* Other, FText& ErrorMessage);
#endif
};
