// Copyright 2017-2023 Rafael Marques Almeida. All Rights Reserved.
using UnrealBuildTool;
using System.IO;

public class RMAMirrorAnimationEditor : ModuleRules
{

	public RMAMirrorAnimationEditor(ReadOnlyTargetRules Target) : base(Target)
	{

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "Slate", "SlateCore", "UMG", "UnrealEd", "BlueprintGraph", "AnimGraph", "AssetTools", 
			"AssetRegistry", "Blutility", "EditorWidgets", "AnimationDataController", "AnimationBlueprintLibrary", "RMAMirrorAnimation", "ScriptableEditorWidgets" });

    }

}
