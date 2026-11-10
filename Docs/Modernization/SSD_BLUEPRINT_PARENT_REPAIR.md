# SSD Blueprint parent-call repair

Started: 2026-11-09. Status: complete. Thirteen packages repaired/saved; fresh Blueprint validation and final production Editor startup passed.

The user confirmed that the startup-crash fix allowed the editor to open, then reported broken parent calls when
trying to play. They explicitly requested repairing those Blueprint calls. The initial SSD rename changed native
function names and supplied CoreRedirects, but did not resave assets because Blueprint validation was then deferred.

## Evidence and scope

Normal startup now reports 40 orphan-pin compiler errors in 10 character Blueprints: BP_Adelaida, BP_Big, BP_Cream,
BP_MechaSonic, BP_MetalSonic, BP_Omega, BP_Rouge, BP_Silver, BP_Tails and BP_Vector. Each affected
`Parent: OnSSDAnimNotify` node has two orphaned execution pins plus AnimSequence and NotifName. This group was absent
from the earlier pre-rename validation logs. The native old/new parameter signatures are identical, so parameter
redirects do not address the missing parent function signature.

The base override is in `/Game/BP_Classes/BP_BasePawn`. Parent calls must continue through each character's immediate
Blueprint parent, preserving the BP_BasePawn override. Stored parent/self-context metadata alone does not prove
runtime dispatch: UE5.8 CallFunctionHandler.cpp resolves CallParent nodes through the compiling class's superclass.
Keep the SSD native naming and existing class/enum/function compatibility redirects.

## Preserved current state

A read-only name scan of the existing 597-Blueprint inventory identified 25 packages containing the old/new notify
symbol, including base and transformed/AI variants. All 25 are candidates for inspection; only proven defective nodes
will be changed. Existing edits to BP_BasePawn and BP_Adelaida were detected and preserved. The current Adelaida parent
call is already healthy, unlike the earlier startup log; it must not be overwritten by an older baseline copy.

Current package bytes/hashes are backed up in `Saved/Migration/NotifyParentRepairOriginalAssets.zip` and
`Saved/Migration/NotifyParentRepairManifest.json`. The initial read-only graph snapshots are in
`Saved/Migration/NotifyParentCallInspection*.json`. No asset has been saved at this planning checkpoint.

The temporary native helper is callable only through the headless editor workflow, is restricted to these packages,
and never saves assets itself. It checks parent ownership, exact parameters, orphan pins, graph links/defaults and
node identities before and after compiling. Healthy nodes return unchanged; a failed check prevents saving.

## Repair and validation plan

1. Inspect the base override and affected parent-call references headlessly; check user edits and back up target assets.
2. Compile the parent first, retarget only the affected OnOOSAnimNotify/OnSSDAnimNotify parent-call nodes to the current
   immediate-parent function, and reconstruct their signature pins. Preserve node identity, execution/data link endpoints,
   defaults and all unrelated graph contents.
3. Verify connections and compile only the affected assets before saving. Record exactly which packages changed.
4. Reopen in a fresh headless Editor process, verify the saved repair persists and the 40 parent-call diagnostics disappear.
5. Remove temporary migration/inspection helpers, rebuild the production-only Editor and complete the startup check.

This is targeted validation authorized by the new repair request. Old effects, broad rendering/gameplay suites,
Development/Shipping game builds and packaging remain outside this pass. No PC control is used. Progress and broader
simulation/rollback/online goals remain in [PROGRESS.md](PROGRESS.md) and [PLAN.md](PLAN.md).

## Saved repair

`Saved/Migration/NotifyParentRepairPass5.json` records a successful, selective save of 13 packages: Balan, Big, Cream,
MechaSonic, SuperMecha_Boss, MetalSonic, MetalSonic_AI, Mighty, Omega, Rouge, Silver, Tails and Vector.
The other 12 candidates, including the user's already repaired Adelaida and the existing BasePawn edits, remain unsaved.
All 25 targeted Blueprints compiled with zero errors and zero warnings during the successful pass.

Nine remaining orphaned notify parent calls were reconstructed. Four additional notify references were stored in
canonical form; earlier metadata-only inspection did not establish that these four caused wrong runtime dispatch.
BasePawn compilation also correctly conformed MetalSonic_AI's OnAnimationEnd parent reference. Its function name,
node identity and every pin/link/value remained unchanged. This validated reference update was retained when saving.

Preservation checks covered node identities, positions/comments, pin identities/types/defaults, all graph connections,
and ordinary call/event references. Only intended notify orphan-state clearance, empty implicit-self pin creation and
validated same-function parent-reference conformance were permitted. Earlier diagnostic passes saved no assets.
Unchanged legacy self-context parent metadata was preserved; compiler source confirms it can be valid.

Fresh-process verification passed: all 25 candidates required zero repairs immediately after loading and before any
explicit compilation, then compiled with zero errors and zero warnings. That verification saved no assets. Evidence:
`Saved/Migration/NotifyParentFreshValidation.json` and `Saved/Logs/NotifyParentFreshValidation.log`.

Independent SHA-256 comparison against the preserved ZIP confirmed exactly 13 changed packages and 12 byte-identical
packages, including BP_BasePawn and the user's BP_Adelaida. The temporary native helper and its two build dependencies
were removed after verification; the original Build.cs bytes were restored. The permanent editor startup fix remains.
The production-only Editor build passed in 10.18 seconds. The final normal-RHI/audio startup then passed with exit 0:
MainMenu world and Content Browser `/Game` assertions succeeded; 853 Slate ticks ran over 10.032 seconds and a forced
GC at tick 30 completed. The final log has no Blueprint compiler errors, Python errors or fatal errors.
Evidence: `Saved/Logs/StartupAndParentRepairFinalEditorBuild.log`,
`Saved/Logs/StartupAndParentRepairFinalStartup.log` and `Saved/Migration/StartupAndParentRepairFinalStartup.jsonl`.
This bounded validation does not replace a full match or rendering regression suite.

The temporary native trace/repair helpers and their eight obsolete Intermediate artifacts were removed. The Python
inspection, diagnosis, repair and startup scripts are removed after their processes finish; JSON/log/backup evidence is
retained. No game Development/Shipping build or cook was run for these repairs. No commits were made.
