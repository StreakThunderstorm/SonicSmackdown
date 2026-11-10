# Editor startup crash: 2026-11-09

Status: the startup crash is fixed and the final production-only Editor startup check passed on 2026-11-09.
The cause was an expired ResonanceAudio asset-definition proxy in UE5.8 AssetTools. The project-side correction
refreshes that cache before garbage collection; no installed engine files or renderer/effect assets were changed.

The initial sections preserve the read-only assessment of the first manual session. At that checkpoint the invalidation
cause was unresolved, and no reproduction, launch or source/configuration change had been performed. The later
reproduction, correction and final validation below supersede that initial uncertainty.

## Session and retained evidence

- Project: `F:/UE4 Work/SonicSmackdown_R/SonicSmackdown.uproject`.
- Engine: UE 5.8.4, changelist 58902052. The recorded command line contains only the project path.
- Original session log: `Saved/Logs/SonicSmackdown.log`, with its retained copy in the first crash directory below; engine initialization completed at 22:21:39 UTC, followed by the fatal report at 22:21:44 UTC. Later sessions reuse the current log path.
- Crash directory: `Saved/Crashes/UECC-Windows-15F01EA94C3377A5B6E9219E083B0198_0000/`. It retains `CrashContext.runtime-xml`, `UEMinidump.dmp`, `Slate.txt`, the session log, and crash-client settings.
- Exception: `EXCEPTION_ACCESS_VIOLATION reading address 0x0000000000000000`. `Slate.txt` identifies the Game thread painting the main-frame `SWindow`.

## Immediate mechanism

The recorded stack enters AssetTools from ContentBrowserClassDataSource, followed by ContentBrowser and Slate frames. There is no rendering-plugin frame in the crashing stack.

Read-only inspection of the installed DLL and its PE imports shows that `UnrealEditor-AssetTools.dll + 0xa24c7` executes `mov rcx, [rax]` immediately after a call to `FWeakObjectPtr::Get()`. The exception is consistent with that weak-object lookup returning null.

The surrounding machine-code sequence strongly matches `FAssetDefinitionProxy::GetSupportedClass`, whose installed engine source is:

```cpp
virtual UClass* GetSupportedClass() const override { return AssetDefinitionPtr.Get()->GetAssetClass().LoadSynchronous(); }
```

Source: `Engine/Source/Developer/AssetTools/Private/AssetTools.cpp:153`. This compatibility proxy stores a weak reference to a `UAssetDefinition`. `UAssetToolsImpl::SyncAssetTypesToAssetDefinitions` invokes registered actions' `GetSupportedClass` while reconciling the definition registry (`AssetTools.cpp:1475`, especially line 1490).

The ContentBrowserClassDataSource stack offset `+0x165ec` falls within the exported `GetClassTypeActions` function. Its source calls `GetAssetTypeActionsForClass(UClass::StaticClass())` at `Engine/Plugins/Editor/ContentBrowser/ContentBrowserClassDataSource/Source/ContentBrowserClassDataSource/Private/ContentBrowserClassDataSource.cpp:62`.

## Confidence and limits

The null weak-object dereference is directly supported by the exception and disassembly. Identification of the specific proxy function is a strong source/binary correspondence; engine PDBs are not installed, so this is not a symbolized source-line stack. At this initial checkpoint the particular asset definition, its owner and why its reference became invalid had not been established; the later traced reproduction below identified all three.

There is no evidence here assigning the crash to CharacterSilhouette or PawnRenderLayers. The manual session compiled shader jobs, including CharacterSilhouette surface-vertex permutations, and did not report a fatal shader compilation failure. That observation is not a rendered validation or proof that all shader permutations work.

At the initial checkpoint all crash evidence and existing effects/assets were preserved. Renderer tests remained deferred at the user's request. No speculative fix, engine patch, cache deletion or editor-layout reset had been applied; identifying the expired definition was the next prerequisite. The subsequent user-authorized startup repair is recorded below.

## Startup repair authorization and original plan

The user subsequently reported that the editor cannot open and requested repair. A second manual startup failed at
22:51:08 UTC (17:51:08 America/New_York), after the successful SSD Editor build. Its stack repeats the same AssetTools
weak-object null dereference; evidence is in `Saved/Crashes/UECC-Windows-AE770ECD46DC779BCC8CE087D8D7B3CE_0000`.

This request authorizes a narrowly scoped startup reproduction and repair. Use headless Python/build tools only;
keep old effects and gameplay/rendering suites outside this investigation. Preserve user edits and crash evidence.
Pre-fix Git status is recorded in the system temporary file `SSD-pre-startup-fix-status.txt`.

Plan: identify the expired definition and registration/lifetime path, reproduce startup headlessly, apply the smallest
project-level fix supported by evidence, compile Editor, rerun the startup check, and remove diagnostic helpers.
Do not treat a successful compile or a commandlet that bypasses the failing Content Browser path as startup validation.

## Reproduced cause and selected correction

Normal audio plus normal RHI, run offscreen through the Editor Python startup path, reproduced the same crash at
23:03:48 UTC. Null-RHI probes with and without audio passed, but bypassed the failing first Slate paint and are not
proof that normal startup works. The normal-RHI run crashed before its Python script could execute.

Temporary native diagnostics recorded an exact causal match with the new crash dump:

- `AssetDefinition_SoundEffectPresetDynamic_3` represented `/Script/ResonanceAudio.ResonanceAudioReverbPluginPreset`.
- Its weak object pair was index 53343, serial 6131. It was registered at the PostEngineInit callback.
- The later PostEngineInit module-loading phase registered the ResonanceAudioEditor static definition, replacing it.
- The first GC collected the retired dynamic object before AssetTools reconciled its cached weak action proxy.
- The crashing proxy in `UECC-Windows-3F536AA04E374F87F62D5BB1E9233F54_0000/UEMinidump.dmp` contains exactly that pair.

Evidence: `Saved/Logs/StartupAssetDefinitionRHIProbe.log`; static registration line 2446, retired dynamic line 2472,
collected dynamic line 2581, and fatal stack from line 12290. The two other expired wrapper definitions did not match
the crashing proxy. Renderer plugin startup source does not access the asset definition registry or GC.

The selected correction refreshes AssetTools through its public API immediately before GC, while retired definitions remain
valid. UE5.8's GC broadcasts this delegate before reacquiring the GC lock and explicitly allows loading in that phase.
Use the existing editor-only FrameData module, an already-loaded AssetTools module, a reentrancy guard, and a UE5.8
version guard. Unregister the callback before module shutdown. Do not disable audio, retain obsolete objects forever,
patch installed engine files, or alter renderer/effect assets. Build and rerun the identical offscreen normal-RHI startup.
After success, remove all diagnostic source/script files and rebuild/recheck the production-only result.

## Implemented correction and final result

The permanent fix is in `Source/FrameData/Editor/Private/FrameDataEditor.cpp`. The editor-only module registers a
pre-GC callback after acquiring AssetTools and removes it before shutdown. On UE5.8, on the game thread and outside
commandlets, it asks the already-loaded AssetTools module for its action list. This public call reconciles the cache
while retired definitions are still valid. A reentrancy guard prevents recursive synchronization. The local result
contains weak action pointers; it does not keep retired definition objects alive. No extra AssetDefinition module
dependency remains.

The matched traced run passed with normal RHI and audio, the MainMenu world and Content Browser open at `/Game`:
801 Slate ticks over 10.016 seconds, including forced GC at tick 30, then exit code 0. The retired Resonance dynamic
definition was still collected. This demonstrates that the correction removes the stale cache entry rather than
masking the problem by keeping its UObject alive. Evidence: `Saved/Logs/StartupAssetDefinitionFixedTrace.log` and
`Saved/Migration/StartupAssetDefinitionFixedTrace.jsonl`.

A separate native-function rename issue then required a bounded repair of Blueprint animation-notify parent calls.
Thirteen packages were saved; all 25 inspected candidates passed fresh-process compilation with zero errors and
warnings. The user's already healthy Adelaida and existing BasePawn assets remained byte-identical. See
[SSD Blueprint parent-call repair](SSD_BLUEPRINT_PARENT_REPAIR.md) for the affected assets and preservation evidence.
That defect is separate from the AssetTools startup crash.

After both temporary native helpers were removed and their build dependencies restored, the final production-only
Editor build passed in 10.18 seconds. Evidence: `Saved/Logs/StartupAndParentRepairFinalEditorBuild.log`.
The subsequent final startup check passed with exit code 0:

- Normal RHI and audio, running offscreen; `/Game/Levels/MainMenu.MainMenu` loaded and Content Browser opened at `/Game`.
- 853 Slate ticks over 10.032 seconds, with forced garbage collection at tick 30.
- 270 live asset definitions before and after GC, with no Blueprint compiler, Python or fatal errors.
- Engine initialization took 37.60 seconds; normal shutdown completed at 23:48:46 UTC on 2026-11-09.

Evidence: `Saved/Logs/StartupAndParentRepairFinalStartup.log` and
`Saved/Migration/StartupAndParentRepairFinalStartup.jsonl`. The user also reported that the editor opened after the
startup correction. Original crash dumps, logs and source/asset backups were retained. Temporary native diagnostics,
parent-repair helpers, their generated artifacts and Python scripts were removed; no helper Python files remain in
`Saved/Migration`. The permanent pre-GC correction remains in the production Editor module.

These checks validate the repaired startup, Content Browser and GC path. They do not establish full match behavior,
legacy gameplay parity or renderer-plugin visual correctness. The user's existing effects remain untouched, and the
broader rendering/gameplay suites remain deferred.
