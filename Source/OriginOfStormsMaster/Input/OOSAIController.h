// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OriginOfStormsMaster/OOSAnimNotify_Hitbox.h"
#include "OriginOfStormsMaster/Input/OOSPlayerController.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"
#include "OOSAIController.generated.h"

// Distance close enough to the opponent that anything should hit
#define JAB_RANGE 80.f

UENUM(BlueprintType)
enum class EOOSAIAction : uint8
{
	OOSAIA_Stand		UMETA(DisplayName = "Stand"),
	OOSAIA_Crouch		UMETA(DisplayName = "Crouch"),
	OOSAIA_Jump			UMETA(DisplayName = "Jump"),
	OOSAIA_SuperJump	UMETA(DisplayName = "SuperJump"),
	OOSAIA_Forward		UMETA(DisplayName = "Forward"),
	OOSAIA_Human		UMETA(DisplayName = "Human"),
	OOSAIA_CPU			UMETA(DisplayName = "CPU")
};

UENUM(BlueprintType)
enum class EOOSAIAutoBurst : uint8
{
	OOSAIAB_Off			UMETA(DisplayName = "Off"),
	OOSAIAB_Random		UMETA(DisplayName = "Random"),
	OOSAIAB_Fixed		UMETA(DisplayName = "Fixed"),
};

UENUM(BlueprintType)
enum class EOOSAIBlock : uint8
{
	OOSAIB_None			UMETA(DisplayName = "None"),
	OOSAIB_All			UMETA(DisplayName = "All"),
	OOSAIB_Crouch		UMETA(DisplayName = "Crouch"),
	OOSAIB_Standing		UMETA(DisplayName = "Standing")
};

UENUM(BlueprintType)
enum class EOOSAIFightState : uint8
{
	OOSAIFS_Neutral		UMETA(DisplayName = "Neutral"),
	OOSAIFS_Pressure	UMETA(DisplayName = "Pressure"),
	OOSAIFS_Defense		UMETA(DisplayName = "Defense"),
	OOSAIFS_Zoning		UMETA(DisplayName = "Zoning"),
	OOSAIFS_Combo		UMETA(DisplayName = "Combo"),
};

// Forward declarations
class UMNComboSet;
class UMNNode_Move;

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSAIController : public AAIController
{
	GENERATED_BODY()

public:
	AOOSAIController();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOOSAIAction Action = EOOSAIAction::OOSAIA_Stand;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOOSAIBlock Block = EOOSAIBlock::OOSAIB_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPushBlock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EOOSAIAutoBurst AutoBurst = EOOSAIAutoBurst::OOSAIAB_Off;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int FixedAutoBurst = 5;
private:
	int RandomAutoBurst;
	bool bRandomAutoBurstSet = false;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AILevel = 1.f;

	void SetPlayer(AOOSPlayerController* NewPlayerController);
	bool WantsPlayerController() const;

	EOOSMoveDir NotifyHit(EOOSHitHeight Height, bool bOH);
	
	UPROPERTY() AOOSPlayerController* PlayerController;

	UPROPERTY(Category = InputBufferDir, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UOOSFighterInputs* Inputs;

private:
	class AOOSPawn* ControlledPawn;
	bool bTransformedPawn = false;
	UWorld* World;

	FTimerHandle AutoJump;
	void DoAutoJump();
	void DoAutoSuperJump();

	EOOSAIFightState FightState = EOOSAIFightState::OOSAIFS_Neutral;
	float StateTime = 0;
	FOOSInput InputBuffer = FOOSInput();
	bool bWantsToDoSuper;
	float SmoothRndDir = 0.f;
	// Releasing the transform button right after pressing it breaks stuff... must wait till the next frame.
	bool bPressedTransform = false;
	UPROPERTY()
		UMNComboSet* ComboSet;
	UPROPERTY()
		TArray<UMNNode_Move*> SuperMoves;
	UPROPERTY()
		TArray<UMNNode_Move*> UltraMoves;

	void UpdateFightState();
	void ChangeFightState(EOOSAIFightState NewState);
	void TickAI(float DeltaTime);
	void UpdateInputBuffer(FOOSInput OldInputBuffer);

	bool RollAIOdds(float MinLevelOdds, float MaxLevelOdds);

	bool RollDistanceOdds(float Odds, float Distance, float MaxAddedDistance, float DistanceToOpponent, float ZeroDistanceOdds = 0.5f);
	bool RollAbsoluteDistanceOdds(float Odds, float Distance, float MaxAddedDistance);
	bool RollXDistanceOdds(float Odds, float Distance, float MaxAddedDistance);
	bool RollZDistanceOdds(float Odds, float Distance, float MaxAddedDistance);
	bool RollJabRange(float JabRangeOdds);
	bool RollTransformOdds(float MinLevelOdds, float MaxLevelOdds, float DesperationOddsScale);

	bool IsHealthLowerThan(float Threshold);

	// Force the AI to stay transformed for as long as this timer goes.
	float TransformTimer = 0.f;

	void PressTransform();

	int RollWeightedIndex(int Num, float Weight);

};
