"""Production EXE integration using synthetic files and PID-bounded messages.

No QA flags, personal clipboard, desktop screenshots, foreign process memory,
or physical keyboard/mouse input. Queries expose only gated numeric state.
"""
from __future__ import annotations

import ctypes
from ctypes import wintypes
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import time
import uuid
import zlib

sys.dont_write_bytecode = True
import lifecycle_regression as native

ROOT, EXE = native.ROOT, native.EXE
USER32, KERNEL32 = native.USER32, native.KERNEL32
send = native.send
WM_COMMAND, WM_KEYDOWN = 0x111, 0x100
QUERY, GATE, ACTIVATE, CLOSE = 0x800F, 0x800E, 0x800A, 0x800B
WS_VISIBLE = 0x10000000
USER32.EnumChildWindows.argtypes = [wintypes.HWND, native.ENUM_PROC, wintypes.LPARAM]
USER32.EnumChildWindows.restype = wintypes.BOOL
USER32.GetWindowRect.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.RECT)]
USER32.GetWindowRect.restype = wintypes.BOOL
USER32.GetClientRect.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.RECT)]
USER32.GetClientRect.restype = wintypes.BOOL
USER32.MapWindowPoints.argtypes = [wintypes.HWND, wintypes.HWND, ctypes.POINTER(wintypes.POINT), wintypes.UINT]
USER32.MapWindowPoints.restype = ctypes.c_int
USER32.SetWindowPos.argtypes = [wintypes.HWND, wintypes.HWND, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, wintypes.UINT]
USER32.SetWindowPos.restype = wintypes.BOOL
USER32.GetDpiForWindow.argtypes = [wintypes.HWND]
USER32.GetDpiForWindow.restype = wintypes.UINT
USER32.PostMessageW.argtypes = [wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM]
USER32.PostMessageW.restype = wintypes.BOOL
USER32.GetWindowDisplayAffinity.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.DWORD)]
USER32.GetWindowDisplayAffinity.restype = wintypes.BOOL
USER32.AttachThreadInput.argtypes = [wintypes.DWORD,wintypes.DWORD,wintypes.BOOL]
USER32.AttachThreadInput.restype = wintypes.BOOL
USER32.GetKeyboardState.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
USER32.GetKeyboardState.restype = wintypes.BOOL
USER32.SetKeyboardState.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
USER32.SetKeyboardState.restype = wintypes.BOOL
USER32.PeekMessageW.argtypes = [ctypes.POINTER(wintypes.MSG),wintypes.HWND,wintypes.UINT,wintypes.UINT,wintypes.UINT]
USER32.PeekMessageW.restype = wintypes.BOOL
KERNEL32.GetCurrentThreadId.restype = wintypes.DWORD


def children(window: int, pid: int) -> list[int]:
    found = []
    @native.ENUM_PROC
    def visit(child, _):
        if native.window_pid(child) == pid:
            found.append(child)
        return True
    USER32.EnumChildWindows(window, visit, 0)
    return found


def class_name(window: int) -> str:
    value = ctypes.create_unicode_buffer(128)
    USER32.GetClassNameW(window, value, len(value))
    return value.value


def visible(window: int) -> bool:
    return bool(USER32.GetWindowLongW(window, -16) & WS_VISIBLE)


def owned_popup(pid: int, name: str = "QuietShareToolMenu") -> int | None:
    found = []
    @native.ENUM_PROC
    def visit(hwnd, _):
        if native.window_pid(hwnd) == pid and class_name(hwnd) == name:
            found.append(hwnd)
        return True
    USER32.EnumWindows(visit, 0)
    return next(iter(found), None)


def rect(window: int, owner: int | None = None) -> tuple[int, int, int, int]:
    bounds = wintypes.RECT()
    if not USER32.GetWindowRect(window, ctypes.byref(bounds)):
        raise ctypes.WinError(ctypes.get_last_error())
    if owner:
        points = (wintypes.POINT * 2)(wintypes.POINT(bounds.left, bounds.top), wintypes.POINT(bounds.right, bounds.bottom))
        USER32.MapWindowPoints(None, owner, points, 2)
        return points[0].x, points[0].y, points[1].x, points[1].y
    return bounds.left, bounds.top, bounds.right, bounds.bottom


def multi_drop(window: int, paths: list[Path]) -> None:
    payload = struct.pack("<IiiII", 20, 0, 0, 0, 1) + ("\0".join(map(str, paths)) + "\0\0").encode("utf-16-le")
    memory = KERNEL32.GlobalAlloc(0x42, len(payload))
    if not memory:
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        address = KERNEL32.GlobalLock(memory)
        ctypes.memmove(address, payload, len(payload))
        KERNEL32.GlobalUnlock(memory)
        copied = send(window, 0x49, len(payload), memory)
        if not copied:
            raise RuntimeError("HDROP marshaling failed")
        # Windows rejects asynchronous posting of this system pointer message;
        # the copied remote HDROP uses the existing bounded synchronous handler.
        send(window,0x233,copied,timeout=15000)
        time.sleep(0.2)
    finally:
        KERNEL32.GlobalFree(memory)


def make_png(path: Path) -> None:
    def chunk(kind: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
    row = b"\0" + bytes([35, 200, 118, 242, 245, 247, 197, 153, 255, 25, 29, 33])
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", 4, 3, 8, 2, 0, 0, 0)) + chunk(b"IDAT", zlib.compress(row * 3)) + chunk(b"IEND", b""))


def wait_for(predicate, timeout=5.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        value = predicate()
        if value:
            return value
        time.sleep(0.04)
    return None


def main() -> int:
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "-" + uuid.uuid4().hex[:8]
    folder = ROOT / "artifacts" / "qa-integration" / stamp
    fixtures = folder / "fixtures"
    fixtures.mkdir(parents=True)
    environment = os.environ.copy()
    environment["LOCALAPPDATA"] = str(folder / "User")
    Path(environment["LOCALAPPDATA"]).mkdir()
    notes = Path(environment["LOCALAPPDATA"]) / "QuietShare" / "notes.dpapi"
    checks, processes = [], []
    app = window = None
    failure = None

    def check(name, passed, details=None):
        checks.append({"check": name, "status": "PASS" if passed else "FAIL", "details": details})
        print(("PASS " if passed else "FAIL ") + name, flush=True)
        if not passed:
            raise AssertionError(name)

    def not_tested(name, reason):
        checks.append({"check": name, "status": "NOT TESTED", "details": reason})

    def launch(arguments=()):
        startup = subprocess.STARTUPINFO()
        startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startup.wShowWindow = 0
        process = subprocess.Popen([str(EXE), *arguments], cwd=EXE.parent, env=environment,
                                   startupinfo=startup, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        processes.append(process)
        return process

    def query(key):
        return send(window, QUERY, key, timeout=6000)

    def command(id):
        if not USER32.PostMessageW(window,WM_COMMAND,id,0):
            raise RuntimeError("Could not post owned UI command")
        time.sleep(0.12)

    def activate(id):
        USER32.PostMessageW(window,ACTIVATE,id,0)
        time.sleep(0.12)
        check(f"activate:{id}", bool(wait_for(lambda:query(2)==id,5)), {"activeId": query(2)})

    def close(process):
        if process.poll() is None:
            for owned in native.app_windows(process.pid):
                send(owned, 0x10, timeout=6000)
            process.wait(timeout=8)

    def find(query_text):
        command(133)
        edit = USER32.GetDlgItem(window, 162)
        check("findControlOwned", bool(edit) and native.window_pid(edit) == app.pid)
        native.set_control_text(edit, query_text)
        command(134)

    def modifier_key(vk,expected_active,shift=False,wait_condition=None):
        # Shared keyboard state belongs only to this test thread and owned app.
        # No SendInput/keybd_event or physical input reaches other applications.
        pending=wintypes.MSG()
        USER32.PeekMessageW(ctypes.byref(pending),None,0,0,0)
        target=USER32.GetWindowThreadProcessId(window,None)
        current=KERNEL32.GetCurrentThreadId()
        if not USER32.AttachThreadInput(current,target,True):
            return None
        original=(ctypes.c_ubyte*256)()
        try:
            if not USER32.GetKeyboardState(original):
                return None
            state=(ctypes.c_ubyte*256).from_buffer_copy(original)
            state[0x11]=state[0xA2]=0x80
            if shift:state[0x10]=state[0xA0]=0x80
            if not USER32.SetKeyboardState(state):return None
            USER32.PostMessageW(code,WM_KEYDOWN,vk,1)
            return bool(wait_for(wait_condition or (lambda:query(2)==expected_active),4))
        finally:
            USER32.SetKeyboardState(original)
            USER32.AttachThreadInput(current,target,False)

    try:
        check("noExistingProductionWindow", not native.app_windows(), "Existing user windows are left untouched")
        check("productionExePresent", EXE.is_file())
        for flag in ("--qa-visible", "--qa-excluded", "--qa-storage", "--qa-scrollbar"):
            rejected = launch((flag, str(fixtures)))
            check("shippingRejects:" + flag, rejected.wait(timeout=8) == 2, {"exitCode": rejected.returncode})
            check("shippingQaCreatedNoWindow:" + flag, not native.app_windows(rejected.pid))
        app = launch()
        window = wait_for(lambda: next(iter(native.app_windows(app.pid)), None), 8)
        check("productionWindowOwned", bool(window) and native.window_pid(window) == app.pid)
        check("productionControlsInitialized", bool(wait_for(lambda: USER32.GetDlgItem(window,110),8)))
        version_label=wait_for(lambda:USER32.GetDlgItem(window,192),8)
        check("homeVersionLabelExistsBeforePrivacyGate",bool(version_label) and native.control_text(version_label)=="v0.9.2")
        check("actualProductionPrivacyPreflight", bool(wait_for(lambda: send(window, GATE, timeout=6000), 20)),
              "Real production WGC gate; no QA bypass")
        USER32.ShowWindow(window, 4)
        check("homeVersionLabelVisible",visible(version_label) and query(39)==902)
        affinity = wintypes.DWORD()
        check("productionAffinityReadback", USER32.GetWindowDisplayAffinity(window, ctypes.byref(affinity)) and affinity.value == 0x11)
        all_children = children(window, app.pid)
        code = USER32.GetDlgItem(window, 170)
        pdf = USER32.GetDlgItem(window, 171)
        editor = USER32.GetDlgItem(window, 110)
        tree = USER32.GetDlgItem(window, 172)
        bar = next((child for child in all_children if class_name(child) == "QuietShareDocumentBar"), 0)
        image = next((child for child in all_children if class_name(child) == "QuietShareImage"), 0)
        check("allNativeHostsPresent", all([code, pdf, editor, tree, bar, image]))
        check("nativeCodeClass", class_name(code) == "Scintilla")
        check("nativePdfClass", class_name(pdf) == "QuietSharePdfCanvas")
        command(120)
        draft = "QUIETSHARE_INTEGRATION_SYNTHETIC_DRAFT\r\nLời nhắc tiếng Việt: giữ nguyên nội dung.\r\nMột, hai, ba."
        check("scriptInserted", native.set_control_text(editor, draft))
        expected = native.control_text(editor)
        check("scriptAutosave", bool(wait_for(lambda: notes.exists() and native.decrypt(notes) == expected, 5)))

        pin = USER32.GetDlgItem(window,188)
        pin_scale=USER32.GetDpiForWindow(window)/96
        for page_id,label in ((122,"home"),(123,"settings"),(139,"presentation"),(154,"browser"),(120,"content")):
            command(page_id)
            pin_bounds = rect(pin,window)
            check("upperPinAvailable:"+label,visible(pin) and pin_bounds[1]>=0 and pin_bounds[3]<=40*pin_scale)
        was_pinned = bool(USER32.GetWindowLongW(window,-20)&8)
        command(188)
        check("upperPinChangesNativeTopmost",bool(USER32.GetWindowLongW(window,-20)&8)!=was_pinned)
        command(123)
        check("upperPinMatchesSettings",(native.control_text(USER32.GetDlgItem(window,106)).endswith("Bật"))==bool(USER32.GetWindowLongW(window,-20)&8))
        command(106)
        check("settingsPinChangesSameState",bool(USER32.GetWindowLongW(window,-20)&8)==was_pinned)
        command(120)
        editor_size=wintypes.RECT();USER32.GetClientRect(editor,ctypes.byref(editor_size))
        USER32.PostMessageW(window,0x112,0xF020,0)
        check("nativeMinimizeCommandMinimizesOwner",bool(wait_for(lambda:USER32.IsIconic(window),3)))
        time.sleep(0.35)
        minimized_size=wintypes.RECT();USER32.GetClientRect(editor,ctypes.byref(minimized_size))
        check("minimizePreservesRealWgcVerification",bool(send(window,GATE,timeout=6000)))
        check("minimizePreservesViewerBounds",(minimized_size.right,minimized_size.bottom)==(editor_size.right,editor_size.bottom))
        USER32.ShowWindow(window,4)
        check("restoreKeepsRealWgcVerification",not USER32.IsIconic(window) and bool(send(window,GATE,timeout=6000)))
        check("restorePreservesScript",visible(editor) and native.control_text(editor)==expected)

        samples = [
            ("main.py", "# comment\ndef hello():\n    return \"tiếng Việt\"\n" + "print('line') # long \n" * 200, ("#", "def", "tiếng"), (1, 5, 3)),
            ("util.cpp", "// comment\nconst char* hello = \"tiếng Việt\";\n", ("//", "const", "tiếng"), (2, 5, 6)),
            ("app.js", "// comment\nconst hello = \"tiếng Việt\";\n", ("//", "const", "tiếng"), (2, 5, 6)),
            ("config.json", "// comment\n{\"hello\": true, \"text\": \"tiếng Việt\"}\n", ("//", "true", "tiếng"), (6, 11, 2)),
            ("view.html", "<!-- comment -->\n<div class=\"hello\">tiếng Việt</div>\n", ("comment", "div", "hello"), (9, 1, 6)),
            ("style.css", "/* comment */\ndiv { color: red; font-family: \"hello\"; }\n", ("comment", "color", "hello"), (9, 6, 13)),
        ]
        paths = []
        for name, text, _, _ in samples:
            path = fixtures / name
            path.write_bytes(b"\xff\xfe" + text.encode("utf-16-le") if name == "util.cpp" else text.encode("utf-8"))
            paths.append(path)
        make_png(fixtures / "image.png")
        project_source=fixtures / "aaa-project-src"
        (project_source / "nested").mkdir(parents=True)
        (project_source / "main.py").write_text("# Project tree fixture\nprint('preview')\n",encoding="utf-8")
        (project_source / "nested" / "leaf.py").write_text("value = 1\n",encoding="utf-8")
        subprocess.run([sys.executable, str(ROOT / "tests" / "viewer_fixtures.py"), str(fixtures)], check=True,
                       stdout=subprocess.DEVNULL)
        (fixtures / "guide.pdf").write_bytes((fixtures / "many.pdf").read_bytes())
        paths += [fixtures / "image.png", fixtures / "guide.pdf"]
        invalid = fixtures / "invalid.bin"
        invalid.write_bytes(b"\0binary fixture")
        multi_drop(window, paths + [invalid])
        check("eightFileMultiDrop", query(1) == 9, {"tabCountIncludingScript": query(1)})
        check("failedFilePreservesOtherTabs", query(2) == 8 and query(1) == 9)
        multi_drop(window, [paths[0], paths[1]])
        check("duplicateTabsPrevented", query(1) == 9)
        check("duplicateActivatesExistingTab", query(2) == 2)
        for id, (name, text, tokens, ids) in enumerate(samples, 1):
            activate(id)
            actual = [send(code, 2010, len(text[:text.index(token)].encode("utf-8"))) for token in tokens]
            check("productionLexer:" + name, actual == list(ids), {"styleIds": actual})
            check("productionReadOnly:" + name, send(code, 2140) == 1)
            check("productionLineNumbers:" + name, send(code, 2241, 0) == 1 and send(code, 2243, 0) > 0)
        activate(1)
        ctrl_tab=modifier_key(9,2)
        if ctrl_tab is None:not_tested("CtrlTabCtrlShiftTab", "Windows did not permit attaching the two owned thread input queues")
        else:
            check("productionCtrlTab",ctrl_tab)
            check("productionCtrlShiftTab",modifier_key(9,1,True))
        find("tiếng Việt")
        start = len(samples[0][1][:samples[0][1].index("tiếng Việt")].encode("utf-8"))
        check("productionCodeFindSelection", send(code, 2143) == start and send(code, 2145) == start + len("tiếng Việt".encode()))
        command(136)
        command(103)
        check("productionCodeZoom", query(4) == 110)
        send(code,0x20A,(120<<16)|8,0)
        check("productionCodeCtrlWheelZoom",query(4)==120 and send(code,2374)==0)
        command(108)
        check("productionCodeResetClearsNativeOffset",query(4)==100 and send(code,2374)==0)
        command(103)
        send(code, 2613, 80)
        check("codeScrollPositionSet", send(code, 2152) == 80)
        wrap_before=send(code,2269)
        command(146)
        check("productionWrapToggle", send(code, 2269) != wrap_before)
        command(146)
        check("productionWrapRestored", send(code, 2269) == wrap_before)
        activate(7)
        check("productionImageViewer", query(3) == 3 and visible(image) and not visible(code))
        command(103)
        check("productionImageZoom", query(4) == 110)
        activate(1)
        check("perTabCodeZoomPreserved", query(4) == 110)
        check("perTabCodeScrollPreserved", send(code, 2152) == 80)
        activate(8)
        check("productionPdf120Pages", query(3) == 4 and query(6) == 120 and visible(pdf))
        command(138)
        check("productionPdfNextPage", query(5) == 2)
        command(137)
        check("productionPdfPreviousPage", query(5) == 1)
        find("page 3 test")
        check("productionPdfTextSearch", query(5) == 3)
        command(136)
        command(147)
        command(103)
        check("productionPdfFitAndZoom", query(4) == 110)
        activate(7)
        check("perTabImageZoomPreserved", query(4) == 110)
        activate(8)
        check("perTabPdfPageZoomPreserved", query(4) == 110 and query(5) == 3)
        send(pdf, 0x20A, (120 << 16), 0)
        check("productionPdfPanResponsive", query(5) == 3 and query(4) == 110)

        activate(1)
        old_style = send(code, 2010, 0)
        paths[0].write_text("value = 42 # revised\n", encoding="utf-8")
        multi_drop(window, [paths[0]])
        check("modifiedFileOffersReload", visible(USER32.GetDlgItem(window, 144)) and query(1) == 9)
        check("modifiedFileNotSilentlyReloaded", send(code, 2010, 0) == old_style)
        command(145)
        check("keepExistingPreview", not visible(USER32.GetDlgItem(window, 144)) and send(code, 2010, 0) == old_style)
        multi_drop(window, [paths[0]])
        command(144)
        check("explicitReloadChangesPreview", send(code, 2010, 0) == 11 and query(1) == 9)

        activate(8)
        command(125)
        for width, height in ((826, 553), (430, 320)):
            USER32.SetWindowPos(window, None, 0, 0, width, height, 0x16)
            scale = USER32.GetDpiForWindow(window) / 96
            br, cr = rect(bar, window), rect(pdf, window)
            focus = USER32.GetDlgItem(window, 125)
            fr = rect(focus, window)
            check(f"focusStripHeight:{width}", query(8) == 32 and 28 * scale <= br[3] - br[1] <= 34 * scale,
                  {"barHeightPixels": br[3] - br[1], "dpi": USER32.GetDpiForWindow(window)})
            check(f"focusNeverOverlaysContent:{width}", br[3] <= cr[1] and cr[3] - cr[1] >= 80 * scale)
            check(f"focusExitIconCompact:{width}", fr[2] - fr[0] <= 30 * scale and fr[3] - fr[1] <= 30 * scale)
            check(f"focusSidebarHidden:{width}", not visible(USER32.GetDlgItem(window, 120)))
            check(f"focusRemovesNativeCaption:{width}", USER32.GetWindowLongW(window,-16)&0xC00000==0)
        before = query(7)
        send(bar, 0x20A, (120 << 16), 0)
        check("tabsHorizontalOverflowScroll", query(7) < before, {"before": before, "after": query(7)})
        command(125)
        check("normalRestoresNativeCaption", USER32.GetWindowLongW(window,-16)&0xC00000==0xC00000)
        def tab_wheel(delta, horizontal=False):
            send(bar, 0x20E if horizontal else 0x20A, (delta & 0xFFFF) << 16)

        for horizontal, label in ((False, "vertical"), (True, "horizontal")):
            direction = 1 if horizontal else -1
            tab_wheel(-direction * 12000, horizontal)
            check("tabWheelClampsAtStart:" + label, query(7) == 0)
            tab_wheel(direction * 120, horizontal)
            whole = query(7)
            check("tabWheelOneNotchDistance:" + label, whole == round(80 * scale))
            tab_wheel(-direction * 12000, horizontal)
            for _ in range(10):
                tab_wheel(direction * 12, horizontal)
            check("tabWheelSmallPacketsEqualWholeNotch:" + label, query(7) == whole)
            tab_wheel(-direction * 12000, horizontal)
            for _ in range(120):
                tab_wheel(direction, horizontal)
            check("tabWheelFractionalPacketsPreserved:" + label, query(7) == whole)
            tab_wheel(-direction * 15, horizontal)
            check("tabWheelReversesWithoutFullNotch:" + label, query(7) == whole - round(10 * scale))
        before = query(7)
        tab_wheel(0)
        tab_wheel(0, True)
        check("tabZeroWheelDoesNotMove", query(7) == before)
        tab_wheel(12000)
        tab_wheel(-1)
        tab_wheel(1, True)
        check("tabWheelAxesKeepSeparateFractionalMotion", query(7) == 0)
        tab_wheel(-2)
        tab_wheel(2, True)
        check("tabWheelAxesEventuallyApplyBothRemainders", query(7) == round(4 * scale))
        activate(0)
        check("tabActivationStillRevealsFirstTab", query(7) == 0)
        activate(8)
        check("tabActivationStillRevealsLastTab", query(7) > 0)
        USER32.SetWindowPos(window, None, 0, 0, 826, 553, 0x16)
        send(window, 0x312, 2)
        check("F8GateLocksEveryNumericQuery", all(query(i) == 0 for i in range(1, 10)))
        check("F8HidesAllNativeViewersAndProject", not any(visible(h) for h in [code, pdf, editor, image, tree]))
        visible_labels = [native.control_text(h) for h in children(window, app.pid) if visible(h)]
        check("F8VisibleControlsRevealNoFileNames", not any(path.name in label for path in paths for label in visible_labels))
        send(window, 0x312, 2)
        check("F8RestoresActivePdf", query(2) == 8 and query(5) == 3)
        command(120)
        check("scriptDraftSurvivesAllPreviews", native.control_text(editor) == expected and native.decrypt(notes) == expected)
        activate(8)
        ctrl_w=modifier_key(ord('W'),0)
        if ctrl_w is None:
            not_tested("CtrlW", "Windows did not permit owned keyboard-state attachment")
            send(window,CLOSE,8,timeout=10000)
        else:check("productionCtrlW",ctrl_w)
        check("closeActivePdfReleasesTab", query(1) == 8 and query(2) != 8)
        activate(7)
        br = rect(bar)
        bar_width, bar_height = br[2] - br[0], br[3] - br[1]
        send(bar, 0x208, 0, ((bar_height // 2) << 16) | max(23, bar_width - 40))
        check("middleClickClosesVisibleActiveTab", bool(wait_for(lambda: query(1) == 7, 3)))
        command(150)
        check("closeAllKeepsPersistentScript", query(1) == 1 and query(2) == 0 and native.control_text(editor) == expected)
        send(window, CLOSE, 0)
        check("scriptTabCannotBeClosed", query(1) == 1)
        command(123)
        size_input=USER32.GetDlgItem(window,163)
        send(size_input,0x7)
        check("exactSizeInputPresent", bool(size_input) and visible(size_input) and query(21)==1)
        check("fontSliderRemoved", not USER32.GetDlgItem(window,130))
        native.set_control_text(size_input,"37")
        send(size_input,WM_KEYDOWN,0x0D)
        check("exactScriptSize37", native.control_text(size_input)=="37" and query(4)==37*100//24)
        native.set_control_text(size_input,"91")
        send(size_input,WM_KEYDOWN,0x0D)
        check("exactScriptSize91", native.control_text(size_input)=="91" and query(4)==91*100//24)
        command(184)
        check("sizeValidationWithStatusHidden", not query(16)&8)
        native.set_control_text(size_input,"9999")
        send(size_input,WM_KEYDOWN,0x0D)
        check("outOfRangeSizePreservesValue", native.control_text(size_input)=="91" and query(4)==91*100//24 and query(22)==1)
        native.set_control_text(size_input,"24")
        send(size_input,WM_KEYDOWN,0x0D)
        check("validSizeClearsInlineError", query(22)==0)
        command(184)
        command(120)
        command(124)
        picker=wait_for(lambda:owned_popup(app.pid,"QuietShareLocalPicker"),3)
        check("protectedNativeFilePickerOpens", bool(picker))
        picker_affinity=wintypes.DWORD()
        check("nativePickerCaptureExclusion", bool(USER32.GetWindowDisplayAffinity(picker,ctypes.byref(picker_affinity))) and picker_affinity.value==0x11)
        send(window,0x219,7)
        send(window,0x218,10)
        time.sleep(0.4)
        check("batteryAndUnrelatedDeviceDoNotClosePicker", owned_popup(app.pid,"QuietShareLocalPicker")==picker and bool(send(window,GATE,timeout=6000)))
        listing=USER32.GetDlgItem(picker,12)
        expected_entries=sorted([p for p in fixtures.iterdir() if not p.is_symlink()],key=lambda p:(not p.is_dir(),p.name.casefold()))
        check("nativePickerShowsLocalFixtureFiles", send(listing,0x18B)==len(expected_entries))
        for target in ("main.py","util.cpp"):
            index=next(i for i,p in enumerate(expected_entries) if p.name==target)
            send(listing,0x185,1,index)
        USER32.PostMessageW(picker,WM_COMMAND,1,0)
        check("nativePickerMultiOpenPreservesScript", bool(wait_for(lambda:not owned_popup(app.pid,"QuietShareLocalPicker") and query(1)==3,5)))
        command(123)
        send(size_input,0x7)
        native.set_control_text(size_input,"137")
        send(size_input,WM_KEYDOWN,0x0D)
        check("exactCodeZoom137", query(4)==137 and native.control_text(size_input)=="137")
        command(120)
        command(131)
        picker=wait_for(lambda:owned_popup(app.pid,"QuietShareLocalPicker"),3)
        check("protectedNativeProjectPickerOpens", bool(picker))
        USER32.PostMessageW(picker,WM_COMMAND,1,0)
        check("nativeProjectPickerLoadsTree", bool(wait_for(lambda:not owned_popup(app.pid,"QuietShareLocalPicker") and query(9)>1,5)))
        scale=USER32.GetDpiForWindow(window)/96
        root_item=send(tree,0x110A,0)
        first_child=send(tree,0x110A,4,root_item)
        check("projectRootExpandedWithRealEntries", bool(root_item and first_child))
        before_expand=query(9)
        send(tree,0x1102,2,first_child)
        nested_item=send(tree,0x110A,4,first_child)
        check("shippingProjectExpandsNestedDirectory", query(9)==before_expand+2 and bool(nested_item))
        send(tree,0x1102,2,nested_item)
        leaf_item=send(tree,0x110A,4,nested_item)
        check("shippingProjectExpandsSecondLevel", bool(leaf_item) and query(9)==before_expand+3)
        send(tree,0x110B,9,nested_item)
        USER32.PostMessageW(tree,WM_KEYDOWN,0x0D,1)
        check("projectTreeEnterCollapsesFolder", bool(wait_for(lambda:not (send(tree,0x1127,nested_item,0x20)&0x20),4)))
        USER32.PostMessageW(tree,WM_KEYDOWN,0x0D,1)
        check("projectTreeEnterExpandsFolder", bool(wait_for(lambda:send(tree,0x1127,nested_item,0x20)&0x20,4)))
        send(tree,0x110B,9,leaf_item)
        USER32.PostMessageW(tree,WM_KEYDOWN,0x0D,1)
        check("projectTreeEnterOpensNestedFile", bool(wait_for(lambda:query(1)==4,4)))
        USER32.SetWindowPos(window,None,0,0,round(840*scale),round(560*scale),0x16)
        time.sleep(0.15)
        multi_drop(window,[paths[0]])
        check("projectTreeAndCodeRemainVisibleAt840", visible(tree) and visible(code))
        tree_bounds,code_bounds=rect(tree,window),rect(code,window)
        check("projectTreeDoesNotOverlapViewer", tree_bounds[2]<=code_bounds[0] and tree_bounds[2]-tree_bounds[0]>=176*scale)
        activate(0)
        check("projectTreeRemainsVisibleSwitchingTabs", visible(tree) and visible(editor))
        command(151)
        check("projectTreeToggleHidesOnlyExplorer", not visible(tree) and visible(editor))
        command(151)
        check("projectTreeToggleRestoresExplorer", visible(tree) and visible(editor))
        check("projectCtrlBHidesExplorer", modifier_key(ord('B'),0,wait_condition=lambda:not visible(tree)) is True)
        check("projectCtrlBRestoresExplorer", modifier_key(ord('B'),0,wait_condition=lambda:visible(tree)) is True)
        USER32.SetWindowPos(window,None,0,0,round(460*scale),round(360*scale),0x16)
        time.sleep(0.15)
        check("narrowExplorerAndScriptStayVisible", visible(tree) and visible(editor))
        multi_drop(window,[paths[0]])
        check("narrowFileOpenKeepsExplorerAndCode", visible(tree) and visible(code))
        check("narrowFileOpenKeepsExpandedProject", send(tree,0x110A,0)==root_item and bool(send(tree,0x1127,nested_item,0x20)&0x20))
        narrow_tree,narrow_code=rect(tree,window),rect(code,window)
        check("narrowExplorerAndCodeDoNotOverlap", narrow_tree[2]<=narrow_code[0] and narrow_code[2]-narrow_code[0]>=176*scale)
        USER32.SetWindowPos(window,None,0,0,round(840*scale),round(560*scale),0x16)
        time.sleep(0.15)
        check("resizeRestoresTreeAlongsideCode", visible(tree) and visible(code))
        tree_bounds=rect(tree,window)
        start_x,start_y=tree_bounds[2]+round(8*scale),round(100*scale)
        send(window,0x201,1,(start_y<<16)|start_x)
        splitter_owned=query(18)
        # Queue the final move/release together before native cursor input can
        # run between separate synchronous calls while this HWND has capture.
        # Derive the Explorer card's origin from its native tree inset instead
        # of assuming a particular activity-rail/gutter width.
        explorer_left=tree_bounds[0]-round(4*scale)
        drag_end=(start_y<<16)|(explorer_left+round(264*scale))
        if not (USER32.PostMessageW(window,0x200,1,drag_end) and USER32.PostMessageW(window,0x202,0,drag_end)):
            raise RuntimeError("Could not queue owned splitter movement/release")
        check("explorerSplitterReleasesDrag",bool(wait_for(lambda:query(18)==0)))
        check("explorerSplitterStoresExactWidth", query(17)==264,
              {"ownedAfterDown":splitter_owned,"widthAfterUp":query(17),
               "treeBounds":tree_bounds,"scale":scale,"gate":query(16)})
        resized_tree,resized_code=rect(tree,window),rect(code,window)
        check("explorerSplitterResizesBothPanes", resized_tree[2]>tree_bounds[2] and resized_tree[2]<=resized_code[0])
        drag_x=resized_tree[2]+round(8*scale)
        send(window,0x201,1,(start_y<<16)|drag_x)
        check("explorerSplitterOwnsDrag",query(18)==1)
        command(151)
        check("hidingExplorerCancelsDrag",query(18)==0 and not visible(tree))
        send(window,0x200,1,(start_y<<16)|round(700*scale))
        check("hiddenExplorerMouseMoveKeepsWidth",query(17)==264)
        command(151)
        send(window,0x201,1,(start_y<<16)|drag_x)
        command(125)
        check("focusCancelsExplorerDrag",query(18)==0 and not visible(tree))
        send(window,0x200,1,(start_y<<16)|round(700*scale))
        command(125)
        check("focusReturnsPreferredExplorerWidth",query(17)==264 and visible(tree))
        USER32.SetWindowPos(window,None,0,0,round(460*scale),round(360*scale),0x16)
        time.sleep(0.1)
        check("narrowResizeClampsWithoutForgettingWidth",query(17)==264 and visible(tree) and visible(code) and rect(tree,window)[2]<=rect(code,window)[0])
        USER32.SetWindowPos(window,None,0,0,round(840*scale),round(560*scale),0x16)
        time.sleep(0.1)
        check("wideResizeRestoresPreferredExplorerWidth",rect(tree,window)[2]-rect(tree,window)[0]==round(256*scale))
        for command_id,bit,label in ((181,1,"activity"),(182,2,"tabs"),(183,4,"toolbar"),(184,8,"status")):
            before_bounds=rect(code,window)
            command(command_id)
            check(label+"CanBeHiddenIndependently",not query(16)&bit and visible(tree) and visible(code))
            check(label+"HidePreservesDocumentAndProject",query(2)!=0 and send(tree,0x110A,0)==root_item)
        check("layoutButtonSurvivesHiddenBars",visible(USER32.GetDlgItem(window,180)))
        check("upperPinSurvivesHiddenBars",visible(pin) and rect(pin,window)[3]<=40*scale)
        check("activityRailHidden",not visible(USER32.GetDlgItem(window,122)))
        check("tabBarHidden",not visible(bar))
        check("toolbarHidden",not visible(USER32.GetDlgItem(window,102)))
        command(123)
        check("layoutPreferencesApplyAcrossPages",not visible(USER32.GetDlgItem(window,122)) and visible(USER32.GetDlgItem(window,180)) and query(16)==16)
        command(120)
        command(185)
        check("resetLayoutRestoresAllBarsAndWidth",query(16)==31 and query(17)==208 and visible(tree) and visible(editor) and visible(bar))
        command(180)
        layout_popup=wait_for(lambda:owned_popup(app.pid),3)
        check("protectedLayoutMenuOpens",bool(layout_popup))
        layout_affinity=wintypes.DWORD()
        check("layoutMenuCaptureExclusion",bool(USER32.GetWindowDisplayAffinity(layout_popup,ctypes.byref(layout_affinity))) and layout_affinity.value==0x11)
        check("layoutMenuPreservesWgcGate",bool(send(window,GATE,timeout=6000)))
        send(layout_popup,WM_KEYDOWN,0x1B)
        command(124)
        picker=wait_for(lambda:owned_popup(app.pid,"QuietShareLocalPicker"),3)
        check("pickerCanReopenAfterProjectSelection", bool(picker))
        send(window,0x312,2)
        check("F8CancelsPickerAndMasksContent", bool(wait_for(lambda:not owned_popup(app.pid,"QuietShareLocalPicker"),3)) and query(1)==0)
        send(window,0x312,2)
        command(150)
        check("nativePickersKeepSavedScript", query(1)==1 and native.control_text(editor)==expected)
        command(132)
        popup = wait_for(lambda: owned_popup(app.pid), 3)
        check("shippingToolsPopupOpens", bool(popup))
        popup_affinity = wintypes.DWORD()
        check("shippingToolsPopupProtected", bool(USER32.GetWindowDisplayAffinity(popup, ctypes.byref(popup_affinity)))
              and popup_affinity.value == 0x11)
        time.sleep(0.4)
        check("shippingToolsPopupKeepsRealWgcGate", query(1) == 1)
        send(popup, WM_KEYDOWN, 0x1B)
        check("shippingToolsPopupEscapeKeepsGate", not owned_popup(app.pid) and query(1) == 1)
        for command_id in (181,182,183,184):command(command_id)
        check("resourceDefaultPerformance60InProduction",query(26)==2 and query(27)==60)
        command(189)
        check("resourceSaver15WithoutStartingProjection",query(26)==0 and query(27)==15 and query(25)==0)
        check("idleResourceProfileKeeps33msWatchdog",query(28)==33)
        check("resourceProfileDoesNotAutoloadBrowser",query(10)==0 and query(11)==0 and not (notes.parent/"WebProfile").exists())
        check("layoutPreferencesWrittenLocally",(notes.parent/"layout.dat").exists())
        persisted_layout=struct.unpack("<8I",(notes.parent/"layout.dat").read_bytes())
        check("layoutV2PersistsResourceAndVisibility",persisted_layout==(2,0,0,0,0,1,208,0))
        close(app)
        check("productionFirstExitClean", app.returncode == 0)
        app = launch()
        window = wait_for(lambda: next(iter(native.app_windows(app.pid)), None), 8)
        check("productionRestartControlsInitialized",bool(wait_for(lambda:USER32.GetDlgItem(window,110),8)))
        check("productionRestartGate", bool(wait_for(lambda: send(window, GATE, timeout=6000), 20)))
        editor = USER32.GetDlgItem(window, 110)
        check("DPAPIRestartRestoresDraftOnly", native.control_text(editor) == expected and query(1) == 1)
        check("restartRestoresLayoutPreferences",query(16)==16 and not visible(USER32.GetDlgItem(window,122)) and visible(USER32.GetDlgItem(window,180)))
        check("restartRestoresResourceSaver15",query(26)==0 and query(27)==15 and query(28)==33)
        check("restartResourceProfileLeavesBrowserOff",query(10)==0 and query(11)==0)
        command(185)
        check("resetRestoresNavigationAfterRestart",query(16)==31 and visible(USER32.GetDlgItem(window,122)))
        check("layoutResetPreservesResourceChoice",query(26)==0 and query(27)==15)
        close(app)
        check("productionSecondExitClean", app.returncode == 0)
        (notes.parent/"layout.dat").write_bytes(struct.pack("<7I",1,0,1,0,1,1,264))
        app=launch()
        window=wait_for(lambda:next(iter(native.app_windows(app.pid)),None),8)
        check("legacyLayoutRestartControlsInitialized",bool(window) and bool(wait_for(lambda:USER32.GetDlgItem(window,110),8)))
        check("legacyLayoutRestartGate",bool(wait_for(lambda:send(window,GATE,timeout=6000),20)))
        check("legacyV1LayoutVisibilityAndWidthLoad",query(16)==26 and query(17)==264)
        check("legacyV1DefaultsResourceToPerformance60",query(26)==2 and query(27)==60 and query(28)==33)
        check("legacyV1DoesNotAutoloadBrowser",query(10)==0 and query(11)==0)
        command(189)
        upgraded_layout=struct.unpack("<8I",(notes.parent/"layout.dat").read_bytes())
        check("legacyV1UpgradesToV2OnResourceChange",upgraded_layout==(2,0,1,0,1,1,264,0))
        close(app)
        check("legacyLayoutExitClean",app.returncode==0)
        not_tested("physicalExplorerDragDrop", "Synthetic HDROP exercises the production handler; no physical pointer input")
        not_tested("physicalKeyboardInput", "Ctrl+Tab/Shift+Tab/W route through posted keys and owned keyboard state; no physical keystrokes")
        not_tested("nativeFolderPickerKeyboard", "Native local picker selection is tested through PID-bound messages; physical keyboard input is not exercised")
        not_tested("productionAffinityRevocation", "Windows restricts SetWindowDisplayAffinity to the caller's process; no process injection is used")
        not_tested("remoteReceiverAndTwoPhysicalMonitors", "No receiver observation or physical Extend/Duplicate claim")
    except Exception as error:
        failure = {"type": type(error).__name__, "message": str(error)}
    finally:
        for process in processes:
            if process.poll() is None:
                try:
                    close(process)
                except Exception:
                    process.terminate()
                    process.wait(timeout=5)
        report = {"method": "Normal shipping EXE; actual WGC gate; PID-bound native messages; isolated LOCALAPPDATA; synthetic fixtures",
                  "exeSha256": hashlib.sha256(EXE.read_bytes()).hexdigest(), "createdUtc": datetime.now(timezone.utc).isoformat(),
                  "checks": checks, "passed": sum(c["status"] == "PASS" for c in checks),
                  "failed": sum(c["status"] == "FAIL" for c in checks), "notTested": sum(c["status"] == "NOT TESTED" for c in checks),
                  "failure": failure, "processes": [{"pid": p.pid, "exitCode": p.returncode} for p in processes]}
        (folder / "production-integration.json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
        (ROOT / "artifacts" / "production-integration.json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
        print(json.dumps({key: report[key] for key in ("passed", "failed", "notTested", "failure")}, ensure_ascii=False), flush=True)
    return 1 if failure or report["failed"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
