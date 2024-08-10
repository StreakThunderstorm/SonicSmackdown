// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSStageBounds.h"
#include "OOSCamera.h"
#include "OriginOfStormsMaster.h"

// Sets default values
AOOSStageBounds::AOOSStageBounds()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent *Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	LeftBound = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftBound"));
	LeftBound->SetupAttachment(Root);
	LeftBound->SetBoxExtent(FVector(VolumeThickness, VolumeThickness, (CeilingHeight / 2)));
	LeftBound->SetRelativeLocation(FVector((-HorizontalBoundsHalfExtent + VolumeThickness), 0, (CeilingHeight / 2)));
	LeftBound->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	LeftBound->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	LeftBound->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);

	RightBound = CreateDefaultSubobject<UBoxComponent>(TEXT("RightBound"));
	RightBound->SetupAttachment(Root);
	RightBound->SetBoxExtent(FVector(VolumeThickness, VolumeThickness, (CeilingHeight / 2)));
	RightBound->SetRelativeLocation(FVector(HorizontalBoundsHalfExtent + VolumeThickness, 0, (CeilingHeight / 2)));
	RightBound->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	RightBound->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	RightBound->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);

	FloorBound = CreateDefaultSubobject<UBoxComponent>(TEXT("FloorBound"));
	FloorBound->SetupAttachment(Root);
	FloorBound->SetBoxExtent(FVector(HorizontalBoundsHalfExtent + (2 * VolumeThickness), VolumeThickness, VolumeThickness));
	FloorBound->SetRelativeLocation(FVector(0, 0, -VolumeThickness));
	FloorBound->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	FloorBound->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FloorBound->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);

	CeilingBound = CreateDefaultSubobject<UBoxComponent>(TEXT("CeilingBound"));
	CeilingBound->SetupAttachment(Root);
	CeilingBound->SetBoxExtent(FVector(HorizontalBoundsHalfExtent + (2 * VolumeThickness), VolumeThickness, VolumeThickness));
	CeilingBound->SetRelativeLocation(FVector(0, 0, CeilingHeight + VolumeThickness));
	CeilingBound->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	CeilingBound->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CeilingBound->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);
}

void AOOSStageBounds::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	LeftBound->SetBoxExtent(FVector(VolumeThickness, VolumeThickness, (CeilingHeight / 2)));
	LeftBound->SetRelativeLocation(FVector(-(HorizontalBoundsHalfExtent + VolumeThickness), 0, (CeilingHeight / 2)));

	RightBound->SetBoxExtent(FVector(VolumeThickness, VolumeThickness, (CeilingHeight / 2)));
	RightBound->SetRelativeLocation(FVector(HorizontalBoundsHalfExtent + VolumeThickness, 0, (CeilingHeight / 2)));


	FloorBound->SetBoxExtent(FVector(HorizontalBoundsHalfExtent + (2 * VolumeThickness), VolumeThickness, VolumeThickness));
	FloorBound->SetRelativeLocation(FVector(0, 0, -VolumeThickness));

	CeilingBound->SetBoxExtent(FVector(HorizontalBoundsHalfExtent + (2 * VolumeThickness), VolumeThickness, VolumeThickness));
	CeilingBound->SetRelativeLocation(FVector(0, 0, CeilingHeight + VolumeThickness));
}

// Called when the game starts or when spawned
void AOOSStageBounds::BeginPlay()
{
	Super::BeginPlay();

	LeftBound->SetBoxExtent(FVector(VolumeThickness, VolumeThickness, (CeilingHeight / 2)));
	LeftBound->SetRelativeLocation(FVector(-(HorizontalBoundsHalfExtent + VolumeThickness), 0, (CeilingHeight / 2)));

	RightBound->SetBoxExtent(FVector(VolumeThickness, VolumeThickness, (CeilingHeight / 2)));
	RightBound->SetRelativeLocation(FVector(HorizontalBoundsHalfExtent + VolumeThickness, 0, (CeilingHeight / 2)));


	FloorBound->SetBoxExtent(FVector(HorizontalBoundsHalfExtent + (2 * VolumeThickness), VolumeThickness, VolumeThickness));
	FloorBound->SetRelativeLocation(FVector(0, 0, -VolumeThickness));

	CeilingBound->SetBoxExtent(FVector(HorizontalBoundsHalfExtent + (2 * VolumeThickness), VolumeThickness, VolumeThickness));
	CeilingBound->SetRelativeLocation(FVector(0, 0, CeilingHeight + VolumeThickness));

}

// Called every frame
void AOOSStageBounds::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}