// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class SonicSmackdownEditorTarget : TargetRules
{
	public SonicSmackdownEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;

		ExtraModuleNames.AddRange(new string[] { "SonicSmackdown" } );
        ExtraModuleNames.AddRange(new string[] { "MoveNetworkEditor" });
        ExtraModuleNames.AddRange(new string[] { "FrameDataEditor" });


    }
}
