using UnrealBuildTool;
using System.IO;

public class CharacterSilhouette : ModuleRules
{
	public CharacterSilhouette(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "Engine", "RenderCore", "Renderer", "RHI", "DeveloperSettings" });
		PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "Projects" });
		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(EngineDirectory, "Source/Runtime/Renderer/Private"),
			Path.Combine(EngineDirectory, "Source/Runtime/Renderer/Internal")
		});
	}
}
