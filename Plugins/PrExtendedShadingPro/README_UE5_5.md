# PrExtendedShadingPro support for Unreal Engine 5.5

This is historical documentation from the donor port. For Sonic Smackdown's UE5.8.4 integration, see
[MIGRATION.md](../../Docs/Modernization/MIGRATION.md). Temporary validation sources and fixtures are not retained in this checkout.

The plugin now contains a shader implementation for UE 5.5.4 (CL 40574608), alongside the existing UE 5.8.3 implementation. The engine version selects the matching shader folder automatically. This extends the plugin without migrating the MechaMadness project or editing engine source.

## Supported rendering

Use desktop forward rendering, legacy Default Lit materials, and the legacy material HLSL translator. Disable Substrate and the material's Enable New HLSL Generator option. The ten existing layer types and custom blend parameters remain available, along with the custom primitive depth offset in the base and depth passes.

Windows is the validation platform. Deferred rendering, mobile, Substrate, Nanite-specific shading, and ray/path tracing are outside this port. Earlier 5.5 patches have no matching shader folder; later patches require validation even when the patch fallback selects 5.5.4.

## Efficiency

The custom base-pass branch replaces standard lighting instead of evaluating both paths. Directional and local lighting share the shadow-mask sample and use one local-light traversal. Materials without dynamic-light layers compile out the local-light loop; materials without a reflection layer compile out reflection sampling. Lightmap and skylight calculations use their existing layer flags, while preserving baked skylight occlusion coordinates.

The shader compiler can eliminate other layer results not consumed by the generated blend functions. Fog, atmosphere transmittance, lighting channels, and light functions remain part of the supported forward path. The primitive depth offset retains its original shared vertex-shader behavior and its small primitive-data read and position addition.

Compiler instruction counts are a check for unnecessary shader work, not a GPU frame-time benchmark. Scene performance still depends on the connected layers, custom blend code, lights, and target hardware.

## Loading and builds

The module loads at PostConfigInit and defers filename replacement to ShaderTypesReady, after shader registration and before caching. Dynamic module reload is disabled because shader filenames refer to storage owned by the module; restart the editor after updating the plugin.

The source descriptor omits a fixed EngineVersion so both supported engine branches can build the plugin. Compiled binaries belong to the engine version that built them. Rebuild the plugin when copying it to a 5.5 project; do not reuse 5.8 binaries. The existing private shader filename layout technique remains guarded against layout mismatches and requires review for future engine releases.

Validation uses separate generated host projects under Saved/PrExtendedUE55Validation and Saved/PrExtendedUE58Validation. Material fixtures are kept in memory unless the isolated test is explicitly launched with -PrSaveCompilationFixtures for cook validation.

## Validation results

Windows editor, development runtime, and shipping runtime plugin builds passed under UE 5.5.4. The material regression passed with all ten layers in opaque, masked/custom-parameter, and fully rough materials, plus diffuse-only and stock control materials. The shared plugin also passed editor/development/shipping builds and material regression under UE 5.8.3.

With the same constant material inputs and skeletal-mesh usage, the maximum compiler-reported base-pass pixel instruction counts in the UE 5.5 SM5 test were 1,243 for the all-layer opaque stack, 351 for diffuse only, and 1,236 for the stock control. This confirms pruning of unused lighting in the simple stack; the full test stack adds seven reported instructions over the control. These are static compiler counts, not measured GPU timings or a guarantee for arbitrary custom blend code.

All five saved material fixtures also passed a targeted Windows cook with zero commandlet errors or warnings. The cook output is under Saved/PrExtendedUE55FixtureCook, with its log at Saved/Logs/PrExtendedUE55FixtureCook.log. This validates the plugin fixtures; the game project was not migrated or cooked.

The offscreen D3D12 test completed successfully. The UE5.5 D3D11 offscreen harness crashed in Slate/D3D11 initialization before material testing; use D3D12 for this automated harness. Visual matching to the original UE4 plugin remains subject to scene testing.
