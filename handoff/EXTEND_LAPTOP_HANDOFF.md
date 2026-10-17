# QuietShare Extend laptop handoff

Updated: 2026-10-17
Repository: private `TDUmii/QuietShare`
Expected baseline: `main` at `f08eede29a354f8302fcd900530368eeea8f6368`, release `v0.9.1`

## Goal

Reproduce or rule out the report that QuietShare stops showing the projection on a physically extended display after about 5-10 minutes. This check must use the company laptop with two physical displays in Windows Extend mode. Do not treat the earlier synthetic destination as physical-display validation.

## What changed in v0.9.1

The previous build had an unconditional five-minute verification lease in `src/Mirror.cpp`. A healthy running session reached the 300,000 ms threshold and stopped. Canary verification could reset the timer, which made the observed interval vary. v0.9.1 removed expiry for a healthy, existing capture session while retaining runtime safety checks. It also requests that Windows keep the system and display awake only while projection is active, then restores the prior execution state. Unexpected projection interruption now raises a non-focus-stealing title/status/taskbar/sound notice.

The code still stops projection on safety failures, explicit Stop/F8, close, suspend, display change, capture/device errors, or failed topology/privacy checks. It does not automatically restart a failed projection.

The v0.9.1 soak ran 12 minutes with WGC and renderer on one physical monitor plus a synthetic destination, including minimize/restore. It did not validate a real second monitor. The physical Extend test below is the missing evidence.

## Laptop test procedure

Admin rights are not required. Do not install drivers, runtimes, or services or change system-wide display/security settings. Chrome/browser access is not part of this test; the earlier Codex URL-recognition failure is unrelated. Use the repository files and run QuietShare directly. If Codex computer-use controls are unavailable, ask the presenter to perform the visible UI actions locally and record the observed results.

1. Use the latest v0.9.1 app or a later build that explicitly includes the fix. If QuietShare is not already installed, prefer the portable ZIP: extract the complete ZIP into a user-writable folder (for example `%LOCALAPPDATA%\QuietShare-Test`) and run `QuietShare.exe` there with all bundled DLLs beside it. The Setup is also per-user (`asInvoker`) and installs under `%LOCALAPPDATA%\Programs\QuietShare`, so it does not require admin rights. Close older QuietShare copies first.
2. On Windows 10/11, connect both physical displays and select **Extend these displays** in Windows Display settings. Record which numbered display is laptop/main and which is the physical destination. Matching resolutions are fine; identify them by Windows display number and physical connection.
3. Start QuietShare, select the physical destination in the projection workflow, start sharing, and confirm that content is visible there. Keep a simple harmless local page or code sample on the source display; do not expose private class, account, or student data during recording.
4. Leave projection running for at least 20 minutes. Record whether the projected content disappears and the elapsed time. Check at 0, 5, 10, 15, and 20 minutes without stopping/restarting the session. If it fails, note whether QuietShare remains open, whether its stopped warning appears, and whether the projector window remains on the destination.
5. If the session survives, continue normal presenter actions for another 10 minutes: switch the viewed file/content, move between app screens, and optionally minimize the control window. Note each action and timestamp. Do not change display topology mid-run unless specifically checking that safety path.
6. Run one repeat after a full app restart if the first run fails or is ambiguous. Do not repeatedly restart during a run, because that erases the failure timing.
7. Return the completed `handoff/EXTEND_FIELD_TEST_RESULT.md` with the tested build, Windows version, GPU/display adapters, display arrangement, elapsed time, observed failure time, app warning text, and exact actions before failure. Include relevant sanitized app/Event Viewer errors only. Never commit user names, school/student data, full local paths, browser profiles, cookies, tokens, or unrelated logs.

## Return path for Codex

On the company laptop, open this private repository in Codex and ask it to follow this handoff. Have it fill `handoff/EXTEND_FIELD_TEST_RESULT.md`, check that the report contains no private data, and commit/push only that result file (and any explicitly requested app fix). Back in this Codex task, pull the private repo or attach the result file. I can then inspect the physical test evidence, reproduce the relevant path, and prepare a bounded fix.

This task currently has no direct connection to the company laptop's Codex host. The repository handoff is the exchange channel; it does not give this task remote control of that computer.
