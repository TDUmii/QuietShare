"""Build the two user manuals from their tracked Markdown sources.

Requires reportlab and the Arial family installed with Windows, or --font-dir.
No network requests or bundled copies of system font files are needed.
"""
from pathlib import Path
from functools import partial
import argparse
import hashlib
import re
from xml.sax.saxutils import escape

from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfgen.canvas import Canvas
from reportlab.platypus import (BaseDocTemplate, CondPageBreak, Frame, Image, KeepTogether,
                               PageBreak, PageTemplate, Paragraph, Spacer, Table, TableStyle)
from reportlab.platypus.tableofcontents import TableOfContents

ROOT = Path(__file__).resolve().parents[1]
MANUALS = {
    "EN": ("USER_MANUAL_EN.md", "QuietShare-User-Manual-EN.pdf", "User manual", "Complete guide", "Contents"),
    "VI": ("USER_MANUAL_VI.md", "QuietShare-Huong-Dan-Su-Dung-VI.pdf", "Hướng dẫn sử dụng", "Hướng dẫn đầy đủ", "Mục lục"),
}
GREEN = colors.HexColor("#18634b")
INK = colors.HexColor("#172c29")
MUTED = colors.HexColor("#526864")
LIGHT = colors.HexColor("#e9f2ee")


def inline(text):
    text = escape(text)
    text = re.sub(r"!\[([^]]*)\]\([^)]*\)", r"\1", text)
    text = re.sub(r"\[([^]]+)\]\((https?://[^)]+)\)",
                  r'<link href="\2" color="#18634b">\1</link>', text)
    text = re.sub(r"\[([^]]+)\]\(([^)]+)\)", r"\1", text)
    text = re.sub(r"`([^`]+)`", r'<font name="ManualMono" size="9">\1</font>', text)
    text = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", text)
    return text.replace("&lt;br&gt;", "<br/>")


class ManualDoc(BaseDocTemplate):
    def __init__(self, path, language, version, source_hash, styles):
        super().__init__(str(path), pagesize=A4, leftMargin=22*mm, rightMargin=22*mm,
                         topMargin=24*mm, bottomMargin=22*mm,
                         title="QuietShare - " + MANUALS[language][2], author="QuietShare",
                         subject=f"QuietShare v{version} | source-sha256={source_hash}", invariant=1)
        self.language, self.version, self.styles = language, version, styles
        self.addPageTemplates(PageTemplate("manual", Frame(self.leftMargin, self.bottomMargin,
            self.width, self.height, leftPadding=0, rightPadding=0, topPadding=0, bottomPadding=0),
            onPage=self.page_chrome))

    def page_chrome(self, canvas, doc):
        canvas.saveState()
        width, height = A4
        if doc.page == 1:
            canvas.setFillColor(GREEN)
            canvas.rect(0, height-12*mm, width, 12*mm, fill=1, stroke=0)
            canvas.setFillColor(LIGHT)
            canvas.rect(width-17*mm, 0, 17*mm, height-12*mm, fill=1, stroke=0)
        else:
            canvas.setStrokeColor(colors.HexColor("#cbdcd4"))
            canvas.line(22*mm, height-17*mm, width-22*mm, height-17*mm)
            canvas.setFont("ManualBold", 8)
            canvas.setFillColor(GREEN)
            canvas.drawString(22*mm, height-13*mm, "QUIETSHARE")
            canvas.setFont("Manual", 8)
            canvas.setFillColor(MUTED)
            canvas.drawRightString(width-22*mm, height-13*mm, MANUALS[self.language][2])
        canvas.setFont("Manual", 8)
        canvas.setFillColor(MUTED)
        canvas.drawString(22*mm, 13*mm, f"v{self.version}  |  {self.language}")
        canvas.drawRightString(width-22*mm, 13*mm, str(doc.page))
        canvas.restoreState()

    def afterFlowable(self, flowable):
        if isinstance(flowable, Paragraph) and flowable.style.name in {"Chapter", "Section"}:
            key = getattr(flowable, "bookmark", None)
            if key:
                title = flowable.getPlainText()
                level = 0 if flowable.style.name == "Chapter" else 1
                self.canv.bookmarkPage(key)
                self.canv.addOutlineEntry(title, key, level, False)
                if level == 0:
                    self.notify("TOCEntry", (0, title, self.page, key))


def make_styles():
    return {
        "body": ParagraphStyle("Body", fontName="Manual", fontSize=10.5, leading=15,
                               textColor=INK, spaceAfter=7, splitLongWords=True),
        "chapter": ParagraphStyle("Chapter", fontName="ManualBold", fontSize=22, leading=28,
                                  textColor=GREEN, spaceAfter=17, keepWithNext=True),
        "section": ParagraphStyle("Section", fontName="ManualBold", fontSize=12, leading=17,
                                  textColor=INK, spaceBefore=12, spaceAfter=7, keepWithNext=True),
        "small": ParagraphStyle("Small", fontName="Manual", fontSize=8.5, leading=12,
                                textColor=MUTED, spaceAfter=6),
        "bullet": ParagraphStyle("Bullet", fontName="Manual", fontSize=10.5, leading=15,
                                 textColor=INK, leftIndent=14, firstLineIndent=0, bulletIndent=0,
                                 spaceAfter=6),
        "toc": ParagraphStyle("ContentsEntry", fontName="Manual", fontSize=11, leading=18,
                              textColor=INK, spaceBefore=7, rightIndent=22),
        "cover": ParagraphStyle("Cover", fontName="ManualBold", fontSize=32, leading=40,
                                textColor=GREEN, spaceAfter=22),
        "code": ParagraphStyle("Code", fontName="ManualMono", fontSize=8.5, leading=12,
                               textColor=INK, backColor=LIGHT, borderPadding=8,
                               spaceBefore=4, spaceAfter=10),
    }


def build(language, output, styles, version):
    source_name, output_name, title, subtitle, contents_title = MANUALS[language]
    source = ROOT / "docs" / source_name
    raw = source.read_bytes()
    text = raw.decode("utf-8-sig")
    destination = output / output_name
    doc = ManualDoc(destination, language, version, hashlib.sha256(raw).hexdigest(), styles)
    story = [Spacer(1, 35*mm), Paragraph("QuietShare", styles["cover"]),
             Paragraph(title, ParagraphStyle("Subtitle", parent=styles["cover"], fontSize=25, leading=33)),
             Paragraph(subtitle, styles["body"]), Spacer(1, 10*mm),
             Paragraph(f"Version {version}" if language == "EN" else f"Phiên bản {version}", styles["body"]),
             Paragraph("English edition" if language == "EN" else "Bản tiếng Việt", styles["small"]),
             Spacer(1, 22*mm),
             Paragraph("Files, scripts, web browsing, presentation and error recovery."
                       if language == "EN" else "File, lời nhắc, trình duyệt, trình chiếu và xử lý lỗi.", styles["body"]),
             PageBreak(), Paragraph(contents_title, styles["chapter"])]
    toc = TableOfContents()
    toc.levelStyles = [styles["toc"]]
    toc.dotsMinLevel = 0
    story.extend([toc])
    paragraphs = []
    headings = 0
    chapters = 0
    code = None
    table = []
    started = False

    def flush():
        if paragraphs:
            story.append(Paragraph(inline(" ".join(paragraphs)), styles["body"]))
            paragraphs.clear()

    def flush_table():
        if not table:
            return
        rows = [[Paragraph(inline(cell), styles["small"]) for cell in row] for row in table]
        widget = Table(rows, colWidths=[doc.width/len(rows[0])]*len(rows[0]), repeatRows=1,
                       hAlign="LEFT")
        widget.setStyle(TableStyle([("BACKGROUND", (0,0), (-1,0), LIGHT),
            ("VALIGN", (0,0), (-1,-1), "TOP"), ("LEFTPADDING", (0,0), (-1,-1), 7),
            ("RIGHTPADDING", (0,0), (-1,-1), 7), ("TOPPADDING", (0,0), (-1,-1), 6),
            ("BOTTOMPADDING", (0,0), (-1,-1), 6),
            ("LINEBELOW", (0,0), (-1,-1), .4, colors.HexColor("#dbe5e0"))]))
        story.extend([widget, Spacer(1, 7)])
        table.clear()

    for raw_line in text.splitlines():
        line = raw_line.strip()
        if not started:
            if not line.startswith("## "):
                continue
            started = True
        if line.startswith("```"):
            flush(); flush_table()
            if code is None:
                code = []
            else:
                story.append(Paragraph("<br/>".join(escape(value) or " " for value in code), styles["code"]))
                code = None
            continue
        if code is not None:
            code.append(raw_line)
            continue
        if line.startswith("# "):
            continue
        if line.startswith("|"):
            flush()
            cells = [cell.strip() for cell in line.strip("|").split("|")]
            if not all(re.fullmatch(r":?-+:?", cell) for cell in cells):
                table.append(cells)
            continue
        flush_table()
        if not line or line == "---":
            flush()
            continue
        if line.startswith("## ") or line.startswith("### "):
            flush()
            chapter = line.startswith("## ")
            if chapter:
                chapters += 1
                story.append(PageBreak() if chapters == 1 else CondPageBreak(210))
                if chapters != 1:
                    story.append(Spacer(1, 14))
            heading = Paragraph(inline(line[3:] if chapter else line[4:]),
                                styles["chapter" if chapter else "section"])
            heading.bookmark = f"heading-{headings}"
            headings += 1
            story.append(heading)
            continue
        image = re.fullmatch(r"!\[([^]]*)\]\(([^)]+)\)", line)
        if image:
            flush()
            path = (source.parent/image[2]).resolve()
            if not path.is_relative_to(ROOT):
                raise ValueError("Image outside repository")
            picture = Image(str(path))
            ratio = min(doc.width/picture.imageWidth, 85*mm/picture.imageHeight)
            picture.drawWidth, picture.drawHeight = picture.imageWidth*ratio, picture.imageHeight*ratio
            story.extend([picture, Spacer(1, 6), Paragraph(inline(image[1]), styles["small"])])
            continue
        item = re.match(r"^(?:([-*])|(\d+)\.)\s+(.+)$", line)
        if item:
            flush()
            indent = len(raw_line)-len(raw_line.lstrip())
            style = styles["bullet"]
            if indent:
                style = ParagraphStyle("NestedBullet", parent=style, leftIndent=28, bulletIndent=14)
            story.append(Paragraph(inline(item[3]), style, bulletText=(item[2]+"." if item[2] else "•")))
        else:
            paragraphs.append(line)
    flush(); flush_table()
    if code is not None:
        raise ValueError("Unclosed code block")
    # Keep each short error recipe together so its actions do not start on
    # another page without the matching code and symptom.
    grouped, recipe = [], []
    for flowable in story:
        heading = isinstance(flowable, Paragraph) and flowable.style.name in {"Chapter", "Section"}
        if recipe and (heading or isinstance(flowable, PageBreak)):
            grouped.append(KeepTogether(recipe)); recipe = []
        if heading and flowable.getPlainText().startswith("QS-P"):
            recipe = [flowable]
        elif recipe:
            recipe.append(flowable)
        else:
            grouped.append(flowable)
    if recipe:
        grouped.append(KeepTogether(recipe))
    doc.multiBuild(grouped, canvasmaker=partial(Canvas, invariant=1))
    print(destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--language", choices=MANUALS, help="Build one edition instead of both")
    parser.add_argument("--font-dir", type=Path, default=Path("C:/Windows/Fonts"))
    parser.add_argument("--output-dir", type=Path, default=ROOT/"docs")
    args = parser.parse_args()
    for name, filename in [("Manual", "arial.ttf"), ("ManualBold", "arialbd.ttf"),
                           ("ManualItalic", "ariali.ttf"), ("ManualMono", "consola.ttf")]:
        pdfmetrics.registerFont(TTFont(name, str(args.font_dir/filename)))
    pdfmetrics.registerFontFamily("Manual", normal="Manual", bold="ManualBold",
                                 italic="ManualItalic", boldItalic="ManualBold")
    version = re.search(r'QUIETSHARE_VERSION_TEXT "([^"]+)"',
                        (ROOT/"src/Version.h").read_text()).group(1)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    styles = make_styles()
    for language in ([args.language] if args.language else MANUALS):
        build(language, args.output_dir, styles, version)


if __name__ == "__main__":
    main()
