#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PawnRenderLayerSubsystem.generated.h"

class UPawnRenderLayerComponent;
class UPrimitiveComponent;
class AActor;

UCLASS()
class PAWNRENDERLAYERS_API UPawnRenderLayerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    void RegisterGroup(UPawnRenderLayerComponent* Group);
    void UnregisterGroup(UPawnRenderLayerComponent* Group);
    bool IsClaimedByOtherGroup(const UPrimitiveComponent* Primitive, const UPawnRenderLayerComponent* Group) const;

    void UpdateGroup(UPawnRenderLayerComponent* Group);
    AActor* GetProjectionAnchor(const UPawnRenderLayerComponent* Group) const;
    FVector2D GetDepthBand(int32 Layer) const;
    FVector2D GetDepthRange(const UPawnRenderLayerComponent* Group, float ReferenceDistance, const FVector& CameraForward) const;

    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    void BringToFront(UPawnRenderLayerComponent* Group);

    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    void SendToBack(UPawnRenderLayerComponent* Group);

private:
    TArray<TWeakObjectPtr<UPawnRenderLayerComponent>> Groups;

public:
    // Far edge of the foreground depth range. Zero uses twice the engine near clip distance.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn Render Layers", meta = (ClampMin = "0"))
    float LayerPlaneDistance = 0.f;
};
