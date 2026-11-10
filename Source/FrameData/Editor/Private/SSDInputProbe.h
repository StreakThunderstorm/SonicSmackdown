#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SSDInputProbe.generated.h"

// Temporary, command-line opt-in diagnostics. Remove after the input/facing investigation.
UCLASS()
class USSDInputProbe : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MigrationProbe")
	static bool StartPlay();

	UFUNCTION(BlueprintCallable, Category = "MigrationProbe")
	static void EndPlay();

	UFUNCTION(BlueprintCallable, Category = "MigrationProbe")
	static FString SendKey(FName KeyName, bool bPressed, int32 DeviceId = 0);

	UFUNCTION(BlueprintCallable, Category = "MigrationProbe")
	static FString SendAxis(FName KeyName, float Value, int32 DeviceId = 0);

	UFUNCTION(BlueprintCallable, Category = "MigrationProbe")
	static FString Snapshot(bool bIncludeWidgets = true);

	UFUNCTION(BlueprintCallable, Category = "MigrationProbe")
	static FString SnapshotFighters();
};
