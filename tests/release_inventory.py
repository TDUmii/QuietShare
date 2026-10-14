"""Verify QuietShare 0.5.0 Windows EXE metadata/resources without running apps."""

from pathlib import Path
import ctypes
from ctypes import wintypes
import hashlib
import json
import struct
import sys
import subprocess
import shutil
import os


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_VERSION = (0, 5, 0, 0)
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


def import_names(data: bytes) -> list[str]:
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    sections = struct.unpack_from("<H", data, pe + 6)[0]
    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    section_start = pe + 24 + optional_size
    def offset(rva):
        for index in range(sections):
            virtual_size, virtual, raw_size, raw = struct.unpack_from("<IIII", data, section_start + index*40 + 8)
            if virtual <= rva < virtual + max(virtual_size,raw_size):
                return raw + rva - virtual
        raise ValueError("Import RVA is outside image sections")
    rva = struct.unpack_from("<I",data,pe+24+112+8)[0]
    imports = []
    if rva:
        cursor = offset(rva)
        for _ in range(128):
            fields = struct.unpack_from("<IIIII",data,cursor)
            if not any(fields):
                break
            name = offset(fields[3])
            imports.append(data[name:data.index(b"\0",name)].decode("ascii"))
            cursor += 20
    return imports


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
            checks[name + ".fileVersion0.5.0"] = versions["fileVersion"] == EXPECTED_VERSION
            checks[name + ".productVersion0.5.0"] = versions["productVersion"] == EXPECTED_VERSION
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
    manifest = json.loads((ROOT / "dependencies" / "viewers-lock.json").read_text(encoding="utf-8"))
    for index, runtime in enumerate(manifest["runtime"],302):
        name = runtime["file"]
        data = (ROOT / "dist" / name).read_bytes()
        metadata = pe_metadata(data)
        digest = hashlib.sha256(data).hexdigest()
        files[name] = {"bytes":len(data),"sha256":digest,"version":runtime["version"],**metadata}
        checks[name+".x64PE"] = metadata["x64PE"]
        checks[name+".pinnedHashAndSize"] = digest == runtime["sha256"] and len(data) == runtime["bytes"]
        checks[name+".installerPayloadParity"] = resource(setup_path,index) == data
    notice = (ROOT / "dist" / "THIRD-PARTY.md").read_bytes()
    browser_lock=json.loads((ROOT/"dependencies/browser-lock.json").read_text(encoding="utf-8"))
    loader=browser_lock['loader'];loader_data=(ROOT/'dist'/loader['file']).read_bytes()
    files[loader['file']]={'bytes':len(loader_data),'sha256':hashlib.sha256(loader_data).hexdigest(),'version':browser_lock['sdkVersion'],**pe_metadata(loader_data)}
    checks['browserLoaderPinnedHashAndSize']=files[loader['file']]['sha256']==loader['sha256'] and len(loader_data)==loader['bytes']
    checks['browserLoaderX64PE']=files[loader['file']]['x64PE']
    checks['browserLoaderInstallerParity']=resource(setup_path,306)==loader_data
    checks["installerFullNoticeParity"] = resource(setup_path,305) == notice
    checks["sourceNoticeMatchesDist"] = (ROOT / "THIRD-PARTY.md").read_bytes() == notice
    codicons = (ROOT / "assets/codicons/codicon.ttf").read_bytes()
    checks["codiconsPinnedFontHash"] = hashlib.sha256(codicons).hexdigest() == "598b7946cbc22abe5e7fcf336b07b465446b47c1b29b4509cf31eca0a4964f1d"
    checks["appEmbeddedCodiconsParity"] = resource(ROOT / "dist/QuietShare.exe",401) == codicons
    checks["installerEmbeddedCodiconsParity"] = resource(setup_path,401) == codicons
    system_imports = {"advapi32.dll","comctl32.dll","comdlg32.dll","crypt32.dll","d3d11.dll",
                      "dwmapi.dll","dxgi.dll","gdi32.dll","imm32.dll","kernel32.dll","msimg32.dll",
                      "ole32.dll","oleaut32.dll","shell32.dll","ucrtbase.dll","msvcrt.dll","user32.dll","uxtheme.dll","version.dll",
                      "api-ms-win-core-winrt-l1-1-0.dll","api-ms-win-core-winrt-string-l1-1-0.dll"}
    for name in files:
        imports = import_names((ROOT / "dist" / name).read_bytes())
        files[name]["imports"] = imports
        checks[name+".systemImportsOnly"] = bool(imports) and all(item.lower() in system_imports for item in imports)
    signing_env = os.environ.copy()
    signing_env.pop("PSModulePath",None)
    signing = subprocess.run([shutil.which("pwsh.exe") or "powershell.exe","-NoProfile","-Command",
        "Get-AuthenticodeSignature -LiteralPath 'dist/QuietShare.exe','dist/QuietShare-Setup.exe' | "
        "Select-Object @{Name='file';Expression={Split-Path -Leaf $_.Path}},@{Name='status';Expression={$_.Status.ToString()}} | ConvertTo-Json -Compress"],
        cwd=ROOT,env=signing_env,check=True,capture_output=True,text=True)
    authenticode = {item["file"]:item["status"] for item in json.loads(signing.stdout)}
    report = {
        "version": "0.5.0",
        "platform": "win-x64",
        "passed": bool(checks) and all(checks.values()),
        "checks": checks,
        "files": files,
        "errors": errors,
        "authenticode": authenticode,
        "runtime": "Native Win32 with pinned x64 viewer DLLs and on-demand WebView2 loader; optional shared Evergreen Runtime; Windows WIC, WGC and D3D11",
        "method": "PE headers, VERSIONINFO and setup RCDATA 301-306 byte parity; dependency SHA-256/size manifest verification; no EXE execution",
    }
    target = ROOT / "artifacts" / "release-inventory.json"
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
