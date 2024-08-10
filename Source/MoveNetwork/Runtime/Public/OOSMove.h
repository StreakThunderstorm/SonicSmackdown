#pragma once

#include "CoreMinimal.h"
#include "FrameData.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerInput.h"
#include "OOSMove.generated.h"

// Directions. Arranged so that a full anti-clockwise circle is 1 to 8 to make directional patterns recognition easier.
UENUM(BlueprintType)
enum class EOOSInputDir : uint8
{
	OOSID_None		UMETA(DisplayName = "None"),
	OOSID_Right		UMETA(DisplayName = "Right"),
	OOSID_UpRight	UMETA(DisplayName = "UpRight"),
	OOSID_Up		UMETA(DisplayName = "Up"),
	OOSID_UpLeft	UMETA(DisplayName = "UpLeft"),
	OOSID_Left		UMETA(DisplayName = "Left"),
	OOSID_DownLeft	UMETA(DisplayName = "DownLeft"),
	OOSID_Down		UMETA(DisplayName = "Down"),
	OOSID_DownRight	UMETA(DisplayName = "DownRight")
};

// Options to use with GetNeighborDir().
UENUM(BlueprintType)
enum class EOOSInputNeighbor : uint8
{
	OOSIN_Clockwise		UMETA(DisplayName = "Clockwise"),
	OOSIN_AntiClockwise UMETA(DisplayName = "AntiClockwise"),
	OOSIN_Opposite		UMETA(DisplayName = "Opposite"),
	OOSIN_Mirror		UMETA(DisplayName = "Mirror"),
};

UENUM(BlueprintType)
enum class EOOSPatternType : uint8
{
	OOSPT_None		UMETA(DisplayName = "None"),
	OOSPT_Single	UMETA(DisplayName = "Single"),
	OOSPT_Double	UMETA(DisplayName = "Double"),
};

UENUM(BlueprintType)
enum class EOOSDirPattern : uint8
{
	OOSDP_None			UMETA(DisplayName = "None"),
	OOSDP_QuarterCircle UMETA(DisplayName = "QuarterCircle"),
	OOSDP_DragonPunch	UMETA(DisplayName = "DragonPunch"),
	OOSDP_HalfCircle	UMETA(DisplayName = "HalfCircle"),
	OOSDP_FullCircle	UMETA(DisplayName = "FullCircle"),
	OOSDP_Charge		UMETA(DisplayName = "Charge"),
	OOSDP_RagingDemon	UMETA(DisplayName = "RagingDemon")
};

// Similar to OOSPlayerController.h EOOSInputDir, but dependant on character orientation.
UENUM(BlueprintType)
enum class EOOSMoveDir : uint8
{
	OOSMD_None			UMETA(DisplayName = "None"),
	OOSMD_Forward		UMETA(DisplayName = "Forward"),
	OOSMD_UpForward		UMETA(DisplayName = "UpForward"),
	OOSMD_Up			UMETA(DisplayName = "Up"),
	OOSMD_UpBack		UMETA(DisplayName = "UpBack"),
	OOSMD_Back			UMETA(DisplayName = "Back"),
	OOSMD_DownBack		UMETA(DisplayName = "DownBack"),
	OOSMD_Down			UMETA(DisplayName = "Down"),
	OOSMD_DownForward	UMETA(DisplayName = "DownForward")
};

UENUM(BlueprintType)
enum class EOOSMoveType : uint8
{
	OOSMT_Normal		UMETA(DisplayName = "Normal"),
	OOSMT_Special		UMETA(DisplayName = "Special"),
	OOSMT_EX			UMETA(DisplayName = "EX"),
	OOSMT_Super			UMETA(DisplayName = "Super"),
	OOSMT_Ultra			UMETA(DisplayName = "Ultra"),
	OOSMT_Flight		UMETA(DisplayName = "Flight"),
	OOSMT_Throw			UMETA(DisplayName = "Throw")
};

// Attacks. Arranged so they stack up like enum flags.
UENUM(BlueprintType)
enum class EOOSInputAttack : uint8
{
	OOSIA_None						UMETA(DisplayName = "None"),
	OOSIA_Light						UMETA(DisplayName = "Light"),
	OOSIA_Medium					UMETA(DisplayName = "Medium"),
	OOSIA_LightMedium				UMETA(DisplayName = "Light+Medium"),
	OOSIA_Heavy						UMETA(DisplayName = "Heavy"),
	OOSIA_LightHeavy				UMETA(DisplayName = "Light+Heavy"),
	OOSIA_MediumHeavy				UMETA(DisplayName = "Medium+Heavy"),
	OOSIA_LightMediumHeavy			UMETA(DisplayName = "Light+Medium+Heavy"),
	OOSIA_Special					UMETA(DisplayName = "Special"),
	OOSIA_LightSpecial				UMETA(DisplayName = "Light+Special"),
	OOSIA_MediumSpecial				UMETA(DisplayName = "Medium+Special"),
	OOSIA_LightMediumSpecial		UMETA(DisplayName = "Light+Medium+Special"),// XXX
	OOSIA_HeavySpecial				UMETA(DisplayName = "Heavy+Special"),
	OOSIA_LightHeavySpecial			UMETA(DisplayName = "Light+Heavy+Special"),// XXX
	OOSIA_MediumHeavySpecial		UMETA(DisplayName = "Medium+Heavy+Special"),// XXX
	OOSIA_LightMediumHeavySpecial	UMETA(DisplayName = "Light+Medium+Heavy+Special"),
	OOSIA_A							UMETA(DisplayName = "Any"),
	OOSIA_AA						UMETA(DisplayName = "Any+Any"),
	OOSIA_Transform					UMETA(DisplayName = "Transform"),
	OOSIA_EX						UMETA(DisplayName = "EX"),
	OOSIA_Super						UMETA(DisplayName = "Super"), 
	OOSIA_Ultra						UMETA(DisplayName = "Ultra")
};

// Controller bindings (to support DInput devices and per local controller bindings).
USTRUCT(BlueprintType)
struct FOOSPlayerBindings
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<EOOSInputDir, FKey> Directions;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<EOOSInputAttack, FKey> Attacks;

		static const FOOSPlayerBindings GamepadDefaults()
		{
			FOOSPlayerBindings PB;

			PB.Attacks.Empty();
			PB.Directions.Empty();

			PB.Directions.Emplace(EOOSInputDir::OOSID_Up, EKeys::Gamepad_DPad_Up);
			PB.Directions.Emplace(EOOSInputDir::OOSID_Down, EKeys::Gamepad_DPad_Down);
			PB.Directions.Emplace(EOOSInputDir::OOSID_Left, EKeys::Gamepad_DPad_Left);
			PB.Directions.Emplace(EOOSInputDir::OOSID_Right, EKeys::Gamepad_DPad_Right);

			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Light, EKeys::Gamepad_FaceButton_Left);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Medium, EKeys::Gamepad_FaceButton_Top);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Heavy, EKeys::Gamepad_FaceButton_Right);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Special, EKeys::Gamepad_FaceButton_Bottom);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_MediumHeavy, EKeys::Gamepad_RightTrigger);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Transform, EKeys::Gamepad_RightShoulder);

			return PB;
		}

		static const FOOSPlayerBindings KeyboardDefaults()
		{
			FOOSPlayerBindings PB;

			PB.Attacks.Empty();
			PB.Directions.Empty();

			PB.Directions.Emplace(EOOSInputDir::OOSID_Up, EKeys::Up);
			PB.Directions.Emplace(EOOSInputDir::OOSID_Down, EKeys::Down);
			PB.Directions.Emplace(EOOSInputDir::OOSID_Left, EKeys::Left);
			PB.Directions.Emplace(EOOSInputDir::OOSID_Right, EKeys::Right);

			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Light, EKeys::A);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Medium, EKeys::S);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Heavy, EKeys::D);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Special, EKeys::Z);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_MediumHeavy, EKeys::V);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Transform, EKeys::X);

			return PB;
		}

		static const FOOSPlayerBindings NullBindings()
		{
			FOOSPlayerBindings PB;

			PB.Attacks.Empty();
			PB.Directions.Empty();

			PB.Directions.Emplace(EOOSInputDir::OOSID_Up, EKeys::Invalid);
			PB.Directions.Emplace(EOOSInputDir::OOSID_Down, EKeys::Invalid);
			PB.Directions.Emplace(EOOSInputDir::OOSID_Left, EKeys::Invalid);
			PB.Directions.Emplace(EOOSInputDir::OOSID_Right, EKeys::Invalid);

			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Light, EKeys::Invalid);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Medium, EKeys::Invalid);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Heavy, EKeys::Invalid);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Special, EKeys::Invalid);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_MediumHeavy, EKeys::Invalid);
			PB.Attacks.Emplace(EOOSInputAttack::OOSIA_Transform, EKeys::Invalid);

			return PB;
		}
};

// Data structure for MoveList.
USTRUCT(BlueprintType)
struct FOOSMove
{
	GENERATED_BODY()

	// If true, no move will be executed. It just breaks the current combo and triggers a jump.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bJumpCancel = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bDashCancel = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bBlockMoveWithActorTag = false;

	//Any spawned actor that contains this tag will prevent the move from executing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bBlockMoveWithActorTag"))
		FName BlockActorTag = FName("None");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UFrameData* AttackFrameData;

	// Leaving animation assets in FOOSMove to not break previous animation system, but should be removed once FrameData is implemented.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimationAsset* Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimationAsset* LandingAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bAllowLandingBuffer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bDontBreakOnLanding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bCantStartSeries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bAir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EOOSMoveType MoveType = EOOSMoveType::OOSMT_Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int RequiredBars = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		EOOSPatternType PatternType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		EOOSDirPattern DirPattern;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		EOOSMoveDir Direction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		EOOSInputAttack Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int> AllowedNextMoves;

};
