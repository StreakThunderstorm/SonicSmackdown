// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Input/OOSPlayerController.h"
#include "MoveNetwork.h"
#include "OOSMove.h"
#include "MNNode_Move.h"
#include "MNNode_Reroute.h"
#include "OOSPawn.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FThrowCollisionDelegate, bool, FVector);

// HP and transform amount
#define MAX_SUPER_POINTS_PER_LV 1000000
#define	MAX_SUPER_LEVELS 5
#define MAX_TRANSFORM_POINTS 500000
#define DETRANSFORM_TIME_PENALTY 5.f
#define TRANSFORM_LOCKOUT_TIME 7.f
#define COMBO_FIRST_HIT_DAMAGE 1.f
#define MINIMUM_DAMAGE_SCALE 0.2f
#define MINIMUM_HITSTUN_SCALE 0.25f

#define MAX_AIR_EXS 1
#define MAX_AIR_SPECIALS 3

#define WALL_TECH_WINDOW 0.1f // 7-ish frames

UENUM(BlueprintType)
enum class EOOSPawnState : uint8
{
	OOSPS_Idle				UMETA(DisplayName = "Idle"),
	OOSPS_Jump				UMETA(DisplayName = "Jump"),
	OOSPS_SuperJump			UMETA(DisplayName = "SuperJump"),
	OOSPS_Stun				UMETA(DisplayName = "Stun"),
	OOSPS_Block				UMETA(DisplayName = "Block"),
	OOSPS_CrouchBlock		UMETA(DisplayName = "CrouchBlock"),
	OOSPS_Launch			UMETA(DisplayName = "Launch"),
	OOSPS_Crouch			UMETA(DisplayName = "Crouch"),
	OOSPS_Dash				UMETA(DisplayName = "Dash"),
	OOSPS_Move				UMETA(DisplayName = "Move"),
	OOSPS_Floored			UMETA(Displayname = "Floored"),
	OOSPS_SpecialMove		UMETA(DisplayName = "SpecialMove"),
	OOSPS_HyperMove			UMETA(DisplayName = "HyperMove"),
	OOSPS_ChargeTransform	UMETA(DisplayName = "ChargeTransform"),
	OOSPS_Transform			UMETA(DisplayName = "Transform"),
	OOSPS_Defeat			UMETA(DisplayName = "Defeat")
};	

UENUM(BlueprintType)
enum class EOOSCounter : uint8
{
	OOSCT_None				UMETA(DisplayName = "None"),
	OOSCT_Low				UMETA(DisplayName = "Low"),
	OOSCT_Mid				UMETA(DisplayName = "Mid"),
	OOSCT_High				UMETA(DisplayName = "High"),
	OOSCT_Super				UMETA(DisplayName = "Super"),
	OOSCT_Projectile		UMETA(DisplayName = "Projectile"),
	OOSCT_SuperProjectile	UMETA(DisplayName = "Super Projectile")
};

UENUM(BlueprintType)
enum class EOOSGrab : uint8
{
	OOSGR_Any				UMETA(DisplayName = "Any"),
	OOSGR_Ground			UMETA(DisplayName = "Ground"),
	OOSGR_Air				UMETA(DisplayName = "Air")	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPawnHitDelegate);

// Forward declarations
class AOOSCamera;
class AOOSGameMode;
class UOOSAnimNotify_Movement;
class UOOSMovementComponent;
class UOOSAnimSingleNodeInstance;
class UOOSSkeletalMeshComponent;
class URMAMirrorAnimationMirrorTable;

UCLASS(Abstract)
class ORIGINOFSTORMSMASTER_API AOOSPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AOOSPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	// Create holder for FighterMovementComponent and make it accesible from BP
	UPROPERTY(Category = FighterPawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UOOSMovementComponent* MovementComponent;

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		AOOSPlayerController* GetPlayerController() const;
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		UOOSFighterInputs* GetFighterInputs() const;
	UFUNCTION()
		bool SetControllerId(int Id);
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		bool WantsPlayerController() const;

private:
	// Checks the MoveBuffer for pushblocking
	void TryPushBlock(UOOSFighterInputs* Inputs);
	// Called from Tick() to act on player inputs
	void TickMoveNetwork(UOOSFighterInputs* Inputs);
	bool CheckMoveInput(FOOSInputMove& MoveInput, UOOSFighterInputs* Inputs);
	void SetReadiedMove(UMNNode_Move* NewNode);
	// Called from Tick() to update hit stop
	void TickHitStop(float DeltaTime);

	class UOOSGameInstance* GameInstance;

public:

	// Collision capsule
	UPROPERTY(Category = FighterPawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* Capsule;

	// Mesh
	UPROPERTY(Category = FighterPawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UOOSSkeletalMeshComponent* SkeletalMesh;

	// Secondary Mesh
	UPROPERTY(Category = FighterPawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UOOSSkeletalMeshComponent* SecondaryMesh;

	UPROPERTY(Category = Transforming, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ChargeSoundLoop;

	// Reference to Opponent
	UPROPERTY(Category = FighterPawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AOOSPawn* Opponent;
	// Reference to Transformed pawn.
	UPROPERTY(Category = FighterPawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		AOOSPawn* Transformed;

	// Events for the transformed button.
	UFUNCTION() virtual void TransformPressed();
	UFUNCTION() virtual void TransformReleased();

	// Transform charge interval of HP drain and amount per cycle.
	UPROPERTY(Category = Transformation, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float ChargeDrainInterval = 0.2f;
	UPROPERTY(Category = Transformation, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) int ChargeDrainAmount = 100;

	// Flag that is true during a combo or single move. Blocks movement input.
	UPROPERTY(Category = FighterPawn, EditAnywhere, BlueprintReadWrite) bool bIsPerformingMove = false;

	// Health gauge.
	UPROPERTY(Category = Health, EditAnywhere, BlueprintReadWrite) int MaxHealth = 1000000;
	UPROPERTY(Category = Health, EditAnywhere, BlueprintReadWrite) int Health;
	UPROPERTY(Category = Health, EditAnywhere, BlueprintReadWrite) int Regen;
	UPROPERTY(Category = Health, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) int HealthRegenAmount = 100;
	UPROPERTY(Category = Health, EditAnywhere, BlueprintReadWrite) int Rings = 50;

	// Super gauge.
	UPROPERTY(Category = Health, EditAnywhere, BlueprintReadWrite)
	int SuperPts = 0;
	UFUNCTION(BlueprintCallable, Category = "Super")
	int GetSuperLevel() const;
	UFUNCTION(BlueprintCallable, Category = "Super")
	int GetCurrentSuperPoints() const;
	UFUNCTION(BlueprintCallable, Category = "Super")
	void TrainingRefill();

	UFUNCTION(BlueprintCallable)
		void AddSuperPts(int Points);

	// CinematicScript reference.
	class AOOSCinematicScript* CinematicScript;

	// Transform gauge.
	UPROPERTY(Category = Transformation, EditAnywhere, BlueprintReadWrite) int Transform = 0;
	// Transform and Super gain factors, initialized at spawn in OOSGameMode.
	float TransformGain;
	float SuperGain;

	// Time locked out of transforming
	UPROPERTY(Category = Transformation, BlueprintReadOnly) float TransformLockout;

	UPROPERTY(Category = Transformation, EditAnywhere, BlueprintReadWrite) bool DefensiveTransform = false;
	UPROPERTY(Category = FighterPawn, EditAnywhere, BlueprintReadWrite) bool bCanAct = false;

	// Reference to the current MoveNetwork node being performed.
	UPROPERTY(Category = FighterPawn, EditAnywhere, BlueprintReadWrite) UMNNode_Move* CurrentNode;
	// Move child node to cancel into at the soonest possibility
	UMNNode_Move* ReadiedChildNode;
	// Move that the player has input and wants to perform (as soon as there is no hitstop)
	UPROPERTY()
		UMNNode_Move* ReadiedInputNode;	

	// Cancels that the player has input and wants to perform (as soon as there is no hitstop)
	EOOSInputDir ReadiedJumpCancel = EOOSInputDir::OOSID_None;
	EOOSInputDir ReadiedDashCancel = EOOSInputDir::OOSID_None;

	//Reference to Camera
	AOOSCamera* Camera;

	UFUNCTION(BlueprintCallable, Category = "Camera")
		void SetActorToFollow(AOOSPawn* ActorToFollow);

	UFUNCTION(BlueprintCallable, Category = "Stage")
		FVector GetStageOrigin() const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
		void SetFlightModeEnabled(bool bEnabled = true);

	UFUNCTION(BlueprintCallable, Category = "Camera")
		void UnFly();

private:
	// Lock flight mode until next landing.
	bool bCanFly = true;

	FVector BlendSpaceInput = FVector::ZeroVector;
	FVector SmoothBlendSpaceInput = FVector::ZeroVector;

public:
	// Event called when landing
	UFUNCTION(BlueprintNativeEvent, Category = "MovementComponent") void OnLanded();
	void OnWallTouch();
	// Event called when hitting a wall
	UFUNCTION(BlueprintNativeEvent, Category = "MovementComponent") void OnWallHit(bool bTech);
	UFUNCTION(BlueprintNativeEvent, Category = "Training") void OnTrainingRefill();

private:
	// Timestamp for filtering wall tech input
	double LastWallBounce;
	// Duration of the wall tech window.
	float WallTechWindow;
	// Getting this into a function because of potential untechable forced wall bounces.
	void BounceOffWall();

public:
	// Play animations directly from AnimSequence assets with looping and blend time options
	UFUNCTION(BlueprintCallable, Category = "Animation")
		bool PlayAnim(UAnimationAsset* AnimationAsset, bool bLoop = false, float DesiredDuration = -1.f, float BlendInTime = 0.f, float BlendOutTime = 0.f, bool bIgnoreMovNotifs = false, bool bDisableBlending = false);
	bool PlayAnim_NoBlend(UAnimationAsset* AnimationAsset, bool bLoop = false, float DesiredDuration = -1.f, float BlendInTime = 0.f, float BlendOutTime = 0.f, bool bIgnoreMovNotifs = false);

private:
	bool bIgnoreMovementNotifies = false;

public:
	// Event called whenever any animation called with PlayAnim() ends
	UFUNCTION(BlueprintNativeEvent, Category = "Animation")
		void OnAnimationEnd(UAnimationAsset* AnimSequence);

	UFUNCTION(BlueprintNativeEvent, Category = "AnimNotification")
		void OnOOSAnimNotify(UAnimationAsset* AnimSequence, FName NotifName);

	UFUNCTION(BlueprintNativeEvent, Category = "Animation")
		void OnTurnaround();

	UPROPERTY(Category = Defeat, EditAnywhere, BlueprintReadWrite) bool bIsLyingDown = false;
	UPROPERTY(Category = Defeat, EditAnywhere, BlueprintReadWrite) bool bIsPerformingSuper = false;
	UPROPERTY(Category = Defeat, EditAnywhere, BlueprintReadWrite) bool bBlockSuper = false;

	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite) bool bDebugHitboxes = false;
	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite) bool bDebugHurtboxes = false;
	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite) FColor HitboxDebugColor = FColor::Red;
	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite) FColor HurtboxDebugColor = FColor::Green;
	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite) float HBDebugThickness = 1.f;

	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bMirrored = false;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool FlyingCharacter = false;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float GroundMovementSpeed = 5.f;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float AirMovementSpeed = 2.5f;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float FlightMovementSpeed = 5.f;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float FlightAccel = 10.f;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) FVector2D JumpSpeed = FVector2D(3.f, 4.f);
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) FVector2D DoubleJumpSpeed = FVector2D(2.f, 3.f);
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) int MaxAirJumps = 0;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) FVector2D SuperJumpSpeed = FVector2D(5.f, 10.f);
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float SuperJumpXAccelFW = 5.f;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float SuperJumpXAccelBW = 2.f;
	// Minimum height for double jumping and air launches.
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float HeightThresholdUp = 200.f;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) float HeightThresholdDown = 200.f;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) EOOSPawnState PawnState = EOOSPawnState::OOSPS_Idle;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) EOOSCounter Counter = EOOSCounter::OOSCT_None;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) EOOSGrab GrabType = EOOSGrab::OOSGR_Any;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bDefeated = false;
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool bDefeatWaitForIdle = false; // This one is raised when defeated but we must wait for next idle (time over defeat).
	UPROPERTY(Category = Movement, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) bool WalkOnly = true;

	//Blocking effects
	UPROPERTY(Category = Blocking, EditAnywhere, BlueprintReadWrite) UParticleSystem* BlockParticle;
	UPROPERTY(Category = Blocking, EditAnywhere, BlueprintReadWrite) USoundBase* BlockSound;
	UPROPERTY(Category = Blocking, EditAnywhere, BlueprintReadWrite) float PushBlockAmt = 10.f;
	UPROPERTY(Category = Transforming, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) UParticleSystem* TransformAbsorbParticle;
	UPROPERTY(Category = Transforming, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) USoundBase* TransformAbsorbSound;
	
	// List of available moves.
	UPROPERTY(Category = FighterPawn, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FOOSMove> MoveList;

	// Reference to Move Network asset.
	UPROPERTY(Category = FighterPawn, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UMoveNetwork* MoveNetwork;

	UPROPERTY(Category = FighterPawn, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	URMAMirrorAnimationMirrorTable* MirrorTable = nullptr;

	void SetFacing(bool FacingRight);

	void RefreshMirror();

	void BecomeInvincible();
	void EndInvincible();

	// Add a HitBox to attach to a bone socket
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		class UOOSHurtbox* AddHurtbox(FName BoneSocket, float CapsuleRadius, float CapsuleHalfHeight);

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		class UOOSHitbox* AddHitbox (
			USkeletalMeshComponent* InMesh, 
			TSubclassOf<UOOSHitbox> HitboxType,
			FVector Offset, 
			FRotator Rotation, 
			float HalfHeight, 
			float Radius, 
			bool bGrab, 
			bool bForceGrab,
			FName GrabSock,
			UParticleSystem* HitParticle, 
			USoundBase* HitSound, 
			bool bBurst,
			int Damage, 
			float HitStun, 
			float SelfHitLag, 
			float OpponentHitLag, 
			float BlockStun, 
			float PushFactor, 
			float Frz, 
			bool FrzTw, 
			bool bOTG, 
			bool bOH, 
			bool bAutoChild, 
			bool bDontCancel,
			bool bUnblockable,
			bool bAntiArmor,
			bool bAntiProjectile,
			EOOSHitHeight HitHeight, 
			EOOSInputAttack Attack, 
			EOOSLaunchType Launch,
			bool bForceComboExtension,
			bool bResetComboExtensions,
			EOOSDirectionMode Direction, 
			FVector2D LaunchSpd, 
			bool bForceTryPostLaunchJump, 
			FName SocketName
		);

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		class AOOSProjectile* AddProjectile (
			USkeletalMeshComponent* InMesh, 
			TSubclassOf<AOOSProjectile> Class, 
			int NumberOfHits, 
			float HitTime, 
			float Lifetime, 
			FVector2D Speed, 
			FVector Offset, 
			FRotator Rotation, 
			bool bGrab, 
			bool bForceGrab,
			FName GrabSock,
			UParticleSystem* HitParticle, 
			USoundBase* HitSound, 
			UParticleSystem* DeathParticle, 
			USoundBase* DeathSound, 
			int Damage, 
			float HitStun, 
			float SelfHitLag, 
			float OpponentHitLag, 
			float BlockStun, 
			float PushFactor, 
			float Frz, 
			bool FrzTw, 
			bool bOTG, 
			bool bOH, 
			bool bAutoChild, 
			bool bDontCancel,
			bool bUnblockable,
			bool bAntiArmor,
			bool bAntiProjectile,
			EOOSHitHeight HitHeight, 
			EOOSInputAttack Attack, 
			EOOSLaunchType Launch,
			bool bForceComboExtension,
			bool bResetComboExtensions,
			EOOSDirectionMode Direction, 
			FVector2D LaunchSpd, 
			bool bForceTryPostLaunchJump
		);

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		void HitboxVisibility(bool Visible);

	// Spawned Hitboxes set this to true when they hit an opponent if cancels allowed.
	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite)
		bool bHasLandedAttack = false;

	// Spawned Hitboxes set this to true when they hit an opponent. 
	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite)
		bool bHasMadeContact = false;

	// Bool driven by OOSAnimNotify_SpecialCancel
	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite)
		bool bSpecialCancel = false;
	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite)
		bool bSpecialCancelRequiresAttackLanded = false;

	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite)
		int SuperArmor = 0;
	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite)
		int HyperArmor = 0;

	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite)
		bool bBlockBurst = false;
	
	//Tech Window Bool
	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite)
		bool bTechWindow = false;
		double TechWindowTime = false;

	UPROPERTY(Category = Rotate, EditAnywhere, BlueprintReadWrite)
		bool bGrabbed = false;

	UPROPERTY(Category = Landing, EditAnywhere, BlueprintReadWrite)
		bool bLandFrozen = false;

	UPROPERTY(Category = Rotate, EditAnywhere, BlueprintReadWrite)
		bool bGrabRotate = false;
	UPROPERTY(Category = Rotate, EditAnywhere, BlueprintReadWrite)
		FRotator GrabRot = FRotator::ZeroRotator;
	UPROPERTY(Category = Rotate, EditAnywhere, BlueprintReadWrite)
		FRotator RotOffset = FRotator::ZeroRotator;
	UPROPERTY(Category = Rotate, EditAnywhere, BlueprintReadWrite)
		float InterpSpd = 0.f;
	UPROPERTY(Category = Rotate, EditAnywhere, BlueprintReadWrite)
		bool SocketRot = false;
	UPROPERTY(Category = Rotate, EditAnywhere, BlueprintReadWrite)
	 FName GrabSocket = FName("None");

	UPROPERTY(Category = Tech, EditAnywhere, BlueprintReadWrite) UParticleSystem* TechParticle;
	UPROPERTY(Category = Tech, EditAnywhere, BlueprintReadWrite) USoundBase* TechSound;

	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite) bool bMeterBlocked = false;

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		void MoveHorizontal(float AxisValue);
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		void MoveVertical(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		bool Jump(bool bForce = false, bool bSkipMoveCheck = false);

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		bool SuperJump();

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		bool Crouch();

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		void Uncrouch();

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		void Dash(EOOSInputDir Direction);
	
	UPROPERTY(Category = FighterPawn, EditAnywhere, BlueprintReadWrite)
		bool JumpTrackX = false;
	UPROPERTY(Category = FighterPawn, EditAnywhere, BlueprintReadWrite)
		bool JumpTrackY = false;

private:
	void AirDashTwoWay(EOOSInputDir Direction);
	void AirDashEightWay(EOOSInputDir Direction);

public:
	void TryPostLaunchJump(bool bForce);

	UFUNCTION(BlueprintCallable)
		virtual void Kill();

	virtual bool IgnoreDeathAnims() const;

	bool GetHit
	(
		enum EOOSHitHeight HitHeight,
		EOOSInputAttack AttackType,
		bool bUnblockable,
		bool bOH,
		EOOSLaunchType LaunchType,
		FVector2D LaunchSpeed,
		bool bForceExtension,
		bool bResetExtensions,
		bool bAntiArmor,
		int Damage,
		int Chip,
		float HitStun,
		float BlockStun,
		bool bIsProjectile,
		bool& bCounter,
		bool bIsSuper
	);

	void CamShake(EOOSInputAttack AttackType, float Scale = 1.f);

public:
	UPROPERTY(Category = Rendering, EditAnywhere, BlueprintReadWrite)
	float DepthOffset = 10.f;
	UPROPERTY(Category = Rendering, EditAnywhere, BlueprintReadWrite)
	bool bIsInFrontDepthLayer = false;

	void BringToFrontDepthLayer();
	void SendToBackDepthLayer();

public:
	UFUNCTION(BlueprintCallable)
	void Freeze(float Duration, bool bTwitch = true, bool bInvincible = false);
	UFUNCTION(BlueprintCallable)
	void Unfreeze();
	void SetHitStop(float Value);
	void TryHitStopOver();
	bool IsHitStopOver();

	FThrowCollisionDelegate OnThrowCollision;

	// Hit counter and scaling
	int HitCount = 0;
	float DamageScale = 1.f;
	UPROPERTY(Category = Input, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float HitStunScale = 1.f;
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	int GetHitCount() const
	{
		return HitCount;
	}
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		int GetHitStunRemaining() const;
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		int AnimationFramesRemaining() const;
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
		int AnimationFramesElapsed() const;

	void ExternalMove_Begin(FVector2D Speed, bool bAir, bool bAllowGravity, bool bKeepMovement, bool bAddMovement, bool bUseXDecelerationAtStart, bool bUseYDecelerationAtStart, bool bDisablePush);
	void ExternalMove_End(bool bUseXDecelerationAtEnd, bool bUseYDecelerationAtEnd, bool bStopAtEnd);

	UFUNCTION(BlueprintCallable)
	void SetPawnState(EOOSPawnState NewState, bool bSkipAnims = false);

	void ResetKnockdowns();

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	bool IsPerformingMove() const;
	bool IsPerformingUltra() const;
	bool IsJumping();
	bool IsSuperJumping();
	bool IsDashing() const;
	bool IsCrouching();
	bool IsCompletelyCrouched();
	bool IsDoubleJumping();
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	bool IsFloored() const;
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	bool IsStunned() const;
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	bool IsLaunched() const;
	bool IsIdle() const;
	bool IsNeutral() const;
	bool IsBlockInputAllowed() const;
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	bool IsInputAllowed() const;
	bool IsInvincible();
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	bool IsFrozen() const;
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	bool IsAttackActive() const;
	bool IsChargingTransform();
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	bool IsTransforming() const;
	bool IsTransformCoolingDown() const;
	void LockoutTransform();
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	float LockoutTime() const;
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	bool IsTransformLockedOut() const;
	bool IsFacingOpponent();
	bool IsBlocking() const;
	bool IsCrouchBlocking() const;
	bool IsPushBlocking();
	bool IsRecoveringOnGround() const;
	bool IsRecoveringInAir() const;
	bool IsLanding() const;
	bool IsKO() const;
	bool CanOnlyWalk() const;
	bool IsTurning();
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	virtual bool IsTransformReady() const;
	UFUNCTION(BlueprintCallable) bool WasKilled() const;
	bool IsCloseToOpponent(float Threshold) const;
	bool CanPerformThrow() const;

	UPROPERTY(BlueprintAssignable)
	FPawnHitDelegate OnPawnHit;

	UPROPERTY(BlueprintAssignable)
	FPawnHitDelegate OnPawnBlock;

	// Reference to the current OOSAnimNotify_Movement being played. Gets cleared when a new anim is played, it's a hacky workaround AnimNotify_State being triggered even though the anim was interrupted.
	UOOSAnimNotify_Movement* MovementNotify = nullptr;

protected:
	bool IsPawnHigherThan(float HeightUp, float HeightDown);

public:
	void ResetTimeDilation();

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	bool IsPausingMatchTimer() const;

	void Stopped();

	void Launch(FVector2D LaunchSpeed, EOOSLaunchType LaunchType);

	UPROPERTY(Category = Input, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true")) float ChargeTime = 1.f;

	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite)
		int PlayerIndex = 0;

	// Common movement animations.	
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Idle;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* IdleToCrouch;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* CrouchToIdle;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* CrouchLoop;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* WalkFW;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* WalkBW;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* SingleJump;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* SingleJumpF;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* SingleJumpB;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* DoubleJump;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* DoubleJumpF;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* DoubleJumpB;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* SupJump;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* SupJumpFall;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Fall;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* FallF;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* FallB;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* DoubleJumpFall;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* DoubleJumpFallF;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* DoubleJumpFallB;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Land;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LandTurn;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* DashFW;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* DashBW;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* AirDashFW;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* AirDashBW;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* AirDashUp;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* AirDashDown;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* AirDashUpFW;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* AirDashDownFW;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* AirDashUpBW;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* AirDashDownBW;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* FlightBlendSpace;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* CrouchTurn;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* StandTurn;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Victory;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* TimeOverDefeat;
	UPROPERTY(Category = MovementAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* InstantDeath;

public:
	// Transform animations.
	UPROPERTY(Category = TransformAnims, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) UAnimationAsset* Charge_Air;
	UPROPERTY(Category = TransformAnims, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) UAnimationAsset* Charge_Ground;
	UPROPERTY(Category = TransformAnims, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) UAnimationAsset* Transformation;
	UPROPERTY(Category = TransformAnims, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) UAnimationAsset* Transformation_Cinematic;
	UPROPERTY(Category = TransformAnims, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) UAnimationAsset* TransitionSequence = nullptr;

public:
	// Common getting hit animations.
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_LowLight;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_LowMedium;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_LowHeavy;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_MidLight;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_MidMedium;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_MidHeavy;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_HighLight;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_HighMedium;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_HighHeavy;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_Block;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* St_PushBlock;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Cr_HighLight;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Cr_HighMedium;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Cr_HighHeavy;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Cr_LowLight;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Cr_LowMedium;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Cr_LowHeavy;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Cr_Block;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Cr_PushBlock;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Crumble;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* FloorBounceUpShort;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* FloorBounceUpMedium;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* FloorBounceUpFar;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* FloorBounceDownShort;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* FloorBounceDownMedium;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* FloorBounceDownFar;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LyingDown;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LyingDownStand;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LyingDownRollF;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LyingDownRollB;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LyingUp;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LyingUpStand;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LyingUpRollF;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LyingUpRollB;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* QuickRecoverF;
	UPROPERTY(Category = GroundHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* QuickRecoverB;

	UPROPERTY(Category = GrabbedAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Grabbed;

	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Air_Hit;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LaunchUpFar;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LaunchUpShort;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LaunchDownFaceDown;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LaunchDownFaceUp;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LaunchBackShort;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* LaunchBackFar;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Spiral;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Air_Block;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* Air_PushBlock;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* HitWall;
	UPROPERTY(Category = AirHitAnims, EditAnywhere, BlueprintReadWrite) UAnimationAsset* AirComboBreak;

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	void PrepareChildMove(UMNNode* Parent);
	UFUNCTION()
	bool PerformChildMove();

	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	void SwapPawns();
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	void RefillTransform();
	UFUNCTION(BlueprintCallable, Category = FighterPawn)
	void ResetReadiedMoves();

protected:
	// Stacks the animation sequences sent to PlayAnim()
	UAnimationAsset* AnimationSequence = nullptr;

private:
	// Internal handler of the OnMontageEnd event. Checks AnimationSequence array and calls the BP event.
	UFUNCTION(meta = (BlueprintInternalUseOnly))
		void CallAnimEndEvent(UAnimMontage* Montage, bool Interrupted);

	// Invincibility
	bool bInvincible = false;
	bool bEndInvincible = false;
	bool bFreezeInvincible = false;


	// Counters.
	uint8 RemainingJumps = 0;
	uint8 TimesLaunched = 0;
	uint8 TimesWallBounced = 0;
	uint8 TimesSpiralled = 0;
	uint8 TimesFloored = 0;
public:
	UPROPERTY(Category = CombatCounters, EditAnywhere, BlueprintReadWrite) uint8 AirSpecials = 0;
	UPROPERTY(Category = CombatCounters, EditAnywhere, BlueprintReadWrite) uint8 AirEXs = 0;
private:

	// True during a spiral after velocity is sufficienctly downward
	bool bSpiralFallCounted = false;
	// True if the current hit should always groundbounce/wallbounce/etc
	bool bForceComboExtension = false;
	// True if current hit reset combo extensions, so extension counters shouldn't increment until the next hit
	bool bComboExtensionsReset = false;

	EOOSMoveDir ResolveDirection(EOOSInputDir InputDirection, bool bUseCurrentFacing = false);

	bool bFrozen = false;
	bool bFreezeTwitch = false;
	FVector FreezeLocation;
	FTimerHandle FreezeTimer;
	FTimerHandle UnflyTimer;

	float HitStop;

	bool ResolveAttack(EOOSInputAttack Moveset, EOOSInputAttack Buffered);
	bool FindMatchingMove(const TArray<UMNNode*> &InputMoves, UMNNode_Move* &OutputMove, const FOOSInputMove BufferedMove, bool bAir, bool bFirstMove, bool bSupersOnly = false);
	void PerformMove(UMNNode_Move* Node, bool bForce = false);


	float TransformChargeDrainTimer = 0.f;

	bool bWantsAnimRefresh = false;

protected:
	virtual void StartTransformation();
	virtual void FinishTransformation();

private:
	bool bHasTransformed = false;

	void WakeUpRecovery(bool bFaceDown, bool bForwardRollOnNeutral = false);

	void AutoJump();

public:
	void PushBlock();

	float LastDetransform;

	UPROPERTY(BlueprintReadOnly)
	int LastDamage = 0;

	AOOSGameMode* GameMode;

};
