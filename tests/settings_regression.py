"""Consolidated Settings behavior in the separately built synthetic QA app.

Targets only the owned child PID and native controls. It never samples desktop
pixels, changes a personal profile, or sends physical keyboard/mouse input.
"""
from __future__ import annotations

import ctypes
from ctypes import wintypes
import hashlib
import json
import os
import subprocess
import sys
import time
import uuid

sys.dont_write_bytecode = True
import lifecycle_regression as native

ROOT, USER32 = native.ROOT, native.USER32
EXE = ROOT / "build/QuietShare-QA.exe"
GDI32 = ctypes.WinDLL("gdi32", use_last_error=True)
USER32.GetParent.argtypes = [wintypes.HWND]
USER32.GetParent.restype = wintypes.HWND
USER32.GetWindowRgn.argtypes = [wintypes.HWND, wintypes.HRGN]
USER32.GetWindowRgn.restype = ctypes.c_int
USER32.GetWindowRect.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.RECT)]
USER32.GetWindowRect.restype = wintypes.BOOL
USER32.SetWindowPos.argtypes = [wintypes.HWND, wintypes.HWND, ctypes.c_int,
                               ctypes.c_int, ctypes.c_int, ctypes.c_int, wintypes.UINT]
USER32.SetWindowPos.restype = wintypes.BOOL
USER32.GetDpiForWindow.argtypes = [wintypes.HWND]
USER32.GetDpiForWindow.restype = wintypes.UINT
GDI32.CreateRectRgn.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int]
GDI32.CreateRectRgn.restype = wintypes.HRGN
GDI32.DeleteObject.argtypes = [wintypes.HGDIOBJ]
GDI32.DeleteObject.restype = wintypes.BOOL
GDI32.GetRgnBox.argtypes = [wintypes.HRGN, ctypes.POINTER(wintypes.RECT)]
GDI32.GetRgnBox.restype = ctypes.c_int


def wait_for(predicate, timeout=8):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        value = predicate()
        if value:
            return value
        time.sleep(.04)
    return None


def region_visible(control):
    """Offscreen settings keep WS_VISIBLE but have an empty clipping region."""
    if not USER32.IsWindowVisible(control):
        return False
    region = GDI32.CreateRectRgn(0, 0, 0, 0)
    if not region:
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        return USER32.GetWindowRgn(control, region) != 1  # NULLREGION
    finally:
        GDI32.DeleteObject(region)


def numeric_clip_bounds(control, scrollbar):
    """Compare the actual native edit region to its full viewport intersection.

    The scrollbar spans the Settings viewport vertically. Reading owned HWND
    geometry/regions avoids desktop pixels and detects footer/header leakage
    even when a partially visible edit still has WS_VISIBLE set.
    """
    bounds, viewport = wintypes.RECT(), wintypes.RECT()
    if not USER32.GetWindowRect(control, ctypes.byref(bounds)) or not USER32.GetWindowRect(scrollbar, ctypes.byref(viewport)):
        raise ctypes.WinError(ctypes.get_last_error())
    region = GDI32.CreateRectRgn(0, 0, 0, 0)
    if not region:
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        kind = USER32.GetWindowRgn(control, region)
        if not kind:
            raise AssertionError("Numeric Settings edit has no explicit viewport clipping region")
        clipped = wintypes.RECT()
        if not GDI32.GetRgnBox(region, ctypes.byref(clipped)):
            raise ctypes.WinError(ctypes.get_last_error())
        width, height = bounds.right - bounds.left, bounds.bottom - bounds.top
        top, bottom = max(0, viewport.top - bounds.top), min(height, viewport.bottom - bounds.top)
        expected = (0, top, width, bottom) if bottom > top else (0, 0, 0, 0)
        actual = (clipped.left, clipped.top, clipped.right, clipped.bottom)
        return actual, expected, height
    finally:
        GDI32.DeleteObject(region)


def main():
    output = ROOT / "artifacts/qa-settings"
    folder = output / uuid.uuid4().hex
    folder.mkdir(parents=True)
    environment = os.environ.copy()
    environment["LOCALAPPDATA"] = str(folder / "User")
    process = None
    window = None
    checks, failure = {}, None

    def check(name, value):
        checks[name] = bool(value)
        print(("PASS " if value else "FAIL ") + name, flush=True)
        if not value:
            raise AssertionError(name)

    def query(key):
        return native.send(window, 0x800F, key, timeout=6000)

    def command(identifier):
        native.send(window, 0x111, identifier)

    def control(identifier):
        return USER32.GetDlgItem(window, identifier)

    def focus(identifier):
        # Exercise the same EnsureSettingsVisible handler used by native Tab.
        # This is a PID-bound focus message, not physical keyboard delivery.
        native.send(control(identifier), 0x7)

    def scroll(code, position=0):
        native.send(window, 0x115, code | (position << 16), control(187))

    def posted_key(identifier, key):
        target = control(identifier)
        if not USER32.PostMessageW(target, 0x100, key, 1) or not USER32.PostMessageW(target, 0x101, key, 0xC0000001):
            raise RuntimeError("Could not post key messages to owned Settings control")
        time.sleep(.1)

    def set_size(value):
        focus(163)
        native.set_control_text(control(163), value)
        # Posted keys pass through the real IsDialogMessage/dispatch loop.
        posted_key(163, 0x0D)

    try:
        check("noExistingQuietShareWindow", not native.app_windows())
        startup = subprocess.STARTUPINFO()
        startup.dwFlags, startup.wShowWindow = subprocess.STARTF_USESHOWWINDOW, 0
        process = subprocess.Popen([str(EXE), "--qa-scrollbar", str(folder)], cwd=ROOT,
                                   env=environment, startupinfo=startup)
        window = wait_for(lambda: next(iter(native.app_windows(process.pid)), None))
        check("syntheticOwnedQaWindow", window and native.window_pid(window) == process.pid)
        check("settingsControlsInitialized", wait_for(lambda: control(187) and control(163)))
        USER32.ShowWindow(window, 4)
        scale = USER32.GetDpiForWindow(window) / 96
        check("resizeToMinimumSettingsWindow", USER32.SetWindowPos(window, None, 0, 0,
              round(430 * scale), round(320 * scale), 0x16))
        command(123)
        check("settingsPageActive", query(21) == 1)
        check("fontSliderRemoved", not control(130))
        identifiers = (106, 107, 163, 108, 104, 146, 181, 151, 182, 183, 184,
                       185, 152, 155, 160, 161, 143, 187)
        check("consolidatedControlsKeepStableMainParent", all(control(identifier) and
              USER32.GetParent(control(identifier)) == window for identifier in identifiers))
        scrollbar_class = ctypes.create_unicode_buffer(64)
        USER32.GetClassNameW(control(187), scrollbar_class, len(scrollbar_class))
        check("settingsUsesNativeScrollbar", scrollbar_class.value.casefold() == "scrollbar")
        check("minimumSettingsCanScroll", query(20) > 0 and region_visible(control(187)))
        scroll(6)
        check("settingsScrollTop", query(19) == 0)
        actual, expected, numeric_height = numeric_clip_bounds(control(163), control(187))
        check("minimumNumericEditPartiallyClipped", 0 < actual[3] - actual[1] < numeric_height)
        check("minimumNumericClipMatchesFullViewportIntersection", actual == expected)
        focus(163)
        actual, expected, numeric_height = numeric_clip_bounds(control(163), control(187))
        check("focusedNumericClipMatchesFullViewportIntersection", actual == expected)
        check("focusedNumericEditFullyVisible", actual[1] == 0 and actual[3] == numeric_height)
        scroll(6)
        command(107)
        command(107)
        actual, expected, numeric_height = numeric_clip_bounds(control(163), control(187))
        check("numericClipPreservedAfterSettingsRelayout", actual == expected and 0 < actual[3] - actual[1] < numeric_height)
        maximum = query(20)
        for index, position in enumerate((maximum // 4, maximum // 2, maximum)):
            scroll(4, position)
            check("thumbPositionUpdatesImmediately:" + str(index), query(19) == position)
        scroll(7)
        check("settingsScrollBottom", query(19) == query(20))
        check("lastDisplayControlVisibleAtBottom", region_visible(control(143)))
        scroll(6)
        check("offscreenDisplayControlIsClipped", not region_visible(control(143)))
        focus(143)
        check("buttonFocusScrollsIntoView", query(19) > 0 and region_visible(control(143)))
        scroll(6)
        focus(160)
        check("comboFocusScrollsIntoView", query(19) > 0 and region_visible(control(160)))
        check("sourceMonitorListAvailableInSettings", native.send(control(160), 0x146) >= 1)
        check("destinationMonitorListAvailableInSettings", native.send(control(161), 0x146) >= 1)
        scroll(6)
        native.send(control(106), 0x20A, 0xFF880000)
        check("wheelOverSettingScrollsPage", query(19) > 0)
        scroll(6)
        native.send(window, 0x20A, 0x00780000)
        check("wheelClampsAtTop", query(19) == 0)

        command(120)
        command(123)
        for value in (8, 37, 91, 200):
            set_size(str(value))
            check("manualFontSize:" + str(value), native.control_text(control(163)) == str(value)
                  and query(4) == value * 100 // 24 and query(22) == 0)
        for label, value in (("below", "7"), ("above", "201"), ("invalid", "abc")):
            set_size(value)
            check("invalidFontRestoresLastValid:" + label, native.control_text(control(163)) == "200"
                  and query(4) == 200 * 100 // 24 and query(22) == 1)
        command(184)
        check("statusBarHiddenIndependently", not query(16) & 8)
        set_size("9999")
        check("sizeErrorSurvivesHiddenStatusBar", query(22) == 1 and query(4) == 200 * 100 // 24)
        set_size("24")
        check("validSizeClearsInlineError", query(22) == 0 and native.control_text(control(163)) == "24")
        command(184)

        before = native.control_text(control(146))
        command(146)
        check("scriptWrapRemainsDisabledAndUnchanged", not USER32.IsWindowEnabled(control(146))
              and native.control_text(control(146)) == before)
        for identifier, label in ((106, "topmost"), (152, "ignoredFolders"), (143, "pointer")):
            before = native.control_text(control(identifier))
            command(identifier)
            after = native.control_text(control(identifier))
            check("settingToggleUpdatesState:" + label, bool(before) and bool(after) and before != after and query(21) == 1)
            command(identifier)
            check("settingToggleRestoresState:" + label, native.control_text(control(identifier)) == before and query(21) == 1)
        before = native.control_text(control(104))
        speed_labels = []
        for _ in range(3):
            command(104)
            speed_labels.append(native.control_text(control(104)))
        check("readingSpeedCyclesInSettings", len(set(speed_labels)) == 3 and speed_labels[-1] == before and query(21) == 1)
        before = native.control_text(control(107))
        command(107)
        check("languageChangesInSettings", bool(native.control_text(control(107))) and native.control_text(control(107)) != before and query(21) == 1)
        command(107)
        check("languageRestoresInSettings", native.control_text(control(107)) == before)
        for identifier, bit, label in ((181, 1, "activity"), (151, 16, "explorer"),
                                       (182, 2, "tabs"), (183, 4, "toolbar"), (184, 8, "status")):
            before = query(16)
            command(identifier)
            check("layoutSettingChangesOnlyOwnBit:" + label, query(16) == before ^ bit and query(21) == 1)
            command(identifier)
        command(185)
        check("resetLayoutAvailableInSettings", query(16) == 31 and query(17) == 208 and query(21) == 1)
        check("qaSettingsDoesNotStartBrowser", query(10) == 0 and query(11) == 0)
        command(121)
        command(123)
        focus(106)
        before = native.control_text(control(106))
        check("readerPausedBeforeSettingsKey", query(23) == 0)
        posted_key(106, 0x20)
        check("settingsSpaceOperatesFocusedButton", native.control_text(control(106)) != before)
        check("settingsSpaceDoesNotStartReader", query(23) == 0 and query(21) == 1)
        posted_key(106, 0x20)
        check("settingsSpaceRestoresFocusedButton", native.control_text(control(106)) == before and query(23) == 0)

        command(139)
        check("monitorSelectorsNotInPresentation", not USER32.IsWindowVisible(control(160))
              and not USER32.IsWindowVisible(control(161)))
        check("displaySettingsShortcutAvailable", region_visible(control(186)))
        command(186)
        check("displayShortcutOpensSettingsAtDisplay", query(21) == 1 and query(19) > 0
              and region_visible(control(160)) and region_visible(control(161)))

        fixture = folder / "settings_sample.py"
        fixture.write_text("# QuietShare synthetic Settings fixture\nprint('sample')\n", encoding="utf-8")
        native.drop_file(window, fixture)
        check("syntheticCodePreviewOpened", wait_for(lambda: query(3) == 2))
        command(123)
        before = native.control_text(control(146))
        command(146)
        check("codeWrapToggleEnabledInSettings", USER32.IsWindowEnabled(control(146))
              and native.control_text(control(146)) != before and query(21) == 1)
        command(146)
        check("codeWrapToggleRestoresInSettings", native.control_text(control(146)) == before)
        for value in (25, 137, 400):
            set_size(str(value))
            check("manualCodeZoom:" + str(value), query(4) == value and native.control_text(control(163)) == str(value) and query(22) == 0)
        set_size("401")
        check("invalidZoomRestoresLastValid", query(4) == 400 and native.control_text(control(163)) == "400" and query(22) == 1)
        command(108)
        check("zoomResetAvailableInSettings", query(4) == 100 and native.control_text(control(163)) == "100")
        scroll(7)
        check("resizeToLargerSettingsWindow", USER32.SetWindowPos(window, None, 0, 0,
              round(840 * scale), round(700 * scale), 0x16))
        check("settingsOffsetClampedAfterResize", 0 <= query(19) <= query(20))
        focus(163)
        check("sizeFocusScrollsIntoViewAfterResize", region_visible(control(163)))
    except Exception as error:
        failure = {"type": type(error).__name__, "message": str(error)}
    finally:
        if process and process.poll() is None:
            if window:
                native.send(window, 0x10, timeout=6000)
            try:
                process.wait(timeout=8)
            except subprocess.TimeoutExpired:
                process.terminate()
                process.wait(timeout=5)
        if process:
            checks["fixtureExitZero"] = process.returncode == 0
        report = {"checks": checks, "passed": sum(checks.values()),
                  "failed": sum(not value for value in checks.values()), "failure": failure,
                  "testedBinarySha256": hashlib.sha256(EXE.read_bytes()).hexdigest(),
                  "method": "Separate synthetic QA EXE; owned HWND commands, native clipping regions and scroll state; no desktop pixels or physical input",
                  "notTested": ["Physical Tab and pointer delivery", "Real cross-monitor DPI changes", "Settings persistence in QA (disabled by design)"]}
        (output / "settings-regression.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({key: report[key] for key in ("passed", "failed", "failure")}), flush=True)
    return 1 if failure or report["failed"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
