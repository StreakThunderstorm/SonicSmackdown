// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "OOSCapsuleComponent.generated.h"


UCLASS(meta = (DisplayName = "2.5D Capsule Collision", BlueprintSpawnableComponent))
class ORIGINOFSTORMSMASTER_API UOOSCapsuleComponent : public UCapsuleComponent
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY()	UMaterialInterface* FaceMat;
	UPROPERTY() UMaterialInterface* WireMat;

	UPROPERTY(Category = GameStats, EditAnywhere, BlueprintReadWrite) bool bDebug = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	FLinearColor DebugColor;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual void PostLoad() override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;

	// Debug mesh buffers
	TArray<uint32> Indices;
	TArray<FVector> Vertices;
	TArray<FVector> Normals;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	// Called every frame.
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called when parent transform is updated.
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;

private:

	void CreateDebugMesh();
};
