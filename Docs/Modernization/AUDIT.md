# Sonic Smackdown modernization audit

Audit and validation update: 2026-11-09. Original repository baseline: `11c52082` (2024-08-10, `Character assets`).

See [PLAN.md](PLAN.md) for the proposed architecture and [PROGRESS.md](PROGRESS.md) for status. Execution evidence is in
[MIGRATION.md](MIGRATION.md), [BLUEPRINTS.md](BLUEPRINTS.md), [AUTHORING.md](AUTHORING.md), and
[GAMEPLAY_VALIDATION.md](GAMEPLAY_VALIDATION.md). Source/configuration line references below describe the original
baseline unless explicitly identified as migration changes; line numbers can differ in the migrated checkout.

## Assessment

The original UE4.27 game has substantial custom movement, combat, animation, and move-authoring code. It is a viable
starting point for the requested work, but deterministic rollback is a major gameplay architecture change. The inspected
code does not contain a fixed-point simulation, frame-indexed rollback history, complete snapshot/restore implementation,
or C++ online match/session layer.

The most important dependencies are combat timing and state ownership: animation notifies, actor/component ticks,
engine collision, global randomness, world timers, and wall clocks currently affect match outcomes. Converting movement
floats alone would not make the game deterministic.

The original shading plugin was old, but the user's existing UE5.8 port is now integrated in the isolated migration
checkout. The migration is validated for the bounded Win64 scope: final Editor, Development and Shipping builds pass;
597 Blueprints compile with 11 nonfatal warnings; five shader-layer variants and representative custom editor initialization
pass; the full Development cook/package and final Shipping archive succeed. Two packaged Development match runs each
pass 44 checks, with prepared Sonic/Shadow materials visible. This establishes neither full-roster readiness nor legacy
gameplay/appearance parity. The authorized baseline audit/ownership work is complete; the broader behavioral specification
and later determinism, rollback, physics, and online implementation remain open.

The final Shipping executable also remained alive for a separate 45-second startup-only check; that check did not
exercise a Shipping match. Temporary helpers, owned capture/save fixtures, and scratch build artifacts were removed
after validation, while logs and final audit JSON were retained. The original project/donor implementation files remain
preserved, and no commits were created.

## Scope and confidence

The initial audit used source/configuration inspection, repository inventory, installed-engine/header comparison,
and historical logs. Authorized follow-up work built UE5.8.4 Editor/Development/Shipping targets, compiled 597 Blueprints,
inspected 57 representative Blueprint/level assets, initialized custom authoring toolkits, cooked 10,515 packages across
the exact 19 configured maps, and ran editor plus two packaged Development Sonic-versus-Shadow smoke checks.
These checks used build tools, engine automation, and headless Python. Further Unreal editing/testing must follow
the user's prohibition on PC control.

The bounded graph export resolves core match/UI ownership, not every inherited default, animation-notify placement,
material appearance, or per-character behavior. The smoke covered one matchup and stage. Initial editor captures showed
"Preparing Shaders"; all five captures in each subsequent packaged run showed colored fighters, HUD and stage without
that overlay. Four other material failures remain and are proven to occur in the original UE4 logs. The supplied UE4
package is fingerprinted, but its gameplay was not observed; see [BASELINE.md](BASELINE.md). Neither that limitation
nor bounded UE5 validation is a completed parity comparison.

Findings about absent systems remain limited to searched C++/configuration and the bounded Blueprint inventory;
they do not prove that uninspected assets contain no experiments or extra gameplay rules.

Source-confirmed defects below mean the code has the described behavior under the stated conditions. Their frequency,
affected content, player-visible impact, and preferred replacement behavior still need runtime validation.

The original working tree was clean before audit documentation. A final narrow read-only Git status check on its
Source, Config, Content, Plugins, and project descriptor again reported no changes. That check does not inspect
ignored outputs or establish byte-for-byte integrity. User-supplied AGENTS instructions govern the work; migration
implementation is isolated on C:, and original/donor content is preserved.

## Project and environment

| Item | Observed evidence |
| --- | --- |
| Original project engine association | `OriginOfStormsMaster.uproject:3` specifies `4.27`; isolated migration copy now specifies `5.8` |
| Historical runtime | `Saved/Logs/OriginOfStormsMaster.log:109` records `4.27.2-18319896`; log is from August 2024 |
| Original engine path | Logs/solution reference `C:/Program Files/Epic Games/UE_4.27`; that path was not present during the audit |
| Installed target | `C:/Program Files/Epic Games/UE_5.8/Engine/Build/Build.version`: 5.8.4, CL58902052, compatible CL55116800 |
| Validated toolchain | VS2022 MSVC 14.44.35228 and Windows SDK 10.0.22621.0 selected by passing Editor/Game builds |
| Project modules | `OriginOfStormsMaster`, `MoveNetworkRuntime`, `MoveNetworkEditor`, `FrameDataRuntime`, `FrameDataEditor` |
| Project source | 205 `.h`, `.cpp`, and `.cs` files under `Source`, excluding plugin source |
| Tracked Unreal packages | 64,039 total: 64,004 `.uasset` and 35 `.umap`; disk inventory matches |
| Content concentration | 59,407 packages under `Content/Characters`, 2,461 under `Content/Stg`, 265 under `Content/UI` |
| LFS inventory | 64,045 indexed entries; all reported full-content `*`, none pointer-only `-`; no integrity hash validation performed |
| Default gameplay assets | Blueprint GameInstance/GameMode and entry map configured in `Config/DefaultEngine.ini:205-208` |

UE5.8 is a released target, confirmed against [Epic's release announcement](https://www.unrealengine.com/news/unreal-engine-5-8-is-now-available).
The pinned local migration target is the installed 5.8.4 build, not an automatic switch to a newer engine version.

## Migration dependencies

### MIG-01: reuse the existing UE5.8 shading port

The original `Plugins/PrExtendedShadingPro` descriptor targets 4.27.0. Its bundled shader implementations cover 4.25.0,
4.26.0, and 4.27.0. Its `Private/PrExtendedShadingPro.cpp:85-107` selects version-specific shader replacements,
`:122-127` reinterprets and modifies internal shader-type memory, and `:133` reports unsupported versions.
Changing only its descriptor would not upgrade this bundled copy.

The user's existing upgrade was found at:

- `F:/UE4 Work/AFHeroes/Plugins/PrExtendedShadingPro` — recommended donor for the initial reuse review.
- `F:/UE4 Work/Head2Head/Plugins/PrExtendedShadingPro` — another copy.
- `F:/UE4 Work/MechaMadness/Plugins/PrExtendedShadingPro` — independently corroborated by runtime logs.

AFHeroes' plugin descriptor reports `1.5.59-ue5.5.4-ue5.8.3-local` and includes a `5.8.3` shader implementation.
`F:/UE4 Work/AFHeroes/Saved/Logs/AFHeroes.log:468` records UE5.8.4 CL58902052, and `:1238` records successful loading
of the 5.8.3 implementation and replacement of 102 shader types. MechaMadness' log at `:474` and `:1077` independently
records the same engine/implementation combination. A shader directory named 5.8.3 therefore does not, by itself,
mean this local port cannot run on the installed 5.8.4.

The donor includes material-compiler changes and UE5.8 shader iteration changes, not just a descriptor edit. Its main
plugin `.cpp` hash matches the copies in Head2Head and MechaMadness; complete-tree identity was not established.
The donor still uses internal shader layout assumptions, so each pinned engine patch needs validation.

Status: donor source/config/resources/shaders integrated in the managed checkout; editor/game compilation, all five
offscreen DX11 layer variants, cook/package, and visible Sonic/Shadow material-readiness checks passed. The four inherited
material failures documented in [MIGRATION.md](MIGRATION.md) remain; these passes do not establish full-library correctness.
AFHeroes' pre-existing `.uproject` modification and all donor files were preserved. No relevant donor plugin edits
were reported by Git before reuse. See [MIGRATION.md](MIGRATION.md) for the integration record.

### MIG-02: mirror animation plugin needs source and content validation

`Plugins/RMAMirrorAnimation/Source/RMAMirrorAnimationEditor/Private/RMAMirrorAnimationAnimSequenceProxy.cpp:216-217`
and `:240-241` directly write raw/source animation arrays. Installed UE5.8 headers retain deprecated raw data but use
`SourceRawAnimationData_DEPRECATED` instead of the old `SourceRawAnimationData` member. This is a concrete source/header
incompatibility in the original copy. The migration reused Head2Head's existing 3.5.0 / UE5.8 source/content port;
both runtime/editor modules now compile and link. It did not merely disable the gameplay dependency.

This plugin is a gameplay dependency, referenced by `OriginOfStormsMaster.Build.cs:11`, `OOSPawn.h:319`, and
`SkeletalAnimation/OOSAnimSingleNodeInstanceProxy.cpp:5,37,57`. Simply disabling it is not sufficient. Preserve mirrored
poses, facing, root motion, and combat-relevant transforms. The editor proxy also defines engine animation-asset member
implementations at `:6,32,54`, increasing version sensitivity. Full mirrored-pose/root-motion/content parity remains
unverified beyond the bounded current gameplay smoke.

### MIG-03: enabled plugins and custom editors need reconciliation

`NiagaraExtras` and `CleanProject` were enabled at original `OriginOfStormsMaster.uproject:268,314`, but neither descriptor
was found in the project or installed UE5.8 engine plugins. They are disabled in the migration copy. A full registry
dependency query across 63,796 `/Game` packages found no references to either plugin and no query errors; dynamically
constructed/configuration-only references remain outside that proof. `GPULightmass` and `EditorScriptingUtilities` exist.

Both custom editor modules compile. Four representative MoveNetwork editors and a transient FrameData editor passed
open/tick/close checks. The FrameData probe exposed an uninitialized preview-pointer crash; pointer initialization and
missing-preview guards fixed that crash, and the targeted recheck passed. See [AUTHORING.md](AUTHORING.md).

Graph edits, save/reopen round trips, full authoring coverage, and functional preview remain unverified. Original
`FrameDataAssetEditor/SFDEditorViewport.cpp` already comments out pawn/animation preview creation; the migration leaves
that unfinished block unchanged. Toolkit initialization must not be reported as working animation preview or scrubbing.
`EditorStyle` exists in the installed engine; its use was not a missing-module failure.

### MIG-04: physics and rendering parity must be measured

The project enables forward shading (`DefaultEngine.ini:115`), disables ray tracing (`:170`), and lists legacy target
RHIs (`:222-225`). Preserve the visual baseline while updating supported settings. There is no need to add Lumen,
Nanite, or unrelated visual changes to satisfy the engine upgrade.

The migration retains DX11/SM5 and forward shading. It replaces obsolete `r.DefaultFeature.AntiAliasing=1` with
`r.AntiAliasingMethod=1` and supplies `r.FXAA.Quality=5` under ConsoleVariables, preserving FXAA and maximum-quality intent.
The repeat packaged smoke verified effective runtime values 1/5; the final Shipping archive stages the revised configuration.
Thirteen byte-unchanged maps retain obsolete `r.PostProcessAAQuality 6` commands, which UE5.8 rejects as harmless no-ops.
This is configuration compatibility, not proof of identical UE4/UE5 pixels or quality presets.

No direct PhysX/APEX SDK dependency was found in the inspected project/plugin C++ and build files. However, the game
uses engine sweeps/overlaps, and UE5 replaces the default PhysX simulation with Chaos. Epic documents behavior differences;
successful compilation alone is not physics parity. See [Epic's migration guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-migration-guide).

## Determinism and rollback findings

| ID | Finding and evidence | Required direction |
| --- | --- | --- |
| DET-01 | Float speeds/gravity/push in `OOSMovementComponent.h:37-59`; variable-delta acceleration/integration at `.cpp:78-100,217-222` | Fixed-point authoritative state and one explicit match step |
| DET-02 | Root motion consumed at movement `.cpp:70-74`; hitbox/movement/action notifies mutate combat | Frame-indexed move events and deterministic root/shape tracks |
| DET-03 | Sweeps/sliding at movement `.cpp:111-128`; overlap delegates at `OOSHitbox.cpp:27-30`; projectile sweep at `OOSProjectile.cpp:333-347` | Deterministic collision and contact ordering for match authority |
| DET-04 | Input expiry uses wall clock at `OOSPawn.cpp:477,506`; motion processor accumulates delta at `Input/MotionProcessors/MotionProcessor.cpp:9-18` | Frame-indexed input and command timers |
| DET-05 | World timers govern flight/freeze/projectiles; pawn time dilation at `OOSPawn.cpp:4127-4138` | Explicit snapshotted timers and freeze policies |
| DET-06 | Freeze twitch moves the actual actor using global RNG at `OOSPawn.cpp:275-279` | Presentation-only mesh offset; no collision movement |
| DET-07 | Random ring collision at `OOSSonicRing.cpp:181-187`; collection alters resources at `:283-287` | Deterministic ring entities and snapshotted RNG |
| DET-08 | AI randomness/timing at `Input/OOSAIController.cpp:150,377,529,975,1001,1082` | Deterministic AI or recorded AI frame inputs for supported modes |
| DET-09 | Runtime hitbox pointers live in animation notifies; `OOSAnimNotify_Hitbox.cpp:31-43,103-110` | Per-entity runtime state; immutable shared content |
| DET-10 | Pawn mutates montage blend/rate fields at `OOSPawn.cpp:818-819,831` and combat state at montage end `:855-900` | Simulation move clock and explicit presentation boundary |
| DET-11 | `BP_HUD` Tick supplies float dTime to TimeSeconds accumulation and also owns round/winner transitions; exact nodes in BLUEPRINTS.md | Match-owned frame clock and snapshotted round/result state |
| DET-12 | `CS_RapidPunch` uses a looping 0.05-second timer and global random float nodes for projectile offsets/speeds, sharing execution with audio | Frame-indexed spawn scheduling, snapshotted RNG, and separate presentation events |

`DefaultEngine.ini:8` sets `bUseFixedFrameRate=False`; its configured 60 FPS value is not an enabled match clock.
Physics substeps (`:284-287`) and the disabled enhanced-determinism flag (`:282`) do not address the clocks, state,
animation, and ordering problems above. Enabling those settings would not prove rollback determinism.

The existing input display history is not rollback input storage: controller/header records are event/timestamp based
(`Input/OOSPlayerController.h:58-69,142`; `.cpp:238,250,257,425,439`), and attack grouping uses a world timer at `:445-446`.
Keep input normalization, edge detection, charge/motion state, and command expiry inside the deterministic frame model.

FrameData is unfinished scaffolding. `Source/MoveNetwork/Runtime/Public/OOSMove.h:208-212` explicitly retains old animation
until FrameData is implemented. `Source/FrameData/Runtime/Private/FDTrigger_Hitbox.cpp:13-23` contains empty handlers,
and no C++ runtime consumer of `AttackFrameData` was found. Its authoring concepts may be reusable, but it is not an
existing rollback-ready execution system.

## Networking and UI findings

No relevant C++ rollback/GGPO integration, snapshot serializer, canonical state hash, online session implementation,
or input transport was found. `OriginOfStormsMaster.Build.cs:19-22` leaves OnlineSubsystem integration commented out;
online subsystem plugins are disabled in the project descriptor. Uninspected Blueprint-only experiments remain unverified.

Match setup assumes local players: `OOSGameMode.cpp:82-95` removes/recreates local players, `:189` creates player two,
and `:248-258` creates additional local controllers. Online participant identity and deterministic fighter slots must
be separated from device ownership and viewport/controller setup.

The unchanged controller-creation loop at current `OOSGameMode.cpp:246` attempts IDs 0 through 7. UE5.8 rejects IDs
4 through 7 after its four-local-player limit is reached, producing four errors in each bounded runtime check. The
Sonic/Shadow input/gameplay checks still pass. This loop matches original source, but its old runtime behavior is not
established by the unavailable matching engine or examined legacy logs; hardware/controller coverage remains open.

The bounded export now confirms GameInstance-owned persistent selection/round data, HUD-owned float timing and
winner/round logic, widget-owned travel, and additional level-script UI creation/travel. Fighter and cinematic graphs
also mutate movement. [BLUEPRINTS.md](BLUEPRINTS.md) records exact nodes and remaining coverage limits. All new UI
connecting code belongs in widgets; existing C++ or level-script patterns do not authorize new C++ widget wiring.

## Source-confirmed physics and input defect backlog

All items below remain open. Priority means proposed investigation/fix order; the generic 44-check migration smoke
did not reproduce these specific cases or verify their expected fixes.

### PHY-01 — projectile push direction changes with tick parity (high)

Evidence: `OOSHitbox.cpp:158-172` repeatedly applies `Push *= Sign(Prj->Speed.X)` in X-launch paths; base push is assigned
at `:86`. Negative X speed alternates the stored push sign each tick. Zero X speed erases it.

Check: use an eligible projectile and hit after odd/even travel frames in both directions, then test zero speed and
reflection. Expected: push direction comes from explicit launch/facing rules and an immutable magnitude, not elapsed tick parity.

### PHY-02 — floored animation condition tests asset existence (high)

Evidence: `OOSPawn.cpp:3901,3944` compares `AnimationSequence` only to the first animation, then ORs other raw animation
pointers. With non-null face-up assets, the face-up branch can be taken for a face-down state.

Check: every floored face-up/face-down animation and supported launch type. Expected: branch selection follows the
actual floored state/current animation. Review knockdown/relaunch intent before changing content behavior.

### PHY-03 — player two spawns using player one's capsule height (medium)

Evidence: `OOSGameMode.cpp:203-204` positions Fighter2 using Fighter1's half-height.

Check: fighters with unequal capsule heights, swap slots, and inspect the first simulation frame. Expected: each fighter's
feet align with the stage based on its own collider. Later floor snapping may conceal the initialization error.

### PHY-04 — landing predictor ignores stage elevation without gravity (medium)

Evidence: `OOSMovementComponent.cpp:438` computes stage-relative `FloorOffset`, but `:445` uses `-ZPos / SpeedTick`.

Check: floor Z=100, fighter bottom Z=200, downward speed 10 units/frame. The current formula predicts 20 frames rather
than 10. Expected: prediction uses stage-relative height and agrees with repeated simulation steps.

### PHY-05 — flight predictions apply gravity that movement suppresses (medium)

Evidence: movement `.cpp:220` requires `bUseGravity && !bFlightMode`; future-position/apex/floor helpers at
`:382,396-403,428` omit the flight condition. AI consumes the floor prediction at `Input/OOSAIController.cpp:395`.

Check: flight enabled while `bUseGravity` remains true. Expected: predictors match the authoritative step function,
including flight, acceleration, bounds, and any intentional limits of the prediction API.

### PHY-06 — ring reflection uses a non-unit projected normal (medium)

Evidence: `OOSSonicRing.cpp:184` projects a random normal onto the fighting plane without renormalizing; `:187` uses
the unit-normal reflection formula. This can change bounce magnitude before the deliberate floor damping at `:190`.

Check: deterministic samples on planar surfaces and degenerate projections. Expected: reflection follows the chosen
energy/damping rule, with a defined zero-normal fallback and reproducible randomness.

### INP-01 — stale command cleanup skips entries (high)

Evidence: `OOSPawn.cpp:499` assigns -1 to a `uint8` sentinel; `:524` checks an always-true `>= 0` condition;
`:526-528` removes from the array while incrementing the index. Shifted stale entries can remain.

Check: zero, one, and multiple consecutive expired commands mixed with valid commands. Expected: all and only expired
commands are removed, preserving intended recognition order and buffer behavior.

### DATA-01 — frame-position conversion truncates to uint8 (lower; authoring)

Evidence: `Source/FrameData/Runtime/Private/FrameData.cpp:89` converts through `uint8` before assigning `uint32 CurrentFrame`.
Long frame indices cannot be represented faithfully. This is in unfinished authoring infrastructure, not proven live combat.

Check: long sequences and boundary/end positions. Expected: full valid frame range and consistent preview selection.

## Inherited Shipping optimizer warnings to investigate

The initial optimized Shipping compilation emitted C4701 diagnostics for Direction and three DPad copy sites
(`Saved/Logs/MigrationShippingBuild.log:116,172,174,189`). These are compiler diagnostics, not proof of reproduced
gameplay faults. Shipping compilation and the final archive subsequently succeeded. The warning-bearing logic was
compared directly with the original F: checkout; no implementation changes were made for these warnings during migration.

### AI-R01 — conditional flight-direction assignment

`Input/OOSAIController.cpp:489-531` is text-identical in current and original source. Direction is declared without an
initializer, assigned through nested odds/flight branches, then copied to InputBuffer.Direction at line 531. In the
long-distance flight branch, assignments depend on strict greater-than/less-than X comparisons without a final fallback.
For stable finite actor positions, the preceding `abs(X1-X2) > 100` condition implies unequal X values, so one comparison
should succeed; the warning alone does not establish a reachable uninitialized read in normal gameplay.

Future check: exercise the AI's flight-direction branches with controlled positions/odds, document the position-validity
invariant, and choose an explicit fallback if needed. Any initialization or position-sampling change should preserve
the intended direction policy and remain separate from the compatibility migration.

### INP-R01 — direction transfer across pawn possession

The three inherited sites are `OOSPawn_Transformed.cpp:79-89` (FinishTransformation), `:154-161` (QuickTransformation),
and current `OOSPawn.cpp:4529-4536` (SwapPawns; original lines 4530-4537). Each warning-bearing block matches the original
after only the migration's local Controller-to-PawnController rename. DPad is initialized only if the source
GetFighterInputs result exists, then copied if the destination inputs exist after unpossess/possess.

An uninitialized read would require source inputs to be absent and destination inputs to become available across that
transition. GetFighterInputs normally returns the existing player/CPU controller's input object, and built-in player
input creation/possession code does not by itself prove that failure path occurs. Actual reachability through controller
state or possession callbacks has not been established.

Future check: normal transformation, quick detransformation, and pawn swaps for player/AI controllers, including absent
source-input cases; define a neutral/fallback or transfer guard only after the desired held-direction behavior is explicit.
The current 44-check smoke did not exercise transformations, and these warnings remain unresolved investigation items.

## Runtime investigations and design questions

| ID | Evidence / question | Reproduction focus |
| --- | --- | --- |
| PHY-R01 | Each fighter moves its opponent during push resolution, movement `.cpp:151-173` | Simultaneous movement, swapped tick/slot order, corners, cross-ups; define symmetric resolution |
| PHY-R02 | Plane floor snaps at movement `.cpp:210-213,267-269`, engine geometry sweeps at `:116-130`, unswept distance correction at `:194-198` | Elevated origins, floor seams, walls/ceilings, high-speed dash and maximum separation |
| PHY-R03 | First overlap's metadata retained at `OOSHitbox.cpp:711-714`; exiting hurtbox path `:736-749` may leave stale metadata | Multiple hurtboxes, callback order, impact point/priority and trade resolution |
| PHY-R04 | Throws use animated socket transforms and teleports at `OOSHitbox.cpp:648-681`, `OOSHitbox_Grab.cpp:209` | Stage plane, walls, frozen throws, simultaneous contacts and mirrored characters |
| PHY-R05 | Negative depth speed collapses immediately at movement `.cpp:96`; native burst writes positive speed at `OOSPawn_Burst.cpp:37` | Inspect Blueprint use of negative ZSpeed before classifying as a visible defect |
| TIM-R01 | Freeze clears all pawn timers at `OOSPawn.cpp:4127` | Freeze during flight/autojump; define which timers pause, continue, or cancel |
| ANI-R01 | Runtime state and montage changes touch shared assets | Mirror matches, simultaneous identical moves, transforms, rollback and repeated match startup |
| UI-R01 | Unchanged GameMode creates controller IDs 0-7; UE5.8 rejects four requests after reaching its local-player limit | Two-player hardware/menu assignment, re-entry and rematch; define needed controller slots without changing fighter ownership |
| VIS-R01 | Four material failures match original UE4 logs; additional animation-track/WorldSettings warning origins remain uncertain | Targeted material and content validation beyond the passing Sonic/Shadow sample; see MIGRATION.md |

"Fix all physics inconsistencies" needs a growing behavior/coverage register, not a promise that static inspection found
every issue. Keep intended quirks separate from accidental defects. Use the initial cases above to establish expected
outcomes, then expand coverage across every shipping character, form, stage, and mechanic.

## Immediate next steps

1. Use the final archived Win64 Shipping build and migrated checkout with the documented validation limits; the
   startup-only Shipping observation and temporary-artifact cleanup are complete, separately from Development gameplay tests.
2. Keep legacy gameplay parity explicitly unverified unless an allowed observation method supplies evidence; retain
   the identified July 2024 package and the warning that it predates the August source revision.
3. Prioritize the documented full-roster/material, controller/menu, transformation and authoring coverage gaps when
   additional validation is authorized. The inherited FrameData animated preview remains unfinished.
4. In later authorized work, implement the smallest deterministic two-fighter slice, prove snapshot restoration/rollback,
   then expand mechanics and add input transport/lobbies against the gates in [PLAN.md](PLAN.md).

The baseline audit/ownership work and bounded UE5.8 Win64 migration are complete. The full Development cook/package,
final Shipping archive, and repeated packaged sample gameplay/material readiness are established. Historical parity,
full release coverage, deterministic gameplay, rollback, and online readiness are not.
