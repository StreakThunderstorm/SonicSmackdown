// Fill out your copyright notice in the Description page of Project Settings.


#include "TechWindow.h"
#include "OOSPawn.h"

void UTechWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	AOOSPawn* FPOwner = Cast<AOOSPawn>(MeshComp->GetOwner());
	if (FPOwner)
	{
		FPOwner->bTechWindow = true;
		FPOwner->TechWindowTime = FPlatformTime::Seconds();
	}
}

void UTechWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);
	AOOSPawn* FPOwner = Cast<AOOSPawn>(MeshComp->GetOwner());
	if (FPOwner)
	{
		FPOwner->bTechWindow = false;
	}
}