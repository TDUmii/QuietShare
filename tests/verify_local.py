"""Verify QuietShare 0.6.0 teleprompter GDI captures and export review PNGs.

This checks one local BitBlt capture path. It cannot establish what Zoom,
Teams, OBS, another capture API, a remote viewer, or a projector receives.
Python's standard library is sufficient; no account or desktop capture is used.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
import zlib
from dataclasses import dataclass
from pathlib import Path


PURPLE = (124, 58, 237)
# Keep the visual smoke test aligned with the native design tokens in src/main.cpp.
# The check intentionally accepts the shell, sidebar and elevated surfaces because
# the redesigned layout uses all of them in a single capture.
DARK_SURFACES = ((30, 30, 30), (37, 37, 38), (51, 51, 51), (60, 60, 60), (70, 70, 70), (25, 29, 33))
REQUIRED_CAPTURES = {
    "qa-visible": ("editing.bmp", "visible.bmp", "large-text.bmp", "scrolling.bmp", "masked.bmp", "compact.bmp", "english.bmp",
                   "minimum-editing.bmp", "minimum-masked.bmp", "wide.bmp", "hover.bmp", "pressed.bmp", "english-editing.bmp", "empty.bmp", "empty-reading.bmp",
                   "dpi-125.bmp", "dpi-150.bmp", "home.bmp", "settings.bmp", "focus.bmp", "code-file.bmp",
                   "image-file.bmp", "image-focus.bmp", "file-error.bmp", "tabs-overflow.bmp", "code-focus.bmp",
                   "project-wide.bmp", "project-medium.bmp", "project-minimum.bmp", "layout-minimal.bmp", "pdf-file.bmp", "pdf-focus.bmp",
                   "presentation.bmp", "presentation-en.bmp", "tabs-masked.bmp","browser-off.bmp","browser-off-en.bmp","browser-minimum.bmp"),
    "qa-excluded": ("excluded.bmp",),
}


@dataclass
class Bitmap:
    width: int
    height: int
    rgb: bytes


def read_bmp(path: Path) -> Bitmap:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError("not a Windows BMP")
    offset = struct.unpack_from("<I", data, 10)[0]
    header_size = struct.unpack_from("<I", data, 14)[0]
    width, signed_height, planes, depth, compression = struct.unpack_from("<iiHHI", data, 18)
    height = abs(signed_height)
    if header_size < 40 or offset < 14 + header_size or planes != 1:
        raise ValueError("invalid BMP header")
    if width <= 0 or height <= 0 or width * height > 50_000_000:
        raise ValueError("invalid or unexpectedly large dimensions")
    if depth not in (24, 32) or compression != 0:
        raise ValueError("expected an uncompressed 24-bit or 32-bit GDI BMP")
    stride = ((width * depth + 31) // 32) * 4
    if offset + stride * height > len(data):
        raise ValueError("truncated BMP pixels")
    pixels = bytearray(width * height * 3)
    step = depth // 8
    for row in range(height):
        source_row = row if signed_height < 0 else height - row - 1
        source = data[offset + source_row * stride : offset + (source_row + 1) * stride]
        target = row * width * 3
        # Slice assignment converts Windows BGR(A) to portable RGB.
        pixels[target : target + width * 3 : 3] = source[2 : width * step : step]
        pixels[target + 1 : target + width * 3 : 3] = source[1 : width * step : step]
        pixels[target + 2 : target + width * 3 : 3] = source[0 : width * step : step]
    return Bitmap(width, height, bytes(pixels))


def write_png(path: Path, bitmap: Bitmap) -> None:
    def chunk(kind: bytes, value: bytes) -> bytes:
        return struct.pack("!I", len(value)) + kind + value + struct.pack("!I", zlib.crc32(kind + value))

    row_bytes = bitmap.width * 3
    raw = b"".join(
        b"\x00" + bitmap.rgb[row * row_bytes : (row + 1) * row_bytes]
        for row in range(bitmap.height)
    )
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack("!IIBBBBB", bitmap.width, bitmap.height, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 6)) + chunk(b"IEND", b"")
    temporary = path.with_suffix(".png.tmp")
    temporary.write_bytes(png)
    temporary.replace(path)


def near(pixel: tuple[int, int, int], color: tuple[int, int, int], tolerance: int = 8) -> bool:
    return all(abs(a - b) <= tolerance for a, b in zip(pixel, color))


def image_metrics(bitmap: Bitmap) -> dict:
    purple = dark_card = bright = panel_bright = panel_count = sample_count = 0
    colors = set()
    # A deterministic grid caps analysis at about one million sampled pixels.
    stride = max(1, (bitmap.width * bitmap.height // 1_000_000) ** 0.5)
    stride = max(1, int(stride))
    for y in range(0, bitmap.height, stride):
        for x in range(0, bitmap.width, stride):
            index = (y * bitmap.width + x) * 3
            pixel = tuple(bitmap.rgb[index : index + 3])
            sample_count += 1
            purple += near(pixel, PURPLE, 5)
            dark_card += any(near(pixel, color) for color in DARK_SURFACES)
            is_bright = min(pixel) >= 145
            bright += is_bright
            if 0.05 <= x / bitmap.width <= 0.95 and 0.33 <= y / bitmap.height <= 0.82:
                panel_count += 1
                panel_bright += is_bright
            if len(colors) < 4096:
                colors.add(pixel)
    return {
        "width": bitmap.width,
        "height": bitmap.height,
        "sampledPixels": sample_count,
        "purpleFraction": round(purple / sample_count, 6),
        "darkCardFraction": round(dark_card / sample_count, 6),
        "brightPixelFraction": round(bright / sample_count, 6),
        "panelBrightPixelFraction": round(panel_bright / max(1, panel_count), 6),
        "distinctColorsAtLeast": len(colors),
    }


def changed_fraction(first: Bitmap, second: Bitmap) -> float:
    if (first.width, first.height) != (second.width, second.height):
        raise ValueError("capture stages have different dimensions")
    total = first.width * first.height
    step = max(1, total // 1_000_000)
    changed = samples = 0
    for pixel in range(0, total, step):
        offset = pixel * 3
        changed += first.rgb[offset : offset + 3] != second.rgb[offset : offset + 3]
        samples += 1
    return round(changed / samples, 6)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    project = Path(__file__).resolve().parents[1]
    parser.add_argument("--artifacts", type=Path, default=project / "artifacts")
    parser.add_argument("--exe", type=Path, default=project / "dist" / "QuietShare.exe")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    artifacts = args.artifacts.resolve()
    output = args.output or artifacts / "local-qa-report.json"
    checks: list[dict] = []
    images: dict[str, Bitmap] = {}
    captures: dict[str, dict] = {}
    diagnostics: dict[str, dict] = {}

    def check(name: str, passed: bool | None, details: object) -> None:
        checks.append({"check": name, "passed": passed, "details": details})

    for folder, files in REQUIRED_CAPTURES.items():
        diagnostic_path = artifacts / folder / "diagnostics.json"
        try:
            diagnostic = json.loads(diagnostic_path.read_text(encoding="utf-8-sig"))
            if not isinstance(diagnostic, dict):
                raise ValueError("expected a JSON object")
            diagnostics[folder] = diagnostic
            check(f"{folder}.diagnosticsReadable", True, str(diagnostic_path))
        except (OSError, ValueError) as error:
            check(f"{folder}.diagnosticsReadable", False, str(error))
            diagnostic = {}
        expected = {
            "applicationVersion": "0.6.0",
            "topologyKnown": True,
            "displayCloned": False,
            "privacyApplied": True,
            "requestedAffinity": 17,
            "actualAffinity": 17,
            "privacyError": 0,
            "captureFilesWritten": True,
            "notesPersistenceTouched": False,
            "visualMode": "visible" in folder,
        }
        if folder == "qa-visible":
            for field in ("readingModeEntered", "editorReadOnly", "fontControlsApplied", "scrollAdvanced",
                          "pauseStable", "maskApplied", "compactLayoutFits", "contentUnchanged"):
                expected[field] = True
            expected["languageToggleApplied"] = True
            for field in ("polishLayoutMatrixFits", "hoverTrackingApplied", "emptyScriptStayedEmpty", "nativeScrollbarStillScrolls"):
                expected[field] = True
            for field in ("focusHidesChrome", "homeControlsWork", "settingsControlsWork", "codeFilePreview", "imageFilePreview", "filePreviewPreservesScript"):
                expected[field] = True
            for field in ("dropFileHandlerWorks", "imageMaskHidesCanvas", "imageZoomAndPanWork", "focusRestoreButtonWorks", "v04TabsProjectPdfFocusFit"):
                expected[field] = True
        for field, value in expected.items():
            actual = diagnostic.get(field)
            check(f"{folder}.{field}", type(actual) is type(value) and actual == value,
                  {"expected": value, "actual": actual})
        build = diagnostic.get("windowsBuild")
        check(f"{folder}.supportedWindowsBuild", type(build) is int and build >= 19041, build)
        check(f"{folder}.remoteReceiverRemainsUnverified",
              "remoteReceiverVerified" in diagnostic and diagnostic["remoteReceiverVerified"] is None,
              diagnostic.get("remoteReceiverVerified", "missing"))
        if folder == "qa-visible":
            start, end = diagnostic.get("scrollStartY"), diagnostic.get("scrollEndY")
            numeric = type(start) in (int, float) and type(end) in (int, float)
            check(f"{folder}.scrollPositionAdvanced", numeric and end > start,
                  {"start": start, "end": end})
            returned = diagnostic.get("editModeReturned")
            check(f"{folder}.editModeReturned", returned is True if returned is not None else None,
                  {"actual": returned, "missingMeans": "optional return-to-edit check not supplied"})
        for file in files:
            source = artifacts / folder / file
            key = f"{folder}/{file}"
            try:
                bitmap = read_bmp(source)
                metrics = image_metrics(bitmap)
                images[key] = bitmap
                captures[key] = {**metrics, "sha256": hashlib.sha256(source.read_bytes()).hexdigest()}
                write_png(source.with_suffix(".png"), bitmap)
                check(f"{key}.validBmpAndReviewPng", True,
                      {"source": str(source), "reviewPng": str(source.with_suffix(".png"))})
                if file == "excluded.bmp":
                    check(f"{key}.syntheticUnderlayVisible", metrics["purpleFraction"] >= 0.95,
                          {"minimum": 0.95, "actual": metrics["purpleFraction"]})
                else:
                    check(f"{key}.windowPixelsVisible", metrics["purpleFraction"] < 0.02,
                          {"maximumExclusive": 0.02, "actual": metrics["purpleFraction"]})
                    if file in ("image-file.bmp", "image-focus.bmp"):
                        # The known 4x2 BMP has a red top half and a blue bottom half.
                        # A focused image can occupy most of the window with no UI text.
                        for label, fraction, expected_color in (("redTop", 0.40, (255, 0, 0)),
                                                               ("blueBottom", 0.60, (0, 0, 255))):
                            index = (int(bitmap.height * fraction) * bitmap.width + bitmap.width // 2) * 3
                            actual_color = tuple(bitmap.rgb[index:index + 3])
                            check(f"{key}.fixtureImage.{label}", near(actual_color, expected_color, 5),
                                  {"expected": expected_color, "actual": actual_color})
                    else:
                        # Native PDF pages keep their actual white paper background.
                        minimum_dark = 0.07 if file in ("pdf-file.bmp", "pdf-focus.bmp") else 0.40
                        check(f"{key}.darkInterfacePresent", metrics["darkCardFraction"] >= minimum_dark,
                              {"minimum": minimum_dark, "actual": metrics["darkCardFraction"]})
                        check(f"{key}.textLikePixelsPresent",
                              metrics["brightPixelFraction"] >= 0.001 and metrics["distinctColorsAtLeast"] >= 40,
                              {"brightPixelFraction": metrics["brightPixelFraction"],
                               "distinctColorsAtLeast": metrics["distinctColorsAtLeast"],
                               "interpretation": "pixel evidence of rendered text; no OCR or semantic assertion"})
                    if file in ("editing.bmp", "visible.bmp", "large-text.bmp", "scrolling.bmp", "compact.bmp", "english.bmp"):
                        check(f"{key}.contentPanelHasTextLikePixels", metrics["panelBrightPixelFraction"] >= 0.0008,
                              {"minimum": 0.0008, "actual": metrics["panelBrightPixelFraction"]})
            except (OSError, ValueError, struct.error) as error:
                check(f"{key}.validBmpAndReviewPng", False, str(error))

    for before, after in (("editing.bmp", "visible.bmp"), ("visible.bmp", "large-text.bmp"),
                          ("large-text.bmp", "scrolling.bmp"), ("scrolling.bmp", "masked.bmp")):
        key_before, key_after = f"qa-visible/{before}", f"qa-visible/{after}"
        if key_before in images and key_after in images:
            try:
                first,second = images[key_before],images[key_after]
                minimum,scope = 0.003,"whole native window"
                if before == "editing.bmp":
                    # Edit/read now share an uncluttered document surface. Verify
                    # the changing commands rather than require decorative body changes.
                    # Tabs and tools occupy separate rows below the native caption.
                    top, bottom = 32, min(112,first.height,second.height)
                    rows = bottom-top
                    first = Bitmap(first.width,rows,first.rgb[top*first.width*3:bottom*first.width*3])
                    second = Bitmap(second.width,rows,second.rgb[top*second.width*3:bottom*second.width*3])
                    minimum,scope = 0.01,"native tabs and toolbar below the caption"
                difference = changed_fraction(first,second)
                check(f"qa-visible.{before}_to_{after}.changed", difference >= minimum,
                      {"minimum": minimum, "actual": difference,"scope":scope})
            except ValueError as error:
                check(f"qa-visible.{before}_to_{after}.changed", False, str(error))

    if "qa-visible/wide.bmp" in images and "qa-visible/hover.bmp" in images:
        difference = changed_fraction(images["qa-visible/wide.bmp"], images["qa-visible/hover.bmp"])
        check("qa-visible.hoverChangesRenderedPixels", difference >= 0.0001,
              {"minimum": 0.0001, "actual": difference, "interpretation": "same-size native captures before and during hover"})
    if "qa-visible/hover.bmp" in images and "qa-visible/pressed.bmp" in images:
        difference = changed_fraction(images["qa-visible/hover.bmp"], images["qa-visible/pressed.bmp"])
        check("qa-visible.pressChangesRenderedPixels", difference >= 0.0001,
              {"minimum": 0.0001, "actual": difference, "interpretation": "native button state while hovered versus pressed"})

    compact_key, normal_key = "qa-visible/compact.bmp", "qa-visible/visible.bmp"
    if compact_key in images and normal_key in images:
        compact, normal = images[compact_key], images[normal_key]
        check("qa-visible.compactCaptureSmaller",
              300 <= compact.width < normal.width and 240 <= compact.height < normal.height,
              {"readingSize": [normal.width, normal.height], "compactSize": [compact.width, compact.height],
               "interpretation": "physical capture dimensions; logical 460 x 360 size varies with monitor DPI"})
    if "qa-visible/masked.bmp" in captures and "qa-visible/scrolling.bmp" in captures:
        masked_text = captures["qa-visible/masked.bmp"]["panelBrightPixelFraction"]
        reading_text = captures["qa-visible/scrolling.bmp"]["panelBrightPixelFraction"]
        check("qa-visible.maskReducesContentTextPixels", masked_text < reading_text,
              {"readingPanelBrightFraction": reading_text, "maskedPanelBrightFraction": masked_text,
               "interpretation": "synthetic script fixture contains more text than the masking placeholder"})

    # Storage is tested in a dedicated QA data folder and never uses real notes.
    storage_path = artifacts / "storage-check.json"
    storage = None
    if storage_path.exists():
        try:
            storage = json.loads(storage_path.read_text(encoding="utf-8-sig"))
            if not isinstance(storage, dict):
                raise ValueError("expected a JSON object")
        except (OSError, ValueError) as error:
            check("storage.diagnosticsReadable", False, str(error))
    for field in ("dpapiRoundTripPassed", "plaintextMarkerAbsent", "autosavePassed", "notesExistingUntouched"):
        value = storage.get(field) if storage else None
        check(f"storage.{field}", value is True if value is not None else None,
              {"actual": value, "source": str(storage_path),
               "missingMeans": "not performed or not supplied"})

    installer_report = artifacts / "installer-self-test.json"
    try:
        setup = json.loads(installer_report.read_text(encoding="utf-8-sig"))
        for field, expected in {"selfTestPassed": True, "version": "0.6.0",
                                "versionResourceVerified": True, "payloadResourceId": 301,
                                "payloadPortableExecutable": True, "fixedInstallPathVerified": True,
                                "installationWrites": False, "error": ""}.items():
            check(f"installer.{field}", type(setup.get(field)) is type(expected) and setup.get(field) == expected,
                  {"expected": expected, "actual": setup.get(field)})
        check("installer.payloadSizeMatchesExe", setup.get("payloadBytes") == args.exe.stat().st_size,
              {"embeddedBytes": setup.get("payloadBytes"), "exeBytes": args.exe.stat().st_size})
    except (OSError, ValueError, AttributeError) as error:
        check("installer.selfTestReadable", False, str(error))
    try:
        installer_bmp = artifacts / "installer-visible.bmp"
        bitmap = read_bmp(installer_bmp)
        write_png(installer_bmp.with_suffix(".png"), bitmap)
        check("installer.nativeUiCapture", bitmap.width >= 500 and bitmap.height >= 350,
              {"width": bitmap.width, "height": bitmap.height,
               "reviewPng": str(installer_bmp.with_suffix(".png"))})
    except (OSError, ValueError, struct.error) as error:
        check("installer.nativeUiCapture", False, str(error))
    try:
        uninstall_bmp = artifacts / "uninstaller-visible.bmp"
        bitmap = read_bmp(uninstall_bmp)
        write_png(uninstall_bmp.with_suffix(".png"), bitmap)
        check("uninstaller.nativeUiCapture", bitmap.width >= 500 and bitmap.height >= 350,
              {"width": bitmap.width, "height": bitmap.height, "reviewPng": str(uninstall_bmp.with_suffix(".png"))})
    except (OSError, ValueError, struct.error) as error:
        check("uninstaller.nativeUiCapture", False, str(error))

    if args.exe.is_file():
        try:
            executable = args.exe.read_bytes()
            pe_offset = struct.unpack_from("<I", executable, 0x3C)[0]
            valid = executable[:2] == b"MZ" and executable[pe_offset : pe_offset + 4] == b"PE\x00\x00"
            machine = struct.unpack_from("<H", executable, pe_offset + 4)[0]
            optional_magic = struct.unpack_from("<H", executable, pe_offset + 24)[0]
            check("binary.x64PE", valid and machine == 0x8664 and optional_magic == 0x20B,
                  {"path": str(args.exe.resolve()), "machine": hex(machine),
                   "optionalHeaderMagic": hex(optional_magic),
                   "sha256": hashlib.sha256(executable).hexdigest()})
        except (OSError, ValueError, struct.error) as error:
            check("binary.x64PE", False, str(error))
    else:
        check("binary.x64PE", None, "executable was not supplied or built at the selected path")

    failed = [item["check"] for item in checks if item["passed"] is False]
    report = {
        "passed": not failed,
        "method": "GDI BitBlt with SRCCOPY | CAPTUREBLT; synthetic purple underlay and exact window crops",
        "applicationVersion": "0.6.0",
        "scope": "Local native text teleprompter editing, reading, font controls, timer scroll, pause, mask and compact layout",
        "limits": [
            "A separate QA executable uses synthetic fixtures and controlled visual/privacy gates. The shipping executable rejects these switches.",
            "Passing checks proves only these local GDI captures at the sampled instants.",
            "Pixel tests detect purple/card/text-like pixels; they do not read or certify text semantics.",
            "API affinity readback is configuration evidence, not a receiver-side privacy guarantee.",
            "Review exported PNGs visually before release.",
        ],
        "notTested": {
            "remoteMeetingReceiver": None,
            "otherCaptureAPIs": None,
            "physicalProjector": None,
            "displayReconnectTransientFrames": None,
            "otherWindowsMachines": None,
            "installerOnCleanMachine": None,
            "resistanceToReverseEngineering": None,
            "binaryImports": None,
        },
        "summary": {"passedChecks": sum(item["passed"] is True for item in checks),
                    "failedChecks": len(failed), "unavailableChecks": sum(item["passed"] is None for item in checks)},
        "failedChecks": failed,
        "checks": checks,
        "captures": captures,
        "diagnostics": diagnostics,
        "storageDiagnostics": storage,
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"passed": report["passed"], "report": str(output.resolve()),
                      **report["summary"], "failedChecks": failed}, ensure_ascii=False, indent=2))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
