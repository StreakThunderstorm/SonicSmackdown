using UnrealBuildTool;

public class PawnRenderLayers : ModuleRules
{
    public PawnRenderLayers(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Projects", "RenderCore", "RHI", "Niagara" });
    }
}
