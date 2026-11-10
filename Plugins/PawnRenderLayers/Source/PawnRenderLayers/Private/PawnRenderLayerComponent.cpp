#include "PawnRenderLayerComponent.h"
#include "PawnRenderLayerSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"

namespace PawnRenderLayers
{
    constexpr int32 FirstDataSlot = 28;
    constexpr float Marker = 7319.f;

    bool IsEffectPlaying(const UFXSystemComponent* Effect)
    {
        const auto* Niagara = Cast<UNiagaraComponent>(Effect);
        return Effect && Effect->IsActive() && (!Niagara || !Niagara->IsComplete()) && !Effect->IsReadyForOwnerToAutoDestroy();
    }
}

UPawnRenderLayerComponent::UPawnRenderLayerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UPawnRenderLayerComponent::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Subsystem = GetLayerSubsystem())
        Subsystem->RegisterGroup(this);
    RefreshGroup();
    UpdateProjection();
}

void UPawnRenderLayerComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    for (const auto& Entry : Members)
        if (Entry.Key.IsValid())
            RestorePrimitive(Entry.Key.Get(), Entry.Value);

    Members.Reset();
    Effects.Reset();
    ReleasedEffects.Reset();
    if (auto* Subsystem = GetLayerSubsystem())
        Subsystem->UnregisterGroup(this);

    Super::EndPlay(Reason);
}

void UPawnRenderLayerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
    Super::TickComponent(DeltaTime, TickType, TickFunction);
    RefreshGroup();
    UpdateProjection();
}

UPawnRenderLayerSubsystem* UPawnRenderLayerComponent::GetLayerSubsystem() const
{
    UWorld* World = GetWorld();
    // Asset editor preview worlds can tick pawn components without creating world subsystems.
    return World && !World->IsPreviewWorld() ? World->GetSubsystem<UPawnRenderLayerSubsystem>() : nullptr;
}

UPawnRenderLayerComponent* UPawnRenderLayerComponent::GetRenderGroup() const
{
    auto* Current = const_cast<UPawnRenderLayerComponent*>(this);
    TSet<const UPawnRenderLayerComponent*> Visited;
    while (Current && !Visited.Contains(Current))
    {
        Visited.Add(Current);
        UPawnRenderLayerComponent* Parent = Current->SharedGroup.Get();
        if (!Parent && Current->bInheritParentGroup)
        {
            TArray<AActor*> Pending;
            if (AActor* Actor = Current->GetOwner())
            {
                Pending.Add(Actor->GetAttachParentActor());
                Pending.Add(Actor->GetOwner());
            }
            TSet<AActor*> Seen;
            for (int32 Index = 0; Index < Pending.Num() && !Parent; ++Index)
            {
                AActor* Actor = Pending[Index];
                if (!IsValid(Actor) || Seen.Contains(Actor) || Actor == Current->GetOwner())
                    continue;
                Seen.Add(Actor);
                Parent = Actor->FindComponentByClass<UPawnRenderLayerComponent>();
                Pending.Add(Actor->GetAttachParentActor());
                Pending.Add(Actor->GetOwner());
            }
        }
        if (!IsValid(Parent) || Parent->GetWorld() != GetWorld())
            return Current;
        Current = Parent;
    }
    // A malformed owner/attachment cycle must not recurse indefinitely.
    return const_cast<UPawnRenderLayerComponent*>(this);
}

bool UPawnRenderLayerComponent::SetSharedGroup(UPawnRenderLayerComponent* Group)
{
    if (Group && (!IsValid(Group) || Group->GetWorld() != GetWorld() || Group->GetRenderGroup() == this))
        return false;
    const auto PreviousGroup = SharedGroup;
    SharedGroup = Group;
    if (Group && GetRenderGroup() == this)
    {
        SharedGroup = PreviousGroup;
        return false;
    }
    if (auto* Subsystem = GetLayerSubsystem())
        Subsystem->UpdateGroup(GetRenderGroup());
    return true;
}

int32 UPawnRenderLayerComponent::GetLayer() const
{
    return GetRenderGroup()->Layer;
}

float UPawnRenderLayerComponent::GetFlatteningPercent() const
{
    return GetRenderGroup()->FlatteningPercent;
}

void UPawnRenderLayerComponent::SetLayer(int32 InLayer)
{
    auto* Root = GetRenderGroup();
    const int32 NewLayer = FMath::Clamp(InLayer, 0, 200);
    if (Root->Layer == NewLayer)
        return;
    Root->Layer = NewLayer;
    if (auto* Subsystem = GetLayerSubsystem())
        Subsystem->UpdateGroup(Root);
}

void UPawnRenderLayerComponent::SetFlatteningPercent(float InPercent)
{
    auto* Root = GetRenderGroup();
    Root->FlatteningPercent = FMath::IsFinite(InPercent) ? FMath::Clamp(InPercent, 0.f, 100.f) : 0.f;
    if (auto* Subsystem = GetLayerSubsystem())
        Subsystem->UpdateGroup(Root);
}

void UPawnRenderLayerComponent::SetGroupLayeringEnabled(bool bEnabled)
{
    auto* Root = GetRenderGroup();
    Root->bGroupLayeringEnabled = bEnabled;
    if (auto* Subsystem = GetLayerSubsystem())
        Subsystem->UpdateGroup(Root);
}

void UPawnRenderLayerComponent::BringToFront()
{
    if (auto* Subsystem = GetLayerSubsystem())
        Subsystem->BringToFront(this);
}

void UPawnRenderLayerComponent::SendToBack()
{
    if (auto* Subsystem = GetLayerSubsystem())
        Subsystem->SendToBack(this);
}

void UPawnRenderLayerComponent::RegisterEffect(UPrimitiveComponent* Effect)
{
    if (!IsValid(Effect))
        return;

    ReleasedEffects.Remove(Effect);
    Effects.Add(Effect);
    RefreshGroup();
    if (auto* Subsystem = GetLayerSubsystem())
        Subsystem->UpdateGroup(GetRenderGroup());
}

void UPawnRenderLayerComponent::UnregisterEffect(UPrimitiveComponent* Effect)
{
    Effects.Remove(Effect);
    if (IsValid(Effect))
    {
        const auto* FX = Cast<UFXSystemComponent>(Effect);
        ReleasedEffects.Add(Effect, FX && !PawnRenderLayers::IsEffectPlaying(FX));
    }
    RefreshGroup();
    if (auto* Subsystem = GetLayerSubsystem())
        Subsystem->UpdateGroup(GetRenderGroup());
}

bool UPawnRenderLayerComponent::ContainsPrimitive(const UPrimitiveComponent* Primitive) const
{
    return Members.Contains(TWeakObjectPtr<UPrimitiveComponent>(const_cast<UPrimitiveComponent*>(Primitive)));
}

void UPawnRenderLayerComponent::RefreshGroup()
{
    auto* Subsystem = GetLayerSubsystem();
    if (!GetOwner() || !Subsystem)
        return;

    TArray<AActor*> Actors;
    Actors.Add(GetOwner());
    TSet<TWeakObjectPtr<UPrimitiveComponent>> Desired;
    TSet<AActor*> Seen;
    for (int32 Index = 0; Index < Actors.Num(); ++Index)
    {
        AActor* Actor = Actors[Index];
        if (!IsValid(Actor) || Seen.Contains(Actor))
            continue;
        Seen.Add(Actor);
        const auto* ActorGroup = Actor->FindComponentByClass<UPawnRenderLayerComponent>();
        // Each child component manages its own primitives while sharing the root's settings.
        if (ActorGroup && ActorGroup != this)
            continue;
        TInlineComponentArray<UPrimitiveComponent*> Components(Actor, false);
        for (UPrimitiveComponent* Primitive : Components)
            Desired.Add(Primitive);
        // Socket-spawned components can belong to another actor (including the world settings actor).
        // Follow component attachments as well as actor attachments to discover those effects.
        TInlineComponentArray<USceneComponent*> AttachedComponents(Actor, false);
        TSet<USceneComponent*> SeenComponents;
        for (int32 ComponentIndex = 0; ComponentIndex < AttachedComponents.Num(); ++ComponentIndex)
        {
            USceneComponent* Component = AttachedComponents[ComponentIndex];
            if (!IsValid(Component) || SeenComponents.Contains(Component))
                continue;
            SeenComponents.Add(Component);
            AActor* ComponentOwner = Component->GetOwner();
            const auto* ComponentGroup = ComponentOwner ? ComponentOwner->FindComponentByClass<UPawnRenderLayerComponent>() : nullptr;
            if (ComponentGroup && ComponentGroup != this)
                continue;
            if (auto* Primitive = Cast<UPrimitiveComponent>(Component))
                Desired.Add(Primitive);
            AttachedComponents.Append(Component->GetAttachChildren());
        }
        if (bIncludeAttachedActors)
        {
            Actor->GetAttachedActors(Actors, false, false);
            Actors.Append(Actor->Children);
        }
    }

    for (auto It = Effects.CreateIterator(); It; ++It)
        if (It->IsValid())
            Desired.Add(*It);
        else
            It.RemoveCurrent();

    // Automatically reclaim an owned/socket-attached effect when a pool or Blueprint activates it again.
    // Unrelated location-spawned effects still need their spawning source registered.
    for (auto It = ReleasedEffects.CreateIterator(); It; ++It)
        if (It.Key().IsValid())
        {
            const auto* FX = Cast<UFXSystemComponent>(It.Key().Get());
            const bool bPlaying = PawnRenderLayers::IsEffectPlaying(FX);
            // A manual unregister during playback remains excluded until a stopped state is observed.
            if (FX && !bPlaying)
                It.Value() = true;
            if (Desired.Contains(It.Key()) && It.Value() && bPlaying)
                It.RemoveCurrent();
            else
                Desired.Remove(It.Key());
        }
        else
            It.RemoveCurrent();

    for (auto It = Members.CreateIterator(); It; ++It)
        if (!It.Key().IsValid() || !Desired.Contains(It.Key()))
        {
            if (It.Key().IsValid())
                RestorePrimitive(It.Key().Get(), It.Value());
            It.RemoveCurrent();
        }

    for (const auto& Entry : Desired)
    {
        UPrimitiveComponent* Primitive = Entry.Get();
        if (!IsValid(Primitive) || Members.Contains(Entry) || Subsystem->IsClaimedByOtherGroup(Primitive, this))
            continue;

        const TArray<float>& Data = Primitive->GetCustomPrimitiveData().Data;
        bool bConflict = false;
        for (int32 Index = PawnRenderLayers::FirstDataSlot; Index < 36; ++Index)
            bConflict |= Data.IsValidIndex(Index) && Data[Index] != 0.f;

        if (bConflict)
        {
            if (!bWarnedCustomDataConflict)
                UE_LOG(LogTemp, Warning, TEXT("PawnRenderLayers skipped occupied custom primitive data slots 28-35 on %s."),
                    *Primitive->GetPathName());
            bWarnedCustomDataConflict = true;
            continue;
        }

        FPawnRenderLayerOriginalState State;
        State.ReservedData.SetNumZeroed(8);
        for (int32 Index = 0; Index < 8; ++Index)
            if (Data.IsValidIndex(PawnRenderLayers::FirstDataSlot + Index))
                State.ReservedData[Index] = Data[PawnRenderLayers::FirstDataSlot + Index];
        State.TranslucencyPriority = Primitive->TranslucencySortPriority;
        Members.Add(Entry, MoveTemp(State));
    }
}

void UPawnRenderLayerComponent::RestorePrimitive(UPrimitiveComponent* Primitive, const FPawnRenderLayerOriginalState& State)
{
    for (int32 Index = 0; Index < State.ReservedData.Num(); ++Index)
        Primitive->SetCustomPrimitiveDataFloat(PawnRenderLayers::FirstDataSlot + Index, State.ReservedData[Index]);
    Primitive->SetTranslucentSortPriority(State.TranslucencyPriority);
}

void UPawnRenderLayerComponent::UpdateProjection()
{
    UWorld* World = GetWorld();
    auto* Subsystem = GetLayerSubsystem();
    if (!World || !GetOwner() || !Subsystem)
        return;

    const auto* Root = GetRenderGroup();
    const APlayerController* Controller = World->GetFirstPlayerController();
    const APlayerCameraManager* Camera = Controller ? Controller->PlayerCameraManager : nullptr;
    if (Camera)
        ReferenceDistance = FMath::Max(1.f, FVector::DotProduct(Subsystem->GetProjectionAnchor(Root)->GetActorLocation() - Camera->GetCameraLocation(),
            Camera->GetCameraRotation().Vector()));

    const FVector2D Band = Subsystem->GetDepthBand(Root->Layer);
    const FVector CameraForward = Camera ? Camera->GetCameraRotation().Vector() : FVector::ForwardVector;
    const FVector2D DepthRange = Subsystem->GetDepthRange(Root, ReferenceDistance, CameraForward);
    // Slot 31 carries the near distance; its sign encodes whether group layering is enabled.
    const FVector4 Settings(PawnRenderLayers::Marker, FMath::Clamp(Root->FlatteningPercent, 0.f, 100.f) * 0.01f,
        FMath::Clamp(Root->Layer, 0, 200), Root->bGroupLayeringEnabled ? DepthRange.X : -DepthRange.X);
    for (const auto& Entry : Members)
        if (UPrimitiveComponent* Primitive = Entry.Key.Get())
        {
            const FVector4 Projection(Primitive->IsA<UFXSystemComponent>() ? -ReferenceDistance : ReferenceDistance, Band.X, Band.Y, DepthRange.Y);
            Primitive->SetCustomPrimitiveDataVector4(28, Settings);
            Primitive->SetCustomPrimitiveDataVector4(32, Projection);
            Primitive->SetTranslucentSortPriority(Root->bGroupLayeringEnabled ? (201 - Root->Layer) * 1000 : Entry.Value.TranslucencyPriority);
        }
}
