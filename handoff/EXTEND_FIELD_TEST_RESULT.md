# QuietShare physical Extend test result

Status: **Presenter-reported physical Extend use recorded. No spontaneous stop reported during afternoon use; moving QuietShare to display 2 caused a disappearance. Exact timed checkpoints and the disappearance details were not recorded. UI follow-up requests are included below.**

## Build and environment

- Date/time and time zone: 2026-10-17, Asia/Saigon (UTC+07:00). The presenter retrospectively reported use throughout the afternoon; exact start/end times were not supplied.
- QuietShare version / source commit: running application UI and EXE identify version **0.9.1**. Repository is on `main` at `14bdf1567fadb43069ea82f258235e9d2f5c1cdf`, equal to the fetched `origin/main` during preflight. This commit includes `f08eede29a354f8302fcd900530368eeea8f6368`, which removed the healthy-session five-minute expiry.
- Installer or ZIP: existing per-user installation; no installation or download performed. Its EXE and all four adjacent DLLs match the repository release manifest and v0.9.1 portable ZIP entries.
- EXE SHA-256: `af40c50ddc518383a5a3ba8f95a0fa977e428011d4ee736c5c785d55cfceaefb`.
- Windows edition/version/build: Windows 10 Pro, 64-bit, 22H2, **19045.6466**.
- CPU: Intel(R) Core(TM) 3 100U.
- GPU/display adapter(s) and driver version: Intel(R) Graphics, **32.0.101.7088**, driver date 2026-06-17.
- Physical displays: the presenter confirms a laptop display and a TV are both physical, powered on, and directly observable. The presenter further confirms that the TV is a separate Extend display and is selectable as a destination in QuietShare.
- Connection: the presenter reports a **wireless/network connection to the TV, not an HDMI cable**. The exact wireless protocol/application has not been established. Windows reports output technology internal / HDMI; those API values do not establish the actual physical transport used by the TV.
- Display mode: read-only `QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS)` reports two active paths with separate, horizontally adjacent desktop regions, consistent with **Extend**. The presenter independently confirms Extend.
- Physical displays and Windows display numbers: API identifiers are `DISPLAY1` (primary/internal) and `DISPLAY6` (secondary, to the right). These GDI identifiers are not confirmed Windows Settings Identify numbers. The presenter calls the destination "display 2"; the exact selected QuietShare labels were not supplied.

| Display | Pixel resolution | Refresh rate | Desktop pixel position | Role |
| --- | --- | --- | --- | --- |
| Primary/internal (`DISPLAY1`) | 1920 × 1200 | approximately 60.0007 Hz | (0, 0) | Laptop/source in the reported arrangement |
| Secondary (`DISPLAY6`) | 1920 × 1080 | 30 Hz | (1920, 0) | TV/destination in the reported arrangement; exact selected label not recorded |

These are pixel dimensions from QueryDisplayConfig. Logical dimensions from a non-DPI-aware process were not substituted for them.

## Physical-use observations supplied by the presenter

The presenter reported directly in this handoff:

> "tôi đã thử share màn cả chiều nay thấy không bị tắt đột ngột nữa chỉ khi tôi kéo QuietShare qua màn 2 thì bị mất luôn"

- Approximate start time: afternoon, 2026-10-17; no exact clock time supplied.
- Total uninterrupted run time: described as "all afternoon"; no stopwatch duration, session boundaries, or restart history supplied. Do not convert this description into a measured uninterrupted duration.
- Did the projection disappear? The presenter reports **no spontaneous stop during ordinary afternoon use**, and a disappearance **when dragging QuietShare onto display 2**. The phrase "bị mất luôn" does not specify whether only the projected image, the projector window, the control window, or the entire application disappeared.
- Immediate action before the reported disappearance: dragging the QuietShare control window from the laptop/source toward or onto the second display/TV.
- Exact elapsed time, overlap threshold, and window position when it disappeared: not recorded.
- Did the app show its stopped notice? Exact warning text and whether a notice appeared were not supplied.
- Was the QuietShare process still running? Not reported for the disappearance. Version 0.9.1 was independently identified during this handoff before the result was reported.
- Was the projector window still present on the destination display? Not reported.
- Harmless content: a synthetic public sample was prepared during preflight. The presenter did not confirm which content was used during the retrospective afternoon observation; no presentation content was collected or included in this report.
- Actions and timestamps during the run: ordinary sharing during the afternoon, followed by dragging QuietShare to display 2. No exact action timestamps were supplied. File switching, app-screen navigation, and minimize/restore were not separately confirmed.
- Result of repeat run, if performed: not supplied; no controlled repeat after an application restart is documented.
- Relevant sanitized error/event text: none supplied. No raw application or system logs were collected for this result.

### Required timed checkpoints

| Checkpoint | Evidence available |
| --- | --- |
| 0 minutes | No timestamped start observation recorded |
| 5 minutes | No separate checkpoint recorded |
| 10 minutes | No separate checkpoint recorded |
| 15 minutes | No separate checkpoint recorded |
| 20 minutes | No separate checkpoint recorded |
| Additional 10 minutes with normal presenter actions | Not separately timed or itemized |

The afternoon-use report is useful **presenter-reported evidence on two physical Extend displays** that the previous spontaneous 5–10-minute stop was not noticed in this use. It is not a completed, instrumented 20/30-minute protocol and does not prove that every related failure is resolved. The agent did not independently watch the physical TV during that use. The earlier automation screenshot/control limitation is not a QuietShare failure result.

## Requested follow-up for Codex on the other machine

The presenter asked to record these requirements and push this report for implementation on the other machine. This handoff changes documentation only.

1. **Prevent dragging the QuietShare control window onto display 2 / the audience display.** The reported trigger is moving the control window to the second display, after which something disappears. Keep the control window within its permitted source display during projection, including at screen edges, so an accidental drag does not reach an unsafe placement. Reproduce and inspect the existing source-location guard before changing behavior. Preserve capture/privacy/topology/device checks and fail-closed handling; do not bypass a guard, expose the private control window on the TV, or automatically restart after a safety failure. The precise clamp/reposition behavior is an implementation decision to verify on real displays.
2. **Make the icons smaller.** The presenter finds the current icons and surrounding controls too large for the compact QuietShare window. Reduce their space consumption while keeping controls usable and recognizable.
3. **Prioritize the browser viewport in the compact window.** Redesign the browser screen so the web page has more of the available area. Icons, panels, spacing, and other application chrome currently take too much of the already small window. Consider compact or collapsible secondary controls while keeping essential browser navigation accessible and preserving the existing privacy hotkeys and projection behavior. The requested outcome is a larger readable browsing area at the same small window size; no exact pixel sizes or mockup were supplied.

For the next implementation/reproduction pass, capture whether the display-move symptom is an expected safety stop or a separate UI/process failure, its exact warning and window/process state, and a controlled before/after physical-display run. A safety stop must remain a stop; preventing an accidental move should avoid triggering it rather than remove its protection.

### Source inspection pointers (not runtime proof)

- At the recorded preflight commit, `src/Mirror.cpp` checks that the entire private control-window rectangle remains within the source monitor. During a running projection, `Tick()` calls `Fail(State::WrongLocation)` if that check fails; the failure clears/hides the projector and queues a stopped notice. This is consistent with the reported drag-triggered disappearance, but no warning/log or controlled reproduction confirms that it was the actual path taken.
- `src/main.cpp` contains the window-move handling and the compact/browser layout and icon drawing. These are the starting points for preventing movement onto the audience display and reclaiming browser viewport space. `src/Browser.cpp` applies the viewport bounds supplied by the main layout. Inspect the layout before changing browser content zoom or the browser engine.

## Confirmed scope and limitations

- Independently checked in preflight: fetched `main`, v0.9.1 binary identity and bundled DLLs, the retained session-expiry fix, Windows/GPU information, and two active Extend desktop regions.
- Confirmed by the presenter: two physical displays, a wireless/network-connected TV selectable as a separate Extend destination, afternoon sharing without a noticed spontaneous stop, and disappearance on dragging QuietShare to display 2.
- Not confirmed: exact selected display labels, exact duration/session continuity, individual timed checkpoints, warning text, process/projector/control-window state at the disappearance, a controlled repeat, and the technical root cause.
- The display-move symptom has not been reproduced by the agent. No source defect is asserted as proven, and none of the requested UI changes has been implemented in this handoff.
- No administrator elevation, driver/runtime/service installation, system-setting changes, source changes, or new release were performed.
