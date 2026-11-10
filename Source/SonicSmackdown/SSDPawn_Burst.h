// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SSDPawn.h"
#include "SSDPawn_Burst.generated.h"

// Forward declarations
class ASSDSonicRing;

/**
 * 
 */
UCLASS(Abstract)
class SONICSMACKDOWN_API ASSDPawn_Burst : public ASSDPawn
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
		TSubclassOf<ASSDSonicRing> RingClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RingLoss)
		USoundBase* RingLossSound;

protected:
	virtual void StartTransformation() override;
	virtual void FinishTransformation() override;
	
};
