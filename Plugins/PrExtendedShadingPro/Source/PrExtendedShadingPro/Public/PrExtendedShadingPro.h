// Copyright 2021 Prudnikov Nikolay. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPrExtendedShadingProModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override { return false; }

private:
	static void InstallShaderOverrides();

public:
	static FString ShaderVersion;
};
