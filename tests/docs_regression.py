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
        if name != "README.md":
            check(name + ": bilingual sections", all(" / " in h for h in re.findall(r"^## (.+)$", text, re.M)))

    overview = (ROOT / "README.md").read_text(encoding="utf-8-sig")
    check("README stays compact", len(overview.splitlines()) <= 100 and len(overview.split()) <= 850)
    check("README English introduction first", overview.splitlines()[2].startswith("A Windows"))
    check("README Vietnamese immediately below introduction", overview.splitlines()[4].startswith("Ứng dụng"))
    check("README points to latest release", "releases/latest" in overview and not re.search(r"# QuietShare \d", overview))
    check("protection distinction retained", "does not verify what the audience sees" in overview and "Duplicate" in overview)
    obsolete = list((ROOT / "docs").glob("*Implementation_Plan.md")) + list((ROOT / "handoff").glob("*"))
    check("obsolete planning and handoff files removed", not obsolete)
    guides = {Path(name).name for name in DOCS if name.startswith("docs/")} | {"quietshare-home.png", "readme-banner.svg", "project-status.svg"}
    check("source docs contain only current guides", {p.name for p in (ROOT / "docs").iterdir()} == guides)
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
        for name in DOCS + ["docs/quietshare-home.png", "docs/readme-banner.svg", "docs/project-status.svg"]:
            source = (ROOT / name).read_bytes()
            if name.endswith((".md", ".svg")):
                source = source.replace(b"\r\n", b"\n")
            check("dist parity: " + name, (ROOT / "dist" / name).read_bytes() == source)
        expected = {Path(name).name for name in DOCS if name.startswith("docs/")} | {"quietshare-home.png", "readme-banner.svg", "project-status.svg"}
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
