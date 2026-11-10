// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SSDAnimNotify_Hitbox.h"
#include "Input/SSDPlayerController.h"
#include "Engine/LocalPlayer.h"
#include "SSDAIController.generated.h"

// Distance close enough to the opponent that anything should hit
#define JAB_RANGE 80.f

UENUM(BlueprintType)
enum class ESSDAIAction : uint8
{
	SSDAIA_Stand		UMETA(DisplayName = "Stand"),
	SSDAIA_Crouch		UMETA(DisplayName = "Crouch"),
	SSDAIA_Jump			UMETA(DisplayName = "Jump"),
	SSDAIA_SuperJump	UMETA(DisplayName = "SuperJump"),
	SSDAIA_Forward		UMETA(DisplayName = "Forward"),
	SSDAIA_Human		UMETA(DisplayName = "Human"),
	SSDAIA_CPU			UMETA(DisplayName = "CPU")
};

UENUM(BlueprintType)
enum class ESSDAIAutoBurst : uint8
{
	SSDAIAB_Off			UMETA(DisplayName = "Off"),
	SSDAIAB_Random		UMETA(DisplayName = "Random"),
	SSDAIAB_Fixed		UMETA(DisplayName = "Fixed"),
};

UENUM(BlueprintType)
enum class ESSDAIBlock : uint8
{
	SSDAIB_None			UMETA(DisplayName = "None"),
	SSDAIB_All			UMETA(DisplayName = "All"),
	SSDAIB_Crouch		UMETA(DisplayName = "Crouch"),
	SSDAIB_Standing		UMETA(DisplayName = "Standing")
};

UENUM(BlueprintType)
enum class ESSDAIFightState : uint8
{
	SSDAIFS_Neutral		UMETA(DisplayName = "Neutral"),
	SSDAIFS_Pressure	UMETA(DisplayName = "Pressure"),
	SSDAIFS_Defense		UMETA(DisplayName = "Defense"),
	SSDAIFS_Zoning		UMETA(DisplayName = "Zoning"),
	SSDAIFS_Combo		UMETA(DisplayName = "Combo"),
};

// Forward declarations
class UMNComboSet;
class UMNNode_Move;

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API ASSDAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASSDAIController();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESSDAIAction Action = ESSDAIAction::SSDAIA_Stand;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESSDAIBlock Block = ESSDAIBlock::SSDAIB_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPushBlock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ESSDAIAutoBurst AutoBurst = ESSDAIAutoBurst::SSDAIAB_Off;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int FixedAutoBurst = 5;
private:
	int RandomAutoBurst;
	bool bRandomAutoBurstSet = false;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AILevel = 1.f;

	void SetPlayer(ASSDPlayerController* NewPlayerController);
	bool WantsPlayerController() const;

	ESSDMoveDir NotifyHit(ESSDHitHeight Height, bool bOH);
	
	UPROPERTY() ASSDPlayerController* PlayerController;

	UPROPERTY(Category = InputBufferDir, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USSDFighterInputs* Inputs;

private:
	class ASSDPawn* ControlledPawn;
	bool bTransformedPawn = false;
	UWorld* World;

	FTimerHandle AutoJump;
	void DoAutoJump();
	void DoAutoSuperJump();

	ESSDAIFightState FightState = ESSDAIFightState::SSDAIFS_Neutral;
	float StateTime = 0;
	FSSDInput InputBuffer = FSSDInput();
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
	void ChangeFightState(ESSDAIFightState NewState);
	void TickAI(float DeltaTime);
	void UpdateInputBuffer(FSSDInput OldInputBuffer);

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
