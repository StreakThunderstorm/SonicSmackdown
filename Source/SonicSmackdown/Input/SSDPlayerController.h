// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "SSDMove.h"
#include "Input/SSDFighterInputs.h"
#include "SSDPlayerController.generated.h"

// Time the move is kept in the buffer after completion
#define MOVE_BUFFER_TIME 0.2f
// Time allowed to input motions
#define KEY_MOVE_TIME 15 / 60.f
// Delay between completing a move and executing it, to allow for multiple button presses
#define KEY_GROUP_TIME 2 / 60.f
// Time a direction must be held to start a charge move
#define CHARGE_TIMEOUT 0.4f

// Just an array that keeps itself circular. Used for input and doesn't get consumed like a queue since SSDHUD prints from it.
template<typename Type>
class TSSDCircularBuffer : public TArray<Type>
{	
public:

	uint8 Size;

	TSSDCircularBuffer()
	{
		Size = 0;
	}

	TSSDCircularBuffer(Type InitValue, uint8 Capacity)
	{
		this->Init(InitValue, Capacity);
		Size = Capacity;
	}

	// Adds a new entry to the buffer. Pushes the oldest out of the buffer.
	void Push(Type Value)
	{
		this->Add(Value);
		this->RemoveAt(0);
	}

	// Get newest entry.
	Type& Peek()
	{
		return (*this)[Size - 1];
	}
	
};

// Data structure for input buffering.
USTRUCT(BlueprintType)
struct FSSDInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESSDInputDir Direction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESSDInputAttack Attack;
	double Time;

	FSSDInput()
	{
		Direction = ESSDInputDir::SSDID_None;
		Attack = ESSDInputAttack::SSDIA_None;
		Time = 0;
	};

	FSSDInput(ESSDInputDir Dir, ESSDInputAttack Att, double T)
	{
		Direction = Dir;
		Attack = Att;
		Time = T;
	};

	void AddAttackFlags(ESSDInputAttack Att)
	{
		Attack = (ESSDInputAttack)((uint8)Attack | (uint8)Att);
	}

	float GetHorizontalAxis() const
	{
		switch (Direction)
		{
		case ESSDInputDir::SSDID_Right:
		case ESSDInputDir::SSDID_UpRight:
		case ESSDInputDir::SSDID_DownRight:
			return 1.f;
		case ESSDInputDir::SSDID_UpLeft:
		case ESSDInputDir::SSDID_Left:
		case ESSDInputDir::SSDID_DownLeft:
			return -1.f;
		case ESSDInputDir::SSDID_Up:
		case ESSDInputDir::SSDID_Down:
		case ESSDInputDir::SSDID_None:
		default:
			return 0.f;
		}
	}
	float GetVerticalAxis() const
	{
		switch (Direction)
		{
		case ESSDInputDir::SSDID_Up:
		case ESSDInputDir::SSDID_UpRight:
		case ESSDInputDir::SSDID_UpLeft:
			return 1.f;
		case ESSDInputDir::SSDID_DownLeft:
		case ESSDInputDir::SSDID_Down:
		case ESSDInputDir::SSDID_DownRight:
			return -1.f;
		case ESSDInputDir::SSDID_Right:
		case ESSDInputDir::SSDID_Left:
		case ESSDInputDir::SSDID_None:
		default:
			return 0.f;
		}
	}
};

/**
 * 
 */
UCLASS()
class SONICSMACKDOWN_API ASSDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASSDPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn *aPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupInputComponent() override;
	class ASSDPawn* PosessedPawn;

	// Buffers
	TSSDCircularBuffer<FSSDInput> InputBuffer = TSSDCircularBuffer<FSSDInput>(FSSDInput(), 32);

	UPROPERTY(Category = InputBufferDir, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USSDFighterInputs* Inputs;

	// Getters for the circular buffer in BP, since we can't expose TSSDCircularBuffer to the editor.
	UFUNCTION(BlueprintCallable)
		FSSDInput GetInputBufferElement(int Element) const;
	UFUNCTION(BlueprintCallable)
		int GetInputBufferSize() const;
	
public:
	// Methods that receive input events.
	UFUNCTION() void DirPressed(ESSDInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey);
	void HandleDirPressed(ESSDInputDir InputDir, bool bACWNeighborDown, bool bCWNeighborDown);
	void ProcessDirPress(ESSDInputDir InputDir, bool bACWNeighborDown, bool bCWNeighborDown);
	UFUNCTION() void DirReleased(ESSDInputDir InputDir, FKey ACWNeighborKey, FKey CWNeighborKey);
	void HandleDirReleased(ESSDInputDir InputDir, bool bACWNeighborDown, bool bCWNeighborDown);
	void ProcessDirRelease(ESSDInputDir InputDir, bool bACWNeighborDown, bool bCWNeighborDown);
	UFUNCTION() void HorizontalAxisPressed(float Value);
	UFUNCTION() void VerticalAxisPressed(float Value);
	UFUNCTION() void AttackPressed(ESSDInputAttack InputAttack);
	UFUNCTION() void AttackReleased(ESSDInputAttack InputAttack);
	UFUNCTION() void TransformPressed();
	UFUNCTION() void TransformReleased();
	UFUNCTION() void KeyPressed(FKey Key);
	UFUNCTION() void KeyReleased(FKey Key);

public:
	void ResetKeyGroupTimer();

private:
	// Custom attack binding holders. Probably filled from a config file later.
	ESSDInputAttack BindFaceBottom = ESSDInputAttack::SSDIA_Special;
	ESSDInputAttack BindFaceRight = ESSDInputAttack::SSDIA_Heavy;
	ESSDInputAttack BindFaceTop = ESSDInputAttack::SSDIA_Medium;
	ESSDInputAttack BindFaceLeft = ESSDInputAttack::SSDIA_Light;
	ESSDInputAttack BindShoulderLeft = ESSDInputAttack::SSDIA_None;
	ESSDInputAttack BindShoulderRight = ESSDInputAttack::SSDIA_None;
	ESSDInputAttack BindTriggerLeft = ESSDInputAttack::SSDIA_None;
	ESSDInputAttack BindTriggerRight = ESSDInputAttack::SSDIA_MediumHeavy;
	ESSDInputAttack BindTSButtonLeft = ESSDInputAttack::SSDIA_None;
	ESSDInputAttack BindTSButtonRight = ESSDInputAttack::SSDIA_None;

	/*Player controller bindings. We need to check both sides of the stl map: values (hardware keys) for actual input events and keys (actions) for neighbor direction pressed checks
	for diagonals, so since reverse find on a regular map is slower and we don't have like, boos bimaps or something, we build a reversed version of each map. They're fixed length
	and sorting so we're cool.*/
	TMap<ESSDInputDir, FKey> DirKey;
	TMap<FKey, ESSDInputDir> KeyDir;
	TMap<ESSDInputAttack, FKey> AttKey;
	TMap<FKey, ESSDInputAttack> KeyAtt;

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
	ESSDInputDir GetNeighborDir(ESSDInputDir Dir, ESSDInputNeighbor Neighbor);
	ESSDInputDir GetCardinalNeighborDir(ESSDInputDir Dir, ESSDInputNeighbor Neighbor);

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
	FSSDInputMove CurrentMove = FSSDInputMove();
	void CheckDirPatterns(ESSDInputDir Old, ESSDInputDir New);
public:
	void ResetDirPatterns();	

};
