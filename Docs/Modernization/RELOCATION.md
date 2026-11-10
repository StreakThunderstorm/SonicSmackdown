# Migration project relocation

Completed: 2026-11-09. Active project: `F:/UE4 Work/SonicSmackdown_R/SonicSmackdown.uproject`.

The user requested moving the whole upgraded project, including generated caches and the historical package, from
`C:/Users/Jaime/.codex/worktrees/sonic-ue58-migration/Sonic_Smackdown` to `F:/UE4 Work/SonicSmackdown_R`.
The original UE4 checkout at `F:/UE4 Work/Sonic_Smackdown` remains separate and preserved.

## Preservation evidence

- Robocopy transferred 89,924 files / 35,393,417,372 bytes, with zero failures, skipped files or mismatches.
- SHA-256 verification confirmed every original file's contents at the destination, with zero preservation failures.
- An independently opened Unreal session created newer destination data during verification. Nine changed or missing
  original files were retained under `Saved/Migration/RelocationOriginalFiles`, while newer destination files were preserved.
  This includes DefaultEngine.ini, whose newer version adds an AndroidFileServer settings section.
- Git worktree repair updated the shared metadata backlink to F:. HEAD and all 363 unstaged/untracked status entries
  matched the saved pre-move state exactly before the subsequent SSD rename. No index changes or commits were made.
- Evidence: `Saved/Migration/RelocationVerification.json` and `Saved/Migration/RelocationCopy.log`.

The old C: project contents were removed after verification. The chat held its empty working directory open, so that
empty directory was converted to a Windows junction pointing to F:. There is one project data copy; the old path is
only a compatibility alias. The app rejected attaching F: directly as an unmanaged location, so the alias keeps the
existing chat working. All ongoing commands and current project links use F: explicitly.

## Follow-up

Project files were regenerated at F: after the OOS-to-SSD rename (11.72 seconds); SonicSmackdownEditor then
compiled successfully (167 actions, 167.77 seconds). Current solutions no longer contain the old checkout path.
Tests, editor launches, game Development/Shipping builds and packaging remain deferred at the user's request.
The manual destination session produced a crash; read-only findings are recorded in
[EDITOR_CRASH_2026_11_09.md](EDITOR_CRASH_2026_11_09.md). No reproduction was launched.
