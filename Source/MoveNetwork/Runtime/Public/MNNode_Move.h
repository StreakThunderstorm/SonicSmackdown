// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MNNode.h"
#include "MNNode_Move.generated.h"

/**
 * 
 */
UCLASS()
class MOVENETWORKRUNTIME_API UMNNode_Move : public UMNNode
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MoveProperties")
	FOOSMove Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MoveListProperties")
	FText MoveName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MoveListProperties")
	FText AdditionalNotes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MoveListProperties")
	bool bDisplayInMoveList = true;

	EOOSInputDir GetInputDir() const;

	static bool Compare(const UMNNode* A, const UMNNode* B)
	{
		const UMNNode_Move* MoveNodeA = Cast<UMNNode_Move>(A);
		const UMNNode_Move* MoveNodeB = Cast<UMNNode_Move>(B);

		if (!MoveNodeA || !MoveNodeB)
			return A > B;

		// Normal, Special, etc
		if (MoveNodeA->Move.MoveType != MoveNodeB->Move.MoveType)
			return MoveNodeA->Move.MoveType < MoveNodeB->Move.MoveType;

		// Single Motion or Double Motion
		if (MoveNodeA->Move.PatternType != MoveNodeB->Move.PatternType)
			return MoveNodeA->Move.PatternType < MoveNodeB->Move.PatternType;

		// 5, 46, 236, 623, etc
		if (MoveNodeA->Move.DirPattern != MoveNodeB->Move.DirPattern)
			return MoveNodeA->Move.DirPattern < MoveNodeB->Move.DirPattern;

		// Use a different sorting for moves without a DirPattern
		if (MoveNodeA->Move.DirPattern == EOOSDirPattern::OOSDP_None)
		{
			// Non-air before air
			if (MoveNodeA->Move.bAir != MoveNodeB->Move.bAir)
				return !MoveNodeA->Move.bAir && MoveNodeB->Move.bAir;

			// Light, Medium, etc
			if (MoveNodeA->Move.Attack != MoveNodeB->Move.Attack)
				return MoveNodeA->Move.Attack < MoveNodeB->Move.Attack;

			// 5, 6, 9, 8, 7, etc
			if (MoveNodeA->Move.Direction != MoveNodeB->Move.Direction)
				return MoveNodeA->Move.Direction < MoveNodeB->Move.Direction;
		}
		else
		{
			// 5, 6, 9, 8, 7, etc
			if (MoveNodeA->Move.Direction != MoveNodeB->Move.Direction)
				return MoveNodeA->Move.Direction < MoveNodeB->Move.Direction;

			// Non-air before air
			if (MoveNodeA->Move.bAir != MoveNodeB->Move.bAir)
				return !MoveNodeA->Move.bAir && MoveNodeB->Move.bAir;

			// Light, Medium, etc
			if (MoveNodeA->Move.Attack != MoveNodeB->Move.Attack)
				return MoveNodeA->Move.Attack < MoveNodeB->Move.Attack;

		}

		// Default
		return A > B;
	}

};
