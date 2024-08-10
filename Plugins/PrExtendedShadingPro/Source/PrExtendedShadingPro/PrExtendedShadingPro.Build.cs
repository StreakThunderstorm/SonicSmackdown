// Copyright 2021 Prudnikov Nikolay. All Rights Reserved.

using UnrealBuildTool;

public class PrExtendedShadingPro : ModuleRules
{
    public PrExtendedShadingPro(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Projects",
                "RenderCore",
                "RHI",
            }
        );

    }
}
