# Prompt for a new QuietShare task

Copy the prompt below into the new Codex task:

```text
Continue QuietShare from the private repository `TDUmii/QuietShare` on Windows. First read `handoff/QUIETSHARE_NEXT_CHAT_HANDOFF.md`, `handoff/QUIETSHARE_CONTEXT.md` and `handoff/EXTEND_LAPTOP_HANDOFF.md`. Verify the current `main`/`origin/main`, working tree, latest tag and GitHub Release before changing anything.

The current product release is v0.10.1, created at commit 90685958e950fd80ebd5633a1ffe8484d38e18eb. Handoff documentation may have a later commit; re-check live main. Keep the repository private and preserve historical Releases.

The unresolved operational observation is that QuietShare remains open while projection disappears during Extend. The user reports using a physical laptop and TV throughout an afternoon without noticing a spontaneous stop, but the image disappeared when moving the controller toward display 2. There is no timestamp, warning text, per-window state or recorded stop reason; the cause has not been reproduced. Do not describe the observation as a measured 20/30-minute soak or claim a root cause. The v0.9.2 source-window guard is intended to keep the controller on its verified source display while projecting. Automated geometry checks are not proof on the physical TV.

Continue by inspecting the current source-window guard, projection stop/revocation flow and fixed-reason local diagnostics. Preserve privacy, capture, topology, source-location and device checks. A real safety failure must stop projection without automatically restarting it. Do not weaken these guards. Do not rerun the physical TV test or timed hardware soak unless I explicitly ask. Do not install drivers, runtimes or services, request admin rights, or change system-wide display/security settings.

Complete the independent code and evidence review first. If a source-level defect is supported, implement a scoped fix with an appropriate automated regression check, then build and validate the release package. If a required physical repro detail is missing, ask me for only that detail after finishing independent work. Do not claim physical validation unless it was actually performed.

After product code changes, follow my default completion workflow: update version and user-facing docs, run appropriate automated checks, package the Windows deliverables, commit and push scoped project files to main, publish the versioned GitHub Release with its assets, and verify remote branch/tag/release asset hashes. Keep test-case descriptions and QA artifacts out of product specs and release notes; keep diagnostics, browser profiles/cookies, secrets, machine-specific paths, personal/student data and unrelated files out of Git.

Use `rtk` before supported CLI commands through the Codex Desktop PowerShell bridge, per AGENTS.md. Keep all findings and the final handoff in Vietnamese unless a technical name must remain in English.
```
