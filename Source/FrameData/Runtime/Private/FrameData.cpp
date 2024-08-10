#include "FrameData.h"
#include "FDTrigger.h"
#include "FrameDataRuntimePCH.h"
#include "Animation/AnimSequenceBase.h"
#include "..\Public\FrameData.h"
#include "Engine.h"


#define LOCTEXT_NAMESPACE "FrameDataRuntime"

UFrameData::UFrameData()
{
	RefreshFrameData();
}

UFrameData::~UFrameData()
{

}

void UFrameData::RefreshFrameData()
{
	UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(Animation);

	if (AnimSequence)
	{
		// Save old framedata array and restore it later in case user wants to revert the animation asset.
		TArray<FFDFrame> OldFD = FrameData;

		int32 NumFrames = AnimSequence->GetNumberOfFrames();
		FrameData.Init(FFDFrame(), NumFrames);

		// Restore old array into fresh one (whole if smaller, clamp to new size if bigger).
		int Size = FMath::Min(OldFD.Num(), FrameData.Num());

		for (int i = 0; i < Size; ++i)
		{
			FrameData[i] = OldFD[i];
		}

		CurrentFrame = 0;

		AnimationLength = AnimSequence->GetPlayLength();
		HalfFrameLength = AnimationLength / (FrameData.Num() - 1) / 2.f;
	}
}

void UFrameData::StepForward(bool bLoop)
{
	if (CurrentFrame == (FrameData.Num() - 1))
	{
		CurrentFrame = bLoop ? 0 : CurrentFrame;
	}
	else
	{
		++CurrentFrame;
	}
}

void UFrameData::StepBack(bool bLoop)
{
	if (CurrentFrame == 0)
	{
		CurrentFrame = bLoop ? (FrameData.Num() - 1) : CurrentFrame;
	}
	else
	{
		--CurrentFrame;
	}
}

void UFrameData::GoToStart()
{
	CurrentFrame = 0;
}

void UFrameData::GoToEnd()
{
	CurrentFrame = FrameData.Num() - 1;
}

void UFrameData::GoToFrame(uint32 InFrame)
{
	CurrentFrame = FMath::Clamp(int(InFrame), 0, FrameData.Num() - 1);
}

void UFrameData::GoToFrame(float InPosition)
{
	CurrentFrame = uint8((InPosition / AnimationLength) * (FrameData.Num() - 1));
}

float UFrameData::GetCurrentFrame_Pos() const
{
	return (AnimationLength / (FrameData.Num() - 1)) * CurrentFrame; 
}

FFDFrame& UFrameData::GetFrameStruct(int32 InFrame)
{
	int32 TargetFrame = (InFrame < 0) ? CurrentFrame : InFrame;
	
	if (FrameData.IsValidIndex(TargetFrame))
	{
		return FrameData[TargetFrame];
	}
	else
	{
		static FFDFrame Null;
		return Null;
	}
}

void UFrameData::AddTrigger(UClass* InClass, int32 InFrame)
{
	int32 TargetFrame = (InFrame < 0) ? CurrentFrame : InFrame;

	if (FrameData.IsValidIndex(TargetFrame))
	{
		UFDTrigger* NewTrigger = NewObject<UFDTrigger>(this, InClass, NAME_None, RF_Transactional);
		if (NewTrigger)
		{
			FrameData[TargetFrame].Triggers.Add(NewTrigger);
		}
	}
}


#undef LOCTEXT_NAMESPACE
