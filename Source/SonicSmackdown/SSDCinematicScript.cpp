// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDCinematicScript.h"
#include "SSDPawn.h"


// Sets default values
ASSDCinematicScript::ASSDCinematicScript()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASSDCinematicScript::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASSDCinematicScript::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASSDCinematicScript::OnCinematicStart_Implementation(ASSDPawn* Pawn, AActor* Camera)
{

}
void ASSDCinematicScript::OnCinematicEnd_Implementation()
{

}

void ASSDCinematicScript::OnCinematicInterrupted_Implementation()
{

}