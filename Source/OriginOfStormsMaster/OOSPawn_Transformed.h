// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OOSPawn.h"
#include "OOSPawn_Transformed.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ORIGINOFSTORMSMASTER_API AOOSPawn_Transformed : public AOOSPawn
{
	GENERATED_BODY()
	
public:

	virtual void Tick(float DeltaTime) override;

	virtual void TransformPressed() override;
	virtual void TransformReleased() override;

	UPROPERTY(Category = FighterPawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AOOSPawn* Normal;
	
	UPROPERTY(Category = Transformation, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float TransformDrainInterval = 0.2f;
	UPROPERTY(Category = Transformation, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) int TransformDrainAmount = 50;

	UPROPERTY(Category = Transforming, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) UParticleSystem* DetransformParticle;
	UPROPERTY(Category = Transforming, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) USoundBase* DetransformSound;

	// Detransform anim slot
	UPROPERTY(Category = TransformAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Detransform;

protected:

	virtual void StartTransformation() override;
	virtual void FinishTransformation() override;
	void QuickTransformation(bool bManual = false);

private:

	float TransformDrainTimer = 0.f;	
	
};
