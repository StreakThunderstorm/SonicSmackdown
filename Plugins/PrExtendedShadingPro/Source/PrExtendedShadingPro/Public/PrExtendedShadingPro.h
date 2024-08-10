// Copyright 2021 Prudnikov Nikolay. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPrExtendedShadingProModule : public IModuleInterface
{
public:
	static FString ShaderVersion;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
