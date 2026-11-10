// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SSDHurtbox.h"
#include "SSDHitbox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHitboxCollisionDelegate, FSSDOverlapInfo, OverlapInfo);

/**
*
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class SONICSMACKDOWN_API USSDHitbox : public USSDHurtbox
{
	GENERATED_BODY()

public:
	USSDHitbox();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;


	UPROPERTY(Category = Hitbox, EditAnywhere, BlueprintReadWrite)
	TArray<FSSDOverlapInfo> HuBList;

	UPROPERTY(BlueprintReadWrite) bool bEnabled = true;
	UFUNCTION(BlueprintCallable, Category = Hitbox)
		virtual void Initialize
		(
			float HalfHeight,
			float Radius,
			bool bGrab,
			bool bForceGrb,
			FName GrabSock,
			UParticleSystem* HitParticle,
			USoundBase* HitSFX,
			bool bBrst,
			int Dmg,
			float HStun,
			float SelfHL,
			float OpponentHL,
			float BStun,
			float PushFactor,
			float Frz,
			bool FrzTw,
			bool bOTG,
			bool bOH,
			bool bAutoChild,
			bool bNoCancel,
			bool bUnBlck,
			bool bAntiA,
			bool bAntiPrj,
		enum ESSDHitHeight Height, 
		enum ESSDInputAttack Att, 
		enum ESSDLaunchType Launch,
		bool bForceExtension,
		bool bResetExtensions,
		enum ESSDDirectionMode Direction,
		FVector2D LaunchSpd, 
		bool bForceJmp
	);

	virtual void DestroyHitbox();

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FHitboxCollisionDelegate OnOpponentPostHit;

	bool IsOverlappingOpponent()
	{
		return HuBList.Num() > 0;
	}

	// Continuous hit detection. Each tick the hitbox is swept from where it was on the previous
	// tick to where it is now, relative to each opponent hurtbox (so the motion of both fighters
	// counts), to catch contacts that start and end between two ticks and never produce an overlap.
	UPROPERTY(Category = Hitbox, EditAnywhere, BlueprintReadWrite)
	bool bSweepEnabled = true;

	// Sweeps longer than this (relative travel in one tick, in world units) are treated as a
	// teleport and skipped, so warping past the opponent doesn't count as a hit.
	UPROPERTY(Category = Hitbox, EditAnywhere, BlueprintReadWrite)
	float MaxSweepDistance = 1000.f;

protected:

	bool bGrabMode = false;
	bool bForceGrab = false;
	FName GrabSocket = "TorsoSocket";

	// Move specific data
	UParticleSystem* HitEffect;
	USoundBase* HitSound;
	bool bBurst = false;
	int Damage = 50000;
	float HitStun = 0.5f;
	float BlockStun = 0.5f;
	float SelfHitLag = 0.1f;
	float OpponentHitLag = 0.1f;
	float Push = 0.f;
	float Freeze = 0.f;
	bool FreezeTwitch = true;
	bool bOffTheGround = false;
	bool bOverhead = false;
	bool bAutoPerformChildMove = false;
	bool bDontCancel = false;
	bool bUnblockable = false;
	bool bAntiArmor = false;
	enum ESSDHitHeight HitHeight;
	enum ESSDInputAttack Attack;
	bool bForceComboExtension;
	bool bResetComboExtensions;
	enum ESSDLaunchType LaunchType;
	enum ESSDDirectionMode DirectionMode;
	FVector2D LaunchSpeed = FVector2D::ZeroVector;
	bool bForceTryPostLaunchJump = false;

public:
	void NotifyOverlap(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp)
	{
		ReceiveHBBeginOverlaps(ThisComp, OtherActor, OtherComp);
	}

	virtual void MainLoop(float DeltaTime);

	bool bAntiProjectile = false;
	// Touch flag, reflects if the hitbox is overlapping any enemy hurtbox.
	bool bIsMakingContact = false;

protected:
	void TryHit(FSSDOverlapInfo OverlapInfo);

	void SetTransformOnHit(bool bBlocked, int Damage, float OwnMult, float OppMult);

	//Internal handlers of Hitbox overlaps.
	UFUNCTION(meta = (BlueprintInternalUseOnly))
		void ReceiveHBBeginOverlaps(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp);
	UFUNCTION(meta = (BlueprintInternalUseOnly))
		void ReceiveHBEndOverlaps(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp);

	bool FindOverlapInfo(UPrimitiveComponent *ThisComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FSSDOverlapInfo& NewOverlap);

	// Whether this hitbox may register hits from sweeps. Hitboxes that act on sustained contact
	// (grabs, gravitational pulls) only use real overlaps.
	virtual bool CanSweepForHits() const;

	// Records this tick's hitbox and hurtbox poses and, if allowed, looks for a hurtbox the hitbox
	// passed through since the previous tick. Returns true and fills OutOverlap with the earliest one.
	bool UpdateSweep(FSSDOverlapInfo& OutOverlap);

	// Poses seen on the previous tick (location and rotation only).
	bool bHasSweepHistory = false;
	FTransform PrevSweepTransform;
	TMap<TWeakObjectPtr<USSDHurtbox>, FTransform> PrevHurtboxTransforms;

	void GetHurtboxesBounds(FVector& Origin, FVector& BoxExtent);

	class ASSDPawn* FPOwner;


	// Hit flag, raised at first contact.
	bool bHasMadeContact = false;
	// Confirm flag, raised if not blocked.
	bool bHitConfirm = false;

	FSSDOverlapInfo OverlapInfo;

	bool bIsProjectile = false;

	float GetDirection();

public:

	virtual void ResetHitbox();
	void HitLag();

	FVector2D GravitationalDirection = FVector2D::ZeroVector;

	FHitResult GrabOpponent(bool bSweep = false, bool bDrag = false, float dTime = 0.f, float Drag = 0.f);
};
