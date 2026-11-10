#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnRenderLayerEffectComponent.generated.h"

class UFXSystemComponent;
class UNiagaraComponent;
class UParticleSystemComponent;
class UPawnRenderLayerComponent;

UCLASS()
class PAWNRENDERLAYERS_API UPawnRenderLayerEffectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Detached effects need an explicit spawning actor; location alone cannot identify their render group.
    UFUNCTION(BlueprintCallable, Category = "Pawn Render Layers")
    static void BindEffect(UFXSystemComponent* Effect, AActor* SpawnOwner);
    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

private:
    void BindToOwner(UFXSystemComponent* Effect, AActor* SpawnOwner);
    void ReleaseEffect();
    UFUNCTION()
    void OnNiagaraFinished(UNiagaraComponent* FinishedSystem);
    UFUNCTION()
    void OnParticleFinished(UParticleSystemComponent* FinishedSystem);

    UPROPERTY(Transient, DuplicateTransient)
    TWeakObjectPtr<UFXSystemComponent> BoundEffect;
    UPROPERTY(Transient, DuplicateTransient)
    TWeakObjectPtr<UPawnRenderLayerComponent> RenderGroup;
};
