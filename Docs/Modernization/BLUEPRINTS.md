# Blueprint ownership and migration evidence

Updated: 2026-11-09. Status: bounded ownership export complete; the saved migration repairs were independently reloaded successfully.

Related evidence: [MIGRATION.md](MIGRATION.md) records builds/full Blueprint compilation, [AUTHORING.md](AUTHORING.md)
records custom toolkit checks, and [GAMEPLAY_VALIDATION.md](GAMEPLAY_VALIDATION.md) records the bounded runtime smoke.

## Method, coverage, and limits

The migrated editor `5.8.4-58902052+++UE5+Release-5.8` ran an unattended Python commandlet with NullRHI.
The inspection read asset-registry, graph, node, and pin data. It did not save assets, edit graphs, or explicitly
compile Blueprints. Compilation can occur in memory when the engine loads assets. This is an ownership inventory,
not a gameplay parity test or proof that every conditional path executes as intended.

The final run exited **0** and exported all **57 selected assets**, comprising **172 graphs and 4,733 nodes**.
There were no missing selected assets, asset-load/export failures, graph/node export failures, or compiler-error nodes.
Four attempts to read `World.persistent_level` were unavailable in Python; read-only loaded-object iteration by package
successfully retrieved all four level Blueprints instead. Protected enum-property reads are recorded as unavailable.

The selection covers core classes; menu, selection, HUD, pause, result, and travel widgets; Sonic, Super Sonic, Shadow,
and Tails; three representative projectiles; five cinematic scripts; and MainMenu, CharacterSelect, FightTransition,
and Training level scripts. It is intentionally bounded and does not inventory every fighter's authored behavior.
The registry discovered 63,798 `/Game` assets, including 532 Blueprint and 60 WidgetBlueprint assets. Registry counts
are not counts of tracked files, and registry discovery does not mean all assets were loaded or tested.

Evidence retained under the managed migration checkout:

- `Saved/Migration/blueprint_inventory.json`: final coverage, inheritance, graphs, nodes, pin defaults/connections, and dependency results.
- `Saved/Migration/blueprint_records.jsonl`: incremental per-asset records from the same run.
- `Saved/Logs/BlueprintOwnershipFinal.log`: run started 20:09 UTC and completed 20:16 UTC on 2026-11-09.
- `Saved/Logs/BlueprintProbeGraphAPI.log`: retained pre-repair inspection log; the temporary probe JSON/JSONL were removed after findings were recorded here.
- `Saved/Migration/direction_enum_repair.json` and `character_selection_repair.json`: narrowly scoped repair assertions.

The temporary inspection script was removed after the export. The API path was `BlueprintEditorLibrary.list_graphs`,
then `BlueprintGraphEditor.get_graph_editor(...).list_all_nodes`, and `BlueprintGraphPinLibrary` pin accessors.
Direct graph-node access is protected. Level-script fallback used `ObjectIterator(LevelScriptBlueprint)` filtered by package.

The initial worktree probe encountered Git LFS placeholders and was stopped. Content was subsequently hydrated from
the original checkout and verified by the migration pass. Only hydrated runs are used as Blueprint evidence here.
Asset-load warnings remain; successful export does not establish a warning-free project. The separate full headless
compile pass loaded/compiled all 597 Blueprint assets with zero failures/errors and 11 nonfatal warnings; those counts
come from [MIGRATION.md](MIGRATION.md), not inference from these 57 inspected assets. The editor smoke and two packaged
Development runs each passed 44 checks; the packaged captures showed prepared Sonic/Shadow/Training materials. Full
Development cook/package, final helper-free builds and the Shipping archive passed; Shipping startup was observed for
45 seconds separately. Historical parity, full-roster coverage and a full Shipping match remain unverified.

## Ownership map

Asset paths below omit the repeated object name after the final dot. Node identifiers are stable paths within the
named asset and graph in the retained JSON, not source-code line numbers.

| Owner | Observed responsibility | Evidence |
| --- | --- | --- |
| `/Game/BP_Classes/BP_OOSGameInstance` | Persistent selection, round/win/gauge data; bindings and save initialization | 21 graphs, 135 nodes; `ResetFightData`, `SetP1Data`, `SetP2Data`, `EventGraph` |
| `/Game/BP_Classes/BP_OOSGameMode` | Initializes player colors and restores gauges/transformed state | `EventGraph.Event Initialized` chain; 95 nodes across 2 graphs |
| `/Game/BP_Classes/BP_OOSPlayerController` | Dispatches menu actions with local controller ID through GameInstance delegates | `EventGraph.K2Node_CallDelegate_6` through `_12`, `_0` through `_3` |
| `/Game/UI/CharacterSelect/BP_ControllerSelection` | Assigns P1/P2 controller IDs | `EventGraph.K2Node_VariableSet_8`, `_9`, `_10`, `_11` |
| `/Game/UI/CharacterSelect/BP_CharacterSelection` | Selection flow and travel | 15 graphs, 600 nodes after the targeted P2Name repair |
| `/Game/UI/HUD/BP_HUD` | Match clock, round/game winners, round progression, pause, result presentation and travel | 4 graphs, 918 nodes; EventGraph alone contains 891 nodes |
| Travel and pause/result widgets | Reset match/mode data, select destination, pause/resume, quit | Specific graph references below |
| Base and transformed pawns | Animation-notify dispatch, movement/state calls, presentation effects | `BP_BasePawn`: 246 nodes; `BP_TransformedPawn`: 151 nodes after enum repair |
| Fighter, projectile, cinematic subclasses | Additional tick-driven movement, timed projectile creation, authored combat behavior | Representative examples below |
| Level scripts | Legacy widget creation, transition events, media/audio, level travel | All 4 selected level scripts exported successfully |

`BP_OOSGameInstance` derives from `/Script/OriginOfStormsMaster.OOSGameInstance`; GameMode and PlayerController likewise
derive from their native OOS classes. `BP_BasePawn` derives from `OOSPawn_Burst`, and `BP_TransformedPawn` from
`OOSPawn_Transformed`. The HUD widget derives from `/Game/UI/BP_Menu_Base`, separate from the native HUD actor wrapper.

## Match state, clock, input, and travel

### Persistent match data and initialization

`BP_OOSGameInstance:ResetFightData` connects its entry through setters that clear P1/P2 transformed-start flags,
win counts, RoundNo, super points, and transform gauges. `K2Node_VariableSet_4` sets RoundNo to 0. `SetP1Data` and
`SetP2Data` store fighter/transformed classes, mesh, display pose, character name, colors, color index, and mesh offset.

Its `EventGraph.K2Node_Event_0` (Init) connects to `K2Node_CallFunction_4` (LoadGameFromSlot, slot `Player0`, user 0).
The graph has successful-load bindings/audio initialization and a failed-cast branch that creates a save object and
saves the defaults. The inspection did not execute this gameplay initialization or change user save data.

`BP_OOSGameMode:EventGraph.K2Node_Event_2` (Initialized) calls the parent before its initialization chain.
For subsequent rounds, branches comparing RoundNo against 0 restore SuperPts and Transform; subsequent material calls
restore colors, and conditional SwapPawns calls handle transformed-start state. Its Blueprint Tick event is unconnected.

`BP_OOSPlayerController` maps InputAction Pressed events to GameInstance menu delegates, using
`K2Node_CallFunction_1` (Get Local Player Controller ID) for their ID inputs. Controller selection then writes
P1ControllerID/P2ControllerID in its own widget graph. These are existing ownership relationships; no input/UI code
was moved into C++ by this inspection or the widget repair.

### The HUD owns authoritative match behavior

In `/Game/UI/HUD/BP_HUD:EventGraph`:

- `K2Node_Event_2` (Tick) connects to `K2Node_VariableSet_14` (dTime), taking `InDeltaTime` directly from the event.
- `K2Node_CommutativeAssociativeBinaryOperator_3` adds float TimeSeconds and dTime; its result feeds
  `K2Node_VariableSet_13` (TimeSeconds). Execution is gated through `K2Node_IfThenElse_11`.
- The gate's NOR Boolean input comes from both pawns' `IsPausingMatchTimer` calls (`K2Node_CallFunction_141`, `_173`).
- The same EventGraph sets TimeOvr (`K2Node_VariableSet_17`), RoundWinner (`_60`, `_45`, `_0`), GameWinner (`_61`, `_91`),
  and RoundNo (`_563`, `_931`). `K2Node_CustomEvent_0` is RoundEnd; calls `_127` and `_133` enter it from result paths.
- `K2Node_CallFunction_91` and `_76` pause the game with `bPaused=true`. The widget also contains result delays,
  global time-dilation calls, and `K2Node_CallFunction_2025` (Open Level by Name).

This establishes that the HUD is more than a display of a native match clock. A future deterministic simulation needs
an explicit owner for round time, winner/round state, pause decisions, and travel requests. The migration preserves this
behavior; the inventory does not claim those changes or rollback compatibility are implemented.

### Widget and level travel

- Main-menu `BP_TravelTo*CharSelect` widgets call ResetModeData and ResetFightData, set mode flags, and open
  `CharacterSelect` through `EventGraph.K2Node_CallFunction_3`.
- `/Game/UI/PauseMenu/BP_TravelToRematch` calls ResetFightData (`K2Node_CallFunction_2`) and opens GameInstance.StageName
  (`K2Node_CallFunction_0`). `/Game/UI/CharacterSelect/BP_TravelToMainMenu` opens `MainMenu` and resets the arcade ladder.
- `BP_StageSelection` opens StageName on one branch (`K2Node_CallFunction_55`) and `FightTransition` on the other (`_56`).
- `BP_VSEndQuote` has ResetFightData followed by Open Level `FightTransition` (`K2Node_CallFunction_2` then `_17`).
- `BP_VSPauseMenu.K2Node_CallFunction_17` and `BP_TrainingPauseMenu.K2Node_CallFunction_58` resume with `bPaused=false`.
- `BP_TravelToQuitGame.K2Node_Event_1` (Construct) connects directly to `K2Node_CallFunction_3` (QuitGame).
- MainMenu and CharacterSelect level scripts create/add widgets and open media. FightTransition contains named events
  for transition presentation and Open Level. Training includes HUD creation, audio handling, time dilation, and level reload.

The legacy UI flow spans widgets, Blueprint controllers, GameInstance delegates, and level scripts. Any future UI work
must preserve designer access and follow the project's requirement to keep connecting UI logic in widgets.

Subsequent runtime checks also exposed the unchanged native GameMode controller loop attempting IDs 0-7, with four
UE5.8 player-limit errors per run. The tested match still passed, but hardware/controller assignment and complete
menu/round/rematch flow were not validated. See [GAMEPLAY_VALIDATION.md](GAMEPLAY_VALIDATION.md) for that distinct
runtime evidence; it is not inferred from this graph export.

## Representative gameplay ownership relevant to later determinism work

All node references in this section are in the named asset's EventGraph unless stated otherwise.

- `/Game/BP_Classes/BP_BasePawn.K2Node_Event_5` (OnOOSAnimNotify) connects through parent notify handling to
  `K2Node_SwitchName_1`. `Fly` calls SetFlightModeEnabled true (`K2Node_CallFunction_40`); `UnFly` calls it false (`_39`).
  `Shake` and `Heavy Shake` from the same switch call ClientStartCameraShake (`_25`, `_18`). Gameplay and presentation
  share this authored dispatch. Actor-location setters `_27` and `_28` have sweep disabled.
- `/Game/BP_Classes/BP_TransformedPawn` has connected Tick, an actor-location setter (`K2Node_CallFunction_2`), and
  a timeline whose Update/Finished paths change material parameters and whose TurnOnLights path enters a loop.
  Visual timelines must be distinguished from authoritative movement rather than classified together.
- `/Game/Characters/Sonic/CinematicScripts/CS_HomingAttack.K2Node_Event_2` (Tick) directly executes XSpeed then YSpeed
  setters (`K2Node_VariableSet_0`, `_1`). Tick DeltaSeconds also feeds their interpolation calls. `CS_HomingRush` has a
  similar speed-mutation graph. `/CS_SuperTeleport.K2Node_CallFunction_6` sets actor location from its Tick sequence,
  with an interpolation DeltaTime input from Tick and sweep disabled.
- `/Game/Characters/Sonic/CinematicScripts/CS_SpeedUnlocker` sets FlightMovementSpeed to 32 after OnCinematicStart's
  parent call and to 8 after the parent OnCinematicEnd or OnCinematicInterrupted paths.
- `/Game/Characters/Sonic/Projectiles/PJ_SonicWind.K2Node_Event_2` (Tick) directly calls SetActorLocation (`_11`) with
  sweep disabled. Shadow's `PJ_ChaosSpearL` contains connected tick-driven XSpeed/YSpeed setters, a 0.5-second Delay,
  and a conditional Destroy Actor path. Tails' `PJ_Buster` also has a connected Tick mutation path.
- `/Game/Characters/Tails/CinematicScripts/CS_RapidPunch.K2Node_CallFunction_4` starts a looping 0.05-second timer.
  Its Spawn delegate (`K2Node_CustomEvent_1`) calls PlaySound2D (`_25`) then AddProjectile (`_16`, class
  `PJ_BoxingGlove_Rapid`). RandomFloatInRange nodes `_23`, `_13`, and `_11` feed projectile X offset (-50 to 50),
  Z offset (10 to 60), and both speed components (800 to 1000), respectively. This is a concrete timer/RNG/gameplay
  spawn path sharing execution with an audio effect; it requires explicit simulation ownership before rollback.

These are authored connections observed in representative assets, not a complete classification of all roster behavior.
They explain why native fixed-point types alone would not make the current game deterministic.

## Migration repair evidence and remaining legacy graph

### Direction enum references repaired

The pre-repair probe found `ResolveDirections.K2Node_SwitchEnum_0` titled `Switch on (bad enum)` in both
`/Game/BP_Classes/BP_BasePawn` and `/Game/BP_Classes/BP_TransformedPawn`. Compilation required a valid enum.
The enum property cannot be read directly through Python; the probe's null field was an unavailable read, not proof
of a null serialized property. The Byte selection input and all nine named branch connections were present.

The branch names match `/Script/MoveNetworkRuntime.EOOSInputDir`: OOSID_None, Right, UpRight, Up, UpLeft, Left,
DownLeft, Down, and DownRight (each directional value also has the OOSID_ prefix).

The migration repair restored that enum and inserted a native byte-to-enum conversion, preserving the existing byte
function input and logical branch wiring. `Saved/Migration/direction_enum_repair.json` records successful compilation,
preserved logical connections, and 582/348 connection endpoints checked for base/transformed assets. Conversion nodes
are `K2Node_CastByteToEnum_0` and `_1`, respectively, in ResolveDirections.

The independent final read-only reload shows `Switch on EOOSInputDir`, the conversion node, and no compiler-error nodes
in either asset. Their node counts changed from 245/150 to 246/151. These asset writes were performed by the migration
repair pass; this ownership exporter made no asset changes.

### Character-selection widget assignment repaired

`/Game/UI/CharacterSelect/BP_CharacterSelection:EventGraph.K2Node_VariableSet_3` was an invalid null assignment to the
designer-owned P2Name TextBlock reference. The editor migration repair removed that setter and connected
`K2Node_VariableSet_1.then` directly to `K2Node_VariableSet_5.execute`, matching the corresponding P1 flow.

`Saved/Migration/character_selection_repair.json` records before/after hashes, the 601-to-600 node change, an assertion
that all other nodes/pins/values/connections remained unchanged, and successful compilation/save of only that asset.
The final independent export confirms 600 nodes and no compiler-error nodes. The fix remains entirely in the widget.

### Retained legacy NewParam function

`BP_TransformedPawn:NewFunction_0.K2Node_FunctionEntry_9` remains a one-node graph with unconnected NewParam
Class Reference and then outputs. The initial load warned that its invalid class was replaced with Object. The final
export contains no compiler-error node for this graph. External callers have not been exhaustively ruled out, so this
inventory does not classify it as safely deletable or perform cleanup of authored assets.

## Disabled plugin dependency scan

The registry query scanned **63,796 unique `/Game` package names** with hard/soft package, searchable-name,
and hard/soft management references enabled. It found **zero** dependencies beginning with `/NiagaraExtras`,
`/CleanProject`, `/Script/NiagaraExtras`, or `/Script/CleanProject`, and **zero query errors**.

This supports disabling these plugins for the recorded registry content. It does not rule out dynamically constructed
paths, configuration-only references, external scripts, or assets outside the scanned `/Game` mount.

## Remaining limits for the larger plan

- Migration compilation/export success does not prove visual or gameplay parity with the older package; see `BASELINE.md`.
- The other fighter/projectile/cinematic graphs need the same state-versus-presentation classification before deterministic conversion.
- Round state, authored timers, randomness, animation-driven gameplay flags, projectile lifetime/spawn order, and tick movement
  need one deterministic execution contract and snapshot policy before rollback can be considered complete.
- The export proves graph structure and recorded references. Runtime condition coverage, cross-machine state hashes,
  resimulation equivalence, lobbies, and online matches are outside this completed migration inventory.
