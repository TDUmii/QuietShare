"""Validate the current bilingual documentation and optional shipping copy."""
from pathlib import Path
from collections import Counter
from datetime import datetime, timedelta
import hashlib
import json
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
DOCS = ["README.md", "docs/README.md", "docs/USER_GUIDE.md",
        "docs/PRIVACY.md", "docs/TROUBLESHOOTING.md", "docs/BUILDING.md", "docs/DEPENDENCIES.md", "docs/BOOK.md",
        "docs/USER_MANUAL_EN.md", "docs/USER_MANUAL_VI.md",
        "docs/VISUAL_GUIDE_EN.md", "docs/VISUAL_GUIDE_VI.md"]
MANUALS = {"docs/USER_MANUAL_EN.md", "docs/USER_MANUAL_VI.md"}
VISUAL_GUIDES = {"docs/VISUAL_GUIDE_EN.md", "docs/VISUAL_GUIDE_VI.md"}
VISUAL_IMAGES = ["docs/images/visual/" + name for name in [
    "01-home.png", "02-files.png", "03-script.png", "04-pdf.png",
    "05-settings.png", "06-browser.png", "07-projection.png", "08-recording.png"]]
PDFS = ["docs/QuietShare-User-Manual-EN.pdf", "docs/QuietShare-Huong-Dan-Su-Dung-VI.pdf"]


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    checks = []
    def check(name, value):
        checks.append({"name": name, "passed": bool(value)})
        print(("PASS " if value else "FAIL ") + name)

    for name in DOCS:
        path = ROOT / name
        text = path.read_text(encoding="utf-8-sig")
        if name != "README.md" and name not in MANUALS | VISUAL_GUIDES:
            check(name + ": bilingual title", " / " in text.splitlines()[0])
        if not name.endswith(("USER_MANUAL_EN.md", "VISUAL_GUIDE_EN.md")):
            check(name + ": Vietnamese present", bool(re.search(r"[ăâđêôơưĂÂĐÊÔƠƯ]", text)))
        prose = re.sub(r"```.*?```", "", text, flags=re.S)
        check(name + ": ideas use lines instead of semicolons", ";" not in prose)
        check(name + ": short punctuation", not any(c in text for c in "\u2013\u2014\u2015"))
        check(name + ": balanced code fences", text.count("```") % 2 == 0)
        check(name + ": no machine-specific paths", not re.search(r"(?i)[A-Z]:[\\/](Users|Code|Microsoft VS Code)[\\/]", text))
        for target in re.findall(r"!?\[[^\]]*\]\(([^)]+)\)", text):
            parts = urlsplit(target)
            if parts.scheme or not parts.path:
                continue
            resolved = (path.parent / unquote(parts.path)).resolve()
            check(name + ": link " + target, resolved.is_relative_to(ROOT) and resolved.is_file())
        if name != "README.md" and name not in MANUALS | VISUAL_GUIDES:
            check(name + ": bilingual sections", all(" / " in h for h in re.findall(r"^## (.+)$", text, re.M)))

    current_version = re.search(r'QUIETSHARE_VERSION_TEXT "([^"]+)"', (ROOT/"src/Version.h").read_text()).group(1)
    mirror = (ROOT / "src/Mirror.cpp").read_text(encoding="utf-8-sig")
    workspace_source = (ROOT / "src/main.cpp").read_text(encoding="utf-8-sig")
    for number in range(1,11):
        code=f"QS-W{number:03d}"
        check(code+": UI and handbook agree",code in workspace_source and code in (ROOT/"docs/BOOK.md").read_text(encoding="utf-8-sig"))
    for number in [*range(1,11),99]:
        code=f'QS-B{number:03d}'
        check(code+': browser recovery documented in both editions',all(code in (ROOT/p).read_text(encoding='utf-8-sig') for p in ['docs/BOOK.md',*MANUALS]))
    notebook_codes=[f'QS-D{n:03d}' for n in range(1,4)]+[f'QS-N{n:03d}' for n in range(1,6)]+['QS-R008','QS-R009']
    runtime_source=(ROOT/'src/MeetingTranscriber.cpp').read_text(encoding='utf-8-sig')
    for code in notebook_codes:
        check(code+': implementation and all books agree',code in workspace_source+runtime_source and all(code in (ROOT/p).read_text(encoding='utf-8-sig') for p in ['docs/BOOK.md',*MANUALS]))
    books = {name: (ROOT / name).read_text(encoding="utf-8-sig") for name in sorted(MANUALS)}
    mapping = re.findall(r'if\(std::strcmp\(reason,"([a-z-]+)"\)==0\)return "(QS-P\d{3})";', mirror)
    check("projection error codes are unique", len(mapping) >= 24 and len({code for _, code in mapping}) == len(mapping))
    for name, book in books.items():
        check(name + ": current edition", "v" + current_version in book)
        for reason, code in mapping:
            heading = re.search(r"^###.*" + code + r".*" + reason + r".*$", book, re.M)
            check(name + ": documents " + code + " " + reason, heading is not None)
        check(name + ": diagnostic phases", all("`" + phase + "`" in book for phase in ["start", "verification", "running"]))
        check(name + ": complete chapters", len(re.findall(r"^## ", book, re.M)) >= 14)
    emitted = set(re.findall(r'(?:code=|Fail\(State::\w+,|fail\(State::\w+,|return )"([a-z]+(?:-[a-z]+)+)"', mirror))
    check("every emitted projection reason has a handbook code", emitted <= {reason for reason, _ in mapping})
    for source, pdf in zip(sorted(MANUALS), PDFS):
        payload = (ROOT / pdf).read_bytes()
        check(pdf + ": current release", ("QuietShare v" + current_version).encode() in payload)
        check(pdf + ": PDF signature", payload.startswith(b"%PDF-") and b"%%EOF" in payload[-1024:])
        check(pdf + ": current Markdown source", hashlib.sha256((ROOT/source).read_bytes()).hexdigest().encode() in payload)

    overview = (ROOT / "README.md").read_text(encoding="utf-8-sig")
    check("README stays compact", len(overview.splitlines()) <= 100 and len(overview.split()) <= 850)
    paragraphs = [part for part in overview.split("\n\n") if part.strip()]
    english_intro = next((i for i, part in enumerate(paragraphs) if part.startswith("A Windows")), -1)
    check("README English introduction first", english_intro >= 0)
    check("README Vietnamese immediately below introduction", english_intro >= 0 and english_intro + 1 < len(paragraphs)
          and paragraphs[english_intro + 1].startswith("Ứng dụng"))
    check("README points to latest release", "releases/latest" in overview and not re.search(r"# QuietShare \d", overview))
    check("protection distinction retained", "does not verify what the audience sees" in overview and "Duplicate" in overview)
    obsolete = list((ROOT / "docs").glob("*Implementation_Plan.md")) + list((ROOT / "handoff").glob("*"))
    check("obsolete planning and handoff files removed", not obsolete)
    guides = {Path(name).name for name in DOCS if name.startswith("docs/")} | {"quietshare-home.png", "readme-banner.svg", "project-status.svg"} | {Path(name).name for name in PDFS}
    guides |= {str(Path(name).relative_to("docs")).replace("\\", "/") for name in VISUAL_IMAGES}
    check("source docs contain only current guides", {p.relative_to(ROOT / "docs").as_posix()
          for p in (ROOT / "docs").rglob("*") if p.is_file()} == guides)
    for name in VISUAL_GUIDES:
        guide = (ROOT / name).read_text(encoding="utf-8-sig")
        check(name + ": current screenshot edition", "QuietShare " + current_version in guide)
        check(name + ": all eight annotated screenshots", all(Path(image).name in guide for image in VISUAL_IMAGES))
    for name in VISUAL_IMAGES:
        check(name + ": PNG signature", (ROOT / name).read_bytes().startswith(b"\x89PNG\r\n\x1a\n"))
    check("English commit guidance retained", "English commit messages" in (ROOT / "docs/BUILDING.md").read_text(encoding="utf-8"))
    for filename in ["readme-banner.svg", "project-status.svg"]:
        graphic = ET.parse(ROOT / "docs" / filename).getroot()
        check(filename + ": standalone accessible SVG", graphic.find("{http://www.w3.org/2000/svg}title") is not None
              and graphic.find("{http://www.w3.org/2000/svg}desc") is not None)
        check(filename + ": no scripts or external resources", all(
            node.tag.rsplit("}", 1)[-1] not in {"script", "foreignObject", "image"}
            and not any(key.rsplit("}", 1)[-1] == "href" for key in node.attrib) for node in graphic.iter()))
    status = ET.parse(ROOT / "docs/project-status.svg").getroot()
    snapshot = json.loads(status.find("{http://www.w3.org/2000/svg}metadata").text)
    check("status has 30 daily points per metric", all(len(snapshot[key]) == 30 and all(type(n) is int and n >= 0 for n in snapshot[key])
          for key in ["commits", "pullRequestsOpened", "issuesOpened"]))
    total = sum(snapshot["commits"])
    check("README status matches SVG snapshot", f"{total} main commits" in overview
          and f'{sum(snapshot["pullRequestsOpened"])} PRs' in overview
          and f'{sum(snapshot["issuesOpened"])} issues' in overview)
    check("status date and source are explicit", snapshot["repository"] == "TDUmii/QuietShare"
          and snapshot["branch"] == "main" and snapshot["timezone"] == "UTC+07:00" and snapshot["revision"])
    updated = datetime.fromisoformat(snapshot["updated"])
    start = datetime.fromisoformat(snapshot["start"]).date()
    days = [start + timedelta(days=i) for i in range(30)]
    dates = subprocess.run(["git", "log", snapshot["revision"], "--since=" + snapshot["start"] + "T00:00:00+07:00",
                            "--until=" + snapshot["updated"], "--format=%cI"], check=True, capture_output=True, text=True).stdout.splitlines()
    actual = Counter(datetime.fromisoformat(date).astimezone(updated.tzinfo).date() for date in dates)
    check("commit chart matches its recorded Git revision", [actual[day] for day in days] == snapshot["commits"])
    notice = subprocess.run(["git", "show", "v0.14.0:THIRD-PARTY.md"], check=True, capture_output=True).stdout
    check("original legal text preserved", notice.replace(b"\r\n", b"\n") == (ROOT / "THIRD-PARTY.md").read_bytes().replace(b"\r\n", b"\n"))
    if "--dist" in sys.argv:
        for name in DOCS + PDFS + VISUAL_IMAGES + ["docs/quietshare-home.png", "docs/readme-banner.svg", "docs/project-status.svg"]:
            source = (ROOT / name).read_bytes()
            if name.endswith((".md", ".svg")):
                source = source.replace(b"\r\n", b"\n")
            check("dist parity: " + name, (ROOT / "dist" / name).read_bytes() == source)
        check("portable docs contain only current guides", {p.relative_to(ROOT / "dist/docs").as_posix()
              for p in (ROOT / "dist/docs").rglob("*") if p.is_file()} == guides)
    report = {"checks": checks, "passed": sum(c["passed"] for c in checks),
              "failed": sum(not c["passed"] for c in checks),
              "readmeLines": len(overview.splitlines()), "sourceSha256": {
                  name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in DOCS}}
    (ROOT / "artifacts/docs-regression.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps({k: report[k] for k in ("passed", "failed", "readmeLines")}))
    return int(report["failed"] != 0)


if __name__ == "__main__":
    raise SystemExit(main())
