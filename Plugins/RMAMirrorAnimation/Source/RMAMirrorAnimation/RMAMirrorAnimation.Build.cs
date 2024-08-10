// Copyright 2017-2020 Rafael Marques Almeida. All Rights Reserved.
using UnrealBuildTool;
using System.IO;

public class RMAMirrorAnimation : ModuleRules
{

	public RMAMirrorAnimation(ReadOnlyTargetRules Target) : base(Target)
	{

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "AnimGraphRuntime", "Projects" });

        if (Target.bBuildEditor == true)
        {

            PublicDependencyModuleNames.AddRange(new string[] { "UnrealEd", "EditorScriptingUtilities" });

        }

    }

}
