// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class OriginOfStormsMasterEditorTarget : TargetRules
{
	public OriginOfStormsMasterEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange(new string[] { "OriginOfStormsMaster" } );
        ExtraModuleNames.AddRange(new string[] { "MoveNetworkEditor" });
        ExtraModuleNames.AddRange(new string[] { "FrameDataEditor" });


    }
}
