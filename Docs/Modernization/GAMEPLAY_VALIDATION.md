# Migration gameplay validation

Recorded: 2026-11-09. Scope: UE5.8 compatibility checks, not determinism or gameplay parity certification.

## Method

An opt-in temporary native helper was driven by a headless Python launcher. Unreal ran with `-RenderOffscreen`
under DX11, without PC control; the editor run also used `-game`. Runtime saves used an isolated directory under this checkout's `Saved` folder.
The helper source and launcher have been removed, and Editor, Development and Shipping binaries have been rebuilt
without the helper. The final repeat below used the earlier staged Development test executable, which still contained it.

The check loads the existing MainMenu, then supplies Sonic/Super Sonic and Shadow/Super Shadow through the existing
GameInstance and opens Training. This bypasses menu navigation and character selection. It observes the existing
Blueprint GameMode/HUD and sends mapped forward, up and light-attack key events through the player controller across
separate real ticks. It does not replace movement, collision, animation events or damage calculation.

## Editor standalone result

The run exited **0**, reporting **44 checks passed and zero failures** at 20:22:18 UTC.
Evidence: `Saved/Logs/MigrationSmoke_editor.log`.

- MainMenu loaded, and the configured GameInstance and all four fighter classes resolved.
- Training used the existing Blueprint GameMode and OOSWorldSettings with one stage-bounds actor.
- Two fighters were visible, with reciprocal opponents, the shared camera, hidden transformed instances, loaded
  move networks, initialized animation instances and positive full health.
- Mapped direction input moved the fighter into attack range; releasing it cleared the direction state.
- Mapped up input produced an airborne jump and returned to grounded state, with blocking floor collision.
- Mapped light input selected a move, created a hitbox, damaged the opponent, finished, and released correctly.
- Shadow's health changed from 1,800,000 to 1,755,000 after the normal attack; training refill restored both fighters.
- Final floor clearance was 2 units for both fighters, with blocking upward-facing floor traces.
- Five engine-generated screenshots were written. Inspection confirmed the title screen, fighters, stage and HUD
  rendered. The match captures still displayed "Preparing Shaders" and fallback materials, so they do not establish
  material readiness or appearance parity. Cooked-package visuals require separate inspection.

## Packaged result

The first Win64 Development cook/package completed successfully for the 19 configured maps. Its executable was then
launched through `Saved/Migration/run_headless_smoke.py packaged`, using the same opt-in helper and an independent
`Saved/MigrationRuntimeUser/packaged` save directory. The process exited **0**, reporting **44 checks passed and zero
failures** at 21:21:45 UTC. Evidence: `Saved/Logs/MigrationSmoke_packaged.log` and `Saved/Migration/SmokeConsole_packaged.log`.

- All 39 gameplay/state checks and five screenshot-file checks passed. MainMenu and Training loaded; the four fighter
  classes, Blueprint GameMode/HUD, stage bounds, opponents, shared camera, move networks and animation instances resolved.
- Mapped forward/up/light input produced walking, a jump with a grounded landing, and an attack that selected a move,
  spawned its hitbox, damaged Shadow and returned control. Button and direction release checks passed.
- Shadow's health changed from 1,800,000 to 1,755,000; refill restored both fighters to 1,800,000. Final floor clearance
  was 2 units for both fighters, and both blocking floor traces passed.
- All five 1280x720 native screenshots were inspected: MainMenu, TrainingReady, Jump, NormalAttack and TrainingFinal.
  The title art and HUD rendered; Sonic's blue and Shadow's black/red character surfaces, colored shoes, outlines,
  shading and the orange Training stage material were visible. No screenshot showed a "Preparing Shaders" overlay.
  This establishes material readiness for the visible matchup, not complete material correctness or legacy appearance parity.

Capture directory during validation:
`Saved/MigrationRuntimeUser/packaged/Saved/MigrationSmoke/2026.11.09-16.21.27`.
All reviewed captures and isolated save directories were removed after validation; the logs retain the results.

### Repeat with migrated FXAA settings

The project configuration now selects FXAA with `r.AntiAliasingMethod=1`, replacing the obsolete
`r.DefaultFeature.AntiAliasing` key, and sets `r.FXAA.Quality=5` under `[ConsoleVariables]` in `Config/DefaultEngine.ini`.
UE5.8 uses 5 as maximum FXAA quality. This preserves the maximum-quality intent of the old per-map value 6 without
resaving the 13 affected maps. Exact pixel or preset equivalence with the unavailable customized UE4 engine is unverified.

The still-staged Development test package was launched again through headless Python under DX11 with
`-ExecCmds="t.MaxFPS 60,r.AntiAliasingMethod 1,r.FXAA.Quality 5"` and a separate
`Saved/MigrationRuntimeUser/packaged_fxaa` save directory. The log explicitly printed effective values
`r.AntiAliasingMethod = "1"` and `r.FXAA.Quality = "5"` at 21:28:47.806 UTC. The run exited **0** and again reported
**44 checks passed and zero failures** at 21:29:06 UTC. Evidence: `Saved/Logs/MigrationSmoke_packaged_fxaa.log`.

All five new native PNGs were inspected. The title screen, HUD, colored Sonic/Shadow surfaces, outlines, shading and
orange Training stage rendered, including the airborne jump and attack captures. No shader-preparation overlay or
gray fighter fallback appeared. The same walking/input-release, jump/landing, hitbox/damage, refill and floor checks
passed; Shadow reached 1,755,000 health after the attack, and both fighters ended at full health with 2-unit floor clearance.

Repeat capture directory during validation:
`Saved/MigrationRuntimeUser/packaged_fxaa/Saved/MigrationSmoke/2026.11.09-16.28.47`.

This repeat validates the revised runtime rendering values against the original cooked Development test package.
Its staged configuration predates the fix, so the command-line overrides do not independently verify final config
packaging. The final Shipping package subsequently staged the actual revised configuration and rebuilt helper-free executable.
The pak response file's staging input was verified to contain both FXAA values and the legacy-input opt-out;
that packaging result is tracked separately in `MIGRATION.md`.

### Runtime diagnostics and qualifications

The functional pass does **not** mean the runtime log is free of errors.

- The package reports invalid/uncooked shader-map errors and default-material fallback for Blaze `Material__72`,
  `M_SRS_LC_Calibration`, `M_SRS_MASTER_CelShader` and `MI_SRS_CelShader`. Their matching UE4 failures and complete asset
  paths are recorded in `MIGRATION.md`. The visible Sonic/Shadow materials work despite these inherited failures;
  readiness across the entire roster/material library is not established.
- Four "Maximum number of players (4) already created" errors occur in the editor and both packaged runs. The unchanged
  `AOOSGameMode::BeginPlay` loop in `Source/OriginOfStormsMaster/OOSGameMode.cpp:246` attempts controller IDs 0 through 7.
  In Training, P1 plus unused IDs 1 through 3 fills UE5.8's four-local-player limit; IDs 4 through 7 are rejected.
  `UGameplayStatics::CreatePlayer` reaches `UGameInstance::CreateLocalPlayer`, which emits the observed error.
  The native loop matches the original source, but the missing original engine and absence of these errors in the two
  inspected legacy logs leave its old runtime behavior unverified. Additional hardware/controller slots were not tested.
- Training's unchanged level Blueprint `EventGraph.K2Node_CallFunction_0` executes `r.PostProcessAAQuality 6` before
  creating the HUD. UE5.8 rejects it as "Command not recognized" in all three runs. The same obsolete value occurs
  in 13 unchanged maps; those commands are harmless no-ops and no longer control quality. The project-level FXAA
  settings above now provide the intended rendering defaults. Training's SHA256 matches the original asset;
  no graph was changed to suppress this diagnostic.
- The runtime also retains the documented Blaze/Vector zero-scale reference-pose and missing `NewTouchInterface`
  warnings. Enhanced Input emits missing-PlayerInput/user-settings warnings with the retained legacy input classes;
  the mapped input checks still pass. The first isolated launch lacks a `Player0.sav`, as expected for an empty save directory.

## Final Shipping startup and cleanup

The final Shipping archive completed with UAT exit 0. Its archived executable remained running during a 45-second
offscreen startup observation, after which the headless Python launcher stopped only its own child process.
Evidence: `Saved/Migration/ShippingStartupResult.log`. This establishes startup liveness, not a Shipping local-match pass.
The 44-check gameplay runs above used instrumented Development executables.

Temporary helper source/scripts, stale helper compiler outputs, all 15 reviewed captures, isolated save directories,
validation crash dumps and scratch reports were removed. Editor, Development and Shipping binaries were rebuilt
without the native helper; the final Shipping archive replaced the earlier Development staging area. Logs retain
the results and the methodology above records the coverage.

## Limits

This is one matchup, one stage, and a basic move sequence. It does not validate the entire roster, transformations,
throws, projectiles, round/KO/rematch flow, menu/controller hardware navigation, audio, multiplayer, rollback or
cross-run determinism. The legacy package has been fingerprinted, but its gameplay/visual parity was not observed.
See `BASELINE.md`, `BLUEPRINTS.md` and `PLAN.md` for the remaining gates and future simulation work.
