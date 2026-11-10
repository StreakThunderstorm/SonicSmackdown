// Fill out your copyright notice in the Description page of Project Settings.


#include "TechWindow.h"
#include "SSDPawn.h"

void UTechWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	ASSDPawn* FPOwner = Cast<ASSDPawn>(MeshComp->GetOwner());
	if (FPOwner)
	{
		FPOwner->bTechWindow = true;
		FPOwner->TechWindowTime = FPlatformTime::Seconds();
	}
}

void UTechWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	ASSDPawn* FPOwner = Cast<ASSDPawn>(MeshComp->GetOwner());
	if (FPOwner)
	{
		FPOwner->bTechWindow = false;
	}
}
