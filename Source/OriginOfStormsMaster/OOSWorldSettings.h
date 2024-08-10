// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "OOSWorldSettings.generated.h"

USTRUCT(BlueprintType)
struct FOOSCameraWorldSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float FoV;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MinDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ZoomPitchBias;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ZoomLiftBias;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MinHeight;
	// Distance from the left/right edges of the screen beyond which the left/right bounds of the players will scroll/zoom the camera.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float HScrollPadding;
	// Distance from the top edge of the screen above which the top bound of the focused player will scroll up/down the camera.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float VScrollPadding;
	// The distance from the 2.5D plane the shake arm orbiting center is. The further, the more the shake contribution on the characters relative to the foreground.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ShakeOrbitOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MaxHitShake;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float HitShakeSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float HitShakeTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MaxBounceShake;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float BounceShakeSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float BounceShakeTime;
};

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	AOOSWorldSettings();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOOSCameraWorldSettings CameraSettings;

};
