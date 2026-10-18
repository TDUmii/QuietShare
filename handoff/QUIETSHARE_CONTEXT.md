# QuietShare project context for Codex handoff

Updated: 2026-10-18
Repository: private `TDUmii/QuietShare`
Current application version: `v0.11.0`
Current implementation retains the 5-minute session-expiry fix, pre-move source-window containment and compact shell/browser controls. The v0.10.0 release includes a local browser New tab page with user-saved shortcuts, six Settings categories, configurable command shortcuts, window opacity and script weight, and quick projection actions on the activity bar. v0.10.1 adds centered numeric/address controls, configurable icon size, browser audio and device permissions, and compact browser zoom controls while preserving the projection safeguards. Historical releases remain available.

## Tình trạng mới nhất - v0.11.0

Home có lịch sử thật được DPAPI mã hóa, ghim/xóa/mở lại; không có dữ liệu mẫu trong sản phẩm. Hai tùy chọn nhớ kích thước cửa sổ và cho phép sửa văn bản/code mặc định bật. File lưu chủ động, kiểm tra thay đổi bên ngoài và bảo vệ bản sửa chưa lưu. UI nhàn rỗi dùng nhịp 250 ms; tránh resize viewer trùng.

Người dùng đã xác nhận toàn bộ kiểm thử thực tế hiện không còn lỗi. Ghi nhận theo lời người dùng, không suy ra thời lượng đo, build cụ thể hay nguyên nhân lần mất hình cũ. Lượt phát triển v0.11.0 không lặp lại TV/hardware soak và không xác nhận độc lập phần cứng. Các mục sự cố/protocol bên dưới giữ làm lịch sử tham khảo.

## Product purpose

QuietShare is a Windows 10/11 desktop app for showing selected code, images, documents, presentation material, or browser content on an extended display while keeping private material off the audience display. The app is an `.exe`/installer product. Obfuscation can raise reverse-engineering effort but cannot guarantee that a Windows binary is impossible to inspect.

## Durable product and UX decisions

- Keep the audience view focused on the selected content. Keep controls in a compact, modern desktop UI inspired by VS Code.
- The app and installation/use guidance support Vietnamese and English. Home shows the current version.
- Support choosing a file or keeping a folder/project open so the user can inspect its structure like an editor. Files should have recognizable type icons.
- The optional browser should be lightweight, support general web search and multiple tabs, and must not stop an active projection when opened or used.
- Make panels/layout customizable where already implemented; keep pin/move/hide controls easy to reach. Hiding the control layer must not stop projection or expose it through ordinary task switching. Preserve the existing privacy hotkey behavior.
- Text size can be entered manually. First focus/click places a caret at the end; zoom should remain responsive. Long settings/menu areas should scroll consistently, including small two-finger touchpad wheel packets.
- Opacity, text size/zoom and icon size use decrement/edit/increment controls with horizontally and vertically centered values. Retain the first-focus end caret in numeric inputs and normal selection afterwards.
- Maintain smooth hover behavior, rounded UI details, useful empty states and responsive layouts. Icon size is configurable from 12 to 24 px, default 16 px; small controls cap the glyph to fit. Keep the activity-bar status dot fully visible and avoid white control-background flashes during page/tab changes.
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

The user subsequently reports that QuietShare remains open while only projection disappears. No exact triggering action, warning or event record has established the cause of that latest report. Do not describe the current changes as a reproduction or proof that every physical Extend failure is fixed.

The v0.10.0 follow-up keeps healthy sessions during an active-session WGC check and display notifications that do not change actual privacy/topology/device conditions. Actual safety failures still revoke output with no automatic restart. A single local `projection-stop.json` is overwritten on unexpected stops and contains fixed reason codes, UTC/session timing, HRESULT and resource mode only. It excludes content, images, URLs, paths and display identifiers; do not commit personal diagnostics or browser profiles.

The control window uses uniform 40-100% alpha, including text; it does not provide background-only transparency. Script weight supports 400/600/700. Settings has Appearance, Reading, Workspace, Projection, Shortcuts and Browser. Browser enablement moves from Workspace to Browser. Appearance defaults reset opacity, weight and icon size; Shortcut defaults reset bindings only. Projection configuration/status replaces the separate Presentation page; WGC verification and start/stop live on the activity bar. Ten command bindings are configurable, with global mask/recall registration preserved on rejected changes.

The browser opens a local New tab page instead of fixed Google/ChatGPT destinations, with up to ten saved shortcut tiles and up to eight simultaneous web tabs. Saved URLs belong to the local browser profile and never to Git or release payloads. The address container remains centered in the navigation row, with single-line text and a vertically centered caret; first-click select-all is retained. Zoom out, zoom level/reset and zoom in are directly available beside navigation, with 25-400% zoom per tab.

Browser Settings contains engine enablement, audio mute, camera and microphone options. The three audio/camera/microphone preferences default to false and persist locally; engine enablement remains off on each application launch. Native camera/microphone grants require the corresponding option, a visible capture-verified HTTPS tab and a requesting origin matching the main page's origin. Website grants are not persisted, and Windows device permissions still apply. Turning off a previously granted permission revokes it and can reload the affected tab to unload active tracks. Keep capture verification, unsolicited-popup blocking, background suspension and the existing restrictions on file upload/download, screen sharing, notifications, DevTools, native context menus, external protocols and native authentication dialogs.

No repeat physical TV or timed hardware run is requested in this follow-up. The consumed field report remains removed from the current Git tree; retain this context and `handoff/EXTEND_LAPTOP_HANDOFF.md`. Keep product documentation and release notes focused on features, installation, use and actual limitations; keep local validation evidence out of the payload.

Clearly distinguish presenter observations, geometry/native HWND checks and physical-hardware evidence. Do not infer that the drag-triggered disappearance was reproduced or that every physical Extend failure is resolved. The user's standing preference is to complete product edits with a versioned build, tests, package, commit/push and GitHub Release by default, while preserving historical releases and respecting explicit task constraints.

## Start prompt for Codex on the company laptop

> Read `handoff/QUIETSHARE_CONTEXT.md` and `handoff/EXTEND_LAPTOP_HANDOFF.md` before continuing QuietShare work. Do not rerun the old timed hardware protocol; the user did not request another TV/soak test. Use the private repository and the same signed-in Codex account. Preserve all capture/privacy/topology/device guards and distinguish automated checks from physical-display observations. After product code changes, follow the user's default release workflow: update version/docs, build, test, package, commit/push and publish a versioned release unless the task gives a specific constraint.
