// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"

#if WITH_EDITOR
#include "ContentBrowserModule.h"
#endif

#include "RMAMirrorAnimationUtility.generated.h"

UENUM()
enum class ERMAMirrorAnimationAxis : uint8
{

	AxisX UMETA(DisplayName = "X"),
	AxisY UMETA(DisplayName = "Y"),
	AxisZ UMETA(DisplayName = "Z")

};

UENUM()
enum class ERMAMirrorAnimationAxisWithNull : uint8
{

	AxisNull UMETA(DisplayName = "Null"),
	AxisX UMETA(DisplayName = "X"),
	AxisY UMETA(DisplayName = "Y"),
	AxisZ UMETA(DisplayName = "Z")

};

USTRUCT()
struct RMAMIRRORANIMATION_API FRMAMirrorAnimationBoneKeyword
{

	GENERATED_BODY()

	//Keyword A
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "")
		FString KeywordA;

	//Keyword B
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "")
		FString KeywordB;

	FRMAMirrorAnimationBoneKeyword(FString PKeywordA = "None", FString PKeywordB = "None");

};

USTRUCT()
struct RMAMIRRORANIMATION_API FRMAMirrorAnimationSingleBoneConfig
{

	GENERATED_BODY()

	//Bone Name
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "")
		FName BoneName;

	//Mirror Axis
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "")
		ERMAMirrorAnimationAxis MirrorAxis;

	FRMAMirrorAnimationSingleBoneConfig(FName PBoneName = "", ERMAMirrorAnimationAxis PMirrorAxis = ERMAMirrorAnimationAxis::AxisX);

};

USTRUCT()
struct RMAMIRRORANIMATION_API FRMAMirrorAnimationDoubleBoneConfig
{

	GENERATED_BODY()

	//Bone A Name
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "", meta = (DisplayName = "Bone A Name"))
		FName BoneAName;

	//Bone B Name
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "", meta = (DisplayName = "Bone B Name"))
		FName BoneBName;

	//Location MirrorAxis
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "")
		ERMAMirrorAnimationAxisWithNull LocationMirrorAxis;

	//Rotation MirrorAxis
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "")
		ERMAMirrorAnimationAxisWithNull RotationMirrorAxis;

	FRMAMirrorAnimationDoubleBoneConfig(FName PBoneAName = "None", FName PBoneBName = "None");

};

//Function Library
UCLASS()
class RMAMIRRORANIMATION_API URMAMirrorAnimationFunctionLibrary : public UBlueprintFunctionLibrary
{

	GENERATED_BODY()

public:

	//Getter Plugin Version
	UFUNCTION(BlueprintPure, Category = "RMAMirrorAnimation")
		static FName GetVersion();

	//Getter User Preference
	UFUNCTION(BlueprintPure, Category = "RMAMirrorAnimation")
		static FString GetUserPreference(FString Key, bool& Successful);

	//Setter User Preference
	UFUNCTION(BlueprintCallable, Category = "RMAMirrorAnimation")
		static bool SetUserPreference(FString Key, FString NewValue);

	//Getter Asset By UniqueID
	static UObject* GetAssetByUniqueID(FString UniqueID, TSubclassOf<UObject> Class = nullptr);

	//Getter Assets By Class
	static TArray<UObject*> GetAssetsByClass(TSubclassOf<UObject> Class);

	//Save Loaded Asset
	UFUNCTION(BlueprintCallable, Category = "RMAMirrorAnimation")
		static bool SaveLoadedAsset(UObject* AssetToSave);

	//Getter Path Name For Loaded Asset
	UFUNCTION(BlueprintCallable, Category = "RMAMirrorAnimation")
		static FString GetPathNameForLoadedAsset(UObject* LoadedAsset);

	//Sync Browser To Objects
	UFUNCTION(BlueprintCallable, Category = "RMAMirrorAnimation")
		static void SyncBrowserToObjects(const TArray<FString>& AssetPaths);

};
