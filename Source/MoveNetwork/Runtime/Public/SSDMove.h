#pragma once

#include "CoreMinimal.h"
#include "FrameData.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerInput.h"
#include "SSDMove.generated.h"

// Directions. Arranged so that a full anti-clockwise circle is 1 to 8 to make directional patterns recognition easier.
UENUM(BlueprintType)
enum class ESSDInputDir : uint8
{
	SSDID_None		UMETA(DisplayName = "None"),
	SSDID_Right		UMETA(DisplayName = "Right"),
	SSDID_UpRight	UMETA(DisplayName = "UpRight"),
	SSDID_Up		UMETA(DisplayName = "Up"),
	SSDID_UpLeft	UMETA(DisplayName = "UpLeft"),
	SSDID_Left		UMETA(DisplayName = "Left"),
	SSDID_DownLeft	UMETA(DisplayName = "DownLeft"),
	SSDID_Down		UMETA(DisplayName = "Down"),
	SSDID_DownRight	UMETA(DisplayName = "DownRight")
};

// Options to use with GetNeighborDir().
UENUM(BlueprintType)
enum class ESSDInputNeighbor : uint8
{
	SSDIN_Clockwise		UMETA(DisplayName = "Clockwise"),
	SSDIN_AntiClockwise UMETA(DisplayName = "AntiClockwise"),
	SSDIN_Opposite		UMETA(DisplayName = "Opposite"),
	SSDIN_Mirror		UMETA(DisplayName = "Mirror"),
};

UENUM(BlueprintType)
enum class ESSDPatternType : uint8
{
	SSDPT_None		UMETA(DisplayName = "None"),
	SSDPT_Single	UMETA(DisplayName = "Single"),
	SSDPT_Double	UMETA(DisplayName = "Double"),
};

UENUM(BlueprintType)
enum class ESSDDirPattern : uint8
{
	SSDDP_None			UMETA(DisplayName = "None"),
	SSDDP_QuarterCircle UMETA(DisplayName = "QuarterCircle"),
	SSDDP_DragonPunch	UMETA(DisplayName = "DragonPunch"),
	SSDDP_HalfCircle	UMETA(DisplayName = "HalfCircle"),
	SSDDP_FullCircle	UMETA(DisplayName = "FullCircle"),
	SSDDP_Charge		UMETA(DisplayName = "Charge"),
	SSDDP_RagingDemon	UMETA(DisplayName = "RagingDemon")
};

// Similar to SSDPlayerController.h ESSDInputDir, but dependant on character orientation.
UENUM(BlueprintType)
enum class ESSDMoveDir : uint8
{
	SSDMD_None			UMETA(DisplayName = "None"),
	SSDMD_Forward		UMETA(DisplayName = "Forward"),
	SSDMD_UpForward		UMETA(DisplayName = "UpForward"),
	SSDMD_Up			UMETA(DisplayName = "Up"),
	SSDMD_UpBack		UMETA(DisplayName = "UpBack"),
	SSDMD_Back			UMETA(DisplayName = "Back"),
	SSDMD_DownBack		UMETA(DisplayName = "DownBack"),
	SSDMD_Down			UMETA(DisplayName = "Down"),
	SSDMD_DownForward	UMETA(DisplayName = "DownForward")
};

UENUM(BlueprintType)
enum class ESSDMoveType : uint8
{
	SSDMT_Normal		UMETA(DisplayName = "Normal"),
	SSDMT_Special		UMETA(DisplayName = "Special"),
	SSDMT_EX			UMETA(DisplayName = "EX"),
	SSDMT_Super			UMETA(DisplayName = "Super"),
	SSDMT_Ultra			UMETA(DisplayName = "Ultra"),
	SSDMT_Flight		UMETA(DisplayName = "Flight"),
	SSDMT_Throw			UMETA(DisplayName = "Throw")
};

// Attacks. Arranged so they stack up like enum flags.
UENUM(BlueprintType)
enum class ESSDInputAttack : uint8
{
	SSDIA_None						UMETA(DisplayName = "None"),
	SSDIA_Light						UMETA(DisplayName = "Light"),
	SSDIA_Medium					UMETA(DisplayName = "Medium"),
	SSDIA_LightMedium				UMETA(DisplayName = "Light+Medium"),
	SSDIA_Heavy						UMETA(DisplayName = "Heavy"),
	SSDIA_LightHeavy				UMETA(DisplayName = "Light+Heavy"),
	SSDIA_MediumHeavy				UMETA(DisplayName = "Medium+Heavy"),
	SSDIA_LightMediumHeavy			UMETA(DisplayName = "Light+Medium+Heavy"),
	SSDIA_Special					UMETA(DisplayName = "Special"),
	SSDIA_LightSpecial				UMETA(DisplayName = "Light+Special"),
	SSDIA_MediumSpecial				UMETA(DisplayName = "Medium+Special"),
	SSDIA_LightMediumSpecial		UMETA(DisplayName = "Light+Medium+Special"),// XXX
	SSDIA_HeavySpecial				UMETA(DisplayName = "Heavy+Special"),
	SSDIA_LightHeavySpecial			UMETA(DisplayName = "Light+Heavy+Special"),// XXX
	SSDIA_MediumHeavySpecial		UMETA(DisplayName = "Medium+Heavy+Special"),// XXX
	SSDIA_LightMediumHeavySpecial	UMETA(DisplayName = "Light+Medium+Heavy+Special"),
	SSDIA_A							UMETA(DisplayName = "Any"),
	SSDIA_AA						UMETA(DisplayName = "Any+Any"),
	SSDIA_Transform					UMETA(DisplayName = "Transform"),
	SSDIA_EX						UMETA(DisplayName = "EX"),
	SSDIA_Super						UMETA(DisplayName = "Super"), 
	SSDIA_Ultra						UMETA(DisplayName = "Ultra")
};

// Controller bindings (to support DInput devices and per local controller bindings).
USTRUCT(BlueprintType)
struct FSSDPlayerBindings
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<ESSDInputDir, FKey> Directions;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<ESSDInputAttack, FKey> Attacks;

		static const FSSDPlayerBindings GamepadDefaults()
		{
			FSSDPlayerBindings PB;

			PB.Attacks.Empty();
			PB.Directions.Empty();

			PB.Directions.Emplace(ESSDInputDir::SSDID_Up, EKeys::Gamepad_DPad_Up);
			PB.Directions.Emplace(ESSDInputDir::SSDID_Down, EKeys::Gamepad_DPad_Down);
			PB.Directions.Emplace(ESSDInputDir::SSDID_Left, EKeys::Gamepad_DPad_Left);
			PB.Directions.Emplace(ESSDInputDir::SSDID_Right, EKeys::Gamepad_DPad_Right);

			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Light, EKeys::Gamepad_FaceButton_Left);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Medium, EKeys::Gamepad_FaceButton_Top);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Heavy, EKeys::Gamepad_FaceButton_Right);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Special, EKeys::Gamepad_FaceButton_Bottom);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_MediumHeavy, EKeys::Gamepad_RightTrigger);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Transform, EKeys::Gamepad_RightShoulder);

			return PB;
		}

		static const FSSDPlayerBindings KeyboardDefaults()
		{
			FSSDPlayerBindings PB;

			PB.Attacks.Empty();
			PB.Directions.Empty();

			PB.Directions.Emplace(ESSDInputDir::SSDID_Up, EKeys::Up);
			PB.Directions.Emplace(ESSDInputDir::SSDID_Down, EKeys::Down);
			PB.Directions.Emplace(ESSDInputDir::SSDID_Left, EKeys::Left);
			PB.Directions.Emplace(ESSDInputDir::SSDID_Right, EKeys::Right);

			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Light, EKeys::A);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Medium, EKeys::S);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Heavy, EKeys::D);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Special, EKeys::Z);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_MediumHeavy, EKeys::V);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Transform, EKeys::X);

			return PB;
		}

		static const FSSDPlayerBindings NullBindings()
		{
			FSSDPlayerBindings PB;

			PB.Attacks.Empty();
			PB.Directions.Empty();

			PB.Directions.Emplace(ESSDInputDir::SSDID_Up, EKeys::Invalid);
			PB.Directions.Emplace(ESSDInputDir::SSDID_Down, EKeys::Invalid);
			PB.Directions.Emplace(ESSDInputDir::SSDID_Left, EKeys::Invalid);
			PB.Directions.Emplace(ESSDInputDir::SSDID_Right, EKeys::Invalid);

			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Light, EKeys::Invalid);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Medium, EKeys::Invalid);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Heavy, EKeys::Invalid);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Special, EKeys::Invalid);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_MediumHeavy, EKeys::Invalid);
			PB.Attacks.Emplace(ESSDInputAttack::SSDIA_Transform, EKeys::Invalid);

			return PB;
		}
};

// Data structure for MoveList.
USTRUCT(BlueprintType)
struct FSSDMove
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

	// Leaving animation assets in FSSDMove to not break previous animation system, but should be removed once FrameData is implemented.
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
		ESSDMoveType MoveType = ESSDMoveType::SSDMT_Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int RequiredBars = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		ESSDPatternType PatternType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		ESSDDirPattern DirPattern;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		ESSDMoveDir Direction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
		ESSDInputAttack Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int> AllowedNextMoves;

};
