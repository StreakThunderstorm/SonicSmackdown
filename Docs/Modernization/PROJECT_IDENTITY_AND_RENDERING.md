# SonicSmackdown identity and rendering port

Started: 2026-11-09. Status: project/module rename and rendering port compile complete; OOS-to-SSD source rename, independent review and Editor compilation complete.
Broad gameplay/rendering tests and game Development/Shipping builds remain deferred. The later requested startup
and Blueprint parent-call repairs have their targeted validation recorded below.

## Authorized scope

Rename the project, native game module, target files and corresponding literal code identifiers from
OriginOfStormsMaster to SonicSmackdown. Preserve serialized Blueprint references with Unreal redirects rather than
bulk-resaving the asset library. The user subsequently explicitly requested renaming remaining OOS/OoS source filenames
and code identifiers to SSD. That extension includes exact reflected class, struct, enum and function redirects.

The user confirmed `F:/UE4 Work/OriginofStorms` as the donor and identified the visual layering system as a plugin.
Port both `PawnRenderLayers` and the separately requested `CharacterSilhouette`, integrate them with the fighters,
and validate against installed UE5.8.4 and PrExtendedShadingPro. Donor files remain untouched.

The donor plugins target UE5.5.4. Copying their files alone does not establish UE5.8 compatibility. Layering must affect
rendering without moving gameplay collision or changing combat state. Normal/transformed forms, foreground swaps,
attached effects and location-spawned Cascade effects need explicit coverage. The user subsequently directed us to
ignore the existing outline effects because they will remove them. Leave those effects/assets unchanged; validate the
new silhouette renderer independently and configure its intended fighter selection without auditing or suppressing old outlines.

## Work and validation

1. Record existing edits and inspect donor plugin dependencies and renderer hooks.
2. Rename project/module/targets/API macros and configure compatibility redirects; generate current project files.
3. Port both plugins to UE5.8.4, coordinate shader overlays, and integrate fighter/effect registration.
4. Build the Editor target and generate renamed project files. The user narrowed this pass to the Editor;
   do not continue game Development/Shipping builds. No editor or test launches in this pass.
5. Deferred until the user removes old effects: Blueprint load/compile checks, shader/GPU checks, rendered overlap/front-back
   swaps, normal/transformed groups, silhouettes, effects and representative gameplay. Future tests must use headless
   Python/engine tools, never PC control.
6. Remove the unused temporary test fixtures and update project links/outcomes. A new cooked/package validation is
   also deferred; the earlier OriginOfStormsMaster archive does not contain these new changes.

The user explicitly requested skipping tests until they remove the old effects, then requested Editor-only work.
The Editor native build passed, but
neither a successful C++ build nor donor validation will be reported as proof that these renderer changes work in-game.

Existing migration edits are preserved. Pre-task status was recorded in `Saved/Migration/PreRenameStatus.txt`;
Git status requires access to the original F: repository's shared LFS cache. No commits are authorized or planned.
The previous OriginOfStormsMaster package remains historical validation evidence until replaced by a validated new build.

## SSD acronym follow-up

The active checkout is now `F:/UE4 Work/SonicSmackdown_R`; see [RELOCATION.md](RELOCATION.md).
The remaining source filename/type acronym is now SSD: 89 filenames were renamed and 115 source files updated.
Existing asset paths remain intact. Exactly 140 additional redirects cover 92 class aliases, 13 struct aliases,
29 enum aliases and 6 function aliases. All 126 authored enum values retain their order and numeric semantics;
123 value names changed, while the three pre-existing RM_* value names retain their spelling.
Compatibility redirects cover both original `/Script/OriginOfStormsMaster` and current `/Script/SonicSmackdown` identities,
plus affected MoveNetworkRuntime types and serialized enum value names.
Evidence: `Saved/Migration/SSDPrefixRenameManifest.json`; original affected text is preserved in
`Saved/Migration/SSDPrefixOriginalSources.zip`. The mechanical source rename did not resave assets; the later
requested parent-call repair selectively saved 13 Blueprint packages, as recorded below.

Two legacy data identities intentionally retain their spelling: the `OOSMovementComponent` default-subobject name and
`OOSAnimNotify` animation-notification identifier. These strings preserve inherited Blueprint component overrides and
serialized animation event identity; source filenames and C++ symbols use SSD. Renaming these data identities would
require a separately validated asset migration, which conflicts with the requested test deferral.

Project regeneration at F: passed in 11.72 seconds. SonicSmackdownEditor Win64 Development compilation passed on its
first attempt: exit 0, 167 actions, 167.77 seconds. Evidence: `Saved/Logs/SSDPrefixProjectGeneration.log` and
`Saved/Logs/SSDPrefixEditorBuild.log`, with console logs in `Saved/Migration`. This is the Editor target's build
configuration, not a game Development build. Existing compiler warnings remain; no game builds or tests were run.

An independent read-only source review checked the mechanical mapping: all 115 source hashes matched the preservation ZIP/manifest;
89 filename moves, 46 classes, 8 structs, 18 enums and 140 redirect directives were checked. All 126 authored enum
entries and 18 generated MAX names are mapped, with ordinal expressions unchanged. Existing asset path strings,
the two legacy serialized identities, ordinary unrelated words and the newer AndroidFileServer section are preserved.
No temporary script or test file was left behind. Current solutions contain no old managed-checkout path.

C++ compilation and static redirect review do not prove that every Blueprint loads or that rendering behaves correctly.
The user subsequently requested fixing startup and broken Blueprint parent calls. The startup crash was reproduced,
traced to an expired Resonance Audio asset-definition adapter and repaired with an editor-only pre-GC synchronization.
See [EDITOR_CRASH_2026_11_09.md](EDITOR_CRASH_2026_11_09.md).

The parent-call follow-up repaired nine remaining orphaned notify nodes and canonicalized four additional notify
references, saving 13 packages after preservation checks. All 25 candidates passed fresh-process loading and compilation
with zero errors/warnings; 12 packages, including BasePawn and Adelaida, stayed byte-identical to their pre-repair backups.
See [SSD_BLUEPRINT_PARENT_REPAIR.md](SSD_BLUEPRINT_PARENT_REPAIR.md). Broader gameplay/rendering checks remain deferred.

## Whole-project boundary

This rendering/identity request adds to the completed initial migration. It does not implement deterministic simulation,
physics fixes, rollback or online lobbies. Their architecture and sequence remain in [PLAN.md](PLAN.md).

## Results

- The mechanical rename is implemented: SonicSmackdown project descriptor, target classes/files, native module
  directory/build/entry files, API macros, editor dependencies and native WorldSettings path. An exact package redirect
  maps `/Script/OriginOfStormsMaster` to `/Script/SonicSmackdown`. Existing ProjectID and save data are preserved.
- Fighter integration now routes foreground/background requests through shared render groups and retains the old
  serialized depth properties. Normal/transformed sharing, visible-form anchors and detached-effect registration
  are implemented pending runtime validation. The existing front/back flag has no non-rendering uses in native source.
- Donor plugin paths were checked and are clean; no donor edits are required.
- PawnRenderLayers' UE5.8.4 engine and PrExtendedShadingPro 5.8.3 shader insertion anchors each match exactly once.
  HLSL compilation and runtime/render validation are still pending; matching text anchors alone is insufficient.
- CharacterSilhouette already has optional integration with both shading and layer plugins. Its renderer-private API,
  native settings and stencil-1 fighter integration compiled successfully. The donor-specific fog material was not
  imported or assumed present. Existing outline cleanup belongs to the user; actual GPU behavior is unverified.
- Renamed Visual Studio project generation passed in 4.49 seconds, producing `SonicSmackdown.sln` and `.slnx` plus
  the corresponding automation solutions.
- The production-only SonicSmackdownEditor Win64 Development build passed on the first attempt: exit 0, 167 actions,
  164.10 seconds. Both new plugin DLLs and the renamed game module linked. Evidence: `Saved/Logs/RenamedEditorBuild.log`.
  The user subsequently stopped the additional game Development/Shipping build work to focus on the Editor.
  Development had already completed when cancellation arrived (96 actions, 121.95 seconds, exit 0); no Shipping
  build was launched for this follow-up. This incidental build is not runtime validation or a new packaged release.
- The temporary match helper and all four copied automation test sources were removed before compilation. No tests,
  GPU fixtures, editor sessions or test captures were run or produced for this follow-up.
- Cleanup removed 27 obsolete old-name artifact roots in this checkout's Binaries/Intermediate (837 files) and the
  old module entry in `UnrealEditor.modules`. Its BuildId and all five current module mappings are preserved; all
  1,161 current Editor receipt products exist. The historical package and source assets were untouched.
  Evidence: `Saved/Migration/ObsoleteProjectArtifactsRemoved.json`. No builds or test processes remain active.

Open [SonicSmackdown.uproject](../../SonicSmackdown.uproject) for Editor work, or
[SonicSmackdown.sln](../../SonicSmackdown.sln) for native code. New plugin configuration is available through the
inherited RenderLayer component and Project Settings > Game > Character Silhouette. No commits were made.

Do not treat donor test results or the previous migration package as proof of the renamed project or either newly imported plugin.
