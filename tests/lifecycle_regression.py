"""Exercise the production teleprompter with an isolated LOCALAPPDATA folder.

No QA application flags, personal clipboard, installer, account content, or
desktop capture is used. Window operations target only child process PIDs.
"""

from __future__ import annotations

import ctypes
from ctypes import wintypes
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import uuid


ROOT = Path(__file__).resolve().parents[1]
EXE = ROOT / "dist" / "QuietShare.exe"
USER32 = ctypes.WinDLL("user32", use_last_error=True)
KERNEL32 = ctypes.WinDLL("kernel32", use_last_error=True)
CRYPT32 = ctypes.WinDLL("crypt32", use_last_error=True)
ENUM_PROC = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
USER32.EnumWindows.argtypes = [ENUM_PROC, wintypes.LPARAM]
USER32.EnumWindows.restype = wintypes.BOOL
USER32.GetWindowThreadProcessId.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.DWORD)]
USER32.GetWindowThreadProcessId.restype = wintypes.DWORD
USER32.GetClassNameW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
USER32.GetClassNameW.restype = ctypes.c_int
USER32.GetWindowTextW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
USER32.GetWindowTextW.restype = ctypes.c_int
USER32.GetDlgItem.argtypes = [wintypes.HWND, ctypes.c_int]
USER32.GetDlgItem.restype = wintypes.HWND
USER32.SendMessageTimeoutW.argtypes = [wintypes.HWND, wintypes.UINT, wintypes.WPARAM,
                                     wintypes.LPARAM, wintypes.UINT, wintypes.UINT,
                                     ctypes.POINTER(ctypes.c_size_t)]
USER32.SendMessageTimeoutW.restype = wintypes.LPARAM
USER32.ShowWindow.argtypes = [wintypes.HWND, ctypes.c_int]
USER32.ShowWindow.restype = wintypes.BOOL
KERNEL32.LocalFree.argtypes = [wintypes.HANDLE]
KERNEL32.LocalFree.restype = wintypes.HANDLE


class DataBlob(ctypes.Structure):
    _fields_ = [("size", wintypes.DWORD), ("data", ctypes.POINTER(ctypes.c_ubyte))]


CRYPT32.CryptUnprotectData.argtypes = [ctypes.POINTER(DataBlob), ctypes.c_void_p,
                                     ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p,
                                     wintypes.DWORD, ctypes.POINTER(DataBlob)]
CRYPT32.CryptUnprotectData.restype = wintypes.BOOL


def window_pid(window: int) -> int:
    pid = wintypes.DWORD()
    USER32.GetWindowThreadProcessId(window, ctypes.byref(pid))
    return pid.value


def app_windows(pid: int | None = None) -> list[int]:
    found = []

    @ENUM_PROC
    def visit(window, _):
        if pid is not None and window_pid(window) != pid:
            return True
        class_name = ctypes.create_unicode_buffer(128)
        USER32.GetClassNameW(window, class_name, len(class_name))
        if class_name.value == "QuietSharePrivate":
            found.append(window)
        return True

    if not USER32.EnumWindows(visit, 0):
        raise ctypes.WinError(ctypes.get_last_error())
    return found


def send(window: int, message: int, wp: int = 0, lp: int = 0, timeout: int = 1000) -> int:
    result = ctypes.c_size_t()
    # Abort if the target is hung; never let a control message block the test.
    if not USER32.SendMessageTimeoutW(window, message, wp, lp, 0x0003, timeout, ctypes.byref(result)):
        raise TimeoutError(f"Window message {message:#x} failed or timed out")
    return result.value


def control_text(control: int) -> str:
    # GetWindowText does not retrieve another process's edit-control contents.
    # WM_GETTEXT is a system message, so Windows marshals this buffer safely.
    length = send(control, 0x000E)
    if length > 500_000:
        raise ValueError("unexpected editor text length")
    buffer = ctypes.create_unicode_buffer(length + 1)
    send(control, 0x000D, len(buffer), ctypes.addressof(buffer))
    return buffer.value


def set_control_text(control: int, text: str) -> bool:
    buffer = ctypes.create_unicode_buffer(text)
    return bool(send(control, 0x000C, 0, ctypes.addressof(buffer)))


def decrypt(path: Path) -> str:
    encrypted = path.read_bytes()
    buffer = ctypes.create_string_buffer(encrypted)
    source = DataBlob(len(encrypted), ctypes.cast(buffer, ctypes.POINTER(ctypes.c_ubyte)))
    plain = DataBlob()
    if not CRYPT32.CryptUnprotectData(ctypes.byref(source), None, None, None, None, 1,
                                    ctypes.byref(plain)):
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        return ctypes.string_at(plain.data, plain.size).decode("utf-8", errors="strict")
    finally:
        if plain.data:
            ctypes.memset(plain.data, 0, plain.size)
            KERNEL32.LocalFree(ctypes.cast(plain.data, wintypes.HANDLE))


def line_endings(text: str) -> str:
    return text.replace("\r\n", "\n").replace("\r", "\n")


def main() -> int:
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ") + "-" + uuid.uuid4().hex[:8]
    folder = ROOT / "artifacts" / "qa-lifecycle" / stamp
    local_data = folder / "User"
    notes = local_data / "QuietShare" / "notes.dpapi"
    notes.parent.mkdir(parents=True, exist_ok=False)
    corrupt = b"QUIETSHARE_CORRUPT_STORAGE_FIXTURE_V02\n" + bytes(range(32))
    notes.write_bytes(corrupt)
    sample = "Lời nhắc kiểm tra vòng đời - tiếng Việt có dấu.\r\nDòng hai: Hà Nội, 123, à ế ộ ư.\r\nGiữ nguyên toàn bộ nội dung."
    checks: list[dict] = []
    processes: list[subprocess.Popen] = []
    process_info: list[dict] = []
    forced_cleanup: list[int] = []
    backups: list[dict] = []
    executable_hash = hashlib.sha256(EXE.read_bytes()).hexdigest() if EXE.is_file() else None
    environment = os.environ.copy()
    environment["LOCALAPPDATA"] = str(local_data)
    expected_text = ""
    first = None

    def check(name: str, passed: bool, details: object) -> None:
        checks.append({"check": name, "passed": passed, "details": details})
        if not passed:
            raise AssertionError(name)

    def launch(label: str) -> subprocess.Popen:
        startup = subprocess.STARTUPINFO()
        startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startup.wShowWindow = 0
        process = subprocess.Popen([str(EXE)], env=environment, cwd=str(EXE.parent),
                                   startupinfo=startup, stdout=subprocess.DEVNULL,
                                   stderr=subprocess.DEVNULL)
        processes.append(process)
        process_info.append({"label": label, "pid": process.pid})
        return process

    def wait_window(process: subprocess.Popen, timeout: float = 5.0) -> int:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            windows = app_windows(process.pid)
            if windows:
                return windows[0]
            if process.poll() is not None:
                raise RuntimeError(f"PID {process.pid} exited before creating its window: {process.returncode}")
            time.sleep(0.05)
        raise TimeoutError(f"PID {process.pid} did not create a window within {timeout}s")

    def close_owned(process: subprocess.Popen) -> None:
        if process.poll() is not None:
            return
        for owned_window in app_windows(process.pid):
            if window_pid(owned_window) == process.pid:
                send(owned_window, 0x0010)
        process.wait(timeout=4)

    try:
        # Avoid launching a second instance against any existing real document.
        existing = app_windows()
        check("noExistingProductionWindow", not existing,
              {"existingWindowCount": len(existing), "reason": "existing app is left untouched"})
        check("productionExePresent", EXE.is_file(), str(EXE))
        first = launch("first")
        window = wait_window(first)
        title = ctypes.create_unicode_buffer(256)
        USER32.GetWindowTextW(window, title, len(title))
        check("firstWindowOwnedByTestPid", window_pid(window) == first.pid,
              {"pid": first.pid, "title": title.value})
        editor = USER32.GetDlgItem(window, 110)
        check("editorFound", bool(editor) and window_pid(editor) == first.pid, {"controlId": 110})
        check("unreadableNotesStartsBlank", control_text(editor) == "", "editor is empty")
        backup_paths = list(notes.parent.glob("notes.dpapi.recovery-*"))
        backups = [{"path": str(path), "bytes": path.stat().st_size,
                    "sha256": hashlib.sha256(path.read_bytes()).hexdigest()} for path in backup_paths]
        check("unreadableBackupPreservesOriginalBytes",
              len(backup_paths) == 1 and backup_paths[0].read_bytes() == corrupt, backups)
        check("unreadableOriginalRetainedBeforeEdit", notes.read_bytes() == corrupt,
              "original corrupt fixture remains until a new autosave")
        text_mode = send(editor, 0x0400 + 90)
        check("editorIsNativePlainText", bool(text_mode & 1), {"textModeBits": text_mode})
        check("unicodeTextInserted", set_control_text(editor, sample), "WM_SETTEXT succeeded")
        expected_text = control_text(editor)
        check("unicodeEditorTextPreserved", line_endings(expected_text) == line_endings(sample),
              {"characters": len(expected_text), "comparison": "only CR/CRLF normalization allowed"})
        time.sleep(1.3)
        check("firstStillRunningAfterAutosave", first.poll() is None, {"pid": first.pid})
        check("autosaveDecryptsExactEditorText", decrypt(notes) == expected_text,
              {"notesBytes": notes.stat().st_size, "characters": len(expected_text)})
        check("autosaveDoesNotWritePlaintext", sample.encode("utf-8") not in notes.read_bytes(),
              "synthetic Unicode marker absent from encrypted file")
        check("backupUnchangedAfterAutosave", backup_paths[0].read_bytes() == corrupt,
              hashlib.sha256(backup_paths[0].read_bytes()).hexdigest())
        second = launch("second-instance")
        second.wait(timeout=4)
        check("secondInstanceExitsSuccessfully", second.returncode == 0, {"exitCode": second.returncode})
        check("firstStillRunningAfterSecondLaunch", first.poll() is None, {"pid": first.pid})
        first_windows = app_windows(first.pid)
        check("singleFirstWindowAfterSecondLaunch", len(first_windows) == 1 and not app_windows(second.pid),
              {"firstWindowCount": len(first_windows), "secondWindowCount": len(app_windows(second.pid))})
        # The product deliberately restores its existing window on a second
        # launch. Hide only our own sample window again; never another PID.
        USER32.ShowWindow(window, 0)
        close_owned(first)
        check("firstClosedNormally", first.returncode == 0, {"exitCode": first.returncode})
        reopened = launch("reopened")
        reopened_window = wait_window(reopened)
        reopened_editor = USER32.GetDlgItem(reopened_window, 110)
        check("reopenRestoresExactText", bool(reopened_editor) and control_text(reopened_editor) == expected_text,
              {"pid": reopened.pid, "characters": len(expected_text)})
        check("reopenKeepsPlainTextMode", bool(send(reopened_editor, 0x0400 + 90) & 1), "TM_PLAINTEXT remains set")
        close_owned(reopened)
        check("reopenedClosedNormally", reopened.returncode == 0, {"exitCode": reopened.returncode})
        check("testedBinaryUnchanged", hashlib.sha256(EXE.read_bytes()).hexdigest() == executable_hash,
              executable_hash)
    except Exception as error:
        if not checks or checks[-1]["passed"]:
            checks.append({"check": "lifecycleExecution", "passed": False,
                           "details": f"{type(error).__name__}: {error}"})
    finally:
        for process in reversed(processes):
            if process.poll() is not None:
                continue
            try:
                close_owned(process)
            except Exception:
                # Popen retains the handle to the exact process launched here.
                # Termination is a last resort for this test-owned PID only.
                if process.poll() is None:
                    process.terminate()
                    process.wait(timeout=2)
                    forced_cleanup.append(process.pid)
        if forced_cleanup:
            checks.append({"check": "allTestProcessesClosedNormally", "passed": False,
                           "details": {"forcedCleanupPids": forced_cleanup}})

    failed = [item["check"] for item in checks if not item["passed"]]
    report = {
        "applicationVersion": "0.2.0",
        "passed": not failed,
        "method": "Production EXE with isolated LOCALAPPDATA; PID-bound Win32 control messages and DPAPI decrypt",
        "scope": "Unreadable-file recovery, Unicode autosave, native plaintext mode, single instance and restart restoration",
        "isolation": {"localAppData": str(local_data), "applicationFlags": [], "personalClipboardUsed": False,
                      "installerRun": False, "desktopCaptured": False, "existingUserWindowTouched": False},
        "exeSha256": executable_hash,
        "processes": process_info,
        "backups": backups,
        "passedChecks": sum(item["passed"] for item in checks),
        "failedChecks": failed,
        "checks": checks,
        "limits": ["Synthetic document only; no personal document was opened.",
                   "This regression does not test meeting capture, display reconnect, or installation."],
    }
    target = ROOT / "artifacts" / "lifecycle-check.json"
    target.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"passed": report["passed"], "passedChecks": report["passedChecks"],
                      "failedChecks": failed, "report": str(target), "exeSha256": executable_hash}, indent=2))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
