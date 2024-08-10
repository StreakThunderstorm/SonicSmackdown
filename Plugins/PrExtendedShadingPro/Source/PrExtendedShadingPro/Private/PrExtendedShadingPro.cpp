// Copyright 2021 Prudnikov Nikolay. All Rights Reserved.

#include "PrExtendedShadingPro.h"

#include "PrExtendedShadingProDefines.h"

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "MaterialShaderType.h"
#include "Runtime/Launch/Resources/Version.h"

namespace Internal
{
class FCustomShaderType
{
public:
	void** vtable;
	FShaderType::EShaderTypeForDynamicCast ShaderTypeForDynamicCast;
	const FTypeLayoutDesc* TypeLayout;
	const TCHAR* Name;
	FName TypeName;
	FHashedName HashedName;
	FHashedName HashedSourceFilename;
	const TCHAR* SourceFilename;
	const TCHAR* FunctionName;
	uint32 Frequency;
	uint32 TypeSize;
	int32 TotalPermutationCount;

	FShaderType::ConstructSerializedType ConstructSerializedRef;
	FShaderType::ConstructCompiledType ConstructCompiledRef;
	FShaderType::ModifyCompilationEnvironmentType ModifyCompilationEnvironmentRef;
	FShaderType::ShouldCompilePermutationType ShouldCompilePermutationRef;
	FShaderType::ValidateCompiledResultType ValidateCompiledResultRef;
	const FShaderParametersMetadata* const RootParametersMetadata;
};

void CheckCustom(FShaderType* Original, FCustomShaderType* Custom)
{
	if (&Original->GetHashedShaderFilename() != &Custom->HashedSourceFilename)
	{
		UE_LOG(LogPrExtendedShadingPro, Fatal, TEXT("Plugin PrExtendedShadingPro not allowed!"));
	}
}
} // namespace Internal

FString FPrExtendedShadingProModule::ShaderVersion = FString::Printf(TEXT("%d.%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION, ENGINE_PATCH_VERSION);

void FPrExtendedShadingProModule::StartupModule()
{
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

			for (auto ShaderTypeLink = FShaderType::GetTypeList(); ShaderTypeLink; ShaderTypeLink = ShaderTypeLink->Next())
			{
				auto& ShaderTypeNode = *ShaderTypeLink;
				auto& ShaderType = *ShaderTypeNode;

				if (auto NewFilenamePtr = ReplaceShaderMap.Find(ShaderType->GetShaderFilename()))
				{
					auto CustomShaderType = reinterpret_cast<Internal::FCustomShaderType*>(ShaderType);

					Internal::CheckCustom(ShaderType, CustomShaderType);

					CustomShaderType->HashedSourceFilename = **NewFilenamePtr;
					CustomShaderType->SourceFilename = **NewFilenamePtr;
				}
			}
		}
		else
		{
			UE_LOG(LogPrExtendedShadingPro, Error, TEXT("Unsupported version of engine: %d.%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION, ENGINE_PATCH_VERSION);
		}
	}
}

void FPrExtendedShadingProModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FPrExtendedShadingProModule, PrExtendedShadingPro)