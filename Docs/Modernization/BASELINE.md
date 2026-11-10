# Legacy runtime baseline

Recorded: 2026-11-09. Status: reference identification/audit complete; historical gameplay and appearance parity remain unverified.

This document covers the supplied UE4 reference. Current UE5.8 results belong in [MIGRATION.md](MIGRATION.md),
[GAMEPLAY_VALIDATION.md](GAMEPLAY_VALIDATION.md), and [AUTHORING.md](AUTHORING.md); they do not establish legacy parity.

## Reference artifact

The user supplied the existing packaged game at `C:/Users/Jaime/Desktop/SonicSmackdown` as the legacy reference.
The build was launched from its existing bootstrap executable, without modifying its executable or pak files.

| Item | Recorded value |
| --- | --- |
| Bootstrap | `C:/Users/Jaime/Desktop/SonicSmackdown/SonicSmackdown.exe` |
| Shipping executable | `OriginOfStormsMaster/Binaries/Win64/OriginOfStormsMaster-Win64-Shipping.exe` |
| Executable size | 76,438,016 bytes |
| Executable SHA-256 | `5BCD7889D702EC34AE28AB0BD72E1F798B6D0CE7862BC0AF2CA1E5906DC20C45` |
| Executable file timestamp | 2024-07-06 18:08:09, as reported by the local filesystem |
| Embedded engine build string | `++UE4+Release-4.27-CL-18319896` |
| Pak | `OriginOfStormsMaster/Content/Paks/OriginOfStormsMaster-WindowsNoEditor.pak` |
| Pak size | 1,530,364,402 bytes |
| Pak SHA-256 | `0F62632CD2577961ACDBFC7B432138938F8C21372EEA622533C44E1DEE3DEBA1` |
| Pak file timestamp | 2024-07-07 11:40:20, as reported by the local filesystem |
| Observed window title | `Sonic Smackdown` |

The engine string was read from the executable's embedded strings. It agrees with the engine changelist recorded
in the source checkout's historical UE4.27.2 logs. Windows executable version metadata did not provide a product
version. No in-game version label has been observed yet.

This package predates repository revision `11c52082` (2024-08-10). Its exact source commit and content parity with
that revision are unknown. Use it as an identified gameplay reference, not proof of exact source-to-binary parity.
If a behavior differs after migration, determine whether it already changed between this package and the source
revision before calling it a UE5 regression.

## What was verified

- The bootstrap launched the shipping executable and it exposed a targetable `Sonic Smackdown` window.
- The running window belongs to the supplied shipping executable, rather than an editor or another project.
- Native accessibility inspection exposes a game window with title bar, system menu, minimize, maximize, and close controls.
- The user reported switching the game to windowed mode during inspection.
- The reference executables and pak were read only. No project/plugin/content files were changed by this baseline task.

No menu, character selection, match rendering, input response, physics result, or audio output has been verified.
An existing process/window alone is not a passed gameplay test.

## Interactive inspection limitation

Computer Use screenshot capture returned `FrameArrived timed out` initially, then repeatedly
`window capture timed out: timed out waiting on channel` after fresh window selection. The same failure occurred
after the user switched to windowed mode and after foreground activation. Accessibility-only inspection works,
but exposes no in-game controls or readable game contents.

UI input was stopped after the documented capture recovery attempts. No blind menu navigation or gameplay commands
were sent. Captures are not available to establish a visual baseline. This is an observation-tool limitation;
it does not establish that the legacy game fails to render for the user.

The user's subsequent instruction requires MCP/headless Python and prohibits PC control for Unreal editing/testing.
No further PC-control baseline attempts will be made. A matching UE4.27.2 engine installation is unavailable, and this
shipping executable does not provide the project's editor Python interface. Runtime/visual comparison remains
unverified under the available method.

The migrated editor standalone Sonic-versus-Shadow smoke later passed 44 checks with zero failures. Its initial captures
showed "Preparing Shaders"; two subsequent packaged Development runs each passed 44/0, and all five captures from each
showed colored fighters, HUD and stage without that overlay. Final helper-free builds and the Shipping archive passed;
Shipping also passed a separate 45-second startup-only observation. The gameplay smoke runs bypassed menu selection
and covered one stage/short move sequence, not the pending observations of this legacy package or a full Shipping match.

The owned temporary captures/save fixtures and helpers were removed after review; validation logs and audit JSON remain.
Successful UE5 material readiness for the sample does not establish historical appearance parity or fix the four
material failures independently confirmed in the old UE4 logs.

## Comparison checklist

Use the same characters, stage, settings, and input sequence in this package and the migrated build. Record the actual
selection and result in this table when observation is available. Do not substitute the planned result for a measurement.

| Check | Required observation | Current result |
| --- | --- | --- |
| Startup and main menu | Version label, rendered menu, selectable options, responsive navigation | Pending visual observation |
| Character selection | Select both participants, verify portraits/models and return navigation | Pending |
| Stage/training entry | Record selected stage and reach a playable training/local-match view | Pending |
| Idle/facing | Both fighters visible, stable floor contact, expected facing before and after crossing | Pending |
| Ground movement | Walk/dash in both directions, stopping behavior, opponent push, corner boundaries | Pending |
| Jump and landing | Neutral/forward/back jump, visible trajectory, landing position and grounded transition | Pending |
| Basic attack and block | Startup/contact/recovery, hit reaction, pushback, resource/HUD changes | Pending |
| Knockdown/recovery | Face-up/down reaction and recovery behavior for a recorded move | Pending |
| Projectile | Record move, direction, travel/contact, clash if supported | Pending |
| Transformation/freeze | Record character and move; check appearance, movement pause and resume | Pending |
| Pause/resume | Pause menu opens, resume returns to play, inputs behave correctly afterward | Pending |
| Match completion | Complete a local match, observe results and rematch/menu return | Pending |

For exact physics comparisons, supplement visual checks with reproducible input recordings and measured state once
instrumentation is available. This packaged build does not expose a verified deterministic replay or frame-state log.
The source-confirmed issue backlog remains in [AUDIT.md](AUDIT.md); none is marked reproduced by this inspection.
The bounded Blueprint ownership map is complete in [BLUEPRINTS.md](BLUEPRINTS.md). Broader baseline/behavior coverage
remains part of the completion gates in [PLAN.md](PLAN.md).
