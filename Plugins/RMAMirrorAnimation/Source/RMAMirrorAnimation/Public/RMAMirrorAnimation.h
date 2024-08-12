// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UAnimSequence;
class URMAMirrorAnimationMirrorTable;

//Delegate
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMirrorAnimations, const TArray<UAnimSequence*>& /*Animations*/, URMAMirrorAnimationMirrorTable& /*MirrorTable*/);

//Module Runtime
class FRMAMirrorAnimation : public IModuleInterface
{

public:

	//Return The Module Instance
	static inline FRMAMirrorAnimation& Get()
	{

		return FModuleManager::LoadModuleChecked<FRMAMirrorAnimation>("RMAMirrorAnimation");

	}

	//Return The Module Was Loaded
	static inline bool IsAvailable()
	{

		return FModuleManager::Get().IsModuleLoaded("RMAMirrorAnimation");

	}

	//Called After The Module Has Been Loaded
	virtual void StartupModule() override;

	//Called Before The Module Is Unloaded
	virtual void ShutdownModule() override;

	//OnMirrorAnimations (Delegate)
	FOnMirrorAnimations OnMirrorAnimationsDelegate;

};
