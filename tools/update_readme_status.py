"""Render a local README status SVG from Git and authenticated GitHub data."""
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timedelta, timezone
from html import escape
import json
import re
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
REPO = "TDUmii/QuietShare"
LOCAL = timezone(timedelta(hours=7))


def run(*args):
    return subprocess.run(args, cwd=ROOT, check=True, capture_output=True, encoding="utf-8").stdout


def search(kind, first, last):
    query = f"repo:{REPO} is:{kind} created:{first.astimezone(timezone.utc):%Y-%m-%dT%H:%M:%SZ}..{last.astimezone(timezone.utc):%Y-%m-%dT%H:%M:%SZ}"
    pages = json.loads(run("gh", "api", "--method", "GET", "search/issues", "-f", "q=" + query,
                          "-f", "per_page=100", "--paginate", "--slurp"))
    items = [item for page in pages for item in page["items"]]
    if pages[0].get("incomplete_results") or len(items) != pages[0]["total_count"]:
        raise RuntimeError("Incomplete GitHub search; keeping the previous status image")
    return Counter(datetime.fromisoformat(item["created_at"].replace("Z", "+00:00")).astimezone(LOCAL).date()
                   for item in items)


def render(days, commits, pulls, issues, revision, updated):
    colors = ["#ff5eac", "#b794ff", "#62bbff"]
    rows = [("Commits", "Lượt commit", commits), ("Pull requests opened", "PR được mở", pulls),
            ("Issues opened", "Issue được mở", issues)]
    data = {"repository": REPO, "branch": "main", "revision": revision,
            "updated": updated.isoformat(), "timezone": "UTC+07:00",
            "start": days[0].isoformat(), "end": days[-1].isoformat(),
            "commits": [commits[d] for d in days], "pullRequestsOpened": [pulls[d] for d in days],
            "issuesOpened": [issues[d] for d in days]}
    svg = ['<svg xmlns="http://www.w3.org/2000/svg" width="960" height="400" viewBox="0 0 960 400" role="img" aria-labelledby="title desc">',
           '<title id="title">QuietShare project status / Tình trạng dự án</title>',
           '<desc id="desc">30-day main-branch commit activity and GitHub pull requests/issues opened. Static snapshot, not live statistics. Thống kê 30 ngày, bản chụp tại thời điểm cập nhật.</desc>',
           '<metadata>' + escape(json.dumps(data)) + '</metadata>',
           '<rect width="960" height="400" rx="20" fill="#10151f"/>',
           '<g font-family="Segoe UI,Arial,sans-serif">',
           '<text x="28" y="38" fill="#f4f7ff" font-size="23" font-weight="700">Project Status</text>',
           '<text x="28" y="61" fill="#a9b7ce" font-size="14">Tình trạng dự án · 30 days / 30 ngày</text>',
           '<rect x="748" y="24" width="184" height="30" rx="15" fill="#243a35"/>',
           '<circle cx="767" cy="39" r="4" fill="#82f0b1"/>',
           '<text x="781" y="44" fill="#b6f7d0" font-size="13">Git + GitHub snapshot</text>',
           '<rect x="28" y="80" width="904" height="58" rx="10" fill="#1a2030" stroke="#343d51"/>',
           f'<text x="45" y="105" fill="#ff75b7" font-size="17" font-weight="700">{sum(commits.values())} commits in the last 30 days</text>',
           '<text x="45" y="125" fill="#a9b7ce" font-size="13">Commit trên nhánh main trong 30 ngày</text>']
    maximum = max(commits.values(), default=1) or 1
    for index, day in enumerate(days):
        count = commits[day]
        opacity = .18 + .82 * count / maximum if count else .1
        svg.append(f'<rect x="{470+14*index}" y="101" width="10" height="16" rx="3" fill="#ff5eac" opacity="{opacity:.2f}"><title>{day}: {count} commits</title></rect>')
    for i, (label, vn, counts) in enumerate(rows):
        x = 28 + i * 308
        color = colors[i]
        svg += [f'<rect x="{x}" y="152" width="288" height="192" rx="12" fill="#171e2b" stroke="#343d51"/>',
                f'<text x="{x+17}" y="179" fill="{color}" font-size="16" font-weight="600">{label}</text>',
                f'<text x="{x+17}" y="199" fill="#a9b7ce" font-size="13">{vn}</text>',
                f'<text x="{x+17}" y="243" fill="#f4f7ff" font-size="32" font-weight="700">{sum(counts.values())}</text>']
        top = max(counts.values(), default=1) or 1
        for index, day in enumerate(days):
            height = round(46 * counts[day] / top)
            svg.append(f'<rect x="{x+17+8.4*index:.1f}" y="{307-max(2,height)}" width="6" height="{max(2,height)}" rx="1" fill="{color}" opacity="{1 if height else .22}"><title>{day}: {counts[day]}</title></rect>')
        note = "Daily activity / Hoạt động mỗi ngày" if counts else "No events / Chưa có hoạt động"
        svg.append(f'<text x="{x+17}" y="329" fill="#a9b7ce" font-size="12">{note}</text>')
    period = f'{days[0]:%d %b} - {days[-1]:%d %b %Y}'
    svg += [f'<text x="28" y="371" fill="#a9b7ce" font-size="12">{period} · UTC+07 · main@{escape(revision)}</text>',
            f'<text x="28" y="389" fill="#a9b7ce" font-size="12">Updated / Cập nhật {updated:%d %b %Y, %H:%M} · Snapshot, not live / Không phải dữ liệu trực tiếp</text>',
            '</g></svg>']
    return "\n".join(svg) + "\n"


def main():
    now = datetime.now(LOCAL)
    first = now.date() - timedelta(days=29)
    days = [first + timedelta(days=i) for i in range(30)]
    dates = run("git", "log", "main", "--since=" + first.isoformat() + "T00:00:00+07:00", "--until=" + now.isoformat(), "--format=%cI").splitlines()
    commits = Counter(datetime.fromisoformat(date).astimezone(LOCAL).date() for date in dates)
    commits = Counter({day: count for day, count in commits.items() if day in days})
    revision = run("git", "rev-parse", "--short", "main").strip()
    with ThreadPoolExecutor(max_workers=2) as pool:
        pending = [pool.submit(search, kind, datetime.combine(first, datetime.min.time(), LOCAL), now) for kind in ("pr", "issue")]
        pulls, issues = [job.result() for job in pending]
    target = ROOT / "docs" / "project-status.svg"
    target.write_text(render(days, commits, pulls, issues, revision, now), encoding="utf-8")
    readme = ROOT / "README.md"
    text = readme.read_text(encoding="utf-8")
    alt = f"QuietShare Project Status: {sum(commits.values())} main commits, {sum(pulls.values())} PRs and {sum(issues.values())} issues opened in 30 days / Thống kê 30 ngày"
    text = re.sub(r"!\[QuietShare Project Status:[^\]]*\]", "![" + alt + "]", text)
    text = re.sub(r"30 days: .*?live dashboard\.", f"30 days: **{sum(commits.values())} commits · {sum(pulls.values())} PRs · {sum(issues.values())} issues opened**. Updated **{now:%d %b %Y}**; this is a snapshot, not a live dashboard.", text)
    text = re.sub(r"30 ngày: .*?dữ liệu trực tiếp\.", f"30 ngày: **{sum(commits.values())} commit · {sum(pulls.values())} PR · {sum(issues.values())} issue được mở**. Cập nhật **{now:%d/%m/%Y}**; đây là bản thống kê tại một thời điểm, không phải dữ liệu trực tiếp.", text)
    readme.write_text(text, encoding="utf-8")
    print(f"Updated {target.name}: {sum(commits.values())} commits, {sum(pulls.values())} PRs, {sum(issues.values())} issues")


if __name__ == "__main__":
    main()
