#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnRenderLayerComponent.generated.h"

class UPrimitiveComponent;
class UPawnRenderLayerSubsystem;

struct FPawnRenderLayerOriginalState
{
    TArray<float> ReservedData;
    int32 TranslucencyPriority = 0;
};

UCLASS(ClassGroup = (Rendering), meta = (BlueprintSpawnableComponent))
class PAWNRENDERLAYERS_API UPawnRenderLayerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPawnRenderLayerComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    void SetLayer(int32 InLayer);
    UFUNCTION(BlueprintPure, Category = "Pawn Render Layers")
    int32 GetLayer() const;
    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    void SetFlatteningPercent(float InPercent);
    UFUNCTION(BlueprintPure, Category = "Pawn Render Layers")
    float GetFlatteningPercent() const;
    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    void SetGroupLayeringEnabled(bool bEnabled);
    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    void BringToFront();
    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    void SendToBack();
    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    void RegisterEffect(UPrimitiveComponent* Effect);
    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    void UnregisterEffect(UPrimitiveComponent* Effect);
    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    void RefreshGroup();

    // Sharing a group also shares its projection anchor, flattening and ordering. Null restores automatic inheritance.
    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    bool SetSharedGroup(UPawnRenderLayerComponent* Group);
    UFUNCTION(BlueprintPure, Category = "Pawn Render Layers")
    UPawnRenderLayerComponent* GetRenderGroup() const;

    bool ContainsPrimitive(const UPrimitiveComponent* Primitive) const;

private:
    friend class UPawnRenderLayerSubsystem;
    UPawnRenderLayerSubsystem* GetLayerSubsystem() const;
    void UpdateProjection();
    void RestorePrimitive(UPrimitiveComponent* Primitive, const FPawnRenderLayerOriginalState& State);

    TMap<TWeakObjectPtr<UPrimitiveComponent>, FPawnRenderLayerOriginalState> Members;
    TSet<TWeakObjectPtr<UPrimitiveComponent>> Effects;
    TMap<TWeakObjectPtr<UPrimitiveComponent>, bool> ReleasedEffects;
    UPROPERTY(Transient)
    TWeakObjectPtr<UPawnRenderLayerComponent> SharedGroup;
    float ReferenceDistance = 1000.f;
    bool bWarnedCustomDataConflict = false;

public:
    // Layer zero is nearest. Positive layers sit progressively farther back in the character depth range.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn Render Layers", meta = (ClampMin = "0", ClampMax = "200"))
    int32 Layer = 0;
    // Resolves actor ownership and attachment, not Blueprint class inheritance.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn Render Layers")
    bool bInheritParentGroup = true;
    // Separate pawn instances receive an unused layer when their initial layer is occupied.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn Render Layers")
    bool bAutoAssignLayer = true;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn Render Layers", meta = (ClampMin = "0", ClampMax = "100"))
    float FlatteningPercent = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn Render Layers")
    bool bGroupLayeringEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn Render Layers")
    bool bIncludeAttachedActors = true;
};
