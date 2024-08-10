// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OOSBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API UOOSBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	static bool PlayAnimAsMontage(USkeletalMeshComponent* ChildMesh, UAnimationAsset* AnimationAsset, bool bLoop, float DesiredDuration, float BlendInTime, float BlendOutTime);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	static class AOOSPawn* GetOOSPawnOwner(UActorComponent* InComp);
};
