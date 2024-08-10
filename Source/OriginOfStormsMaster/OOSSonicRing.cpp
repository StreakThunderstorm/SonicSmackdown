// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSSonicRing.h"
#include "OriginOfStormsMaster.h"
#include "OOSMovementComponent.h"
#include "OOSPawn_Transformed.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AOOSSonicRing::AOOSSonicRing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	Box->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	RootComponent = Box;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttractionField"));
	Sphere->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	Sphere->SetSphereRadius(150.f);
	Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));

	RingSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Sound"));
	RingSound->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	RingSound->bAutoActivate = false;

	Sparkle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Sparkle"));
	Sparkle->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	Sparkle->bAutoActivate = false;
}

void AOOSSonicRing::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Mesh)
	{
		Box->SetBoxExtent(FVector(Mesh->Bounds.BoxExtent.Z, Mesh->Bounds.BoxExtent.Z, Mesh->Bounds.BoxExtent.Z));
	}
}

void AOOSSonicRing::SetDisableTimer(float Time)
{
	DisableTimer = (Time > 0.f) ? Time : DefaultDisableTime;
}

// Called when the game starts or when spawned
void AOOSSonicRing::BeginPlay()
{
	Super::BeginPlay();
	
	FScriptDelegate BoxOverlap;
	BoxOverlap.BindUFunction(this, TEXT("OnBoxOverlap"));
	Box->OnComponentBeginOverlap.Add(BoxOverlap);

	FScriptDelegate SparkleFinish;
	SparkleFinish.BindUFunction(this, TEXT("OnSparkleFinish"));
	Sparkle->OnSystemFinished.Add(SparkleFinish);

	FScriptDelegate SphereOverlap;
	SphereOverlap.BindUFunction(this, TEXT("OnSphereOverlap"));
	Sphere->OnComponentBeginOverlap.Add(SphereOverlap);

	RotationSpeed = FMath::FRandRange(0.75f, 1.25f);
	LifeTimer = LifeTime;
}

// Called every frame
void AOOSSonicRing::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (bFrozen) return;

	// 60fps proportional delta time.
	float dTime = DeltaTime / (1 / 60.f);

	// Decrease disable timer.
	DisableTimer -= FMath::Min(DisableTimer, dTime);

	// Super duper ring rotation effect.
	if (Mesh)
	{
		Mesh->AddRelativeRotation(FQuat(FVector(0, 0, 1), 2 * PI * RotationSpeed * DeltaTime));
	}

	// Flicker when one second remaining
	if (LifeTimer < 60.f && Box->OnComponentBeginOverlap.IsBound())
	{
		Mesh->SetVisibility((int)LifeTimer % 4 < 2);
	}

	// Despawn if too far from a fighter
	AOOSPawn *Owner = Cast<AOOSPawn>(GetOwner());
	if (IsTooFarFromPawn(Owner))
	{
		Destroy();
		return;
	}
	if (Owner)
	{
		if (IsTooFarFromPawn(Owner->Opponent))
		{
			Destroy();
			return;
		}
	}

	switch (Mode)
	{
		case EOOSSonicRingMode::RM_Attraction:
		{
			FVector PawnLoc = PawnToFollow->GetActorLocation();
			FVector ThisLoc = GetActorLocation();
			FVector ToPawn = PawnLoc - ThisLoc;
			ToPawn = ToPawn.GetSafeNormal();
			
			// X tracking.
			float ToPawn_X = FMath::Sign(ToPawn.X);
			if (FMath::Sign(Velocity.X) == ToPawn_X)
				Velocity.X += ToPawn_X * 0.1875f * dTime;
			else
				Velocity.X += ToPawn_X * 0.75f * dTime;

			// Y tracking.
			float ToPawn_Y = FMath::Sign(ToPawn.Y);
			if (FMath::Sign(Velocity.Y) == ToPawn_Y)
				Velocity.Y += ToPawn_Y * 0.1875f * dTime;
			else
				Velocity.Y += ToPawn_Y * 0.75f * dTime;

			// Z tracking.
			float ToPawn_Z = FMath::Sign(ToPawn.Z);
			if (FMath::Sign(Velocity.Z) == ToPawn_Z)
				Velocity.Z += ToPawn_Z * 0.1875f * dTime;
			else
				Velocity.Z += ToPawn_Z * 0.75f * dTime;

			AddActorWorldOffset(Velocity * DeltaTime * MASTER_SCALE);
			
			// Lightning shield attraction stuff
			/*if (!(PawnToFollow->Shield == ES3KRMElementShield::ES_Lightning))
			{
				Mode = EOOSSonicRingMode::RM_Free;
				Box->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
				Velocity = FVector::VectorPlaneProject(Velocity, GetActorRightVector());
			}*/
		}
		break;

		case EOOSSonicRingMode::RM_Free:
		{
			// Decrease lifetime counter.
			LifeTimer -= FMath::Min(LifeTimer, dTime);
			if (LifeTimer == 0.f)
			{
				Destroy();
				return;
			}

			// Apply gravity and move.
			Velocity.Z -= FREEMODE_GRAVITY * dTime;

			FHitResult Hit;
			AddActorWorldOffset(Velocity * DeltaTime * MASTER_SCALE, true, &Hit);
			if (Hit.IsValidBlockingHit())
			{
				// 5° of randomness
				FVector RndNormal = FMath::VRandCone(Hit.ImpactNormal, PI / 36.f);

				// Snap impact normal to the 2.5D plane.
				FVector SafeNormal = FVector::VectorPlaneProject(RndNormal, GetActorRightVector());
				
				// Bounce velocity vector around impact normal.
				Velocity = Velocity - 2 * (Velocity | SafeNormal) * SafeNormal;
				
				if (SafeNormal.Z > 0.7f)
					Velocity.Z = Velocity.Z * 0.75f;
			}
		}
		break;
	}	
}

void AOOSSonicRing::Freeze(float Duration)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (Duration >= 0.f)
	{
		World->GetTimerManager().SetTimer(FreezeTimer, this, &AOOSSonicRing::Unfreeze, Duration);
	}

	bFrozen = true;
	CustomTimeDilation = 0.001f;
}

void AOOSSonicRing::Unfreeze()
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(FreezeTimer);

	bFrozen = false;
	CustomTimeDilation = 1.f;
}

bool AOOSSonicRing::IsTooFarFromPawn(AOOSPawn* Pawn)
{
	if (Pawn)
	{
		UOOSMovementComponent* PawnMovement = Cast<UOOSMovementComponent>(Pawn->MovementComponent);
		if (PawnMovement)
		{
			return DistanceFromPawn(Pawn) >= PawnMovement->MaxDistanceToOpponent * 1.2f;// 1.5f;
		}
	}

	return false;
}

float AOOSSonicRing::DistanceFromPawn(AOOSPawn* Pawn)
{
	FVector VectorToPawn = Pawn->GetActorLocation() - this->GetActorLocation();
	float DistanceToPawn = VectorToPawn.ProjectOnTo(this->GetActorForwardVector()).Size();
	return DistanceToPawn;
}

void AOOSSonicRing::OnCollected_Implementation(AOOSPawn* Pawn)
{
	
}

void AOOSSonicRing::OnBoxOverlap(UPrimitiveComponent *Component, AActor *OtherActor, UPrimitiveComponent *OtherComponent)
{
	if (DisableTimer > 0.f) return;

	AOOSPawn *Pawn = Cast<AOOSPawn>(OtherActor);
	if (!Pawn) return;

	// Allow pawns to pick up their own rings, to encourage bursting after knockdowns
	//AOOSPawn *Owner = Cast<AOOSPawn>(GetOwner());
	//if (Pawn == Owner) return; // We're overlapping the pawn that spawned us.

	// No picking up rings when being hit/bursting
	if (Pawn->IsLaunched() || Pawn->IsFloored() || Pawn->IsRecoveringOnGround() || Pawn->IsStunned() ||
		Pawn->IsTransforming()) return;

	Mode = EOOSSonicRingMode::RM_Stationary;

	if (Mesh)
	{
		Mesh->SetVisibility(false);
	}

	if (RingSound)
	{
		RingSound->Play();
	}

	if (Sparkle)
	{
		Sparkle->Activate(true);
	}
	else Destroy();

	OnCollected(Pawn);

	Pawn->Rings++;

	Pawn->Health = FMath::Min(Pawn->Health + HealthRecover, Pawn->Regen);
	AOOSPawn_Transformed* Transformed = Cast<AOOSPawn_Transformed>(Pawn);
	if(Transformed) Pawn->Transform = FMath::Min(Pawn->Transform + FMath::FloorToInt(TransformRecover * Pawn->TransformGain), MAX_TRANSFORM_POINTS);

	Component->OnComponentBeginOverlap.Clear();
}

void AOOSSonicRing::OnSphereOverlap(UPrimitiveComponent *Component, AActor *OtherActor, UPrimitiveComponent *OtherComponent)
{
	if (DisableTimer > 0.f) return;

	AOOSPawn *Pawn = Cast<AOOSPawn>(OtherActor);
	if (!Pawn) return;

	// Lightning shield attraction stuff
	/*if (Pawn->Shield == ES3KRMElementShield::ES_Lightning)
	{
		PawnToFollow = Pawn;
		Mode = EOOSSonicRingMode::RM_Attraction;		
		Component->OnComponentBeginOverlap.Clear();
	}*/
}

void AOOSSonicRing::OnSparkleFinish()
{
	Destroy();
}