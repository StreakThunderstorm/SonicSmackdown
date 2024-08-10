// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
#include "RMAMirrorAnimation.h"
#include "UObject/CoreRedirects.h"

void FRMAMirrorAnimation::StartupModule()
{

#if WITH_EDITOR

	//Load The Editor Module
	FModuleManager::Get().LoadModule(TEXT("RMAMirrorAnimationEditor"));

#endif

	TArray<FCoreRedirect> LRedirects;

	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationMirrorTable._Skeleton", "Skeleton"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationMirrorTable._BoneKeyword", "BoneKeyword"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationMirrorTable._SingleBoneConfigList", "SingleBoneConfig"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationMirrorTable._DoubleBoneConfigList", "DoubleBoneConfig"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationMirrorTable._ComponentRightAxis", "ComponentRightAxis"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationMirrorTable._MirrorLocation", "MirrorLocationData"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationMirrorTable._Version", "FileVersion"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationAnimNodeMirrorAnimation._Enabled", "Enabled"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationAnimNodeMirrorAnimation._MirrorTable", "MirrorTable"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationAnimGraphNodeMirrorAnimation._Node", "Node"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationBoneKeyword._Keyword1", "KeywordA"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationBoneKeyword._Keyword2", "KeywordB"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationSingleBoneConfig._BoneName", "BoneName"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationSingleBoneConfig._MirrorAxis", "MirrorAxis"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationDoubleBoneConfig._Bone1Name", "BoneAName"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationDoubleBoneConfig._Bone2Name", "BoneBName"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationDoubleBoneConfig._LocationMirrorAxis", "LocationMirrorAxis"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Property, "RMAMirrorAnimationDoubleBoneConfig._RotationMirrorAxis", "RotationMirrorAxis"));
	LRedirects.Add(FCoreRedirect(ECoreRedirectFlags::Type_Class, "/Script/RMAMirrorAnimation.RMAMirrorAnimationAnimSequence", "/Script/Engine.AnimSequence"));

	FCoreRedirects::AddRedirectList(LRedirects, TEXT("RMAMirrorAnimationRedirects"));

}

void FRMAMirrorAnimation::ShutdownModule()
{



}

IMPLEMENT_MODULE(FRMAMirrorAnimation, RMAMirrorAnimation)
