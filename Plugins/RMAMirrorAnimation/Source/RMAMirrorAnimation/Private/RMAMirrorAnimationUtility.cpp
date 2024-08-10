// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimationUtility.h"
#include "Interfaces/IPluginManager.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#include "AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#endif

#include "AssetData.h"

FRMAMirrorAnimationBoneKeyword::FRMAMirrorAnimationBoneKeyword(FString PKeywordA /* = "None" */, FString PKeywordB /* = "None" */)
{

	KeywordA = PKeywordA;
	KeywordB = PKeywordB;

}

FRMAMirrorAnimationSingleBoneConfig::FRMAMirrorAnimationSingleBoneConfig(FName PBoneName /* = "None" */, ERMAMirrorAnimationAxis PMirrorAxis /* = ERMAMirrorAnimationAxis::AxisX */)
{

	BoneName = PBoneName;
	MirrorAxis = PMirrorAxis;

}

FRMAMirrorAnimationDoubleBoneConfig::FRMAMirrorAnimationDoubleBoneConfig(FName PBoneAName /* = "None" */, FName PBoneBName /* = "None" */)
{

	BoneAName = PBoneAName;
	BoneBName = PBoneBName;
	LocationMirrorAxis = ERMAMirrorAnimationAxisWithNull::AxisNull;
	RotationMirrorAxis = ERMAMirrorAnimationAxisWithNull::AxisNull;

}

FName URMAMirrorAnimationFunctionLibrary::GetVersion()
{

	return *IPluginManager::Get().FindPlugin("RMAMirrorAnimation")->GetDescriptor().VersionName;

}

FString URMAMirrorAnimationFunctionLibrary::GetUserPreference(FString Key, bool& Successful)
{

#if WITH_EDITOR

	if (GConfig && Key.Len() > 0)
	{

		FString LValue = "";
		const FString LFilename = IPluginManager::Get().FindPlugin("RMAMirrorAnimation")->GetBaseDir() / TEXT("Config") / TEXT("UserPreferences.ini");

		Successful = GConfig->GetString(*FString("UserPreferences"), *Key, LValue, *LFilename);
		return LValue;

	}

#endif

	Successful = false;
	return "";

}

bool URMAMirrorAnimationFunctionLibrary::SetUserPreference(FString Key, FString NewValue)
{

#if WITH_EDITOR

	if (GConfig && Key.Len() > 0)
	{

		const FString LFilename = IPluginManager::Get().FindPlugin("RMAMirrorAnimation")->GetBaseDir() / TEXT("Config") / TEXT("UserPreferences.ini");
		GConfig->SetString(*FString("UserPreferences"), *Key, *NewValue, *LFilename);

		GConfig->Flush(false, LFilename);

		return true;

	}

#endif

	return false;

}

UObject* URMAMirrorAnimationFunctionLibrary::GetAssetByUniqueID(FString UniqueID, TSubclassOf<UObject> Class /* = nullptr */)
{

#if WITH_EDITOR

	if (UniqueID.Len() > 0)
	{

		TArray<UObject*> LAssets;

		if (Class)
		{

			LAssets = GetAssetsByClass(Class);

		}

		else
		{

			LAssets = GetAssetsByClass(UObject::StaticClass());

		}

		for (int LIndex = 0; LIndex < LAssets.Num(); LIndex++)
		{

			if (FString::FromInt(LAssets[LIndex]->GetUniqueID()) == UniqueID)
			{

				return LAssets[LIndex];

			}

		}

	}

#endif

	return nullptr;

}

TArray<UObject*> URMAMirrorAnimationFunctionLibrary::GetAssetsByClass(TSubclassOf<UObject> Class)
{

	TArray<UObject*> LAssets;
	TArray<FAssetData> LAssetsData;

#if WITH_EDITOR

	if (Class)
	{

		FAssetRegistryModule& LAssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		//Find Assets
		LAssetRegistryModule.Get().GetAssetsByClass(Class->GetFName(), LAssetsData);

		//Cast AssetData To Object
		for (int LIndex = 0; LIndex < LAssetsData.Num(); LIndex++)
		{

			LAssets.Add(LAssetsData[LIndex].GetAsset());

		}

	}

#endif

	return LAssets;

}

bool URMAMirrorAnimationFunctionLibrary::SaveLoadedAsset(UObject* AssetToSave)
{

#if WITH_EDITOR

	if (AssetToSave)
	{

		return UEditorAssetLibrary::SaveLoadedAsset(AssetToSave, false);

	}

#endif

	return false;

}

FString URMAMirrorAnimationFunctionLibrary::GetPathNameForLoadedAsset(UObject* LoadedAsset)
{

#if WITH_EDITOR

	if (LoadedAsset)
	{

		return UEditorAssetLibrary::GetPathNameForLoadedAsset(LoadedAsset);

	}

#endif

	return "None";

}

void URMAMirrorAnimationFunctionLibrary::SyncBrowserToObjects(const TArray<FString>& AssetPaths)
{

#if WITH_EDITOR

	UEditorAssetLibrary::SyncBrowserToObjects(AssetPaths);

#endif

}
