// Copyright 2021 Prudnikov Nikolay. All Rights Reserved.

#include "PrClearColorPostProcessVolume.h"

#include "Runtime/Launch/Resources/Version.h"

APrClearColorPostProcessVolume::APrClearColorPostProcessVolume()
{
	Settings.bOverride_BloomIntensity = true;
	Settings.BloomIntensity = 0.f;

	Settings.bOverride_VignetteIntensity = true;
	Settings.VignetteIntensity = 0.f;

#if ENGINE_MINOR_VERSION >= 26
	Settings.bOverride_ToneCurveAmount = true;
	Settings.ToneCurveAmount = 0.f;
#endif

	bUnbound = true;
}
