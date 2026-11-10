#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SSDBlueprintAudit.generated.h"

class UBlueprint;
class UWorld;

// Temporary read-only migration diagnostics. Never saves or reconstructs assets.
UCLASS()
class USSDBlueprintAudit : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Migration Audit")
    static FString InspectBlueprint(UBlueprint* Blueprint, bool bDetailedGraphs = false);

    UFUNCTION(BlueprintCallable, Category = "Migration Audit")
    static FString CompileBlueprintForAudit(UBlueprint* Blueprint);

    UFUNCTION(BlueprintCallable, Category = "Migration Audit")
    static TArray<UBlueprint*> GetExistingLevelBlueprints(UWorld* World);
};
