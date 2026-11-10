// Copyright 2021 Prudnikov Nikolay. All Rights Reserved.

#include "PrExtendedShadingPro.h"

#include "PrExtendedShadingProDefines.h"

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "MaterialShaderType.h"
#include "Misc/DelayedAutoRegister.h"
#include "ShaderParameterMacros.h"
#include "Runtime/Launch/Resources/Version.h"

namespace Internal
{
class FCustomShaderType
{
public:
	void** vtable; // FShaderType has a virtual destructor.
	FShaderType::EShaderTypeForDynamicCast ShaderTypeForDynamicCast;
	const FTypeLayoutDesc* TypeLayout;
	const TCHAR* Name;
	FName TypeName;
	FHashedName HashedName;
	FHashedName HashedSourceFilename;
	const TCHAR* SourceFilename;
};

void CheckCustom(FShaderType* Original, FCustomShaderType* Custom)
{
	if (&Original->GetHashedShaderFilename() != &Custom->HashedSourceFilename || Original->GetShaderFilename() != Custom->SourceFilename)
	{
		UE_LOG(LogPrExtendedShadingPro, Fatal, TEXT("PrExtendedShadingPro shader layout does not match this engine build; refusing to overwrite shader filenames."));
	}
}
} // namespace Internal

FString FPrExtendedShadingProModule::ShaderVersion = FString::Printf(TEXT("%d.%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION, ENGINE_PATCH_VERSION);

void FPrExtendedShadingProModule::StartupModule()
{
#if WITH_EDITOR
	FDelayedAutoRegisterHelper(EDelayedRegisterRunPhase::ShaderTypesReady, &FPrExtendedShadingProModule::InstallShaderOverrides);
#endif
}

void FPrExtendedShadingProModule::InstallShaderOverrides()
{
#if WITH_EDITOR
	if (!FPlatformProperties::RequiresCookedData())
	{
		const auto PluginName = TEXT("PrExtendedShadingPro");
		const auto PluginDirectory = IPluginManager::Get().FindPlugin(PluginName)->GetBaseDir();
		const auto PluginShaderDirectory = FPaths::Combine(PluginDirectory, TEXT("Shaders"));
		const auto PluginVirtualShaderDirectory = FPaths::Combine(TEXT("/Plugin"), PluginName);

		TMap<FString, FString> ShaderMap;

		const auto ClosedPluginShaderDirectory = PluginShaderDirectory / "";
		const auto ClosedPluginVirtualShaderDirectory = PluginVirtualShaderDirectory / "";
		IFileManager::Get().IterateDirectoryRecursively(*ClosedPluginShaderDirectory, [&ClosedPluginShaderDirectory, &ClosedPluginVirtualShaderDirectory, &ShaderMap](const TCHAR* FullFilename, const bool InIsDirectory) -> bool {
			if (!InIsDirectory)
			{
				FString Filename = FullFilename;
				if (FPaths::GetExtension(Filename) == TEXT("usf"))
				{
					if (FPaths::MakePathRelativeTo(Filename, *ClosedPluginShaderDirectory))
					{
						ShaderMap.Add(FPaths::Combine(TEXT("/Engine"), Filename), FPaths::Combine(ClosedPluginVirtualShaderDirectory, Filename));
					}
				}
			}

			return true;
		});

		static TMap<FString, FString> ReplaceShaderMap; // STATIC!

		bool bShaderVersionFound = false;
		for (const auto& Pair : ShaderMap)
		{
			int32 PatchVersion = ENGINE_PATCH_VERSION;
			while (PatchVersion >= 0 && !bShaderVersionFound)
			{
				ShaderVersion = FString::Printf(TEXT("%d.%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION, PatchVersion);
				if (Pair.Key.Contains(FString::Printf(TEXT("%s/"), *ShaderVersion)))
				{
					bShaderVersionFound = true;
				}
				PatchVersion -= 1;
			}

			if (bShaderVersionFound)
			{
				break;
			}
		}

		const auto EngineVersionPath = FString::Printf(TEXT("%s/"), *ShaderVersion);
		for (const auto& Pair : ShaderMap)
		{
			if (Pair.Key.Contains(EngineVersionPath))
			{
				ReplaceShaderMap.Add(Pair.Key.Replace(*EngineVersionPath, TEXT(""), ESearchCase::CaseSensitive), Pair.Value);
			}
		}

		if (ReplaceShaderMap.Num() > 0)
		{
			AddShaderSourceDirectoryMapping(PluginVirtualShaderDirectory, PluginShaderDirectory);

			// ShaderTypesReady runs after lazy registration and before global or material shader caching.
#if ENGINE_MAJOR_VERSION > 5 || ENGINE_MINOR_VERSION >= 8
			const bool bShaderTypesReady = FShaderTypeRegistration::AreShaderTypesInitialized();
#else
			const bool bShaderTypesReady = FShaderTypeRegistration::GetInstances().IsEmpty();
#endif
			if (!FShaderParametersMetadataRegistration::IsReadyForRegistration() || !bShaderTypesReady)
			{
				UE_LOG(LogPrExtendedShadingPro, Error, TEXT("Shader registration is not ready; refusing to replace shader types."));
				return;
			}
			int32 ReplacedShaderTypes = 0;
			for (auto ShaderTypeLink = FShaderType::GetTypeList(); ShaderTypeLink; ShaderTypeLink = ShaderTypeLink->Next())
			{
				auto& ShaderTypeNode = *ShaderTypeLink;
				auto& ShaderType = *ShaderTypeNode;

				if (auto NewFilenamePtr = ReplaceShaderMap.Find(ShaderType->GetShaderFilename()))
				{
					// Only the verified filename prefix is accessed; UE5 added fields after this prefix.
					auto CustomShaderType = reinterpret_cast<Internal::FCustomShaderType*>(ShaderType);

					Internal::CheckCustom(ShaderType, CustomShaderType);

					CustomShaderType->HashedSourceFilename = **NewFilenamePtr;
					CustomShaderType->SourceFilename = **NewFilenamePtr;
					++ReplacedShaderTypes;
				}
			}
			UE_LOG(LogPrExtendedShadingPro, Display, TEXT("Loaded shader implementation %s; replaced %d shader types."), *ShaderVersion, ReplacedShaderTypes);
		}
		else
		{
			UE_LOG(LogPrExtendedShadingPro, Error, TEXT("Unsupported version of engine: %d.%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION, ENGINE_PATCH_VERSION);
		}
	}
#endif // WITH_EDITOR
}

void FPrExtendedShadingProModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FPrExtendedShadingProModule, PrExtendedShadingPro)
