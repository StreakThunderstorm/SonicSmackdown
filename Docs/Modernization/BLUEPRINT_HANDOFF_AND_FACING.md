# Blueprint rename audit, Training input, and Rouge facing

Updated: 2026-11-09. Status: stopped at user request; animation candidate compiled but runtime validation pending.

## Authorized scope

Audit remaining Blueprints for renamed references and parent calls, fix the reported character-select-to-stage loss of control,
and investigate Rouge repeatedly facing the wrong direction then quickly turning back after attacks and jumps.
The input failure is Training mode using a gamepad; the preserved user log travels from CharacterSelect to Ark.
Use Editor builds and headless Unreal tooling only. Preserve user edits, especially Adelaida, and leave old effects outside this task.
Designer-facing UI connections remain in widgets. Deterministic simulation, rollback, lobbies, game builds and packaging are deferred.

## Plan and completion gates

1. Preserve the current user log and source/asset status before changes; back up any packages that require saving.
2. Refresh the complete project Blueprint inventory, including animation/widget and embedded level Blueprints.
   Inspect call/event/property resolution, parent targets, orphan pins and renamed enum literals in addition to compilation.
   Account for legitimate parent-call self-context metadata; do not classify it as broken without engine resolution evidence.
3. Reproduce the actual Training menu handoff with gamepad input and inspect local player IDs, possession, bindings, focus and match state.
   The earlier smoke supplied character choices directly and used keyboard input; its 44 passing checks do not cover this failure.
4. Trace Rouge's native animation/mirroring and Blueprint transitions after attacks and jumps, including both facing directions.
   Separate gameplay facing state from pose/root rotation and pose blending before choosing a fix.
5. Apply only proven corrections, reload/compile affected assets, and repeat targeted input and animation probes.
6. Remove temporary helpers/test files, rebuild the production Editor if native tooling was added, and record results and limits here.

## Preserved state

- Full Git status captured before this pass at `C:/Users/Jaime/AppData/Local/Temp/SSD-before-full-blueprint-control-audit.txt`.
- User log preserved byte-identically as `Saved/Migration/UserTrainingGamepadFailure-20261110T002114Z.log`.
- Log records MainMenu -> CharacterSelect -> Ark, menu list index errors, and four player-cap errors on stage startup.
  The player-cap errors also occurred in the older passing smoke and are not by themselves a proven cause.
- Native controller/GameInstance input logic is unchanged except the SSD rename. Current saved Player0 bindings contain all 40 expected pairs.
- Original runtime saves/configuration were backed up with a 67-file hash manifest. All remained unchanged after the first launch.
  The runtime probe uses an isolated UserDir seeded from the user's settings; it does not write to the original SaveGames directory.
- Pre-instrumentation animation source bytes are preserved in `Saved/Migration/FacingTraceOriginalSources.zip`.
- Input configuration/build rules before the proposed correction are preserved in `Saved/Migration/InputRoutingOriginalConfigAndBuild.zip`.

## Interim audit findings

The initial full pass inspected 597 Blueprints: 532 ordinary, 60 widget, four animation and one ControlRig Blueprint.
All 597 compiled with zero errors and the 11 previously recorded warnings. It examined 28,479 nodes, 110,224 pins,
22,492 call/event/variable references, 842 parent calls and 2,281 enum pins. No unresolved parent functions or invalid used enum defaults were found.
All 616 source asset hashes remained unchanged. Eighteen old enum literals still resolve correctly through redirects.

Disconnected NullifyAttackBindings nodes in BP_ControlSettings and the CS_FloorSmash Instigator orphan pin are byte-identical to original HEAD.
These are inherited graph debt, not rename regressions. Old Adelaida effect assets remain outside the repair scope.
Valid non-enum Select nodes and empty Make/BreakStruct references were filtered as scanner false positives.

Embedded level-script discovery missed PersistentLevel in the first helper; those maps do not count as validated yet.
A read-only supplement adds explicit persistent-level coverage, delegate references and widget designer data.
Evidence: `Saved/Migration/FullBlueprintAuditSummary.json`, `FullBlueprintAudit.jsonl` and `FullBlueprintAuditGraphs`.

## Input and facing investigation

The reviewed viewport correction is now built and activated; corrected-flow validation is in progress. UE5.8 offsets gamepads using local-player array order, whereas match startup creates
selected P1 first. The correction targets the selected legacy controller ID and preserves player/camera ordering.
The pre-fix actual-flow failure is proven; the corrected comparison remains required. The probe injects at GameViewportClient: it exercises engine device remapping,
controller input, widget delegates and actual travel, but does not emulate Windows/Slate hardware focus delivery.

Rouge's Blueprint parent calls resolve correctly. Fighters use a native single-node animation instance and snapshot blending.
Rouge's mirror-table path appears inactive with bMirrored=false; root-pose interpolation and montage recovery are hypotheses pending traces.
Temporary opt-in SSDFacingTrace logs compare captured, raw, mirrored and blended root rotations. No animation behavior change has been made.

## Tooling and build status

The initial combined diagnostic Editor build passed in 25.10 seconds.
The first isolated-save probe failed before input execution because Unreal parsed backslashes in the Python script path as escapes.
The launcher now uses forward slashes, and the isolated asset-registry cache is warm.

The next diagnostic build encountered a full F: disk and a new viewport local-variable shadowing diagnostic.
The variable was renamed to CurrentGameInstance. Lossless NTFS compression of two generated Intermediate/Build PCH files recovered 2.814 GB.
No project file was deleted or moved. Logical sizes 2,153,509,544 and 2,575,237,104 bytes now occupy 775,651,328 and 926,822,400 bytes.
A subsequent Markdown write encountered another disk-full error and truncated only this newly created task document; it was restored from session text.
Available disk space is checked again before resuming the build. No source asset saves have occurred.

## Remaining work

Complete the supplemental audit, capture the actual Training gamepad failure, validate the proposed routing correction,
reproduce and correct Rouge's facing snap, remove temporary helpers, and finish the production Editor build and Markdown handoff.

## Proven Training gamepad handoff failure

`InputHandoffEvents_baseline.jsonl` and `Saved/Logs/InputHandoffBaseline.log` record a normal exit and the actual gamepad flow:
Title -> MainMenu Training -> CharacterSelect -> fighter/color selection -> MatchSettings -> stage/music selection -> Training.
The menu local players have controller IDs [0,1,2,3], and raw gamepad device 0 correctly reaches controller ID 1.
The match recreates local players in order [1,0,2,3]. UE5.8's offset remap sends the same gamepad to local-player index 1,
which is unassigned controller ID 0. DPadRight and FaceLeft reached that controller; fighter controller ID 1 received no keys,
its direction/attack state stayed zero, and Sonic remained at X=-75. This is an engine input-routing compatibility issue, not a broken Blueprint parent call.

The project viewport override resolves the offset target by legacy controller ID, preserving P1-first camera/cinematic assumptions.
Keyboard, disabled-offset, disabled/missing engine cvar, missing-target and invalid-user cases retain the engine fallback before any event mutation.
It uses the engine's platform-user/device allocation behavior and increments the legacy controller ID once.
New native dependencies are EngineSettings and ApplicationCore. Editor build4 passed in 8.32 seconds after correcting the new class's
member-shadowing diagnostic and explicit mapper linkage. The sole intended config change registers SSDGameViewportClient in DefaultEngine.ini.
The baseline selected Sonic/Training stage for fast reproduction; the corrected run will select Rouge for facing diagnostics.

## Stopped at user request — 2026-11-09

The user explicitly requested stopping. No further validation, implementation or cleanup is authorized by this checkpoint.

- Blueprint audit complete: all 597 assets compiled with zero errors (11 inherited warnings), all 19 configured level Blueprints
  compiled with zero errors/warnings, all 842 parent calls and 120 delegate references resolved. All 616 source asset hashes unchanged.
  Full supplement evidence is in `Saved/Migration/FullBlueprintAuditSupplement.json` and the corresponding log/graph exports.
- Gamepad routing fix is active and runtime-validated through real Training menu selections into Rouge versus Sonic on Ark.
  Match player order remained [1,0,2,3]; raw gamepad device 0 correctly reached fighter controller ID 1.
  D-pad right moved Rouge X=-75 to10.306, left moved her to-122.375, attacks executed, and jump rose from Z52 to131.235 then landed.
  This tests the viewport input boundary, not physical Windows/Slate focus delivery. Analog stick validation is still pending.
- Rouge's animation problem was reproduced independently of facing flags/root rotation: four automatic recovery transitions
  snapped hips/head to identical transforms then blended back over about0.1 seconds. Hips jumps measured51.53,104.90,88.23,51.53 degrees.
  Input-triggered transitions did not show that common fallback pose. Evidence: InputHandoffFacing_fixed.jsonl and InputHandoffFixed2.log.
- A candidate fix is present in SSDAnimSingleNodeInstanceProxy.h/.cpp and compiled successfully in17.20 seconds just before stopping.
  It preserves the previous evaluated pose/curves/attributes while a montage's active slot has zero weight, preventing the end callback
  from snapshotting the reference fallback. It validates bone asset/serial/layout, resets on initialization/bone-cache/mirror-table changes,
  and handles a changed mirror orientation. This animation change has NOT yet been runtime-validated. Do not mark it fixed.
- Latest successful Editor build includes that candidate and temporary diagnostic helpers/instrumentation.
  Log: `Saved/Logs/FacingPoseCacheCandidateEditorBuild.log`. No runtime run followed the build.
- The prior corrected-input run completed gameplay and full logged shutdown but returned0xC0000005 after log closure.
  No matching crash dump/WER evidence was found. A later clean helper-free exit remains a validation requirement.
- Temporary SSDBlueprintAudit and SSDInputProbe source/helpers, audit/input Python drivers, isolated InputHandoffUser data,
  and SSDFacingTrace instrumentation remain intentionally in place because work stopped before validation/cleanup.
  Do not delete the permanent SSDGameViewportClient files during later helper cleanup.
- Source backups: FacingTraceOriginalSources.zip (pre-instrumentation animation .cpp files), FacingPoseFixOriginalHeader.zip,
  InputRoutingOriginalConfigAndBuild.zip, SSDBlueprintAudit.Build.cs.before, and InputHandoffUserStateBefore.zip/.json.
  Original saves/settings remain unchanged apart from intended viewport registration and generated Editor module timestamps/FBX whitespace.

On an explicit resume: run the prepared identical Rouge/Ark recovery comparison with raw-body/slot and reference-pose evidence,
validate analog movement and opposite-facing recovery, review the candidate behavior, then remove temporary tooling and rebuild/validate
production Editor exit. Restore shared FrameDataEditor.Build.cs only after both temporary native helpers are removed. Keep logs/JSON/backups.
