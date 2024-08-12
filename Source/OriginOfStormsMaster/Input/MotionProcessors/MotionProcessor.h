// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OriginOfStormsMaster/Input/OOSPlayerController.h"
#include "MotionProcessor.generated.h"

/**
* Data structure for tracking progress of an input and when it was started.
*/
USTRUCT()
struct FMotionProgress
{
	GENERATED_BODY()

public:
	void Tick(float DeltaTime);

	int GetIndex() const
	{
		return Index;
	}

	void Increment();

	void SetBufferTime();

	void Reset();

	bool IsExpired(float BufferLength) const;

	bool IsBufferExpired() const;

private:
	int Index;
	float MotionTime;
	bool bBufferringButtons;
	float BufferTime;

};

/**
* Abstract base class for motion processors.
*/
UCLASS(Abstract)
	class ORIGINOFSTORMSMASTER_API UMotionProcessor : public UObject
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime);

	virtual void CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New) PURE_VIRTUAL(UMotionProcessor::CheckDirPatterns, ;);

	void AttackPressed(EOOSInputAttack InputAttack);
	void AttackReleased(EOOSInputAttack InputAttack);
	virtual void AttackButton(EOOSInputAttack InputAttack);

	FOOSInputMove* GetMoveAndReset();

	void RemoveExpired();

protected:
	// Defined so that the UObject system works
	UMotionProcessor();

	const int STORED_COMMANDS = 5;

	virtual void SetDirection(EOOSInputDir Direction);
	void SetHorizontalDirection(EOOSInputDir Direction);
	
	EOOSInputDir GetNeighborDir(EOOSInputDir Dir, EOOSInputNeighbor Neighbor);

	void AddNewMotion();

	virtual int CompletedMotionIndex() const PURE_VIRTUAL(UMotionProcessor::CompletedMotionIndex, return 1;);
	bool IsMotionCompleted() const;
	virtual bool IsMotionCompleted(FMotionProgress Motion) const;

	void RemoveDuplicateMotions();

	bool IsNegativeEdgeAllowed() const;

	void SetCurrentMove(EOOSInputAttack InputAttack);
	void AddMoveButtons(EOOSInputAttack InputAttack);
	virtual EOOSPatternType GetPatternType() const PURE_VIRTUAL(UMotionProcessor::GetPatternType, return EOOSPatternType::OOSPT_None;);
	virtual EOOSDirPattern GetPattern() const PURE_VIRTUAL(UMotionProcessor::GetPattern, return EOOSDirPattern::OOSDP_None;);

	virtual float MotionTimeout() const;

	EOOSInputDir MotionDirection = EOOSInputDir::OOSID_None;
	UPROPERTY()
		TArray<FMotionProgress> InputProgress;

	bool NegativeEdgeAllowed = true;

private:
	FOOSInputMove CurrentMove;

};