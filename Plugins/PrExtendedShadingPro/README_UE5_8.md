# UE 5.8 plugin-only port

This document preserves the donor port's history. Sonic Smackdown's UE5.8.4 integration and validation are recorded in
[MIGRATION.md](../../Docs/Modernization/MIGRATION.md). The temporary material automation source was removed from this
checkout after its five variants passed, following the project's test-file cleanup requirement.

This local port is based on PrExtendedShadingPro 1.5.59 and the installed Unreal Engine 5.8.3 shaders (CL 58210709). All authored changes are inside this plugin. No engine source, project settings, or existing material assets were edited.

## Supported setup

- Desktop forward rendering with Shader Model 5 or higher.
- Legacy materials using Default Lit and the legacy HLSL translator. Leave the material's **Enable New HLSL Generator** option disabled.
- Substrate disabled. This port does not implement the Substrate material pipeline.
- Windows was built and cooked locally. Mac/Linux remain in the descriptor but are untested. Mobile platforms were removed because their shader pipeline is not ported.
- Restart the editor after replacing the plugin. The first load may compile many shaders because the base-pass and depth shader source paths change.

The source descriptor omits a fixed EngineVersion so it can build with both supported engine branches. The actual shader implementations live in `Shaders/Private/5.8.3` and `Shaders/Private/5.5.4`. Earlier patches have no matching implementation; later patches require validation even if the patch fallback finds an existing folder. Compiled binaries must be rebuilt for the engine version that uses them.

The plugin is enabled by default so the project's existing material can load without changing the project descriptor. An explicit project-level disable still takes precedence.

## Changes

The C++ API changes include the new material input traversal API, UE5 version checks, public Engine/CoreUObject dependencies, and desktop-only compile validation. Layer nodes now return their generated custom-expression result so parameter assignments remain connected to the execution chain. Generated line endings are normalized before adding CRLF.

The forward base-pass shader was rebased onto UE5.8.3. The layer implementation uses current large-world coordinates, light-grid data, light functions, reflection and indirect-lighting interfaces. Directional, local, shadow, self-shadow, diffuse, specular, reflection, skylight, and lightmap layers are retained. The legacy custom primitive depth offset is present in both base-pass and depth-only vertex shaders through the shared primitive-data accessor.

The module loads at PostConfigInit and installs shader replacements through the ShaderTypesReady callback after lazy registration and before shader caching. This installation replaces 102 shader types. Cooked runtime builds consume the cooked shaders and do not rewrite shader registrations.

## Limitations

The plugin still relies on the original technique of accessing Unreal's private shader-type filename layout. The accessed prefix has been updated and checked against both public filename accessors before writing. This is a compatibility guard, not a stable engine API; future engine releases require a fresh audit.

Deferred rendering, mobile shaders, Substrate, the new HLSL generator, Nanite-specific shading, ray tracing, and path tracing are not restored by this port. Visual parity, atmospheric/cloud-lighting combinations, stereo rendering, static lighting permutations beyond the existing asset cook, and every custom blend-code combination still need scene testing.

## Validation

- MechaMadnessEditor, Win64 Development: build succeeded.
- MechaMadness, Win64 Development: build succeeded.
- Windows cook of `/Engine/Maps/Entry` and `/Game/Characters/SonicLost/Materials`, including the existing `M_Chr` material: succeeded with 0 errors and 0 warnings. See `Saved/Logs/PrExtendedShaderCook3.log` in the project.
- Automated `PrExtendedShadingPro.Materials.CompileLayers`: passed. Creates in-memory material graphs covering all ten layers in opaque, masked/custom-parameter, and fully rough variants with skeletal mesh usage; checks shader compile errors, renderable shader maps, and a nonzero shader count. No test material assets are saved.
- Editor startup: shader replacements installed without shader-registration initialization errors.

A successful build and cook prove compilation and asset compatibility for the tested paths. They do not prove that all layer combinations reproduce their UE4 appearance. Open the existing Sonic material and inspect directional shadows, self-shadow bands, local lights, reflections, and depth offsets in a scene before treating this as a production-ready replacement.

The UE5.5 extension and its efficiency considerations are documented in [README_UE5_5.md](README_UE5_5.md). The UE5.8.3 shader files are unchanged by that extension.
