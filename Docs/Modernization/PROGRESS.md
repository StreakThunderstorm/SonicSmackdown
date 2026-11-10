# Sonic Smackdown modernization progress

Last updated: 2026-11-09 (America/New_York).

## Current state

The startup fix passed a matched normal-RHI/audio headless run, and the user confirmed the editor now opens.
The requested Blueprint parent-call repair saved 13 character packages. All 25 candidates passed a fresh reload
and compiled with zero errors/warnings; 12 packages, including BasePawn and user-edited Adelaida, remain byte-identical. See [SSD_BLUEPRINT_PARENT_REPAIR.md](SSD_BLUEPRINT_PARENT_REPAIR.md) and
[EDITOR_CRASH_2026_11_09.md](EDITOR_CRASH_2026_11_09.md).

The entire upgraded project now resides at `F:/UE4 Work/SonicSmackdown_R`; the former C: path is a compatibility junction.
All 89,924 original files were verified and preserved; see [RELOCATION.md](RELOCATION.md).
The user subsequently requested renaming remaining OOS/OoS source filenames and code identifiers to SSD.
The SSD rename is complete: 89 source filenames and 115 source files updated, with 140 exact compatibility redirects.
Independent source review found no defects, and the Editor build passed all 167 actions in 167.77 seconds.
Targeted notify-parent compatibility is now validated; broader gameplay/rendering remains untested for these changes.
Two legacy serialized data strings are intentionally preserved.

A follow-up implements the project/module rename to SonicSmackdown and ports the Origin of Storms PawnRenderLayers
and CharacterSilhouette plugins. The combined Editor C++ build passed; see [PROJECT_IDENTITY_AND_RENDERING.md](PROJECT_IDENTITY_AND_RENDERING.md).
The build/package results below describe the completed migration baseline; they do not yet validate these new changes.
The user initially deferred tests until they remove old effects, then explicitly requested startup and Blueprint parent-call
repairs. Those targeted checks are authorized and recorded above. Broad gameplay/rendering suites, game builds and
packaging remain deferred; work stays focused on the Editor.

Current project: [SonicSmackdown.uproject](../../SonicSmackdown.uproject).
Current IDE solution: [SonicSmackdown.sln](../../SonicSmackdown.sln).
No follow-up Shipping build or new cook/package was run. The previously observed AssetTools crash was reproduced
and fixed using a project-only pre-GC cache synchronization; see [crash evidence](EDITOR_CRASH_2026_11_09.md).
The final production Editor build passed in 10.18 seconds after removing native diagnostic/repair helpers.
The final normal-RHI/audio startup passed: MainMenu and Content Browser initialized, 853 Slate ticks completed,
and forced garbage collection was survived with exit 0 and no Blueprint compiler errors. Dedicated shader/rendering
validation remains deferred. Evidence: `Saved/Logs/StartupAndParentRepairFinalStartup.log`.
The already-started Development native build finished before cancellation; further game build work stopped.

The requested baseline audit/ownership work is complete, and UE5.8 migration is validated for the documented bounded
Win64 scope. The user's eventual goals also include deterministic match simulation, rollback, online lobbies/matches,
and consistent physics; those systems are not implemented by this migration. See [PLAN.md](PLAN.md), [AUDIT.md](AUDIT.md),
[BASELINE.md](BASELINE.md), [BLUEPRINTS.md](BLUEPRINTS.md), [MIGRATION.md](MIGRATION.md),
[AUTHORING.md](AUTHORING.md), and [GAMEPLAY_VALIDATION.md](GAMEPLAY_VALIDATION.md).

Final helper-free Editor, Development and Shipping builds pass, as do 597 Blueprint compilations (11 nonfatal warnings),
five shader-layer variants, four MoveNetwork toolkit checks, the repaired FrameData toolkit check, and the editor smoke.
The full Development cook/package and final Shipping archive succeeded. Two packaged Development runs each passed all
44 checks, and all five screenshots in each run showed prepared Sonic/Shadow and Training materials without a shader
preparation overlay. The final Shipping executable passed a separate 45-second startup-only observation.

Legacy gameplay/appearance parity, full-roster release readiness, hardware/controller navigation, and full match-result
coverage remain unverified. FrameData's pre-existing unfinished animated preview is not a newly completed feature.
Runtime logs retain documented material/controller diagnostics despite the bounded functional passes.

Final Shipping deliverable: `Saved/Packages/UE5.8-Win64/Windows/OriginOfStormsMaster.exe` in the managed checkout.
The archive contains 30 files totaling 1,818,145,708 bytes; its pak is 1,467,559,888 bytes. No commits were created.

Baseline repository revision: `11c52082` (2024-08-10, `Character assets`). Git reported no tracked modifications or
untracked files before documentation work. Existing ignored build/cache/log directories were inspected only where
relevant and were not cleaned or changed. Git ownership was handled with a per-command `safe.directory` option;
global Git configuration was not changed.

The audit initially changed only documentation and its `.gitignore` allowlist. Following explicit authorization,
migration work initially used a managed C: checkout; it now resides at `F:/UE4 Work/SonicSmackdown_R`.
The original implementation files at `F:/UE4 Work/Sonic_Smackdown` and donor projects are preserved. No commits were made.
A final narrow read-only Git status check found no tracked/untracked changes under original Source, Config, Content,
Plugins, or `OriginOfStormsMaster.uproject`. This excludes ignored outputs and is not a bulk integrity/hash check.

## Milestone status

| Work | Status | Evidence / next action |
| --- | --- | --- |
| Initial static audit | Complete | Source/config/plugin inspection and environment inventory; runtime limitations in AUDIT.md |
| Baseline audit and Blueprint ownership | Complete within authorized scope | Legacy reference fingerprint and 57-asset ownership inventory recorded; historical gameplay parity remains unobserved |
| Overall architecture and sequence | Proposed | PLAN.md; decisions remain proposals until implementation |
| Existing UE5.8 shader plugin | Validated for sampled content | Builds, five DX11 layer variants and visible packaged Sonic/Shadow materials pass; four inherited material failures remain |
| M0: Runtime baseline/specification | In progress | Legacy package identified and bounded Blueprint inventory complete; legacy gameplay parity unobserved; further PC control prohibited |
| M1: UE5.8 migration | Validated for bounded Win64 scope | Full Development cook/package, two 44/0 packaged Development runs, prepared sample materials, final helper-free builds/Shipping archive and 45-second Shipping startup pass |
| M2: Deterministic simulation slice | Not started | Fixed-point contract, frame clock, move/collision data and repeatable playback |
| M3: Local rollback slice | Not started | Canonical snapshots/hashes and forced restore/replay comparison |
| M4: Full gameplay conversion | Not started | Roster/stage coverage and physics issue resolution |
| M5: Network rollback | Not started | Input transport, handshake, prediction and adverse-network validation |
| M6: Lobbies and online flow | Not started | Backend selection and widget-owned user flow |
| M7: Release validation | Not started | Packaged build, compatibility, coverage and performance gates |

## Session log

### 2026-11-09 — initial audit

- Confirmed project engine association 4.27 and historical runtime version 4.27.2.
- Confirmed installed target engine 5.8.4, changelist 58902052. Visual Studio 2022 and 2026 were installed;
  compiler/SDK compatibility had not yet been established at this initial audit stage.
- Inventoried five project modules, two bundled plugins, 64,039 tracked Unreal packages, and 64,045 LFS entries
  reported as full-content files. This is not an asset-load or hash-integrity validation.
- Audited movement/collision, input timing, animation events, match setup, rollback state, and migration dependencies.
- Found no implemented fixed-point combat core, snapshot/rollback system, or C++ online match/session layer.
- Recorded source-confirmed physics/input defects and separate runtime investigations in AUDIT.md.
- User clarified that the shading plugin has already been upgraded to 5.8 in another project; changed the proposed
  migration path from a possible new port to locating, validating, and reusing that existing work.
- Located the donor in AFHeroes and copies in Head2Head/MechaMadness. AFHeroes and MechaMadness historical logs confirm
  shader implementation loading on UE5.8.4. AFHeroes has an existing `.uproject` edit; it and all donor files were preserved.
- Created audit, whole-project plan, and progress records. No editor launch, build, gameplay test, or asset conversion
  was performed. No temporary test files were created.
- Verified internal document links, normalized all touched files to CRLF, and passed `git diff --check`.
  An independent review confirmed the rollback/input evidence, snapshot coverage, milestone dependencies, and widget rule.

### 2026-11-09 — authorized baseline and engine migration

- User authorized stages 1 and 2, then explicitly authorized subagents.
- Created the managed migration checkout on C:; original F: checkout remains the comparison source.
- User supplied the old desktop build; identified and launched it, but native screenshot capture failed even after
  the user switched it to windowed mode. BASELINE.md records the limitation without claiming gameplay passes.
- Reused both existing UE5.8 plugin ports, adapted project C++/editor interfaces, and passed Editor Development and
  Win64 Game Development compilation.
- Materialized the managed checkout's initially unhydrated LFS assets from local originals/cache; no project content
  conversion or bulk resave has been performed.
- Initial Blueprint compilation attempted 597 assets: 6 errors across 5 assets, 11 warnings, and no load failures.
  This identified the ring metadata, enum, XRBase, and widget compatibility defects repaired below.
- All five shader-layer material variants passed the offscreen DX11 automation check.
- User explicitly reconfirmed this pass covers the baseline audit and UE5.8 migration, with deterministic simulation deferred.
- Updated user instructions require MCP/headless Python for Unreal editing/testing and prohibit PC control.
  Subsequent validation followed that constraint; temporary helpers were removed after use.
- Repaired the two fighter direction switches, the designer-owned character-selection label assignment, ring getter
  visibility, and XRBase loading. A fresh headless pass compiled all 597 assets with zero errors or load failures;
  11 nonfatal compiler warnings remain, reconciled in [MIGRATION.md](MIGRATION.md).
- Completed a bounded read-only ownership export: 57 assets, 172 graphs, 4,733 nodes. The HUD owns the float match
  timer and round/winner logic; level scripts own some widget creation and travel; cinematic scripts mutate movement.
  These remain explicit boundaries for later deterministic simulation work, not changes in this migration.
- Full registry dependency scan found no recorded references to the disabled CleanProject/NiagaraExtras plugins.
- Four representative MoveNetwork editors opened and closed successfully. Fixed a pre-existing uninitialized pointer
  crash discovered when opening the unused FrameData editor; targeted open/tick/close revalidation passed.
  Animated preview/scrubbing remains inherited unfinished functionality; graph edit/save/reopen workflows were not tested.
- Editor standalone local-match smoke passed all 44 checks, covering setup, mapped walking/jumping/attacking,
  hitbox/damage, floor collision and refill. First-run screenshots still contained compiling-material placeholders.
- Full Development cook/package completed successfully with AutomationTool exit 0: 10,515 packages, the exact 19
  configured maps, and 7,161 compiled shaders. The cook summary reports zero errors and 11,268 warning events, not
  that many distinct defects. Four material failures are proven to predate migration; other warning origins/effects
  remain qualified in [MIGRATION.md](MIGRATION.md). The initial competing-project UBT mutex rejection was resolved by retry.
- Independent runtime source review found no actionable migration regression in notify routing, reflected pin names,
  rendering conversions, mirror quaternion argument order or BlendSpace compatibility.
- Win64 Shipping compilation passed all 67 actions with exit 0. Four optimizer warnings were compared against the
  original source and documented as inherited investigation items in AUDIT.md; no gameplay behavior was changed.
- Packaged Development smoke exited 0 at 21:21:45 UTC with 44 passed / 0 failed checks. All five inspected screenshots
  showed colored Sonic/Shadow surfaces, outlines, HUD and Training stage; none retained "Preparing Shaders".
- Updated obsolete anti-aliasing configuration to `r.AntiAliasingMethod=1` and ConsoleVariables `r.FXAA.Quality=5`,
  preserving FXAA/maximum-quality intent. The repeat Development run verified effective runtime overrides 1/5 and
  again passed 44/0 at 21:29:06 UTC with five prepared-material captures. Its older staged config was not the final config.
- The 13 affected maps retain byte-unchanged obsolete `r.PostProcessAAQuality 6` commands; these are harmless no-ops.
  Final Shipping UAT staging used the actual revised DefaultEngine.ini, including FXAA 1/5, disabled Enhanced Input
  auto-upgrade, and no AndroidFileServer section. Exact UE4/UE5 pixel or preset equivalence is unverified.
- Removed the temporary native smoke helper and Python launcher, then rebuilt/relinked all three targets: Editor 4.10 s,
  Development 11.15 s, Shipping 13.15 s. Their refreshed linker inputs contain no MigrationSmoke references.
- Final Shipping archive succeeded with exit 0 in 12.66 s at `Saved/Packages/UE5.8-Win64/Windows`. A separate headless
  startup check observed its child process alive for 45 seconds, then terminated only that child; it did not exercise
  a Shipping match. Evidence: `Saved/Migration/ShippingStartupResult.log`.
- Cleanup completed: removed four owned test-output directories (isolated runtime saves/15 captures, shader report,
  and two crash directories), 14 scratch files, and 20 compiled helper files. Logs and final audit JSON remain.

## Current next actions

Hand off the final Shipping archive and isolated migrated project with their recorded validation limits. Current
baseline/migration implementation and temporary-artifact cleanup are complete for the bounded Win64 scope.

Next authorized work should follow [PLAN.md](PLAN.md): define the behavior needed by a small deterministic simulation
slice, then prove frame-state replay/rollback before broad conversion or lobbies. Keep the unresolved historical parity,
full-roster/material, controller/menu, transformation, and authoring coverage gaps visible.

The unchanged GameMode loop attempts local controller IDs 0-7 and produces four UE5.8 player-limit errors; the tested
match still functions, but hardware/controller slots need separate validation. The four inherited material failures,
11 Blueprint warnings, conditional-initialization warnings, and unfinished FrameData preview remain documented work.

Do not treat a successful editor launch as completion of migration, a 60 FPS cap as determinism, or a lobby connection
as proof of rollback. Use the completion gates in PLAN.md.

## Update discipline

At each work session, update this file with changes, validation evidence, unresolved issues, and the next concrete
action. Update PLAN.md when scope or architectural decisions change. Update AUDIT.md issue statuses and evidence as
reproductions/fixes land; do not mark a suspected issue fixed without checking the intended behavior.

Use status terms consistently: not started, in progress, needs information, validated, or deferred with a reason.
For physics issues, record reproduction, expected behavior, actual behavior, fix revision, and validation result.
Normalize touched text files' line endings and remove temporary test files after completing a feature.

## Active follow-up: Blueprint audit and gameplay regressions

The user authorized a full remaining-Blueprint rename audit and targeted repairs for Training gamepad control after character selection
and Rouge's repeated facing snap after attacks/jumps. This work is in progress; see [BLUEPRINT_HANDOFF_AND_FACING.md](BLUEPRINT_HANDOFF_AND_FACING.md).
Earlier deferral of broad effects/rendering tests does not defer these requested checks. Editor-only scope and preservation of user edits still apply.

## Stopped at user request — 2026-11-09

Blueprint audit completed and Training gamepad routing was fixed/validated. Rouge's recovery-pose snap was reproduced; the candidate animation fix compiles but has not been runtime-validated. Temporary helpers remain. Work stopped at the user's explicit request; see the checkpoint in [BLUEPRINT_HANDOFF_AND_FACING.md](BLUEPRINT_HANDOFF_AND_FACING.md) before resuming.
