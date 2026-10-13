"""Verify QuietShare 0.2.0 Windows EXE metadata/resources without running apps."""

from pathlib import Path
import ctypes
from ctypes import wintypes
import hashlib
import json
import struct
import sys


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_VERSION = (0, 2, 0, 0)
KERNEL = ctypes.WinDLL("kernel32", use_last_error=True)
VERSION = ctypes.WinDLL("version", use_last_error=True)
KERNEL.LoadLibraryExW.argtypes = [wintypes.LPCWSTR, wintypes.HANDLE, wintypes.DWORD]
KERNEL.LoadLibraryExW.restype = wintypes.HMODULE
KERNEL.FindResourceW.argtypes = [wintypes.HMODULE, ctypes.c_void_p, ctypes.c_void_p]
KERNEL.FindResourceW.restype = ctypes.c_void_p
KERNEL.LoadResource.argtypes = [wintypes.HMODULE, ctypes.c_void_p]
KERNEL.LoadResource.restype = ctypes.c_void_p
KERNEL.LockResource.argtypes = [ctypes.c_void_p]
KERNEL.LockResource.restype = ctypes.c_void_p
KERNEL.SizeofResource.argtypes = [wintypes.HMODULE, ctypes.c_void_p]
KERNEL.SizeofResource.restype = wintypes.DWORD
KERNEL.FreeLibrary.argtypes = [wintypes.HMODULE]
KERNEL.FreeLibrary.restype = wintypes.BOOL
VERSION.GetFileVersionInfoSizeW.argtypes = [wintypes.LPCWSTR, ctypes.POINTER(wintypes.DWORD)]
VERSION.GetFileVersionInfoSizeW.restype = wintypes.DWORD
VERSION.GetFileVersionInfoW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, ctypes.c_void_p]
VERSION.GetFileVersionInfoW.restype = wintypes.BOOL
VERSION.VerQueryValueW.argtypes = [ctypes.c_void_p, wintypes.LPCWSTR, ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(wintypes.UINT)]
VERSION.VerQueryValueW.restype = wintypes.BOOL


class FixedFileInfo(ctypes.Structure):
    _fields_ = [(name, wintypes.DWORD) for name in (
        "signature", "structureVersion", "fileVersionMS", "fileVersionLS",
        "productVersionMS", "productVersionLS", "flagsMask", "flags", "os",
        "fileType", "subtype", "dateMS", "dateLS")]


def resource(path: Path, identity: int) -> bytes:
    # LOAD_LIBRARY_AS_DATAFILE does not execute the EXE or its entry point.
    module = KERNEL.LoadLibraryExW(str(path), None, 2)
    if not module:
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        entry = KERNEL.FindResourceW(module, identity, 10)
        if not entry:
            raise ctypes.WinError(ctypes.get_last_error())
        size = KERNEL.SizeofResource(module, entry)
        loaded = KERNEL.LoadResource(module, entry)
        address = KERNEL.LockResource(loaded)
        if not address or not size:
            raise ctypes.WinError(ctypes.get_last_error())
        return ctypes.string_at(address, size)
    finally:
        KERNEL.FreeLibrary(module)


def file_versions(path: Path) -> dict:
    unused = wintypes.DWORD()
    size = VERSION.GetFileVersionInfoSizeW(str(path), ctypes.byref(unused))
    if not size:
        raise ctypes.WinError(ctypes.get_last_error())
    buffer = ctypes.create_string_buffer(size)
    if not VERSION.GetFileVersionInfoW(str(path), 0, size, buffer):
        raise ctypes.WinError(ctypes.get_last_error())
    address, length = ctypes.c_void_p(), wintypes.UINT()
    if not VERSION.VerQueryValueW(buffer, "\\", ctypes.byref(address), ctypes.byref(length)):
        raise ctypes.WinError(ctypes.get_last_error())
    if length.value < ctypes.sizeof(FixedFileInfo):
        raise ValueError("truncated VS_FIXEDFILEINFO")
    info = ctypes.cast(address, ctypes.POINTER(FixedFileInfo)).contents
    if info.signature != 0xFEEF04BD:
        raise ValueError("invalid VS_FIXEDFILEINFO signature")

    def parts(ms: int, ls: int) -> tuple[int, int, int, int]:
        return ms >> 16, ms & 0xFFFF, ls >> 16, ls & 0xFFFF

    return {"fileVersion": parts(info.fileVersionMS, info.fileVersionLS),
            "productVersion": parts(info.productVersionMS, info.productVersionLS)}


def pe_metadata(data: bytes) -> dict:
    offset = struct.unpack_from("<I", data, 0x3C)[0]
    valid = data[:2] == b"MZ" and data[offset:offset + 4] == b"PE\0\0"
    machine = struct.unpack_from("<H", data, offset + 4)[0]
    magic = struct.unpack_from("<H", data, offset + 24)[0]
    return {"x64PE": valid and machine == 0x8664 and magic == 0x20B,
            "machine": hex(machine), "optionalHeaderMagic": hex(magic)}


def main() -> int:
    checks, files, errors = {}, {}, {}
    app_path = ROOT / "dist" / "QuietShare.exe"
    setup_path = ROOT / "dist" / "QuietShare-Setup.exe"
    app = None
    for name, path in (("QuietShare.exe", app_path), ("QuietShare-Setup.exe", setup_path)):
        try:
            data = path.read_bytes()
            metadata = pe_metadata(data)
            versions = file_versions(path)
            files[name] = {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest(),
                           **metadata, **versions}
            checks[name + ".x64PE"] = metadata["x64PE"]
            checks[name + ".fileVersion0.2.0"] = versions["fileVersion"] == EXPECTED_VERSION
            checks[name + ".productVersion0.2.0"] = versions["productVersion"] == EXPECTED_VERSION
            if path == app_path:
                app = data
        except (OSError, ValueError, struct.error) as error:
            checks[name + ".readableMetadata"] = False
            errors[name] = str(error)
    try:
        checks["installerPayloadMatchesPortableExe"] = app is not None and resource(setup_path, 301) == app
    except (OSError, ValueError) as error:
        checks["installerPayloadMatchesPortableExe"] = False
        errors["installerPayload"] = str(error)
    report = {
        "version": "0.2.0",
        "platform": "win-x64",
        "passed": bool(checks) and all(checks.values()),
        "checks": checks,
        "files": files,
        "errors": errors,
        "authenticode": None,
        "runtime": "Native text teleprompter using Windows system libraries",
        "method": "PE header parsing, Windows VERSIONINFO reading and setup RCDATA 301 byte comparison; no EXE execution",
    }
    target = ROOT / "artifacts" / "release-inventory.json"
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
