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


def owned_popup(pid: int) -> int | None:
    found = []
    @native.ENUM_PROC
    def visit(hwnd, _):
        if native.window_pid(hwnd) == pid and class_name(hwnd) == "QuietShareToolMenu":
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

    def modifier_key(vk,expected_active,shift=False):
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
            return bool(wait_for(lambda:query(2)==expected_active,4))
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
        check("actualProductionPrivacyPreflight", bool(wait_for(lambda: send(window, GATE, timeout=6000), 20)),
              "Real production WGC gate; no QA bypass")
        USER32.ShowWindow(window, 4)
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
        close(app)
        check("productionFirstExitClean", app.returncode == 0)
        app = launch()
        window = wait_for(lambda: next(iter(native.app_windows(app.pid)), None), 8)
        check("productionRestartControlsInitialized",bool(wait_for(lambda:USER32.GetDlgItem(window,110),8)))
        check("productionRestartGate", bool(wait_for(lambda: send(window, GATE, timeout=6000), 20)))
        editor = USER32.GetDlgItem(window, 110)
        check("DPAPIRestartRestoresDraftOnly", native.control_text(editor) == expected and query(1) == 1)
        close(app)
        check("productionSecondExitClean", app.returncode == 0)
        not_tested("physicalExplorerDragDrop", "Synthetic HDROP exercises the production handler; no physical pointer input")
        not_tested("physicalKeyboardInput", "Ctrl+Tab/Shift+Tab/W route through posted keys and owned keyboard state; no physical keystrokes")
        not_tested("nativeFolderPickerKeyboard", "Project module has separate lazy-tree tests; this run does not automate the modal OS picker")
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
