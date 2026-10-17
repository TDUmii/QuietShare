# QuietShare project context for Codex handoff

Updated: 2026-10-17
Repository: private `TDUmii/QuietShare`
Current application release: `v0.9.1`
Current `main` includes the 5-minute session-expiry fix plus unreleased source-window containment and compact UI changes. The tracked `dist` binaries, root v0.9.1 ZIP and published Release remain the previously shipped v0.9.1 payload; build the current source before checking the new containment/UI behavior. The application version has not been bumped and no release has been created for this follow-up.

## Product purpose

QuietShare is a Windows 10/11 desktop app for showing selected code, images, documents, presentation material, or browser content on an extended display while keeping private material off the audience display. The app is an `.exe`/installer product. Obfuscation can raise reverse-engineering effort but cannot guarantee that a Windows binary is impossible to inspect.

## Durable product and UX decisions

- Keep the audience view focused on the selected content. Keep controls in a compact, modern desktop UI inspired by VS Code.
- The app and installation/use guidance support Vietnamese and English. Home shows the current version.
- Support choosing a file or keeping a folder/project open so the user can inspect its structure like an editor. Files should have recognizable type icons.
- The optional browser should be lightweight, support general web search and multiple tabs, and must not stop an active projection when opened or used.
- Make panels/layout customizable where already implemented; keep pin/move/hide controls easy to reach. Hiding the control layer must not stop projection or expose it through ordinary task switching. Preserve the existing privacy hotkey behavior.
- Text size can be entered manually. First focus/click places a caret at the end; zoom should remain responsive. Long settings/menu areas should scroll consistently, including small two-finger touchpad wheel packets.
- Maintain smooth hover behavior, consistent icon sizing and rounded UI details, useful empty states, and responsive layouts.
- Keep presentation rendering near the expected smooth 60 FPS on a suitable Extend setup, and control resource use without destabilizing an active projection.
- When multiple displays have similar resolutions, identify the Windows display numbers and physical displays clearly before the user starts projecting.
- No new business features during a visual polish task; bug fixes must preserve current projection/privacy behavior and be regression checked.

## Privacy and safety rules

- Fail closed when capture privacy, affinity, display topology, or device safety checks fail. Do not automatically restart after a safety failure.
- Unexpected projection stops should be visible to the presenter without taking focus or restoring a hidden control window over the audience content.
- Never include credentials, tokens, browser profiles/cookies, personal or student content, machine-specific paths, raw logs, or unrelated files in Git or release payloads.
- Keep project-scoped changes only. Current physical display test notes belong under `handoff/`, which `package.ps1` does not include in Setup/ZIP.
- Keep historical GitHub Releases/tags. Root/current Git should contain only the current ZIP archive. Release docs must not contain test-case descriptions or QA reports; keep evidence local/ignored.

## Current Extend-session incident

The report was that an Extend projection disappeared after roughly 5-10 minutes. In v0.9.0, `src/Mirror.cpp` used a fixed 300,000 ms verification lease, and a healthy running capture could be stopped when that lease expired. v0.9.1 removed this age-based stop while preserving runtime privacy/topology/capture guards. It also requests that Windows keep the system/display awake only during projection and restores the previous thread execution state after projection ends. Unexpected interruptions now notify without stealing focus.

A real 720-second soak passed on one physical monitor with WGC/renderer and a synthetic destination, including minimize/restore. It did **not** prove behavior on two physical displays. The presenter later reported using a laptop and a separate physical TV in Extend throughout the afternoon without noticing a spontaneous stop. That report has no timed checkpoints or exact session duration and must not be recorded as a measured 20/30-minute soak. Dragging the control window to the audience display reportedly caused a disappearance, but warning text and window/process state were not captured; its cause has not been reproduced.

## Current handoff task

The current source adds pre-move containment of the control window within its captured source display during projection, keeping the runtime fail-closed guards. It also reduces shell controls and browser chrome for small windows. The user requested source/native automated checks only for this implementation; no repeat physical TV test, no 20/30-minute soak and no release. The consumed field report is removed from the current Git tree at the user's request; retain this context and `handoff/EXTEND_LAPTOP_HANDOFF.md`.

Clearly distinguish presenter observations, geometry/native HWND checks and physical-hardware evidence. Do not infer that the drag-triggered disappearance was reproduced or that every physical Extend failure is resolved. Do not cut a new release unless that is the user-requested next step.

## Start prompt for Codex on the company laptop

> Read `handoff/QUIETSHARE_CONTEXT.md` and `handoff/EXTEND_LAPTOP_HANDOFF.md` before continuing QuietShare work. The physical-use report has already been consumed; do not rerun the old timed hardware protocol or create a release unless the user requests it. Use the private repository and the same signed-in Codex account. Preserve all capture/privacy/topology/device guards and distinguish automated checks from physical-display observations.
