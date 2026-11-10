// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class SonicSmackdown : ModuleRules
{
	public SonicSmackdown(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicIncludePaths.Add(ModuleDirectory);
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "AIModule", "InputCore", "RHI", "RenderCore", "Niagara", "RMAMirrorAnimation", "UMG"});


        PrivateDependencyModuleNames.AddRange(new string[] { "MoveNetworkRuntime", "FrameDataRuntime", "PawnRenderLayers", "EngineSettings", "ApplicationCore" });

        // Uncomment if you are using Slate UI
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
