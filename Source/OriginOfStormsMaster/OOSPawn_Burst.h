// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OOSPawn.h"
#include "OOSPawn_Burst.generated.h"

// Forward declarations
class AOOSSonicRing;

/**
 * 
 */
UCLASS(Abstract)
class ORIGINOFSTORMSMASTER_API AOOSPawn_Burst : public AOOSPawn
{
	GENERATED_BODY()

public:
	virtual bool IsTransformReady() const override;
	virtual void TransformPressed() override;
	virtual void TransformReleased() override;

	virtual void Kill() override;
	virtual bool IgnoreDeathAnims() const override;

	UFUNCTION(BlueprintCallable, Category = RingLoss)
		void ScatterRings(int Amt);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RingLoss)
		TSubclassOf<AOOSSonicRing> RingClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RingLoss)
		USoundBase* RingLossSound;

protected:
	virtual void StartTransformation() override;
	virtual void FinishTransformation() override;
	
};
