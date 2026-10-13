# Native viewer dependencies

`prepare-viewers.ps1` downloads fixed archives, checks every SHA-256, extracts build headers into ignored `vendor/viewers/`, copies three runtime DLLs beside `QuietShare.exe`, then verifies the runtime hashes and AMD64 PE architecture. It never resolves a moving `latest` release. The shipping app loads these libraries only from its executable directory with Windows DLL search restricted to that directory and system32.

| Library | Pinned version | Publisher / distribution | Use |
|---|---|---|---|
| Scintilla | 5.6.7 | Official `scintilla.org/wscite567.zip` | Native read-only code control, selection, line numbers, find and wrapping |
| Lexilla | 5.5.4 | Same official SciTE archive | Real token lexers, including Python, C/C++, JavaScript, JSON, HTML and CSS |
| PDFium | 157.0.8100.0 / chromium/8100 | Immutable `bblanchon/pdfium-binaries` non-V8 x64 release, built from PDFium upstream | In-memory native PDF page rendering and text selection/search |

All archive and DLL hashes, sizes, versions, license paths and original URLs are recorded in [viewers-lock.json](viewers-lock.json). The PDFium archive hash was also checked against the release asset SHA-256 published by GitHub. Scintilla/Lexilla hashes pin the bytes fetched from the publisher's official HTTPS downloads. The PDFium build configuration is `pdf_enable_v8=false`, `pdf_enable_xfa=false`, standalone x64 Release. None of the three DLLs imports a separately installed MSVC/GCC runtime; their direct imported DLLs are Windows system components.

The complete license texts are under `licenses/`. Include this folder in portable and installed distributions. Scintilla and Lexilla use their permissive copyright notices. PDFium uses its BSD-style license and the bundled dependency notices under `licenses/PDFium-third-party/`.

## Reproduce

From the project directory in PowerShell:

```powershell
./prepare-viewers.ps1
./tests/run-viewers.ps1
```

The regression runner compiles with MinGW-w64 C++17 and the downloaded public headers. It generates harmless fixtures under ignored `artifacts/qa-viewers/`, never user document pages or thumbnails. The Vietnamese text-layer PDF uses Arial from the local Windows font directory only for this synthetic test; this font/PDF is not redistributed. No fixture is copied to the release.

The PDF reader intentionally uses a single active page and one in-memory bitmap rather than continuous multi-page rendering. A PDF is capped at 64 MiB / 10,000 pages; one rendered page is capped at 16 million pixels / 64 MiB. The cache therefore stays below the specification's 256 MiB ceiling, and a 120-page test verifies that only the requested page is rendered. PDFium's progressive renderer services masking/display/power safety messages and the local F8 fallback between chunks; normal file/tab/close commands remain queued to avoid reentrancy. A 750 ms aggregate render budget is enforced cooperatively at PDFium checkpoints and after completion, then releases the unfinished bitmap and shows a sanitized reduce-zoom/change-page error. This is not a hard real-time interruption guarantee: an individual PDFium operation can overrun a checkpoint. The generated transparent-page stress fixture verifies actual timeout rejection and later recovery. Password-protected PDFs report a sanitized unsupported error. Rotation is not implemented. A PDF's text layer is selectable/searchable; image-only documents render normally and have no fabricated text layer. Clipboard output is produced only by an explicit copy action.
