"""Check the two generated manuals with pypdf. Rendered pages need visual review too."""
from pathlib import Path
import hashlib
import json
import re
import sys

from pypdf import PdfReader

ROOT = Path(__file__).resolve().parents[1]
VOLUMES = [
    ("EN", "USER_MANUAL_EN.md", "QuietShare-User-Manual-EN.pdf", "Contents"),
    ("VI", "USER_MANUAL_VI.md", "QuietShare-Huong-Dan-Su-Dung-VI.pdf", "Mục lục"),
]


def plain(text):
    return re.sub(r"\s+", " ", text).strip()


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    checks, files = [], {}

    def check(name, result):
        checks.append({"name": name, "passed": bool(result)})
        print(("PASS " if result else "FAIL ") + name)

    version = re.search(r'QUIETSHARE_VERSION_TEXT "([^"]+)"',
                        (ROOT/"src/Version.h").read_text()).group(1)
    for language, source, filename, contents in VOLUMES:
        path = ROOT/"docs"/filename
        markdown = (ROOT/"docs"/source).read_text(encoding="utf-8-sig")
        reader = PdfReader(path, strict=True)
        pages = [page.extract_text() for page in reader.pages]
        text = plain(" ".join(pages))
        prefix = language + ": "
        check(prefix + "complete readable volume", 16 <= len(pages) <= 40 and len(text) > 18000)
        check(prefix + "current version on cover", version in pages[0])
        check(prefix + "current source fingerprint", hashlib.sha256((ROOT/"docs"/source).read_bytes()).hexdigest()
              in str(reader.metadata.subject))
        check(prefix + "selectable Unicode text", not any(char in text for char in ["\ufffd", "\u25a0", "\x00"]))
        check(prefix + "dedicated contents page", contents in pages[1])
        check(prefix + "contents links", len(reader.pages[1].get("/Annots", [])) >= 14)
        check(prefix + "chapter bookmarks", len([item for item in reader.outline if not isinstance(item, list)]) >= 14)
        for heading in re.findall(r"^## (.+)$", markdown, re.M):
            check(prefix + "chapter " + heading, plain(heading) in text)
        for code in [f"QS-P{number:03d}" for number in range(1,25)] + ["QS-P000", "QS-P099"]:
            check(prefix + "error recipe " + code, code in text)
        for index, page in enumerate(reader.pages, 1):
            check(prefix + f"page {index} A4", abs(float(page.mediabox.width)-595.28)<1
                  and abs(float(page.mediabox.height)-841.89)<1)
            check(prefix + f"page {index} has content", len(plain(pages[index-1])) > 100)
        embedded = set()
        for page in reader.pages:
            for font in page["/Resources"].get("/Font", {}).values():
                font = font.get_object()
                descriptor = font.get("/FontDescriptor")
                if descriptor and descriptor.get_object().get("/FontFile2"):
                    embedded.add(str(font.get("/BaseFont")))
        check(prefix + "embedded regular and bold fonts", len(embedded) >= 2
              and any("Arial" in name for name in embedded))
        files[filename] = {"pages": len(pages), "characters": len(text),
            "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            "sourceSha256": hashlib.sha256((ROOT/"docs"/source).read_bytes()).hexdigest(),
            "embeddedFonts": sorted(embedded)}
    report = {"version": version, "checks": checks,
              "passed": sum(item["passed"] for item in checks),
              "failed": sum(not item["passed"] for item in checks), "files": files}
    (ROOT/"artifacts").mkdir(exist_ok=True)
    (ROOT/"artifacts/manuals-regression.json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps({key: report[key] for key in ["passed", "failed", "files"]}, ensure_ascii=False))
    return int(report["failed"] != 0)


if __name__ == "__main__":
    raise SystemExit(main())
