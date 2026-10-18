# QuietShare - handoff for the next Codex chat

- Updated: 2026-10-18
- Repository: private `TDUmii/QuietShare`
- Branch: `main`
- Current app release: `v0.10.1`
- Product release commit: `90685958e950fd80ebd5633a1ffe8484d38e18eb`

## Current state

QuietShare v0.10.1 is published as the latest GitHub Release. Its tag points to product commit `90685958e950fd80ebd5633a1ffe8484d38e18eb`. Handoff documentation may be added in a later commit, so verify the live `main` SHA and working tree before continuing. The private repository and older v0.10.0 Release remain intact. The project tree contains only the current portable ZIP; the v0.10.0 ZIP was replaced while its historical release asset was retained.

- Release: <https://github.com/TDUmii/QuietShare/releases/tag/v0.10.1>
- Installer: <https://github.com/TDUmii/QuietShare/releases/download/v0.10.1/QuietShare-Setup.exe>
- Portable ZIP: <https://github.com/TDUmii/QuietShare/releases/download/v0.10.1/QuietShare-v0.10.1-win-x64.zip>
- Portable executable: <https://github.com/TDUmii/QuietShare/releases/download/v0.10.1/QuietShare.exe>

## Product and safeguards

QuietShare is a Windows 10/11 desktop app for presenting selected code, files, folders, images, PDFs and optional browser pages on a separate display while keeping private content off the audience display. The app has Vietnamese and English UI, a compact VS Code-inspired control surface, a native file/project viewer, an optional multi-tab browser and a separate projection output.

Projection remains fail-closed. Preserve capture exclusion, privacy, source-window containment, display topology and device checks. A real safety failure must stop output and must never restart projection automatically. Hiding/minimizing the controller must not expose private content or stop a healthy session. Do not weaken these guards to mask a report of a disappearing image.

## Work completed

- v0.9.1 removed the fixed five-minute verification lease that could stop a healthy long-running capture. It retained runtime safety guards, requested that Windows keep the display awake during projection and restored the previous execution state afterward.
- v0.9.2 added source-display containment for the controller during projection, including interactive moves/resizes and maximize/restore paths. Native checks cover edge/corner drag constraints, resize, minimized parking, restore clamping and fail-closed behavior.
- v0.10.0 added a local New tab page, saved quick links, multi-tab browsing, configurable shortcuts and projection actions on the activity bar.
- v0.10.1 polished compact controls: centered numeric/address text and carets, decrement/edit/increment controls, configurable 12-24 px icons, visible status dot and reduced page/tab repaint flashes. Browser settings now include audio mute and camera/microphone permissions; device access defaults off, requires an eligible visible HTTPS same-origin tab and capture verification, and revoking an active permission can reload that tab. Browser zoom controls are directly available on the navigation row.
- Preferences migrate from v1 to v2 while keeping existing appearance and shortcuts. No new runtime, driver or service was installed.

## Extend observation and what remains unknown

The user reports that QuietShare stayed open while the projected image disappeared. On a laptop with a separate physical TV in Windows Extend, the user used projection through an afternoon and did not notice the earlier spontaneous stop. They also reported that the image disappeared when moving the QuietShare controller toward display 2. There is no timestamp, warning text, per-window state or captured stop reason for that event. The cause has not been reproduced. Do not record this as a measured 20/30-minute soak or claim that the drag path caused a reproduced failure.

The v0.9.2 controller guard is intended to keep the controller on the verified source display while projecting. Automated/native geometry checks passed, but they do not prove behavior on the user's physical TV. A later clarification says the application remains open and only projection stops. A local `projection-stop.json` may contain one overwritten record with a fixed reason code, UTC/session timing, HRESULT and resource mode; it excludes content, images, URLs, paths and display identifiers. Inspect it locally only when useful; never commit the user's log or profile.

There is no unfinished code change from the v0.10.1 release. The outstanding project question is whether any new evidence can explain or reproduce the physical Extend disappearance. The user explicitly said not to repeat the TV test or a 20/30-minute hardware soak for the preceding change. No physical camera/microphone test was run.

## v0.10.1 build and validation evidence

- Built with `D:\Microsoft VS Code\mingw64\bin` using `build.ps1`; shipping EXE and per-user Setup completed successfully.
- Full `tests/run-all.ps1` completed with exit code 0. The release validation report recorded `1,799 PASS, 0 FAIL`.
- Relevant final suite results: production integration `239 passed, 0 failed, 5 not tested`; browser regression `293 passed, 0 failed`; New Tab fixture `31 passed, 0 failed`; Settings `197 passed, 0 failed`; installer smoke `19 passed, 0 failed`; local QA `261 passed, 0 failed, 0 unavailable`.
- WGC automated canary passed on a one-monitor test setup. External receiver privacy and the user's two-display TV setup were not verified by that automated run.
- The portable ZIP contains 46 files byte-identical to `dist`; QA/build/profile data is excluded. All three published asset SHA-256 digests matched local artifacts.

## Rules for the next task

- Start by checking `git status`, `main` versus `origin/main`, and current release/tag. The product release commit is listed above; re-verify live Git state before continuing.
- Read `handoff/QUIETSHARE_CONTEXT.md` and `handoff/EXTEND_LAPTOP_HANDOFF.md` for durable decisions and historical hardware protocol details. Those files remain project references.
- Keep project changes scoped. Do not put test cases or QA reports in product specifications/release notes. Do not commit local diagnostics, browser profiles, cookies, personal/student content, credentials, machine paths or unrelated files.
- Do not install drivers, runtimes or services, request admin rights, or change system-wide display/security settings. Do not repeat TV/soak hardware checks unless the user later requests them.
- The user wants product completion to include a versioned build, appropriate automated tests, package, commit/push and GitHub Release by default. Preserve prior Releases. The repository is private.
- On the Codex Desktop PowerShell bridge, prefix RTK-supported CLI commands such as git, gh and rg with `rtk`, as the repository's `AGENTS.md` directs. Leave PowerShell built-ins and unsupported commands unwrapped.

## Next task focus

Continue from the unresolved Extend observation only if the new chat is meant to investigate it. Inspect the existing source-window guard, projection stop/revocation paths and fixed-reason diagnostics. Separate user observations from automated/native evidence. Build a bounded source-level hypothesis and regression coverage before proposing or making a code fix. If new repro detail is needed, first finish independent code review, then ask for the smallest missing detail. Do not infer root cause from the unmeasured drag observation.

## Ready-to-copy prompt

Use the companion file `handoff/QUIETSHARE_NEW_TASK_PROMPT.md` to start the next chat.
