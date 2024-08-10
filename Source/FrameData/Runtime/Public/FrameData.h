#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FrameData.generated.h"

class UFDTrigger;

USTRUCT(BlueprintType)
struct FFDFrame
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BLueprintReadWrite)
	TArray<UFDTrigger*> Triggers;
};

UCLASS(BlueprintType)
class FRAMEDATARUNTIME_API UFrameData : public UObject
{
	GENERATED_BODY()

public:
	UFrameData();
	virtual ~UFrameData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimationAsset* Animation;

	UPROPERTY()
		TArray<FFDFrame> FrameData;

	void RefreshFrameData();

	void StepForward(bool bLoop);
	void StepBack(bool bLoop);
	void GoToStart();
	void GoToEnd();
	void GoToFrame(uint32 InFrame);
	void GoToFrame(float InPosition); // Version that takes frames mapped from 0.0 to SequenceDuration.

	uint32 GetCurrentFrame() { return CurrentFrame; }
	float GetCurrentFrame_Pos() const; 
	FFDFrame& GetFrameStruct(int32 InFrame = -1);

	float GetAnimationLength() const { return AnimationLength; }
	float GetHalfFrameLength() const { return HalfFrameLength; }

	void AddTrigger(UClass* InClass, int32 InFrame = -1);

private:

	uint32 CurrentFrame = 0;
	float AnimationLength;
	float HalfFrameLength;
};
