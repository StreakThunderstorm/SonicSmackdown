#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CharacterSilhouetteProjectSettings.generated.h"

UENUM()
enum class ECharacterSilhouetteSampling : uint8
{
	High = 2 UMETA(DisplayName = "High (2 x 2)"),
	Ultra = 4 UMETA(DisplayName = "Ultra (4 x 4)")
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Character Silhouette"))
class CHARACTERSILHOUETTE_API UCharacterSilhouetteProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }
	virtual FName GetSectionName() const override { return TEXT("CharacterSilhouette"); }
	void ApplySettings(FName PropertyName = NAME_None, bool bOverrideSessionValues = false) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(Config, EditAnywhere, Category = "Outline", meta = (DisplayName = "Enable Outlines"))
	bool bEnabled = false;

	// Outward width at ReferenceHeight, independent of internal screen percentage.
	UPROPERTY(Config, EditAnywhere, Category = "Outline", meta = (ClampMin = "0", ClampMax = "64", UIMin = "0", UIMax = "12"))
	float LineThickness = 3.f;

	// Zero means a fixed output-pixel thickness; 1080 maintains the same visual weight across output resolutions.
	UPROPERTY(Config, EditAnywhere, Category = "Outline", meta = (ClampMin = "0", UIMin = "0", UIMax = "4320"))
	float ReferenceHeight = 1080.f;

	UPROPERTY(Config, EditAnywhere, Category = "Outline", meta = (HideAlphaChannel))
	FLinearColor LineColor = FLinearColor::Black;

	UPROPERTY(Config, EditAnywhere, Category = "Quality")
	ECharacterSilhouetteSampling SamplingQuality = ECharacterSilhouetteSampling::High;

	// Renders the selected character geometry independently of the scene's screen percentage.
	UPROPERTY(Config, EditAnywhere, Category = "Quality")
	bool bHighQualityGeometry = true;

	UPROPERTY(Config, EditAnywhere, Category = "Quality", meta = (ClampMin = "1", ClampMax = "4"))
	int32 GeometryResolutionScale = 2;

	UPROPERTY(Config, EditAnywhere, Category = "Character Selection", meta = (ClampMin = "1", ClampMax = "255"))
	int32 StencilValue = 1;

	// Zero selects only StencilValue. A higher value includes that inclusive stencil range.
	UPROPERTY(Config, EditAnywhere, Category = "Character Selection", meta = (ClampMin = "0", ClampMax = "255"))
	int32 StencilRangeEnd = 0;

	// Meshes on the same actor share an identity even when multiple actors use the same stencil value.
	UPROPERTY(Config, EditAnywhere, Category = "Character Overlaps")
	bool bCharacterOverlapLines = true;

	UPROPERTY(Config, EditAnywhere, Category = "Internal Lines")
	bool bSelfDepthLines = true;

	UPROPERTY(Config, EditAnywhere, Category = "Internal Lines")
	bool bInnerEdgeLines = true;

	UPROPERTY(Config, EditAnywhere, Category = "Internal Lines", meta = (DisplayName = "Self Depth Line Thickness",
		ClampMin = "0", ClampMax = "64", UIMax = "6"))
	float InternalLineThickness = 1.25f;

	UPROPERTY(Config, EditAnywhere, Category = "Internal Lines", meta = (ClampMin = "0", ClampMax = "64", UIMax = "6"))
	float InnerEdgeLineThickness = 1.25f;

	// Minimum separation from the neighboring surface tangent planes, in Unreal world units (normally cm).
	UPROPERTY(Config, EditAnywhere, Category = "Internal Lines", meta = (ClampMin = "0.01", Units = "cm"))
	float SelfDepthThreshold = 2.f;

	UPROPERTY(Config, EditAnywhere, Category = "Internal Lines", meta = (ClampMin = "1", ClampMax = "179", Units = "deg"))
	float InnerEdgeAngle = 45.f;

	// Visibility tests apply to the strokes, never to the contour mask.
	UPROPERTY(Config, EditAnywhere, Category = "Visibility")
	bool bVisibleOnly = true;

	UPROPERTY(Config, EditAnywhere, Category = "Visibility", AdvancedDisplay, meta = (ClampMin = "0", UIMin = "0", UIMax = "0.0001"))
	float DepthTolerance = 0.000001f;
};
