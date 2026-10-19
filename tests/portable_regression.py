"""Run production checks from an extracted, strictly minimal portable ZIP."""
from pathlib import Path
import json
import sys
import uuid
import zipfile

import production_integration as production


def main():
    root = production.ROOT
    version = json.loads((root / "artifacts/release-inventory.json").read_text())["version"]
    archive = root / ("QuietShare-v" + version + "-win-x64.zip")
    allowed = {"QuietShare.exe", "Scintilla.dll", "Lexilla.dll", "pdfium.dll", "WebView2Loader.dll",
               "README.md", "THIRD-PARTY.md", "dependencies/licenses/PDFium-third-party/freetype.txt"}
    allowed.update("docs/" + name for name in ["README.md", "USER_GUIDE.md", "PRIVACY.md",
                   "TROUBLESHOOTING.md", "BUILDING.md", "DEPENDENCIES.md", "BOOK.md", "quietshare-home.png", "readme-banner.svg", "project-status.svg"])
    target = root / "artifacts" / "qa-portable" / uuid.uuid4().hex
    target.mkdir(parents=True)
    with zipfile.ZipFile(archive) as package:
        names = package.namelist()
        assert len(names) == len(allowed) and set(names) == allowed, "Unexpected portable payload"
        for name in names:
            assert package.read(name) == (root / "dist" / name).read_bytes(), name
            destination = (target / name).resolve()
            assert destination.is_relative_to(target.resolve()), name
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(package.read(name))
    production.EXE = target / "QuietShare.exe"
    result = production.main()
    report = {"version": version, "fileCount": len(allowed), "exactPayloadAndDistParity": True,
              "productionChecksExitCode": result}
    (root / "artifacts/portable-regression.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps(report))
    return result


if __name__ == "__main__":
    sys.exit(main())
