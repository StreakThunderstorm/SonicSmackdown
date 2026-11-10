#include "PawnRenderLayerEffectComponent.h"
#include "PawnRenderLayerComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"

void UPawnRenderLayerEffectComponent::BindEffect(UFXSystemComponent* Effect, AActor* SpawnOwner)
{
    if (!IsValid(Effect) || !IsValid(Effect->GetOwner()) || !IsValid(SpawnOwner) || Effect->GetWorld() != SpawnOwner->GetWorld())
        return;

    TInlineComponentArray<UPawnRenderLayerEffectComponent*> Bindings(Effect->GetOwner());
    for (auto* Binding : Bindings)
        if (Binding->BoundEffect == Effect)
        {
            Binding->BindToOwner(Effect, SpawnOwner);
            return;
        }

    auto* Binding = NewObject<UPawnRenderLayerEffectComponent>(Effect->GetOwner(), NAME_None, RF_Transient);
    Effect->GetOwner()->AddInstanceComponent(Binding);
    Binding->RegisterComponent();
    Binding->BindToOwner(Effect, SpawnOwner);
}

void UPawnRenderLayerEffectComponent::BindToOwner(UFXSystemComponent* Effect, AActor* SpawnOwner)
{
    ReleaseEffect();
    BoundEffect = Effect;
    if (auto* Niagara = Cast<UNiagaraComponent>(Effect))
        Niagara->OnSystemFinished.AddUniqueDynamic(this, &UPawnRenderLayerEffectComponent::OnNiagaraFinished);
    if (auto* Particle = Cast<UParticleSystemComponent>(Effect))
        Particle->OnSystemFinished.AddUniqueDynamic(this, &UPawnRenderLayerEffectComponent::OnParticleFinished);

    TArray<AActor*> Pending = { SpawnOwner };
    TSet<AActor*> Seen;
    for (int32 Index = 0; Index < Pending.Num(); ++Index)
    {
        AActor* Actor = Pending[Index];
        if (!IsValid(Actor) || Seen.Contains(Actor))
            continue;
        Seen.Add(Actor);
        if (auto* Group = Actor->FindComponentByClass<UPawnRenderLayerComponent>())
        {
            if (Group->HasBegunPlay())
            {
                RenderGroup = Group;
                Group->RegisterEffect(Effect);
            }
            return;
        }
        Pending.Add(Actor->GetAttachParentActor());
        Pending.Add(Actor->GetOwner());
    }
}

void UPawnRenderLayerEffectComponent::ReleaseEffect()
{
    if (RenderGroup.IsValid() && BoundEffect.IsValid())
        RenderGroup->UnregisterEffect(BoundEffect.Get());
    RenderGroup.Reset();
}

void UPawnRenderLayerEffectComponent::OnNiagaraFinished(UNiagaraComponent* FinishedSystem)
{
    if (FinishedSystem == BoundEffect.Get())
        DestroyComponent();
}

void UPawnRenderLayerEffectComponent::OnParticleFinished(UParticleSystemComponent* FinishedSystem)
{
    if (FinishedSystem == BoundEffect.Get())
        DestroyComponent();
}

void UPawnRenderLayerEffectComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    ReleaseEffect();
    if (auto* Niagara = Cast<UNiagaraComponent>(BoundEffect.Get()))
        Niagara->OnSystemFinished.RemoveDynamic(this, &UPawnRenderLayerEffectComponent::OnNiagaraFinished);
    if (auto* Particle = Cast<UParticleSystemComponent>(BoundEffect.Get()))
        Particle->OnSystemFinished.RemoveDynamic(this, &UPawnRenderLayerEffectComponent::OnParticleFinished);
    Super::OnComponentDestroyed(bDestroyingHierarchy);
}
