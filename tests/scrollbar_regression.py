"""Check the production EXE's native scrollbar across idle and interaction states.

Only the test child PID is targeted. The app's --qa-scrollbar fixture uses its
own Data directory and renders sample text without capture exclusion. Pixels
are read from that editor's nonclient scrollbar, never the full desktop.
"""

from __future__ import annotations

import ctypes
from ctypes import wintypes
from datetime import datetime, timezone
import hashlib
import json
import subprocess
import sys
import time
import uuid

sys.dont_write_bytecode = True
from lifecycle_regression import EXE, ROOT, USER32, app_windows, send


GDI32 = ctypes.WinDLL("gdi32", use_last_error=True)


class ScrollBarInfo(ctypes.Structure):
    _fields_ = [("cbSize", wintypes.DWORD), ("rcScrollBar", wintypes.RECT),
                ("dxyLineButton", ctypes.c_int), ("xyThumbTop", ctypes.c_int),
                ("xyThumbBottom", ctypes.c_int), ("reserved", ctypes.c_int),
                ("rgstate", wintypes.DWORD * 6)]


class ScrollInfo(ctypes.Structure):
    _fields_ = [("cbSize", wintypes.UINT), ("fMask", wintypes.UINT),
                ("nMin", ctypes.c_int), ("nMax", ctypes.c_int),
                ("nPage", wintypes.UINT), ("nPos", ctypes.c_int),
                ("nTrackPos", ctypes.c_int)]


class MouseInput(ctypes.Structure):
    _fields_ = [("dx", wintypes.LONG), ("dy", wintypes.LONG),
                ("mouseData", wintypes.DWORD), ("dwFlags", wintypes.DWORD),
                ("time", wintypes.DWORD), ("dwExtraInfo", ctypes.c_size_t)]


class InputUnion(ctypes.Union):
    _fields_ = [("mi", MouseInput)]


class Input(ctypes.Structure):
    _fields_ = [("type", wintypes.DWORD), ("data", InputUnion)]


USER32.GetScrollBarInfo.argtypes = [wintypes.HWND, wintypes.LONG, ctypes.POINTER(ScrollBarInfo)]
USER32.GetScrollBarInfo.restype = wintypes.BOOL
USER32.GetScrollInfo.argtypes = [wintypes.HWND, ctypes.c_int, ctypes.POINTER(ScrollInfo)]
USER32.GetScrollInfo.restype = wintypes.BOOL
USER32.GetWindowRect.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.RECT)]
USER32.GetWindowRect.restype = wintypes.BOOL
USER32.GetWindowDC.argtypes = [wintypes.HWND]
USER32.GetWindowDC.restype = wintypes.HDC
USER32.ReleaseDC.argtypes = [wintypes.HWND, wintypes.HDC]
USER32.ReleaseDC.restype = ctypes.c_int
USER32.GetCursorPos.argtypes = [ctypes.POINTER(wintypes.POINT)]
USER32.GetCursorPos.restype = wintypes.BOOL
USER32.SetCursorPos.argtypes = [ctypes.c_int, ctypes.c_int]
USER32.SetCursorPos.restype = wintypes.BOOL
USER32.SetWindowPos.argtypes = [wintypes.HWND, wintypes.HWND, ctypes.c_int,
                               ctypes.c_int, ctypes.c_int, ctypes.c_int, wintypes.UINT]
USER32.SetWindowPos.restype = wintypes.BOOL
USER32.GetDpiForWindow.argtypes = [wintypes.HWND]
USER32.GetDpiForWindow.restype = wintypes.UINT
USER32.WindowFromPoint.argtypes = [wintypes.POINT]
USER32.WindowFromPoint.restype = wintypes.HWND
USER32.SendInput.argtypes = [wintypes.UINT, ctypes.POINTER(Input), ctypes.c_int]
USER32.SendInput.restype = wintypes.UINT
GDI32.GetPixel.argtypes = [wintypes.HDC, ctypes.c_int, ctypes.c_int]
GDI32.GetPixel.restype = wintypes.DWORD


def bar_info(editor: int) -> ScrollBarInfo:
    info = ScrollBarInfo()
    info.cbSize = ctypes.sizeof(info)
    if not USER32.GetScrollBarInfo(editor, -5, ctypes.byref(info)):
        raise ctypes.WinError(ctypes.get_last_error())
    if info.rgstate[0] & (0x8000 | 0x10000):
        raise AssertionError("Fixture scrollbar is invisible or offscreen")
    return info


def scroll_position(editor: int) -> int:
    info = ScrollInfo()
    info.cbSize, info.fMask = ctypes.sizeof(info), 0x17
    if not USER32.GetScrollInfo(editor, 1, ctypes.byref(info)):
        raise ctypes.WinError(ctypes.get_last_error())
    return info.nPos


def dark_track_sample(editor: int) -> dict:
    info = bar_info(editor)
    bounds = wintypes.RECT()
    if not USER32.GetWindowRect(editor, ctypes.byref(bounds)):
        raise ctypes.WinError(ctypes.get_last_error())
    bar = info.rcScrollBar
    dc = USER32.GetWindowDC(editor)
    if not dc:
        raise ctypes.WinError(ctypes.get_last_error())
    colors = []
    try:
        # The shaft is sampled outside the thumb and arrow buttons. A native
        # light scrollbar yields #f0f0f0; dark mode yields a dark neutral.
        first = bar.top + info.dxyLineButton + 2
        last = bar.bottom - info.dxyLineButton - 2
        for screen_y in range(first, last, 3):
            if bar.top + info.xyThumbTop - 2 <= screen_y <= bar.top + info.xyThumbBottom + 2:
                continue
            pixel = GDI32.GetPixel(dc, bar.left + 2 - bounds.left, screen_y - bounds.top)
            if pixel == 0xFFFFFFFF:
                raise AssertionError("Scrollbar GetPixel failed")
            colors.append((pixel & 255, (pixel >> 8) & 255, (pixel >> 16) & 255))
    finally:
        USER32.ReleaseDC(editor, dc)
    if not colors:
        raise AssertionError("Fixture has no exposed scrollbar shaft pixels")
    maximum = max(max(color) for color in colors)
    return {"sampleCount": len(colors), "maximumChannel": maximum,
            "colors": ["#%02x%02x%02x" % color for color in sorted(set(colors))],
            "dark": maximum < 180}


def mouse_button(flags: int) -> None:
    event = Input()
    event.type, event.data.mi.dwFlags = 0, flags
    if USER32.SendInput(1, ctypes.byref(event), ctypes.sizeof(event)) != 1:
        raise ctypes.WinError(ctypes.get_last_error())


def main() -> int:
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ") + "-" + uuid.uuid4().hex[:8]
    folder = ROOT / "artifacts" / "qa-scrollbar" / stamp
    folder.mkdir(parents=True)
    startup = subprocess.STARTUPINFO()
    startup.dwFlags, startup.wShowWindow = subprocess.STARTF_USESHOWWINDOW, 0
    process = subprocess.Popen([str(EXE), "--qa-scrollbar", str(folder)], cwd=ROOT,
                               startupinfo=startup)
    saved_cursor = wintypes.POINT()
    cursor_saved = bool(USER32.GetCursorPos(ctypes.byref(saved_cursor)))
    checks, samples = {}, {}
    window = None

    def check(name: str, passed: bool) -> None:
        checks[name] = bool(passed)

    def sample(name: str, editor: int) -> None:
        samples[name] = dark_track_sample(editor)
        check(name + ".darkTrack", samples[name]["dark"])

    try:
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            windows = app_windows(process.pid)
            if windows:
                window = windows[0]
                break
            if process.poll() is not None:
                raise AssertionError(f"QA child exited before its window appeared: {process.returncode}")
            time.sleep(0.05)
        if not window:
            raise TimeoutError("QuietShare fixture window did not appear")
        editor = USER32.GetDlgItem(window, 110)
        if not editor:
            raise AssertionError("QuietShare fixture editor is missing")
        # Park outside our fixture so the initial state is truly idle. Restore
        # the pointer in finally, including after a failed check.
        USER32.SetCursorPos(0, 0)
        time.sleep(0.35)
        sample("idleInitial", editor)
        time.sleep(2)
        sample("idleTwoSeconds", editor)
        time.sleep(4.2)
        sample("idleOverSixSeconds", editor)

        info = bar_info(editor)
        bar = info.rcScrollBar
        hover = wintypes.POINT((bar.left + bar.right) // 2,
                              bar.top + (info.xyThumbTop + info.xyThumbBottom) // 2)
        check("nativeScrollbarPointerTarget", USER32.WindowFromPoint(hover) == editor)
        USER32.SetCursorPos(hover.x, hover.y)
        time.sleep(0.25)
        sample("hover", editor)
        USER32.SetCursorPos(0, 0)
        send(editor, 0x02A2)  # WM_NCMOUSELEAVE
        time.sleep(0.25)
        sample("afterPointerLeave", editor)

        send(editor, 0x031A)  # WM_THEMECHANGED
        time.sleep(0.25)
        sample("afterThemeChange", editor)
        before = scroll_position(editor)
        send(editor, 0x0115, 3)  # Native WM_VSCROLL / SB_PAGEDOWN
        time.sleep(0.15)
        check("nativePageScrollMovesDocument", scroll_position(editor) > before)
        sample("afterNativePageScroll", editor)

        send(editor, 0x0100, 0x22)  # WM_KEYDOWN / VK_NEXT
        time.sleep(0.15)
        sample("afterNavigationKey", editor)

        before = scroll_position(editor)
        send(editor, 0x020A, (0xFF88 << 16))  # WM_MOUSEWHEEL / -120
        time.sleep(0.15)
        check("nativeWheelMovesDocument", scroll_position(editor) > before)
        sample("afterWheel", editor)

        info = bar_info(editor)
        bar = info.rcScrollBar
        thumb = wintypes.POINT((bar.left + bar.right) // 2,
                              bar.top + (info.xyThumbTop + info.xyThumbBottom) // 2)
        if USER32.WindowFromPoint(thumb) != editor:
            raise AssertionError("Native thumb is not owned by the fixture editor")
        before = scroll_position(editor)
        USER32.SetCursorPos(thumb.x, thumb.y)
        mouse_button(0x0002)  # MOUSEEVENTF_LEFTDOWN
        try:
            for step in range(1, 6):
                USER32.SetCursorPos(thumb.x, thumb.y + step * 6)
                time.sleep(0.06)
        finally:
            mouse_button(0x0004)  # MOUSEEVENTF_LEFTUP
        time.sleep(0.15)
        check("nativeThumbDragMovesDocument", scroll_position(editor) > before)
        USER32.SetCursorPos(0, 0)
        time.sleep(0.3)
        sample("afterThumbDragAndLeave", editor)

        scale = USER32.GetDpiForWindow(window) / 96
        if not USER32.SetWindowPos(window, None, 0, 0, round(530 * scale),
                                   round(420 * scale), 0x0016):
            raise ctypes.WinError(ctypes.get_last_error())
        time.sleep(0.3)
        sample("afterResize", editor)
        time.sleep(1.2)
        sample("idleAfterResize", editor)
    except Exception as error:
        checks["regressionCompleted"] = False
        samples["error"] = {"type": type(error).__name__, "message": str(error)}
    finally:
        if cursor_saved:
            USER32.SetCursorPos(saved_cursor.x, saved_cursor.y)
        if window and process.poll() is None:
            try:
                send(window, 0x0010)  # WM_CLOSE, only the owned child PID
            except TimeoutError:
                pass
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.terminate()
            process.wait(timeout=3)
        check("fixtureExitZero", process.returncode == 0)

    result = {"scope": "Actual EXE native RichEdit scrollbar; isolated sample fixture only",
              "windowsVersion": sys.getwindowsversion().build,
              "testedBinarySha256": hashlib.sha256(EXE.read_bytes()).hexdigest(),
              "checks": checks, "samples": samples,
              "passed": bool(checks) and all(checks.values())}
    output = ROOT / "artifacts" / "scrollbar-check.json"
    output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(f"Scrollbar regression: {sum(checks.values())}/{len(checks)} passed; {output}")
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
