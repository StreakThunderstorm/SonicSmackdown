using UnrealBuildTool;

public class MoveNetworkRuntime : ModuleRules
{
	public MoveNetworkRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
				"MoveNetwork/Runtime/Private",
				// ... add other private include paths required here ...
			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "InputCore",
				"CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "FrameDataRuntime"
                
               
				// ... add other public dependencies that you statically link with here ...
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...
                //"Slate",
                //"SlateCore",
                "GameplayTags",
           
                
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