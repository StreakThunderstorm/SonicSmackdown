#include "PawnRenderLayerSubsystem.h"
#include "PawnRenderLayerComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

void UPawnRenderLayerSubsystem::RegisterGroup(UPawnRenderLayerComponent* Group)
{
    Groups.RemoveAll([](const auto& Entry) { return !Entry.IsValid(); });
    Groups.AddUnique(Group);
    if (Group)
        Group->Layer = FMath::Clamp(Group->Layer, 0, 200);
    if (!Group || !Group->bAutoAssignLayer || Group->GetRenderGroup() != Group)
    {
        UpdateGroup(nullptr);
        return;
    }
    TSet<int32> Occupied;
    for (const auto& Entry : Groups)
        if (Entry.IsValid() && Entry.Get() != Group && Entry->GetRenderGroup() != Group)
            Occupied.Add(Entry->GetLayer());
    int32 Candidate = FMath::Clamp(Group->Layer, 0, 200);
    while (Candidate <= 200 && Occupied.Contains(Candidate))
        ++Candidate;
    if (Candidate > 200)
    {
        Candidate = 0;
        while (Candidate <= 200 && Occupied.Contains(Candidate))
            ++Candidate;
    }
    if (Candidate <= 200)
        Group->SetLayer(Candidate);
    else
        UE_LOG(LogTemp, Warning, TEXT("PawnRenderLayers exhausted the 201 available layer indices."));
}

void UPawnRenderLayerSubsystem::UnregisterGroup(UPawnRenderLayerComponent* Group)
{
    Groups.Remove(Group);
    UpdateGroup(nullptr);
}

bool UPawnRenderLayerSubsystem::IsClaimedByOtherGroup(const UPrimitiveComponent* Primitive, const UPawnRenderLayerComponent* Group) const
{
    for (const auto& Entry : Groups)
        if (Entry.IsValid() && Entry.Get() != Group && Entry->ContainsPrimitive(Primitive))
            return true;

    return false;
}

void UPawnRenderLayerSubsystem::UpdateGroup(UPawnRenderLayerComponent* Group)
{
    for (const auto& Entry : Groups)
        if (Entry.IsValid())
            Entry->UpdateProjection();
}

FVector2D UPawnRenderLayerSubsystem::GetDepthBand(int32 Layer) const
{
    TArray<int32> Layers;
    for (const auto& Entry : Groups)
        if (Entry.IsValid())
            Layers.AddUnique(Entry->GetLayer());
    Layers.AddUnique(Layer);
    // Keep the zero-depth reference even when no group currently occupies layer zero.
    Layers.AddUnique(0);
    Layers.Sort();
    const float NearClip = FMath::Max(GNearClippingPlane, 1.f);
    const float Plane = FMath::IsFinite(LayerPlaneDistance) ? FMath::Max(LayerPlaneDistance, NearClip * 2.f) : NearClip * 2.f;
    const float FarDepth = NearClip / Plane;
    const float Spacing = (0.95f - FarDepth) / Layers.Num();
    // Layer zero is the foreground reference. Each later occupied layer moves farther into the protected range.
    // Keep the foreground edge fixed as groups join or leave, and reserve a gap behind each group's band.
    const float NearDepth = 0.95f - Spacing * Layers.IndexOfByKey(Layer);
    const float Width = Spacing * 0.9f;
    return FVector2D(NearDepth - Width, Width);
}

AActor* UPawnRenderLayerSubsystem::GetProjectionAnchor(const UPawnRenderLayerComponent* Group) const
{
    AActor* Anchor = Group->GetOwner();
    if (Anchor && !Anchor->IsHidden())
        return Anchor;
    // Sonic swaps normal/transformed actors while retaining one render group.
    for (const auto& Entry : Groups)
        if (Entry.IsValid() && Entry->GetRenderGroup() == Group && Entry->GetOwner() && !Entry->GetOwner()->IsHidden())
            return Entry->GetOwner();
    return Anchor;
}

FVector2D UPawnRenderLayerSubsystem::GetDepthRange(const UPawnRenderLayerComponent* Group,
    float ReferenceDistance, const FVector& CameraForward) const
{
    float NearDistance = ReferenceDistance;
    float FarDistance = ReferenceDistance;
    const FVector Anchor = GetProjectionAnchor(Group)->GetActorLocation();
    const FVector AxisExtent = CameraForward.GetAbs();
    for (const auto& Entry : Groups)
        if (Entry.IsValid() && Entry->GetRenderGroup() == Group)
            for (const auto& Member : Entry->Members)
                if (const UPrimitiveComponent* Primitive = Member.Key.Get())
                {
                    if (!Primitive->ShouldRender())
                        continue;
                    const float Center = ReferenceDistance + FVector::DotProduct(Primitive->Bounds.Origin - Anchor, CameraForward);
                    const float Extent = FVector::DotProduct(Primitive->Bounds.BoxExtent, AxisExtent);
                    NearDistance = FMath::Min(NearDistance, Center - Extent);
                    FarDistance = FMath::Max(FarDistance, Center + Extent);
                }
    // Project the group's actual bounds onto the camera axis instead of reserving an oversized sphere around its root.
    const float Padding = FMath::Max((FarDistance - NearDistance) * 0.01f, 1.f);
    NearDistance = FMath::Max(NearDistance - Padding, 1.f);
    return FVector2D(NearDistance, FMath::Max(FarDistance + Padding, NearDistance + 1.f));
}

void UPawnRenderLayerSubsystem::BringToFront(UPawnRenderLayerComponent* Group)
{
    if (!IsValid(Group))
        return;

    Group = Group->GetRenderGroup();
    TArray<UPawnRenderLayerComponent*> Ordered;
    for (const auto& Entry : Groups)
        if (Entry.IsValid() && Entry->GetRenderGroup() != Group)
            Ordered.AddUnique(Entry->GetRenderGroup());

    Ordered.StableSort([](const auto& A, const auto& B) { return A.GetLayer() < B.GetLayer(); });
    Ordered.Insert(Group, 0);
    for (int32 Index = 0; Index < Ordered.Num(); ++Index)
        Ordered[Index]->SetLayer(Index);
}

void UPawnRenderLayerSubsystem::SendToBack(UPawnRenderLayerComponent* Group)
{
    if (!IsValid(Group))
        return;

    Group = Group->GetRenderGroup();
    TArray<UPawnRenderLayerComponent*> Ordered;
    for (const auto& Entry : Groups)
        if (Entry.IsValid() && Entry->GetRenderGroup() != Group)
            Ordered.AddUnique(Entry->GetRenderGroup());

    Ordered.StableSort([](const auto& A, const auto& B) { return A.GetLayer() < B.GetLayer(); });
    Ordered.Add(Group);
    for (int32 Index = 0; Index < Ordered.Num(); ++Index)
        Ordered[Index]->SetLayer(Index);
}
