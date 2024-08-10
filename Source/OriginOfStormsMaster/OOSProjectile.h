// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OOSHitbox.h"
#include "OOSProjectile.generated.h"

UCLASS()
class ORIGINOFSTORMSMASTER_API AOOSProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOOSProjectile();

	UPROPERTY(Category = FighterPawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UOOSHitbox* Hitbox;

	UPROPERTY(Category = FighterPawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UOOSHitbox* BeamBox;

	UFUNCTION(BlueprintCallable, Category = Projectile)
	void Initialize
	(
		int NHits, 
		float HitTime, 
		float Lifetime, 
		FVector2D Spd, 
		FVector FW, 
		bool bGrab, 
		bool bForceGrab,
		FName GrabSock,
		UParticleSystem* HitParticle, 
		USoundBase* HitSFX, 
		UParticleSystem* DeathParticle, 
		USoundBase* DeathSound,
		bool bBrst,
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
		enum EOOSHitHeight Height, 
		enum EOOSInputAttack Att, 
		enum EOOSLaunchType Launch,
		bool bForceExtension,
		bool bResetExtensions,
		enum EOOSDirectionMode Direction, 
		FVector2D LaunchSpd, 
		bool bForceTryPostLaunchJump
	);
	UFUNCTION(BlueprintCallable, Category = Projectile)
	AOOSProjectile* SpawnChild
	(
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
		bool bForceExtension,
		bool bResetExtensions,
		EOOSDirectionMode Direction, 
		FVector2D LaunchSpd, 
		bool bForceTryPostLaunchJump
	);

	AActor* PawnOwner;

	// BP event called when Lifetime is over.
	UFUNCTION(BlueprintNativeEvent, Category = "Projectile") void OnTimeOut();
	UFUNCTION(BlueprintNativeEvent, Category = "Projectile") void OnOpponentHit(FOOSOverlapInfo OverlapInfo);
	UFUNCTION(BlueprintNativeEvent, Category = "Projectile") void ProjectileDeath();

	// Just calls DestroyWithEffects.
	void Destroy_Effects();

public:
	UFUNCTION(BlueprintCallable)
		void ResetLifetime(float NewLifetime);
	//Internal handlers of Hitbox overlaps.
	UFUNCTION(meta = (BlueprintInternalUseOnly))
		void ProjectileBeginOverlap(UPrimitiveComponent* ThisComp, AActor* OtherActor, UPrimitiveComponent* OtherComp);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	bool bPawnVelocityLaunch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	bool bPrjXLaunch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	float PrjXLaunchMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	bool bPrjYLaunch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	float PrjYLaunchMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsBeam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector InitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector InitRelative = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector BeamRoot = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector2D Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector2D OGSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bLockBeam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bProtected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int TimesHit = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsSuper;

protected:
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOneHitKO;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDeceleration;



	// Forward vector from owner
	FVector ForwardVector = FVector::ForwardVector;

public:
	void Freeze(float Duration);
	void Unfreeze();
	void StopAnim(float Duration);
	void ResumeAnim();

protected:
	UFUNCTION(meta = (BlueprintInternalUseOnly))
	void OnContact(FOOSOverlapInfo OverlapInfo);

	UFUNCTION(meta = (BlueprintInternalUseOnly))
	void DestroyWithEffects();

private:

	FTimerHandle NewHit;
	FTimerHandle Death;

	int RemainingHits = 0;
	float TimeBetweenHits = KINDA_SMALL_NUMBER;

	UParticleSystem* HitFX;
	USoundBase* HitSound;

	UParticleSystem* DeathFX;
	USoundBase* DeathSFX;

	void DestroyProjectile();
	void ProjectileTimeOver();
	void ResetProjectile();

	class AOOSPawn* FPOwner = nullptr;

	FTimerHandle FreezeTimer;
	bool bFrozen = false;
	
};
