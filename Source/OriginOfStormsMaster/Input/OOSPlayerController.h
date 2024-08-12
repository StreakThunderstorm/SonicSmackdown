// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OOSMove.h"
#include "OriginOfStormsMaster/Input/OOSFighterInputs.h"
#include "OOSPlayerController.generated.h"

// Time the move is kept in the buffer after completion
#define MOVE_BUFFER_TIME 0.2f
// Time allowed to input motions
#define KEY_MOVE_TIME 15 / 60.f
// Delay between completing a move and executing it, to allow for multiple button presses
#define KEY_GROUP_TIME 2 / 60.f
// Time a direction must be held to start a charge move
#define CHARGE_TIMEOUT 0.4f

// Just an array that keeps itself circular. Used for input and doesn't get consumed like a queue since OOSHUD prints from it.
template<typename Type>
class TOOSCircularBuffer : public TArray<Type>
{	
public:

	uint8 Size;

	TOOSCircularBuffer()
	{
		Size = 0;
	}

	TOOSCircularBuffer(Type InitValue, uint8 Capacity)
	{
		TArray<Type>::Init(InitValue, Capacity);
		Size = Capacity;
	}

	// Adds a new entry to the buffer. Pushes the oldest out of the buffer.
	void Push(Type Value)
	{
		TArray<Type>::Add(Value);
		TArray<Type>::RemoveAt(0);
	}

	// Get newest entry.
	Type& Peek()
	{
		return TArray<Type>::operator[](Size - 1);
	}
	
};

// Data structure for input buffering.
USTRUCT(BlueprintType)
struct FOOSInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOOSInputDir Direction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOOSInputAttack Attack;
	double Time;

	FOOSInput()
	{
		Direction = EOOSInputDir::OOSID_None;
		Attack = EOOSInputAttack::OOSIA_None;
		Time = 0;
	};

	FOOSInput(EOOSInputDir Dir, EOOSInputAttack Att, double T)
	{
		Direction = Dir;
		Attack = Att;
		Time = T;
	};

	void AddAttackFlags(EOOSInputAttack Att)
	{
		Attack = (EOOSInputAttack)((uint8)Attack | (uint8)Att);
	}

	float GetHorizontalAxis() const
	{
		switch (Direction)
		{
		case EOOSInputDir::OOSID_Right:
		case EOOSInputDir::OOSID_UpRight:
		case EOOSInputDir::OOSID_DownRight:
			return 1.f;
		case EOOSInputDir::OOSID_UpLeft:
		case EOOSInputDir::OOSID_Left:
		case EOOSInputDir::OOSID_DownLeft:
			return -1.f;
		case EOOSInputDir::OOSID_Up:
		case EOOSInputDir::OOSID_Down:
		case EOOSInputDir::OOSID_None:
		default:
			return 0.f;
		}
	}
	float GetVerticalAxis() const
	{
		switch (Direction)
		{
		case EOOSInputDir::OOSID_Up:
		case EOOSInputDir::OOSID_UpRight:
		case EOOSInputDir::OOSID_UpLeft:
			return 1.f;
		case EOOSInputDir::OOSID_DownLeft:
		case EOOSInputDir::OOSID_Down:
		case EOOSInputDir::OOSID_DownRight:
			return -1.f;
		case EOOSInputDir::OOSID_Right:
		case EOOSInputDir::OOSID_Left:
		case EOOSInputDir::OOSID_None:
		default:
			return 0.f;
		}
	}
};

/**
 * 
 */
UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AOOSPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn *aPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupInputComponent() override;
	class AOOSPawn* PosessedPawn;

	// Buffers
	TOOSCircularBuffer<FOOSInput> InputBuffer = TOOSCircularBuffer(FOOSInput(), 32);

	UPROPERTY(Category = InputBufferDir, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UOOSFighterInputs* Inputs;

	// Getters for the circular buffer in BP, since we can't expose TOOSCircularBuffer to the editor.
	UFUNCTION(BlueprintCallable)
		FOOSInput GetInputBufferElement(int Element) const;
	UFUNCTION(BlueprintCallable)
		int GetInputBufferSize() const;
	
public:
	// Methods that receive input events.
	UFUNCTION() void DirPressed(EOOSInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey);
	void ProcessDirPress(EOOSInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey);
	UFUNCTION() void DirReleased(EOOSInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey);
	void ProcessDirRelease(EOOSInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey);
	UFUNCTION() void HorizontalAxisPressed(float Value);
	UFUNCTION() void VerticalAxisPressed(float Value);
	UFUNCTION() void AttackPressed(EOOSInputAttack InputAttack);
	UFUNCTION() void AttackReleased(EOOSInputAttack InputAttack);
	UFUNCTION() void TransformPressed();
	UFUNCTION() void TransformReleased();
	UFUNCTION() void KeyPressed(FKey Key);
	UFUNCTION() void KeyReleased(FKey Key);

public:
	void ResetKeyGroupTimer();

private:
	// Custom attack binding holders. Probably filled from a config file later.
	EOOSInputAttack BindFaceBottom = EOOSInputAttack::OOSIA_Special;
	EOOSInputAttack BindFaceRight = EOOSInputAttack::OOSIA_Heavy;
	EOOSInputAttack BindFaceTop = EOOSInputAttack::OOSIA_Medium;
	EOOSInputAttack BindFaceLeft = EOOSInputAttack::OOSIA_Light;
	EOOSInputAttack BindShoulderLeft = EOOSInputAttack::OOSIA_None;
	EOOSInputAttack BindShoulderRight = EOOSInputAttack::OOSIA_None;
	EOOSInputAttack BindTriggerLeft = EOOSInputAttack::OOSIA_None;
	EOOSInputAttack BindTriggerRight = EOOSInputAttack::OOSIA_MediumHeavy;
	EOOSInputAttack BindTSButtonLeft = EOOSInputAttack::OOSIA_None;
	EOOSInputAttack BindTSButtonRight = EOOSInputAttack::OOSIA_None;

	/*Player controller bindings. We need to check both sides of the stl map: values (hardware keys) for actual input events and keys (actions) for neighbor direction pressed checks
	for diagonals, so since reverse find on a regular map is slower and we don't have like, boos bimaps or something, we build a reversed version of each map. They're fixed length
	and sorting so we're cool.*/
	TMap<EOOSInputDir, FKey> DirKey;
	TMap<FKey, EOOSInputDir> KeyDir;
	TMap<EOOSInputAttack, FKey> AttKey;
	TMap<FKey, EOOSInputAttack> KeyAtt;

public:
	UFUNCTION(BlueprintCallable)
	void BuildBindingsFromSettings();

private:
	// Wrapper for standard InputAction binding function to allow sending arguments to the methods bound to input (check SetupInputComponent()).
	template <typename... T>
	void BindAction_WithParam(FName ActionName, EInputEvent InputEvent, FName MethodName, T... Parameter)
	{
		FInputActionBinding ActionBinding(ActionName, InputEvent);
		FInputActionHandlerSignature ActionHandler;
		ActionHandler.BindUFunction(this, MethodName, Parameter...);
		ActionBinding.ActionDelegate = ActionHandler;
		ActionBinding.bConsumeInput = false;
		InputComponent->AddActionBinding(ActionBinding);
	};

	// Utility for getting neighboring directions.
	EOOSInputDir GetNeighborDir(EOOSInputDir Dir, EOOSInputNeighbor Neighbor);
	EOOSInputDir GetCardinalNeighborDir(EOOSInputDir Dir, EOOSInputNeighbor Neighbor);

	float GetHorizontalAxisValueFromDirections();
	float GetVerticalAxisValueFromDirections();

	// Keeping track of the last thumbstick state to generate digital dir events.
	float LastStick_H = 0.f;
	float LastStick_V = 0.f;

	void DispatchDigitalStickEvents_H(float Value);
	void DispatchDigitalStickEvents_V(float Value);

	// Handles attack grouping timer.
	FTimerHandle KeyGroupTimer;
	void OnAttackGroupTimeout();

	// State machine for directional patterns.
	int	D = 0;		// Dash
	FOOSInputMove CurrentMove = FOOSInputMove();
	void CheckDirPatterns(EOOSInputDir Old, EOOSInputDir New);
public:
	void ResetDirPatterns();	

};
