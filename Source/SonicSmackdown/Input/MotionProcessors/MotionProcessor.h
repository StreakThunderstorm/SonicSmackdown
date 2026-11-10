// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Input/SSDPlayerController.h"
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
	class SONICSMACKDOWN_API UMotionProcessor : public UObject
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime);

	virtual void CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New) PURE_VIRTUAL(UMotionProcessor::CheckDirPatterns, ;);

	void AttackPressed(ESSDInputAttack InputAttack);
	void AttackReleased(ESSDInputAttack InputAttack);
	virtual void AttackButton(ESSDInputAttack InputAttack);

	FSSDInputMove* GetMoveAndReset();

	void RemoveExpired();

protected:
	// Defined so that the UObject system works
	UMotionProcessor();

	const int STORED_COMMANDS = 5;

	virtual void SetDirection(ESSDInputDir Direction);
	void SetHorizontalDirection(ESSDInputDir Direction);
	
	ESSDInputDir GetNeighborDir(ESSDInputDir Dir, ESSDInputNeighbor Neighbor);

	void AddNewMotion();

	virtual int CompletedMotionIndex() const PURE_VIRTUAL(UMotionProcessor::CompletedMotionIndex, return 1;);
	bool IsMotionCompleted() const;
	virtual bool IsMotionCompleted(FMotionProgress Motion) const;

	void RemoveDuplicateMotions();

	bool IsNegativeEdgeAllowed() const;

	void SetCurrentMove(ESSDInputAttack InputAttack);
	void AddMoveButtons(ESSDInputAttack InputAttack);
	virtual ESSDPatternType GetPatternType() const PURE_VIRTUAL(UMotionProcessor::GetPatternType, return ESSDPatternType::SSDPT_None;);
	virtual ESSDDirPattern GetPattern() const PURE_VIRTUAL(UMotionProcessor::GetPattern, return ESSDDirPattern::SSDDP_None;);

	virtual float MotionTimeout() const;

	ESSDInputDir MotionDirection = ESSDInputDir::SSDID_None;
	UPROPERTY()
		TArray<FMotionProgress> InputProgress;

	bool NegativeEdgeAllowed = true;

private:
	FSSDInputMove CurrentMove;

};