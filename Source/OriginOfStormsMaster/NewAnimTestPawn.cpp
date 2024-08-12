// Fill out your copyright notice in the Description page of Project Settings.


#include "NewAnimTestPawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSingleNodeInstance.h"

// Sets default values
ANewAnimTestPawn::ANewAnimTestPawn()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USkeletalMeshComponent* InMesh = GetMesh();
	if(InMesh) InMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);

}

// Called when the game starts or when spawned
void ANewAnimTestPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANewAnimTestPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ANewAnimTestPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ANewAnimTestPawn::PlayAnimation(UAnimationAsset* Anim, bool bLoop)
{
	USkeletalMeshComponent* InMesh = GetMesh();
	if (!InMesh) return;

	UAnimSingleNodeInstance* SNI = InMesh->GetSingleNodeInstance();
	if (!SNI) return;

	SNI->SetAnimationAsset(Anim, bLoop);

}

