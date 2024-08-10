// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class OriginOfStormsMasterTarget : TargetRules
{
	public OriginOfStormsMasterTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "OriginOfStormsMaster" } );
        ExtraModuleNames.AddRange(new string[] { "MoveNetworkRuntime" });
        ExtraModuleNames.AddRange(new string[] { "FrameDataRuntime" });

    }
}
