// Fill out your copyright notice in the Description page of Project Settings.

#include "MNNode_Reroute.h"


UMNNode_Reroute::UMNNode_Reroute()
{
#if WITH_EDITORONLY_DATA
	NodeTitle = FText::FromString("RR");
	
	BackgroundColor = FLinearColor::Gray;
#endif
}

