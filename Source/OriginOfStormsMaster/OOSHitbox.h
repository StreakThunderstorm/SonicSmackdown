// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "OOSHurtbox.h"
#include "OOSHitbox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHitboxCollisionDelegate, FOOSOverlapInfo, OverlapInfo);

/**
*
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class ORIGINOFSTORMSMASTER_API UOOSHitbox : public UOOSHurtbox
{
	GENERATED_BODY()

public:
	UOOSHitbox();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;


	UPROPERTY(Category = Hitbox, EditAnywhere, BlueprintReadWrite)
	TArray<FOOSOverlapInfo> HuBList;

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
		enum EOOSHitHeight Height, 
		enum EOOSInputAttack Att, 
		enum EOOSLaunchType Launch,
		bool bForceExtension,
		bool bResetExtensions,
		enum EOOSDirectionMode Direction,
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
	enum EOOSHitHeight HitHeight;
	enum EOOSInputAttack Attack;
	bool bForceComboExtension;
	bool bResetComboExtensions;
	enum EOOSLaunchType LaunchType;
	enum EOOSDirectionMode DirectionMode;
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
	void TryHit(FOOSOverlapInfo OverlapInfo);

	void SetTransformOnHit(bool bBlocked, int Damage, float OwnMult, float OppMult);

	//Internal handlers of Hitbox overlaps.
	UFUNCTION(meta = (BlueprintInternalUseOnly))
		void ReceiveHBBeginOverlaps(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp);
	UFUNCTION(meta = (BlueprintInternalUseOnly))
		void ReceiveHBEndOverlaps(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp);

	bool FindOverlapInfo(UPrimitiveComponent *ThisComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FOOSOverlapInfo& NewOverlap);

	void GetHurtboxesBounds(FVector& Origin, FVector& BoxExtent);

	class AOOSPawn* FPOwner;


	// Hit flag, raised at first contact.
	bool bHasMadeContact = false;
	// Confirm flag, raised if not blocked.
	bool bHitConfirm = false;

	FOOSOverlapInfo OverlapInfo;

	bool bIsProjectile = false;

	float GetDirection();

public:

	virtual void ResetHitbox();
	void HitLag();

	FVector2D GravitationalDirection = FVector2D::ZeroVector;

	FHitResult GrabOpponent(bool bSweep = false, bool bDrag = false, float dTime = 0.f, float Drag = 0.f);
};
