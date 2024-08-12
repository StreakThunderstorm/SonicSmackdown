using UnrealBuildTool;

public class FrameDataEditor : ModuleRules
{
	public FrameDataEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
                // ... add other private include paths required here ...
                "FrameData/Editor/Private",

			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
                "Engine",
                "AnimGraph",
                "OriginOfStormsMaster",
                "MoveNetworkRuntime"
				// ... add other public dependencies that you statically link with here ...
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "AssetTools",
                "GraphEditor",
                "Slate",
                "SlateCore",
                "InputCore",
                "PropertyEditor",
                "EditorStyle",
                "EditorWidgets",
                "Kismet",
                "KismetWidgets",
                "ApplicationCore",
                "FrameDataRuntime",
                "UnrealEd",
                "Engine",
                "AdvancedPreviewScene",
               
             
				// ... add private dependencies that you statically link with here ...
			}
			);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
            }
			);
	}
}