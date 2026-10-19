"""Validate the current bilingual documentation and optional shipping copy."""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import sys
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
DOCS = ["README.md", "CHANGELOG.md", "docs/README.md", "docs/USER_GUIDE.md",
        "docs/PRIVACY.md", "docs/TROUBLESHOOTING.md", "docs/BUILDING.md", "docs/DEPENDENCIES.md"]


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    checks = []
    def check(name, value):
        checks.append({"name": name, "passed": bool(value)})
        print(("PASS " if value else "FAIL ") + name)

    for name in DOCS:
        path = ROOT / name
        text = path.read_text(encoding="utf-8-sig")
        if name != "README.md":
            check(name + ": bilingual title", " / " in text.splitlines()[0])
        check(name + ": Vietnamese present", bool(re.search(r"[ăâđêôơưĂÂĐÊÔƠƯ]", text)))
        check(name + ": short punctuation", not any(c in text for c in "\u2013\u2014\u2015"))
        check(name + ": balanced code fences", text.count("```") % 2 == 0)
        check(name + ": no machine-specific paths", not re.search(r"(?i)[A-Z]:[\\/](Users|Code|Microsoft VS Code)[\\/]", text))
        for target in re.findall(r"!?\[[^\]]*\]\(([^)]+)\)", text):
            parts = urlsplit(target)
            if parts.scheme or not parts.path:
                continue
            resolved = (path.parent / unquote(parts.path)).resolve()
            check(name + ": link " + target, resolved.is_relative_to(ROOT) and resolved.is_file())
        if name not in ("README.md", "CHANGELOG.md"):
            check(name + ": bilingual sections", all(" / " in h for h in re.findall(r"^## (.+)$", text, re.M)))

    overview = (ROOT / "README.md").read_text(encoding="utf-8-sig")
    check("README stays compact", len(overview.splitlines()) <= 100 and len(overview.split()) <= 850)
    check("README English introduction first", overview.splitlines()[2].startswith("A Windows"))
    check("README Vietnamese immediately below introduction", overview.splitlines()[4].startswith("Ứng dụng"))
    check("README points to latest release", "releases/latest" in overview and not re.search(r"# QuietShare \d", overview))
    check("protection distinction retained", "does not verify what the audience sees" in overview and "Duplicate" in overview)
    original = subprocess.run(["git", "show", "v0.14.0:CHANGELOG.md"],
                              check=True, capture_output=True).stdout.decode("utf-8-sig")
    previous = set(re.findall(r"^## v?(\d+\.\d+\.\d+)", original, re.M))
    current = set(re.findall(r"^## v?(\d+\.\d+\.\d+)", (ROOT / "CHANGELOG.md").read_text(encoding="utf-8-sig"), re.M))
    check("all historical release versions retained", previous < current and "0.14.1" in current)
    notice = subprocess.run(["git", "show", "v0.14.0:THIRD-PARTY.md"], check=True, capture_output=True).stdout
    check("original legal text preserved", notice.replace(b"\r\n", b"\n") == (ROOT / "THIRD-PARTY.md").read_bytes().replace(b"\r\n", b"\n"))
    if "--dist" in sys.argv:
        for name in DOCS + ["docs/quietshare-home.png"]:
            source = (ROOT / name).read_bytes()
            if name.endswith(".md"):
                source = source.replace(b"\r\n", b"\n")
            check("dist parity: " + name, (ROOT / "dist" / name).read_bytes() == source)
        expected = {Path(name).name for name in DOCS if name.startswith("docs/")} | {"quietshare-home.png"}
        check("portable docs contain only current guides", {p.name for p in (ROOT / "dist/docs").iterdir()} == expected)
    report = {"checks": checks, "passed": sum(c["passed"] for c in checks),
              "failed": sum(not c["passed"] for c in checks),
              "readmeLines": len(overview.splitlines()), "sourceSha256": {
                  name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in DOCS}}
    (ROOT / "artifacts/docs-regression.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps({k: report[k] for k in ("passed", "failed", "readmeLines")}))
    return int(report["failed"] != 0)


if __name__ == "__main__":
    raise SystemExit(main())
