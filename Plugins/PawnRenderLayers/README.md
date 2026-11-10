# Pawn Render Layers for Sonic Smackdown

Ported from `F:/UE4 Work/OriginofStorms/Plugins/PawnRenderLayers` for UE 5.8.4 and this project's
PrExtendedShadingPro 5.8.3 shader implementation. The donor was clean on branch `main` when copied.
`README_DONOR_UE5_5.md` preserves the donor's detailed design and historical verification; those UE5.5 results are
not evidence that the UE5.8 port has passed validation.

## Behavior

`UPawnRenderLayerComponent` groups a fighter's conventional meshes, attachments and registered effects. Layer 0 is
the foreground; larger values are farther back. `BringToFront` and `SendToBack` reorder registered groups without
moving actors or changing collision. Normal and transformed forms can use `SetSharedGroup` to share ordering and
projection settings. Groups can independently choose 0–100% projection flattening; 0 retains perspective.

The component discovers owned/attached primitives each tick. Detached, location-spawned effects require registration
with the spawning fighter's component. The plugin preserves custom primitive data outside reserved slots 28–35 and
restores reserved data and translucent sort priority when a primitive leaves its group.

## Shader integration

Editor/cook startup mirrors engine and PrExtendedShadingPro shader sources into the plugin's ignored
`Intermediate/ShaderOverlay/5.8.4` directory. Exact, unique anchors patch the copies for base-pass, full and
position-only depth, and velocity shaders. More-specific virtual shader directories select these overlays.
Engine files and the PrExtendedShadingPro source remain untouched. Packaged builds use shaders generated during cook.

The UE5.8.4 engine anchors and the PrExtendedShadingPro 5.8.3 anchors match the donor's injection locations. Startup
still refuses unsupported engine versions, missing/duplicate anchors and pre-owned virtual directories. No mappings
are installed after a failed preparation. A successful editor/cook log must report
`UE 5.8.4 pawn layer shader overlays enabled`.

## Limits and validation

This is a single-primary-camera perspective renderer integration. Split screen, stereo, Nanite and specialized
rendering paths are outside its scope. Synthetic depth can affect fog, depth fades, decals, screen-space effects and
stage intersections. Original primitive bounds still drive culling and depth fitting. Accurate animated mesh/effect
bounds remain necessary. FXAA avoids the donor's documented temporal-history limitations during ordering changes.

The port retains the donor's projection/depth mathematics. Existing authored vertex deformation, orthographic
flattening and depth bias can conflict with it and must be neutralized for participating primitives. PrExtendedShadingPro
slot 3 is distinct from this plugin's reserved range; the game integration must stop applying its old ordering bias
to opted-in fighters. Niagara and Cascade particle rendering require actual rendered validation.

Native compilation and deferred validation are recorded in `Docs/Modernization` at the project root. At the user's
request, shader, rendering, gameplay and packaged tests are deferred until the old effects are removed. The temporary
copied automation fixtures and opt-in match smoke helper were removed without running them.
