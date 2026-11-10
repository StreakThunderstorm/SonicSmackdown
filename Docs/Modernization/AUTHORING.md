# Custom authoring tool validation

Checked on 2026-11-09 with UE 5.8.4, changelist 58902052, in the isolated migration worktree.
Validation used Unreal Python and an offscreen DX11/SM5 editor. No desktop control or content saves were used.

## Inventory and tool initialization

The Asset Registry reports 27 Move Network assets and no saved Frame Data assets under `/Game`.
Four representative Move Networks were opened through `AssetEditorSubsystem.open_editor_for_assets`, allowed
to process Slate ticks, and closed through `close_all_editors_for_asset`. Each open returned true and each close
reported exactly one editor. These calls invoke the project's registered custom asset-type actions and toolkits;
this check exercised editor initialization beyond simply loading the assets.

| Asset | Runtime nodes | Root nodes | Editor nodes | Open/tick/close |
| --- | ---: | ---: | ---: | --- |
| `/Game/Characters/Sonic/MN_Sonic` | 71 | 13 | 168 | Passed |
| `/Game/Characters/Sonic/MN_SuperSonic` | 32 | 4 | 92 | Passed |
| `/Game/Characters/Shadow/MN_Shadow` | 78 | 14 | 208 | Passed |
| `/Game/Characters/Shadow/MN_SuperShadow` | 30 | 4 | 76 | Passed |

All four editor graphs are `MNEdGraph` instances. Their runtime/root arrays contain no null entries.
SHA-256 checks after the run confirmed that all four asset files still match the original checkout byte for byte.
Evidence: `Saved/Logs/AuthoringValidation4.log`, tagged `[AUTHORING_PROBE]`.

## Frame Data result and inherited limits

Because no saved Frame Data asset exists, the probe used an unsaved transient `FrameData` object referencing
`/Game/Characters/Sonic/Animations/Sonic/Sonic_Standing_L`. The initial attempt reached the custom toolkit and
crashed during `SFDEditorViewport::Construct` / `SetPreviewAnimation` before playback (process exit 3).
The viewport's raw preview pointers were uninitialized; `SetPreviewAnimation` reads `PreviewPawn` before attempting
to destroy it. The migration now initializes those pointers and guards play/seek/tick against missing preview data.
The Editor Development rebuild passed (`Saved/Logs/AuthoringRebuild.log`). A targeted offscreen recheck then opened
the custom Frame Data editor, processed 92 Slate ticks, and closed exactly one editor instance. The probe reported
zero failures, no dirty packages, and process exit 0 (`Saved/Logs/AuthoringFrameDataFinal.log`).

The original source already comments out preview-pawn and animation-instance creation in
`Source/FrameData/Editor/Private/FrameDataAssetEditor/SFDEditorViewport.cpp`. That block remains unchanged.
Opening the tool therefore does not establish working animated preview or scrubbing. Python also cannot read the
protected `FrameData` array through its public property API; no native exposure was added solely for this probe.
These limits agree with the unfinished Frame Data infrastructure recorded in `AUDIT.md`.

The successful open/close run still reported the missing optional outline material
`/Game/PostProcess/MI_PP_Outlines`, obsolete toolbar-layout tabs, and a missing properties-tab brush.
It also exposed inherited asset-load warnings for the removed `/Engine/EngineMeshes/Humanoid` dependency and
the legacy `MSM_CelShading` enum on a Sonic material. Tool initialization success does not resolve those asset or
appearance issues. The final run applied `EnhancedInput.bEnableAutoUpgrade=0` and logged no input auto-upgrade action.

## Scope of evidence

This check covers asset discovery, representative graph data, custom toolkit initialization, and orderly editor closure.
It does not cover graph edits, save/reopen round trips, visual layout assessment, full roster authoring, or functional
Frame Data playback. The temporary Python probe and copied shader test source were removed after validation;
logs remain as evidence. No gameplay source or asset was altered by this authoring probe.
