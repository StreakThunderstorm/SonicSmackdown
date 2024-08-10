// Fill out your copyright notice in the Description page of Project Settings.


#include "OOSPawn_CharacterSelection.h"

// Sets default values
AOOSPawn_CharacterSelection::AOOSPawn_CharacterSelection()
{
}

void AOOSPawn_CharacterSelection::RefreshAnimation(USkeletalMeshComponent* SkeletalMesh)
{
	//@TODO: this mesh should be a property of this class, but it's having
	// issues for the child class for some reason

	// Immediately start up the animation so it's shown during hitstop
	SkeletalMesh->TickAnimation(0.f, false);
	SkeletalMesh->RefreshBoneTransforms();
}
