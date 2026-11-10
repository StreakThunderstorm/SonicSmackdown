# Character Silhouette (UE5.8)

Standalone character silhouette and internal-line renderer. Existing plugins, character assets, and materials are not modified.
The plugin loads by default in this project; its automatic rendering adapter is opt-in and defaults to off.

## Project Settings

After building, restart the editor and open **Edit > Project Settings > Game > Character Silhouette**.
Enable **Enable Outlines** and adjust **Line Thickness**. Changes apply immediately; Unreal's settings panel saves project defaults.
**Reference Height** defaults to 1080: a thickness of 3 produces three output pixels at 1080p and six at 2160p.
Set Reference Height to 0 for a fixed output-pixel width. Internal screen percentage does not change the requested width.
The panel also exposes line color, sampling quality, stencil selection, and foreground visibility.
Lines are opaque at full coverage; smooth antialiasing is retained at their edges. Color alpha and old opacity settings are ignored.
**High Quality Geometry** is enabled by default. **Geometry Resolution Scale** defaults to 2, rendering character geometry
at twice the final view width and height regardless of the scene's internal screen percentage.
Meshes must have Render CustomDepth Pass enabled and a stencil value within the selected range. Stencil Range End = 0 selects only Stencil Value.
**Character Overlap Lines** separates characters even when they share the same stencil. Components on one actor share one identity;
attached or pawn-owned actor components are grouped with their owning pawn.
**Self Depth Lines** detects self-occlusion; **Inner Edge Lines** detects geometric normal creases.
**Self Depth Line Thickness** and **Inner Edge Line Thickness** independently tune the two interior line families.
The existing InternalLineThickness config value now controls Self Depth Line Thickness. **Self Depth Threshold** (world units)
and **Inner Edge Angle** control which discontinuities are drawn.
Internal thickness is the full centered line width; the exterior thickness measures only the outward stroke.

## Dedicated character geometry

The high-quality path draws visible, selected mesh batches into independent depth, geometry-normal, and actor-identity targets.
It uses the current view's geometry and projection in the same frame, including skeletal skinning, material world-position offset,
masked cutouts, and pixel-depth offset. It does not move a separate scene-capture camera or resize the scene's binary stencil mask.
The high-resolution coverage feeds the existing marching-squares contour and analytic supersampled stroke renderer.
Visible actor-identity transitions generate overlap lines; only the nearest selected surface participates.
Self-depth lines use surface tangent-plane residuals so continuous slopes do not become depth contours.
Physical surface positions are retained independently of render depth. The pass reads existing PrExtendedShadingPro depth bias and
PawnRenderLayers projection when those plugins are active, without modifying either plugin.
Inner edges use geometry normals rather than normal-map details, avoiding texture-frequency line noise.
All line families composite after opaque-scene fog and clouds, before translucency so Niagara retains its normal material blending.
Existing fog and intentional post-process overlays retain their normal rendering behavior.
`CharacterSilhouetteLines` is a native UE5.8 User Scene Texture containing the actual visible line coverage in **alpha**.
An optional post-process fog material can multiply its final fog blend amount by `1 - CharacterSilhouetteLines.a`, protecting full line coverage
and retaining partial antialiased edges. The donor project used `M_DepthFog` for this; this port does not copy or alter that material.
The mask does not expand the character stencil or exclude a ring of background around it.
Alpha is used because Unreal automatically exposure-corrects RGB samples in post-process materials; data in alpha is unchanged.
The mask contains exterior, overlap, self-depth and crease lines. It belongs to the current render graph and view family, so captures
have independent masks and there is no persistent render target or previous-frame history. Disabled outlines publish a zero mask and leave fog unchanged.
Other post-process materials, including super move color overlays, continue to affect the scene normally. No earlier scene color is restored.

This path supports conventional desktop skeletal and static mesh batches; Nanite geometry is excluded.
Weapon and armor actors attached to or owned by a pawn share its identity. Unowned actors receive separate identities.
Source geometry still uses the LOD and visible mesh batches selected by the scene; it does not force the highest asset LOD.
Material-color boundaries and painted details are not extracted. Geometry normals and depth determine the internal lines.
The source targets cap at the device texture dimension and 64 million pixels, reducing resolution proportionally when necessary.
Raising geometry resolution and stroke sampling increases GPU time and memory; inspect performance at the intended output resolution.

## Try the stencil adapter

Restart the editor after building the plugin. On the character's participating mesh components, enable **Render CustomDepth Pass**
and choose a **CustomDepth Stencil Value**. The project already enables custom depth with stencil.
Use that value below (example: 1). These console commands change the current session only:

```text
r.CharacterSilhouette.Stencil 1
r.CharacterSilhouette.Width 3
r.CharacterSilhouette.ReferenceHeight 1080
r.CharacterSilhouette.Samples 4
r.CharacterSilhouette.Enable 1
```

The example produces a three-pixel outward stroke at 1080p, six pixels at 2160p, with 4x4 stroke sampling.
Set `ReferenceHeight 0` for a fixed number of output pixels instead. Disable with `r.CharacterSilhouette.Enable 0`.
Use `r.CharacterSilhouette.Debug 1` to inspect the source mask and `Debug 2` to inspect the untrimmed contour stroke.
Restore `Debug 0` for the exterior stroke. Existing outline effects should be disabled in the test view for a fair comparison;
this plugin does not disable them automatically.

Other controls: `ColorR`, `ColorG`, `ColorB`, `VisibleOnly` (default 1), and `DepthTolerance` (default 0.000001 in reversed device depth).
`InnerWidth` controls self-depth thickness; `InnerEdgeWidth` independently controls inner edge thickness. Both measure the full centered width.
`VisibleOnly` depth-tests exterior and internal strokes against foreground geometry. It never removes samples from the character mask.
`Samples` supports 1, 2, and 4 samples per axis; the default is 2. Sampling falls back when the stroke target exceeds the device texture limit.

## How the line is drawn

1. The geometry pass produces independent high-resolution coverage. Disabling High Quality Geometry uses the legacy scene-stencil adapter.
2. Marching squares extracts the 0.5 coverage contour, interpolates crossings between samples, and handles ambiguous saddle cells.
3. A compute pass compacts contour segments. An indirect draw rasterizes a round-capped distance stroke around each segment.
   When enabled, visibility is tested at each stroke destination using character depth beside the contour and scene depth at the destination.
4. Maximum blending unions segment coverage, avoiding accumulated opacity at joins.
5. The supersampled stroke is averaged into final output pixels and restricted to the mask exterior. Destination alpha is preserved.

The adapter uses the post-opaque delegate after opaque-scene fog and clouds, before translucency, in UE5.8's desktop forward renderer.
Foreground visibility remains depth-tested.
Translucent and additive Niagara effects subsequently blend over the line using their normal material behavior.
The line is drawn into HDR scene color with pre-exposure and MSAA support. Supersampling and analytic round joins are retained.
Stroke distances are measured in final view pixels even when the scene target uses a different internal resolution.
While enabled, the adapter requests custom depth before the base pass using a temporary tagged `r.CustomDepth.Order` override.
Disabling it restores the previous order without writing engine configuration. A higher-priority console override can prevent this;
the plugin warns and skips unavailable early masks instead of falling back to a late overlay.
There is no temporal history and no blur kernel in the stroke renderer.
Contours are represented by piecewise straight segments; accuracy depends on source coverage, not just stroke sampling.

## Coverage API

`AddCharacterSilhouettePass` in `Public/CharacterSilhouetteRenderer.h` accepts any render-graph coverage texture.
Its red channel contains linear coverage from 0 to 1. `Coverage.ViewRect` and `Output.ViewRect` describe the same camera region,
at potentially different resolutions. The function runs on the render thread, loads the output, and blends the exterior stroke into it.
Caller-supplied width is in `ReferenceViewSize` pixels, or output pixels when that size is zero.
Callers control invocation timing and reference-resolution scaling. Coverage and sampled depth inputs must be single-sample 2D textures;
the output can be an MSAA target. Desktop SM5 or newer is required. All work belongs to the supplied render graph.
An optional `FCharacterSilhouetteOcclusion` supplies reversed-Z character and scene depth for stroke visibility.
Each depth view rect describes the same camera region as the coverage and output. The mask remains unchanged by depth disagreements.

`FCharacterSilhouetteFeatures` optionally supplies geometry normals, depth, actor identity, and extraction thresholds.
Feature lines share the same capsule rasterizer and compositing quality, clipped to character interiors.
Stroke supersampling alone does **not** recreate geometry detail lost by a low-resolution source; geometry resolution controls that input.

## Scope and limitations

- The legacy stencil adapter remains available as a compatibility option. Character overlaps and internal lines require High Quality Geometry.
- The legacy adapter merges meshes sharing its stencil value; the geometry path assigns per-actor identities.
- Visible-only testing applies to the strokes, not mask generation. Foreground occluder edges do not become new character contours.
  Depth is taken from the nearest covered sample beside a contour segment; steep depth changes and resolved-MSAA edges need visual evaluation.
- The automatic adapter targets UE5.8 desktop forward raster views. Deferred rendering and path tracing are not integrated.
- Movie Render Queue tiling, temporal accumulation, lens distortion, stereo, and motion-blur alignment need dedicated validation.
  In particular, tiled output requires the full-image reference scale plus padded coverage so tile edges are not treated as silhouette edges.
- The early stroke inherits later scene postprocessing, including motion blur, depth of field, and upscaling filters.
  Width compensation preserves requested geometric thickness; very low internal resolution still limits edge detail and final sharpness.
- The contour buffer is bounded to 1,048,576 segments by default (16 MiB). Overflow suppresses the whole stroke rather than drawing a partial outline.
  The public API can raise the limit to 4,194,304 segments. A 4x4 stroke target uses sixteen times the output pixel count and can be expensive at 4K.

## Sonic Smackdown integration

`SonicSmackdown.uproject` enables this runtime module. Project defaults are in
`Config/DefaultGame.ini`, section `/Script/CharacterSilhouette.CharacterSilhouetteProjectSettings`.
The plugin itself keeps its reusable default disabled; the game configuration enables it for selected fighter meshes.
Use stencil value 1 for participating opaque or masked skeletal/static meshes. Collision/debug components and particle effects do not participate.
The project already sets `r.CustomDepth=3` and uses desktop forward shading.

The initial game settings preserve the donor's black line style: 2-pixel exterior, 1.5-pixel self-depth, and 0.75-pixel crease lines at 1080p,
with a 1 cm self-depth threshold and a 45-degree crease angle. Geometry scale 2 and High (2 x 2) stroke sampling use the plugin's quality defaults;
the donor's 4x geometry and Ultra (4 x 4) sampling are available when GPU memory and frame time permit.

With `PawnRenderLayers` enabled, the dedicated surface vertex shader includes its public shader helper and reads custom primitive data floats 28-35.
It applies the same layer projection as the base/depth passes, retaining physical surface positions for self-depth detection.
`PrExtendedShadingPro` custom primitive data float 3 supplies the legacy depth bias; opted-in fighter layer integration sets that bias to zero.
No engine installation shaders are modified by this plugin.

Existing Sonic outline effects and post-process assets remain unchanged at the user's request; their removal is user-managed.
Both effects can be visible until that cleanup is complete. Controlled plugin validation must disable unrelated effects only in the transient test view.
No stage or character asset resave is required by this module's rendering API.

## Validation

GPU, runtime, and automated tests are deferred at the user's request until their old outline effects are removed.
No rendered result is claimed for this port. The following checks describe future validation.

Build `SonicSmackdownEditor Win64 Development` using the installed UE5.8 engine. A real SM5+ GPU is required to validate rendered output;
NullRHI can inspect settings and assets but cannot establish rendering correctness. The donor's test sources are not part of this production port.
Use an unattended offscreen editor session or headless Python-driven game session for project validation, not desktop control.

Check enabled and disabled views using an intended fighter mesh, then two overlapping fighters with different PawnRenderLayers groups.
Verify a nonempty source mask, visible exterior/internal lines, shared identity across a fighter's mesh components, and consistent visibility
between the surface pass and the game's base/depth passes. Include a foreground occluder and translucent/additive effects.
An enabled module, successful C++ compilation, or a nonempty stencil alone does not prove that the GPU pass rendered correctly.
Record executed checks and limitations in `Docs/Modernization`; do not treat this checklist as completed test evidence.
