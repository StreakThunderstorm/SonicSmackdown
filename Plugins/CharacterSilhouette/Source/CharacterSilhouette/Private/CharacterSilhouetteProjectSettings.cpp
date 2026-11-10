#include "CharacterSilhouetteProjectSettings.h"
#include "HAL/IConsoleManager.h"

template <typename T>
static void SetSilhouetteVariable(const TCHAR* Name, T Value, bool bOverrideSessionValues)
{
	if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
	{
		if (bOverrideSessionValues)
			Variable->SetWithCurrentPriority(Value);
		else
			Variable->Set(Value, ECVF_SetByProjectSetting);
	}
}

void UCharacterSilhouetteProjectSettings::ApplySettings(FName PropertyName, bool bOverrideSessionValues) const
{
	auto Changed = [PropertyName](FName Name) { return PropertyName.IsNone() || PropertyName == Name; };
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, bEnabled)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.Enable"), int32(bEnabled), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, LineThickness)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.Width"), FMath::Clamp(LineThickness, 0.f, 64.f), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, ReferenceHeight)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.ReferenceHeight"), FMath::Max(ReferenceHeight, 0.f), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, LineColor)))
	{
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.ColorR"), LineColor.R, bOverrideSessionValues);
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.ColorG"), LineColor.G, bOverrideSessionValues);
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.ColorB"), LineColor.B, bOverrideSessionValues);
	}
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, SamplingQuality)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.Samples"), int32(SamplingQuality), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, bHighQualityGeometry)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.Geometry"), int32(bHighQualityGeometry), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, GeometryResolutionScale)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.GeometryScale"), FMath::Clamp(GeometryResolutionScale, 1, 4), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, StencilRangeEnd)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.StencilEnd"), FMath::Clamp(StencilRangeEnd, 0, 255), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, bCharacterOverlapLines)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.Overlaps"), int32(bCharacterOverlapLines), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, bSelfDepthLines)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.SelfDepth"), int32(bSelfDepthLines), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, bInnerEdgeLines)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.InnerEdges"), int32(bInnerEdgeLines), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, InternalLineThickness)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.InnerWidth"), FMath::Clamp(InternalLineThickness, 0.f, 64.f), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, InnerEdgeLineThickness)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.InnerEdgeWidth"), FMath::Clamp(InnerEdgeLineThickness, 0.f, 64.f), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, SelfDepthThreshold)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.SelfDepthThreshold"), FMath::Max(SelfDepthThreshold, 0.01f), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, InnerEdgeAngle)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.InnerEdgeAngle"), FMath::Clamp(InnerEdgeAngle, 1.f, 179.f), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, StencilValue)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.Stencil"), FMath::Clamp(StencilValue, 1, 255), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, bVisibleOnly)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.VisibleOnly"), int32(bVisibleOnly), bOverrideSessionValues);
	if (Changed(GET_MEMBER_NAME_CHECKED(UCharacterSilhouetteProjectSettings, DepthTolerance)))
		SetSilhouetteVariable(TEXT("r.CharacterSilhouette.DepthTolerance"), FMath::Max(DepthTolerance, 0.f), bOverrideSessionValues);
}

#if WITH_EDITOR
void UCharacterSilhouetteProjectSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.MemberProperty)
		ApplySettings(PropertyChangedEvent.MemberProperty->GetFName(), true);
}
#endif
