"""Tabbed Settings behavior in the separately built synthetic QA app.

Targets only the owned child PID and native controls. It never samples desktop
pixels, changes a personal profile, or sends physical keyboard/mouse input.
"""
from __future__ import annotations

import ctypes
from ctypes import wintypes
import hashlib
import json
import os
import statistics
import subprocess
import sys
import time
import uuid
from types import SimpleNamespace

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
USER32.GetClientRect.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.RECT)]
USER32.GetClientRect.restype = wintypes.BOOL
USER32.SetWindowPos.argtypes = [wintypes.HWND, wintypes.HWND, ctypes.c_int,
                               ctypes.c_int, ctypes.c_int, ctypes.c_int, wintypes.UINT]
USER32.SetWindowPos.restype = wintypes.BOOL
USER32.GetDpiForWindow.argtypes = [wintypes.HWND]
USER32.GetDpiForWindow.restype = wintypes.UINT
USER32.AttachThreadInput.argtypes = [wintypes.DWORD, wintypes.DWORD, wintypes.BOOL]
USER32.AttachThreadInput.restype = wintypes.BOOL
USER32.GetFocus.restype = wintypes.HWND
USER32.PostMessageW.argtypes = [wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM]
USER32.PostMessageW.restype = wintypes.BOOL
USER32.SetFocus.argtypes = [wintypes.HWND]
USER32.SetFocus.restype = wintypes.HWND
USER32.PeekMessageW.argtypes = [ctypes.POINTER(wintypes.MSG), wintypes.HWND,
                              wintypes.UINT, wintypes.UINT, wintypes.UINT]
USER32.PeekMessageW.restype = wintypes.BOOL
native.KERNEL32.GetCurrentThreadId.restype = wintypes.DWORD

class ScrollInfo(ctypes.Structure):
    _fields_ = [("cbSize", wintypes.UINT), ("fMask", wintypes.UINT),
                ("nMin", ctypes.c_int), ("nMax", ctypes.c_int), ("nPage", wintypes.UINT),
                ("nPos", ctypes.c_int), ("nTrackPos", ctypes.c_int)]

class GuiThreadInfo(ctypes.Structure):
    _fields_ = [("cbSize", wintypes.DWORD), ("flags", wintypes.DWORD),
                ("hwndActive", wintypes.HWND), ("hwndFocus", wintypes.HWND),
                ("hwndCapture", wintypes.HWND), ("hwndMenuOwner", wintypes.HWND),
                ("hwndMoveSize", wintypes.HWND), ("hwndCaret", wintypes.HWND), ("rcCaret", wintypes.RECT)]

USER32.GetGUIThreadInfo.argtypes = [wintypes.DWORD, ctypes.POINTER(GuiThreadInfo)]
USER32.GetGUIThreadInfo.restype = wintypes.BOOL
USER32.GetScrollInfo.argtypes = [wintypes.HWND, ctypes.c_int, ctypes.POINTER(ScrollInfo)]
USER32.GetScrollInfo.restype = wintypes.BOOL
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
    checks, metrics, failure = {}, {}, None

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

    def step(identifier):
        # WM_COMMAND alone skips the focus change made by a real button click.
        if USER32.IsWindowEnabled(control(identifier)):
            real_focus(identifier)
        command(identifier)

    def focus(identifier):
        # Native focus triggers EnsureSettingsVisible just as keyboard Tab does.
        # Keep the EDIT's internal focus flag consistent with GetFocus().
        real_focus(identifier)

    def real_focus(identifier, change=True):
        target = control(identifier)
        if native.window_pid(target) != process.pid:
            raise AssertionError("Focus target is outside the synthetic owned app")
        pending = wintypes.MSG()
        USER32.PeekMessageW(ctypes.byref(pending), None, 0, 0, 0)
        current = native.KERNEL32.GetCurrentThreadId()
        owner = USER32.GetWindowThreadProcessId(target, None)
        if not USER32.AttachThreadInput(current, owner, True):
            raise ctypes.WinError(ctypes.get_last_error())
        try:
            if change:
                USER32.SetFocus(target)
            if USER32.GetFocus() != target:
                raise AssertionError("Owned control did not receive native focus")
            return True
        finally:
            USER32.AttachThreadInput(current, owner, False)

    def selection():
        value = native.send(control(163), 0xB0)  # EM_GETSEL; input limit is four digits.
        return value & 0xFFFF, (value >> 16) & 0xFFFF

    def click_size():
        position = native.send(control(163), 0xD6, 0)  # EM_POSFROMCHAR / first glyph.
        if position & 0xFFFFFFFF == 0xFFFFFFFF:
            raise AssertionError("Numeric EDIT could not locate its first character")
        x, y = max(0, (position & 0xFFFF) - 2), ((position >> 16) & 0xFFFF) + 4
        native.send(control(163), 0x201, 1, (y << 16) | x)
        native.send(control(163), 0x202, 0, (y << 16) | x)

    def scroll(code, position=0):
        native.send(window, 0x115, code | (position << 16), control(187))

    def scroll_geometry():
        info, bounds = ScrollInfo(), wintypes.RECT()
        info.cbSize, info.fMask = ctypes.sizeof(info), 0x17
        if not USER32.GetScrollInfo(control(187), 2, ctypes.byref(info)) or not USER32.GetClientRect(control(187), ctypes.byref(bounds)):
            raise ctypes.WinError(ctypes.get_last_error())
        # GetScrollBarInfo on the remote subclassed HWND faults inside User32.
        # Read only actual in-process SBI integers through privacy-gated queries.
        height, top, arrow = query(31), query(32), query(33)
        if height <= 0 or arrow <= 0:
            raise AssertionError("Owned native scrollbar metrics are unavailable")
        bar = SimpleNamespace(rcScrollBar=bounds, dxyLineButton=arrow,
                              xyThumbTop=top, xyThumbBottom=top + height)
        return info, bar

    def owned_capture():
        info = GuiThreadInfo()
        info.cbSize = ctypes.sizeof(info)
        thread = USER32.GetWindowThreadProcessId(window, None)
        if not USER32.GetGUIThreadInfo(thread, ctypes.byref(info)):
            raise ctypes.WinError(ctypes.get_last_error())
        return info.hwndCapture

    def scrollbar_pointer(message, x, y, buttons=0):
        native.send(control(187), message, buttons, ((y & 0xFFFF) << 16) | (x & 0xFFFF))

    def wheel(delta, identifier=None):
        native.send(window if identifier is None else control(identifier), 0x20A, (delta & 0xFFFF) << 16)

    def posted_key(identifier, key):
        target = control(identifier)
        if not USER32.PostMessageW(target, 0x100, key, 1) or not USER32.PostMessageW(target, 0x101, key, 0xC0000001):
            raise RuntimeError("Could not post key messages to owned Settings control")
        time.sleep(.1)

    def set_size(value):
        real_focus(163)
        native.set_control_text(control(163), value)
        # Posted keys pass through the real IsDialogMessage/dispatch loop.
        posted_key(163, 0x0D)

    def category(index):
        command(240 if index == 5 else 200 + index)
        if query(40) != index:
            raise AssertionError("Requested Settings category did not activate")

    def edit_alignment(identifier, label, caret=False):
        target = control(identifier)
        bounds, formatting = wintypes.RECT(), wintypes.RECT()
        if not USER32.GetClientRect(target, ctypes.byref(bounds)):
            raise ctypes.WinError(ctypes.get_last_error())
        native.send(target, 0xB2, 0, ctypes.addressof(formatting))  # EM_GETRECT
        top, bottom = formatting.top, bounds.bottom - formatting.bottom
        check(label + "FormattingVerticallyCentered", top > 0 and abs(top - bottom) <= 2
              and formatting.bottom > formatting.top)
        if caret:
            real_focus(identifier)
            info = GuiThreadInfo()
            info.cbSize = ctypes.sizeof(info)
            thread = USER32.GetWindowThreadProcessId(target, None)
            if not USER32.GetGUIThreadInfo(thread, ctypes.byref(info)):
                raise ctypes.WinError(ctypes.get_last_error())
            check(label + "NativeCaretVerticallyCentered", info.hwndCaret == target
                  and abs(info.rcCaret.top + info.rcCaret.bottom - bounds.bottom) <= 3
                  and info.rcCaret.top > 0 and info.rcCaret.bottom <= bounds.bottom)
        metrics[label + "EditGeometry"] = {
            "clientHeight": bounds.bottom, "formatTop": formatting.top,
            "formatBottom": formatting.bottom,
        }

    def set_opacity(value):
        real_focus(205)
        native.set_control_text(control(205), value)
        posted_key(205, 0x0D)

    def record_shortcut(index, key, modifiers=0):
        # A registered chord reaches WM_HOTKEY while recording. Send that same
        # bounded message to our owned QA HWND, without changing keyboard state.
        command(220 + index)
        if query(43) != index + 1:
            raise AssertionError("Shortcut button did not enter capture mode")
        native.send(window, 0x312, 0, (key << 16) | modifiers)

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
        check("settingsStartsWithAppearance", query(40) == 0)
        check("themeSelectorAvailableInAppearance", region_visible(control(255)))
        def theme_popup():
            found=[]
            @native.ENUM_PROC
            def visit(hwnd,_):
                if native.window_pid(hwnd)==process.pid:
                    name=ctypes.create_unicode_buffer(80);USER32.GetClassNameW(hwnd,name,len(name))
                    if name.value=='QuietShareToolMenu' and USER32.IsWindowVisible(hwnd):
                        info=GuiThreadInfo();info.cbSize=ctypes.sizeof(info)
                        thread=USER32.GetWindowThreadProcessId(hwnd,None)
                        if USER32.GetGUIThreadInfo(thread,ctypes.byref(info)) and info.hwndFocus==hwnd:
                            found.append(hwnd)
                return True
            USER32.EnumWindows(visit,0)
            return next(iter(found),None)
        USER32.PostMessageW(window,0x111,255,0)
        popup=wait_for(theme_popup)
        check("themeMenuOpensAsOwnedProtectedPopup",bool(popup))
        # Establish the first-row hover in this owned popup, then queue Down
        # and Enter. Native popup hover can otherwise select the row under the
        # user's stationary pointer, making a fixed expected theme unreliable.
        check("themeMenuKeysQueued", USER32.PostMessageW(popup,0x200,0,(16 << 16) | 16)
              and USER32.PostMessageW(popup,0x100,0x28,0)
              and USER32.PostMessageW(popup,0x100,0x0D,0))
        selected_light=wait_for(lambda:native.control_text(control(255))=='Sáng')
        metrics['themeMenuResult']={'label':native.control_text(control(255)),
                                    'popupStillExists':bool(USER32.IsWindow(popup))}
        check("themeMenuKeyboardSelectsLight",selected_light)
        command(1000)
        check("themeReturnsToClassicWithoutRestart",native.control_text(control(255))=='Tối cổ điển')
        check("fontSliderRemoved", not control(130))
        identifiers = (106, 107, 163, 108, 104, 146, 181, 151, 182, 183, 184,
                       185, 152, 155, 160, 161, 143, 187, 189, 200, 201, 202,
                       203, 204, 205, 206, 207, *range(220, 230), *range(240, 249),
                       *range(249, 269))
        unstable = [identifier for identifier in identifiers if not control(identifier) or
                    USER32.GetParent(control(identifier)) != window]
        check("consolidatedControlsKeepStableMainParent", not unstable)
        scrollbar_class = ctypes.create_unicode_buffer(64)
        USER32.GetClassNameW(control(187), scrollbar_class, len(scrollbar_class))
        check("settingsUsesNativeScrollbar", scrollbar_class.value.casefold() == "scrollbar")
        check("appearanceHidesOtherCategoryControls", not region_visible(control(163)) and not region_visible(control(160))
              and not region_visible(control(220)))
        check("customThemeControlsAreAvailable", all(control(identifier) for identifier in (264, 265, 266, 267, 268)))
        check("opacityStartsOpaque", query(41) == 100 and native.control_text(control(205)) == "100")
        edit_alignment(205, "opacity", caret=True)
        check("ownerDrawButtonSuppressesSystemErase", native.send(control(200), 0x14) == 1)
        step(242)
        check("opacityMinusStepsOnePercent", query(41) == 99 and native.control_text(control(205)) == "99")
        step(243)
        check("opacityPlusStepsOnePercent", query(41) == 100 and native.control_text(control(205)) == "100")
        step(243)
        check("opacityPlusClampsAtMaximum", query(41) == 100)
        for value in (40, 67, 100):
            set_opacity(str(value))
            check("windowOpacityManualValue:" + str(value), query(41) == value
                  and native.control_text(control(205)) == str(value) and query(44) == 0)
        for label, value in (("below", "39"), ("above", "101"), ("invalid", "abc")):
            set_opacity(value)
            check("invalidOpacityRestoresLastValid:" + label, query(41) == 100 and query(44) == 1
                  and native.control_text(control(205)) == "100")
        set_opacity("100")
        check("validOpacityClearsError", query(44) == 0)
        set_opacity("40")
        step(242)
        check("opacityMinusClampsAtMinimum", query(41) == 40)
        set_opacity("100")
        check("iconsStartAtDefaultSize", query(60) == 16 and native.control_text(control(241)) == "16")
        edit_alignment(241, "iconSize", caret=True)
        step(244)
        check("iconMinusStepsOnePixel", query(60) == 15 and native.control_text(control(241)) == "15")
        step(245)
        check("iconPlusStepsOnePixel", query(60) == 16 and native.control_text(control(241)) == "16")
        for value in (12, 19, 24):
            real_focus(241)
            native.set_control_text(control(241), str(value))
            posted_key(241, 0x0D)
            check("manualIconSize:" + str(value), query(60) == value and native.control_text(control(241)) == str(value))
        step(245)
        check("iconPlusClampsAtMaximum", query(60) == 24)
        for value in (11, 25):
            real_focus(241)
            native.set_control_text(control(241), str(value))
            posted_key(241, 0x0D)
            check("invalidIconSizeRetainsPrevious:" + str(value), query(60) == 24 and native.control_text(control(241)) == "24")
        real_focus(241)
        native.set_control_text(control(241), "12")
        posted_key(241, 0x0D)
        step(244)
        check("iconMinusClampsAtMinimum", query(60) == 12)
        real_focus(241)
        native.set_control_text(control(241), "16")
        posted_key(241, 0x0D)
        check("scriptWeightStartsRegular", query(42) == 400)
        for weight in (600, 700, 400):
            command(206)
            check("scriptWeightCycles:" + str(weight), query(42) == weight and bool(native.control_text(control(206))))
        category(1)
        check("readingHidesAppearanceControls", not region_visible(control(205)) and not region_visible(control(206)))
        check("transcriptionLanguageControlAvailable", bool(control(256)) and native.control_text(control(256)) in ("Tiếng Việt", "Tiếng Anh", "Vietnamese", "English"))
        check("transcriptionToggleAvailable", bool(control(259)) and any(label in native.control_text(control(259)) for label in ("Bật", "Tắt", "On", "Off")))
        check("meetingProviderControlAvailable", bool(control(260)) and bool(native.control_text(control(260))))
        check("meetingOutputControlAvailable", bool(control(261)) and bool(native.control_text(control(261))))
        check("meetingFolderControlAvailable", bool(control(262)) and bool(native.control_text(control(262))))
        check("meetingImportControlAvailable", bool(control(263)))
        edit_alignment(163, "contentSize", caret=True)
        real_focus(108)
        reading_window, reading_client = wintypes.RECT(), wintypes.RECT()
        USER32.GetWindowRect(window, ctypes.byref(reading_window))
        USER32.GetClientRect(window, ctypes.byref(reading_client))
        metrics["readingMinimumGeometry"] = {"window": [reading_window.left,reading_window.top,reading_window.right,reading_window.bottom],
              "client": [reading_client.right,reading_client.bottom],"dpi":USER32.GetDpiForWindow(window),
              "offset":query(19),"maximum":query(20),"scrollbarVisible":region_visible(control(187))}
        # Opacity's focus commit deliberately posts its final layout. Wait for
        # the new category's native viewport instead of racing that message.
        check("minimumSettingsCanScroll", bool(wait_for(lambda: query(20) > 0 and region_visible(control(187)), 2)))
        scroll(6)
        check("settingsScrollTop", query(19) == 0)
        numeric, viewport = wintypes.RECT(), wintypes.RECT()
        USER32.GetWindowRect(control(163), ctypes.byref(numeric))
        USER32.GetWindowRect(control(187), ctypes.byref(viewport))
        partial_offset = round((numeric.top - viewport.top + (numeric.bottom - numeric.top) // 2) / scale)
        scroll(4, min(query(20), partial_offset))
        actual, expected, numeric_height = numeric_clip_bounds(control(163), control(187))
        check("minimumNumericEditPartiallyClipped", 0 < actual[3] - actual[1] < numeric_height)
        check("minimumNumericClipMatchesFullViewportIntersection", actual == expected)
        focus(163)
        actual, expected, numeric_height = numeric_clip_bounds(control(163), control(187))
        check("focusedNumericClipMatchesFullViewportIntersection", actual == expected)
        check("focusedNumericEditFullyVisible", actual[1] == 0 and actual[3] == numeric_height)
        scroll(4, min(query(20), partial_offset))
        command(107)
        command(107)
        actual, expected, numeric_height = numeric_clip_bounds(control(163), control(187))
        check("numericClipPreservedAfterSettingsRelayout", actual == expected and 0 < actual[3] - actual[1] < numeric_height)
        category(4)
        check("categorySwitchResetsScrollOffset", query(19) == 0)
        check("shortcutCategoryShowsOnlyBindings", region_visible(control(220)) and not region_visible(control(163))
              and not region_visible(control(160)))
        maximum = query(20)
        thumb_heights, native_pages = [], []
        for index, position in enumerate((maximum // 4, maximum // 2, maximum)):
            scroll(4, position)
            check("thumbPositionUpdatesImmediately:" + str(index), query(19) == position)
            info, bar = scroll_geometry()
            thumb_heights.append(bar.xyThumbBottom - bar.xyThumbTop)
            native_pages.append(info.nPage)
            check("nativeScrollbarPositionMatchesSettings:" + str(index), info.nPos == position)
        check("nativeThumbHeightStaysStableAcrossPositions", min(thumb_heights) > 0 and max(thumb_heights) - min(thumb_heights) <= 1)
        check("nativeScrollbarPageSizeStaysStable", min(native_pages) > 0 and len(set(native_pages)) == 1)
        before_thumb = thumb_heights[-1]
        native.send(control(187), 0x200, 0, (((bar.xyThumbTop + bar.xyThumbBottom) // 2) << 16) | 7)
        _, hovered_bar = scroll_geometry()
        check("nativeThumbHoverPreservesGeometry", hovered_bar.xyThumbBottom - hovered_bar.xyThumbTop == before_thumb)
        native.send(control(187), 0x2A3)
        metrics["nativeScrollbar"] = {"thumbHeights": thumb_heights, "pageSizes": native_pages,
              "method": "Owned native scrollbar SCROLLINFO, client geometry and privacy-gated in-process SCROLLBARINFO integers; no desktop pixels"}
        scroll(4, maximum // 3)
        _, drag_bar = scroll_geometry()
        before_position = query(19)
        thumb_height = drag_bar.xyThumbBottom - drag_bar.xyThumbTop
        bar_width = drag_bar.rcScrollBar.right - drag_bar.rcScrollBar.left
        bar_height = drag_bar.rcScrollBar.bottom - drag_bar.rcScrollBar.top
        pointer_x, pointer_y = bar_width // 2, (drag_bar.xyThumbTop + drag_bar.xyThumbBottom) // 2
        scrollbar_pointer(0x201, pointer_x, pointer_y, 1)
        try:
            check("settingsThumbDragCapturesOwnedScrollbar", owned_capture() == control(187))
            check("settingsThumbPressPreservesOffset", query(19) == before_position)
            scrollbar_pointer(0x200, pointer_x, pointer_y + 20, 1)
            check("settingsThumbDragMovesSettings", before_position < query(19) < maximum)
            _, moved_bar = scroll_geometry()
            check("settingsThumbDragPreservesGeometry", abs(moved_bar.xyThumbBottom - moved_bar.xyThumbTop - thumb_height) <= 1
                  and moved_bar.rcScrollBar.right - moved_bar.rcScrollBar.left == bar_width)
            scrollbar_pointer(0x200, pointer_x, bar_height + 40, 1)
            check("settingsThumbDragClampsBeyondBottom", query(19) == maximum)
            scrollbar_pointer(0x200, pointer_x, -40, 1)
            check("settingsThumbDragClampsBeyondTop", query(19) == 0)
        finally:
            scrollbar_pointer(0x202, pointer_x, -40)
        check("settingsThumbMouseUpReleasesCapture", owned_capture() != control(187))
        scrollbar_pointer(0x200, pointer_x, bar_height + 40, 1)
        check("releasedThumbStopsDragging", query(19) == 0)
        scroll(4, maximum // 2)
        _, drag_bar = scroll_geometry()
        pointer_y = (drag_bar.xyThumbTop + drag_bar.xyThumbBottom) // 2
        scrollbar_pointer(0x201, pointer_x, pointer_y, 1)
        try:
            check("settingsThumbCanCaptureAgain", owned_capture() == control(187))
            native.send(control(187), 0x1F)  # WM_CANCELMODE
            check("settingsThumbCancelReleasesCapture", owned_capture() != control(187))
            before_position = query(19)
            scrollbar_pointer(0x200, pointer_x, bar_height + 40, 1)
            check("cancelledThumbStopsDragging", query(19) == before_position)
        finally:
            scrollbar_pointer(0x202, pointer_x, pointer_y)
        scroll(7)
        check("settingsScrollBottom", query(19) == query(20))
        check("lastShortcutControlVisibleAtBottom", region_visible(control(207)))
        scroll(6)
        check("offscreenShortcutControlIsClipped", not region_visible(control(229)))
        focus(229)
        check("buttonFocusScrollsIntoView", query(19) > 0 and region_visible(control(229)))
        category(3)
        scroll(7)
        projection_bottom = query(19)
        focus(160)
        check("comboFocusScrollsIntoView", query(19) < projection_bottom and region_visible(control(160)))
        check("sourceMonitorListAvailableInSettings", native.send(control(160), 0x146) >= 1)
        check("destinationMonitorListAvailableInSettings", native.send(control(161), 0x146) >= 1)
        primary_number, monitor_count = query(29), query(30)
        source_text = native.control_text(control(160))
        check("windowsDisplayNumbersAvailable", primary_number >= 1 and monitor_count >= 1)
        check("defaultSourceUsesPrimaryWindowsDisplay", source_text.startswith("Màn " + str(primary_number) + " ·") and "Chính" in source_text)
        check("windowDisplayMarkerPresentOnSingleDisplay", monitor_count != 1 or "Cửa sổ này" in source_text)
        check("destinationDefaultsOtherDisplayWhenAvailable", monitor_count == 1 or
              native.send(control(160), 0x147) != native.send(control(161), 0x147))
        category(4)
        scroll(6)
        native.send(control(220), 0x20A, 0xFF880000)
        check("wheelOverSettingScrollsPage", query(19) > 0)
        scroll(6)
        native.send(window, 0x20A, 0x00780000)
        check("wheelClampsAtTop", query(19) == 0)
        wheel(-15)
        check("smallWheelDeltaMovesImmediately", query(19) == 6)
        for _ in range(7):
            wheel(-15)
        check("fractionalWheelEventsComposeToOneNotch", query(19) == 48)
        wheel(15)
        check("smallWheelDirectionReversesImmediately", query(19) == 42)
        wheel(120)
        check("wheelOvershootClampsAtTop", query(19) == 0)
        for _ in range(5):
            wheel(-1)
        check("subpixelWheelRemaindersAccumulate", query(19) == 2)
        for _ in range(5):
            wheel(1)
        check("signedWheelRemaindersReverseToOrigin", query(19) == 0)
        wheel(15)
        for _ in range(5):
            wheel(-1)
        check("topEdgeDiscardsOutwardWheelCarry", query(19) == 2)
        before = query(19)
        wheel(-15, 220)
        check("smallWheelOverShortcutButtonScrollsPage", query(19) == before + 6)
        before = query(19)
        wheel(-15, 221)
        check("smallWheelOverButtonScrollsPage", query(19) == before + 6)
        scroll(7)
        wheel(-15)
        check("wheelClampsAtBottom", query(19) == query(20))
        wheel(15)
        check("bottomEdgeAllowsImmediateReverse", query(19) == query(20) - 6)
        scroll(6)
        durations, movements = [], []
        previous = query(19)
        stream_events = min(120, query(20) // 6)
        for _ in range(stream_events):
            begin = time.perf_counter_ns()
            wheel(-15)
            durations.append((time.perf_counter_ns() - begin) / 1_000_000)
            current = query(19)
            movements.append(current - previous)
            previous = current
        metrics["wheelStreamDelta15"] = {"events": stream_events, "deltaPerEvent": -15,
              "positionsDistribution": {str(value): movements.count(value) for value in sorted(set(movements))},
              "totalMovement": sum(movements), "medianMs": statistics.median(durations),
              "p95Ms": sorted(durations)[int(.95 * (len(durations) - 1))], "maxMs": max(durations),
              "method": "Distribution of offset changes per owned WM_MOUSEWHEEL event at 430x320; no physical touchpad; timings are observations, not pass thresholds"}
        check("smallWheelStreamMovesOnEveryEvent", stream_events >= 20 and movements == [6] * stream_events)

        category(0)
        check("appearanceHasIndependentScrollOrigin", query(19) == 0)
        category(4)
        check("returningCategoryResetsScrollOrigin", query(19) == 0)
        check("allShortcutButtonsHaveNativeLabels", all(native.control_text(control(identifier)) for identifier in range(220, 230)))
        check("allExistingShortcutQueriesRemainAvailable", all(query(key) for key in range(45, 55)))
        check("verifyShortcutStartsF9", query(47) == 0x78)
        custom_verify_binding = ord("K") | ((2 | 4) << 8)
        record_shortcut(2, ord("K"), 2 | 4)
        check("customShortcutRecorded", query(47) == custom_verify_binding and query(43) == 0 and query(44) == 0)
        check("customShortcutLabelUpdated", "Ctrl" in native.control_text(control(222))
              and "Shift" in native.control_text(control(222)) and "K" in native.control_text(control(222)))
        record_shortcut(2, 0x7A)  # Existing F11 focus binding.
        check("duplicateShortcutRejectedWithoutChangingBinding", query(47) == custom_verify_binding
              and query(43) == 0 and query(44) == 1)
        command(222)
        posted_key(222, 0x1B)
        check("escapeCancelsShortcutCapture", query(43) == 0 and query(47) == custom_verify_binding)
        record_shortcut(2, ord("T"), 2)
        check("browserReservedShortcutRejected", query(47) == custom_verify_binding and query(44) == 1)
        command(222)
        category(0)
        check("changingCategoryCancelsShortcutCapture", query(43) == 0 and query(44) == 0 and query(19) == 0)
        set_opacity("40")
        command(206)
        check("customAppearanceReadyBeforeReset", query(41) == 40 and query(42) == 600)
        category(4)
        focus(207)
        command(207)
        check("restoreShortcutDefaultsPreservesAppearance", query(47) == 0x78 and query(45) == 0x77 and query(46) == (ord("Q") | ((2 | 1) << 8))
              and query(41) == 40 and query(42) == 600 and query(44) == 0)
        category(0)
        command(207)
        check("restoreAppearanceDefaultsPreservesShortcutDefaults",query(41) == 100 and query(42) == 400 and query(47) == 0x78 and query(44) == 0)

        category(5)
        check("browserSettingsCategoryAvailable", query(40) == 5 and region_visible(control(246))
              and not region_visible(control(205)) and not region_visible(control(163)))
        check("browserPolicyDefaultsPrivate", query(61) == 0 and query(62) == 0 and query(63) == 0)
        for identifier, key, label in ((246, 61, "mute"), (247, 62, "camera"), (248, 63, "microphone")):
            focus(identifier)
            before = native.control_text(control(identifier))
            command(identifier)
            check("browserPolicyCanBeSetBeforeStarting:" + label, query(key) == 1
                  and native.control_text(control(identifier)) != before and query(10) == 0 and query(11) == 0)
            command(identifier)
            check("browserPolicyRestoresPrivateDefault:" + label, query(key) == 0
                  and native.control_text(control(identifier)) == before)
        # The synthetic QA app keeps the browser engine disabled. Exercise the
        # real hidden address EDIT without revealing content or navigating.
        address = control(164)
        address_bounds = wintypes.RECT()
        USER32.GetWindowRect(address, ctypes.byref(address_bounds))
        address_text = native.control_text(address)
        check("hiddenAddressSizingForNativeFormatting", USER32.SetWindowPos(address, None, 0, 0,
              round(240 * scale), round(30 * scale), 0x16))
        native.set_control_text(address, "")
        edit_alignment(164, "emptyAddress")
        native.set_control_text(address, "https://example.test/")
        edit_alignment(164, "populatedAddress")
        first = native.send(address, 0xD6, 0)  # EM_POSFROMCHAR
        check("addressFirstCharacterUsesLeftInset", first & 0xFFFFFFFF != 0xFFFFFFFF
              and 0 < (first & 0xFFFF) <= round(16 * scale))
        native.send(address, 0x102, 13, 1)  # WM_CHAR / carriage return
        check("addressRejectsNewline", native.send(address, 0xBA) == 1
              and "\r" not in native.control_text(address) and "\n" not in native.control_text(address))
        check("addressRemainsHiddenWithoutEngine", not USER32.IsWindowVisible(address)
              and query(10) == 0 and query(11) == 0)
        native.set_control_text(address, address_text)
        USER32.SetWindowPos(address, None, 0, 0, address_bounds.right - address_bounds.left,
                            address_bounds.bottom - address_bounds.top, 0x16)

        command(120)
        command(123)
        category(1)
        real_focus(108)
        native.send(control(163), 0xB1, 0, 0)
        focus(163)
        length = len(native.control_text(control(163)))
        check("numericFocusCollapsesCaretAtEnd", selection() == (length, length))
        real_focus(108)
        native.send(control(163), 0xB1, 0, 0)
        click_size()
        check("firstNumericClickCollapsesCaretAtEnd", selection() == (length, length))
        check("firstNumericClickRetainsActualNativeFocus", real_focus(163, change=False))
        click_size()
        check("secondNumericClickKeepsNativeHitTesting", selection() == (0, 0))
        native.send(control(163), 0xB1, 0, 1)
        native.send(control(163), 0x102, ord("3"), 1)
        check("numericSelectionStillAllowsEditing", native.control_text(control(163)) == "32")
        posted_key(163, 0x1B)
        check("escapeRestoresValueWithCaretAtEnd", native.control_text(control(163)) == "12" and selection() == (2, 2))
        for value in (8, 37, 91, 200):
            set_size(str(value))
            check("manualFontSize:" + str(value), native.control_text(control(163)) == str(value)
                  and query(4) == value * 100 // 24 and query(22) == 0)
            check("manualSizeCommitKeepsCaretAtEnd:" + str(value), selection() == (len(str(value)), len(str(value))))
        for label, value in (("below", "7"), ("above", "201"), ("invalid", "abc")):
            set_size(value)
            check("invalidFontRestoresLastValid:" + label, native.control_text(control(163)) == "200"
                  and query(4) == 200 * 100 // 24 and query(22) == 1)
        category(2)
        for identifier, label in ((257, "numberedScriptTabs"), (258, "numberedBrowserTabs")):
            before = native.control_text(control(identifier))
            command(identifier)
            after = native.control_text(control(identifier))
            check(label + "ToggleUpdatesState", before != after and ("Bật" in after or "On" in after))
            command(identifier)
            check(label + "ToggleRestoresState", native.control_text(control(identifier)) == before)
        command(184)
        check("statusBarHiddenIndependently", not query(16) & 8)
        category(1)
        set_size("9999")
        check("sizeErrorSurvivesHiddenStatusBar", query(22) == 1 and query(4) == 200 * 100 // 24)
        set_size("24")
        check("validSizeClearsInlineError", query(22) == 0 and native.control_text(control(163)) == "24")
        step(102)
        check("contentMinusChangesSizeFromVisibleStepper", native.control_text(control(163)) == "22")
        step(103)
        check("contentPlusChangesSizeFromVisibleStepper", native.control_text(control(163)) == "24")
        category(2)
        command(184)
        category(1)

        before = native.control_text(control(146))
        command(146)
        check("scriptWrapCanBeChangedInSettings", USER32.IsWindowEnabled(control(146))
              and native.control_text(control(146)) != before)
        command(146)
        check("scriptWrapRestoresInSettings", native.control_text(control(146)) == before)
        for identifier, label, tab in ((106, "topmost", 0), (152, "ignoredFolders", 2), (143, "pointer", 3)):
            category(tab)
            before = native.control_text(control(identifier))
            command(identifier)
            after = native.control_text(control(identifier))
            check("settingToggleUpdatesState:" + label, bool(before) and bool(after) and before != after and query(21) == 1)
            command(identifier)
            check("settingToggleRestoresState:" + label, native.control_text(control(identifier)) == before and query(21) == 1)
        category(1)
        before = native.control_text(control(104))
        speed_labels = []
        for _ in range(3):
            command(104)
            speed_labels.append(native.control_text(control(104)))
        check("readingSpeedCyclesInSettings", len(set(speed_labels)) == 3 and speed_labels[-1] == before and query(21) == 1)
        category(0)
        before = native.control_text(control(107))
        command(107)
        check("languageChangesInSettings", bool(native.control_text(control(107))) and native.control_text(control(107)) != before and query(21) == 1)
        category(3)
        source_text = native.control_text(control(160))
        check("englishMonitorIdentityUsesWindowsDisplayNumber", source_text.startswith("Display " + str(primary_number) + " ·") and "Main" in source_text)
        check("englishWindowDisplayMarkerOnSingleDisplay", monitor_count != 1 or "This window" in source_text)
        category(0)
        command(107)
        check("languageRestoresInSettings", native.control_text(control(107)) == before)
        category(3)
        focus(189)
        check("resourceSettingFocusScrollsIntoView", region_visible(control(189)) and query(19) > 0)
        check("resourceDefaultIsPerformance60", query(26) == 2 and query(27) == 60)
        for mode, fps in ((0, 15), (1, 30), (2, 60)):
            command(189)
            check("resourceModeCycles:" + str(mode), query(26) == mode and query(27) == fps and query(21) == 1)
            check("resourceModeLabelMatchesTarget:" + str(mode), str(fps) + " FPS" in native.control_text(control(189)))
            check("resourceModeDoesNotStartProjectionOrBrowser:" + str(mode), query(25) == 0 and query(10) == 0 and query(11) == 0)
        category(2)
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
        category(0)
        focus(106)
        before = native.control_text(control(106))
        check("readerPausedBeforeSettingsKey", query(23) == 0)
        posted_key(106, 0x20)
        check("settingsSpaceOperatesFocusedButton", native.control_text(control(106)) != before)
        check("settingsSpaceDoesNotStartReader", query(23) == 0 and query(21) == 1)
        posted_key(106, 0x20)
        check("settingsSpaceRestoresFocusedButton", native.control_text(control(106)) == before and query(23) == 0)

        command(139)
        check("projectionEntryOpensDedicatedSettingsCategory", query(21) == 1 and query(40) == 3)
        check("projectionSelectorsVisibleInDedicatedSettings", region_visible(control(160))
              and region_visible(control(161)))
        command(186)
        check("displayShortcutOpensSettingsAtDisplay", query(21) == 1 and query(40) == 3 and query(19) == 0
              and region_visible(control(160)) and region_visible(control(161)))

        fixture = folder / "settings_sample.py"
        fixture.write_text("# QuietShare synthetic Settings fixture\nprint('sample')\n", encoding="utf-8")
        native.drop_file(window, fixture)
        check("syntheticCodePreviewOpened", wait_for(lambda: query(3) == 2))
        command(123)
        category(1)
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
        check("zoomResetAvailableInSettings", query(4) == 50 and native.control_text(control(163)) == "50")
        scroll(7)
        check("resizeToLargerSettingsWindow", USER32.SetWindowPos(window, None, 0, 0,
              round(840 * scale), round(700 * scale), 0x16))
        check("settingsOffsetClampedAfterResize", 0 <= query(19) <= query(20))
        real_focus(108)
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
                  "metrics": metrics,
                  "testedBinarySha256": hashlib.sha256(EXE.read_bytes()).hexdigest(),
                  "method": "Separate synthetic QA EXE; owned HWND commands, native clipping/scroll/selection state and temporary owned-thread focus attachment; no desktop pixels, foreground activation or physical input",
                  "notTested": ["Physical Tab and pointer delivery", "Real cross-monitor DPI changes", "Settings persistence in QA (disabled by design)"]}
        (output / "settings-regression.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({key: report[key] for key in ("passed", "failed", "failure")}), flush=True)
    return 1 if failure or report["failed"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
