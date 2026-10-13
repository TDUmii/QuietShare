"""Generate harmless local PDFs. Uses only stdlib and a local Windows test font."""
from pathlib import Path
import struct
import hashlib
import sys


def pdf(objects: list[bytes], trailer: bytes = b"") -> bytes:
    result = bytearray(b"%PDF-1.7\n%\xe2\xe3\xcf\xd3\n")
    offsets = [0]
    for n, obj in enumerate(objects, 1):
        offsets.append(len(result))
        result.extend(f"{n} 0 obj\n".encode() + obj + b"\nendobj\n")
    xref = len(result)
    result.extend(f"xref\n0 {len(offsets)}\n0000000000 65535 f \n".encode())
    for offset in offsets[1:]:
        result.extend(f"{offset:010d} 00000 n \n".encode())
    result.extend(f"trailer\n<< /Size {len(offsets)} /Root 1 0 R ".encode() + trailer + f" >>\nstartxref\n{xref}\n%%EOF\n".encode())
    return bytes(result)


def stream(data: bytes, extra: bytes = b"") -> bytes:
    return b"<< /Length " + str(len(data)).encode() + b" " + extra + b" >>\nstream\n" + data + b"\nendstream"


def simple(pages: int, scanned: bool = False) -> bytes:
    kids = b" ".join(f"{4 + n * 2} 0 R".encode() for n in range(pages))
    objects = [b"<< /Type /Catalog /Pages 2 0 R >>", b"<< /Type /Pages /Count " + str(pages).encode() + b" /Kids [" + kids + b"] >>", b"<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>"]
    for n in range(pages):
        # Vector-only page is a representative textless/scanned rendering path.
        content = b"0.2 0.7 0.4 rg 40 200 220 80 re f" if scanned else f"BT /F1 20 Tf 40 240 Td (QuietShare synthetic page {n + 1} test) Tj ET".encode()
        objects += [f"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 300 300] /Resources << /Font << /F1 3 0 R >> >> /Contents {5 + n * 2} 0 R >>".encode(), stream(content)]
    return pdf(objects)


def scanned_pdf() -> bytes:
    pixels = bytes([35, 200, 118, 242, 245, 247, 25, 29, 33, 197, 153, 255] * 4)
    return pdf([
        b"<< /Type /Catalog /Pages 2 0 R >>", b"<< /Type /Pages /Count 1 /Kids [3 0 R] >>",
        b"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 300 300] /Resources << /XObject << /Image 5 0 R >> >> /Contents 4 0 R >>",
        stream(b"q 240 0 0 240 30 30 cm /Image Do Q"),
        stream(pixels, b"/Type /XObject /Subtype /Image /Width 4 /Height 4 /ColorSpace /DeviceRGB /BitsPerComponent 8")
    ])


def password_pdf() -> bytes:
    padding = bytes.fromhex("28bf4e5e4e758a4164004e56fffa01082e2e00b6d0683e802f0ca9fe6453697a")

    def rc4(key: bytes, data: bytes) -> bytes:
        state = list(range(256)); j = 0
        for i in range(256):
            j = (j + state[i] + key[i % len(key)]) & 255
            state[i], state[j] = state[j], state[i]
        i = j = 0; result = bytearray()
        for byte in data:
            i = (i + 1) & 255; j = (j + state[i]) & 255
            state[i], state[j] = state[j], state[i]
            result.append(byte ^ state[(state[i] + state[j]) & 255])
        return bytes(result)

    user = (b"synthetic-test-password" + padding)[:32]
    owner_key = hashlib.md5((b"test-owner" + padding)[:32]).digest()[:5]
    owner = rc4(owner_key, user)
    doc_id = hashlib.md5(b"QuietShare synthetic encryption fixture").digest()
    key = hashlib.md5(user + owner + struct.pack("<i", -4) + doc_id).digest()[:5]
    user_value = rc4(key, padding)
    stream_key = hashlib.md5(key + b"\x05\x00\x00\x00\x00").digest()[:10]
    content = rc4(stream_key, b"BT /F1 20 Tf 40 240 Td (Private test PDF) Tj ET")
    encrypt = b"<< /Filter /Standard /V 1 /R 2 /Length 40 /P -4 /O <" + owner.hex().encode() + b"> /U <" + user_value.hex().encode() + b"> >>"
    return pdf([
        b"<< /Type /Catalog /Pages 2 0 R >>", b"<< /Type /Pages /Count 1 /Kids [4 0 R] >>",
        b"<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>",
        b"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 300 300] /Resources << /Font << /F1 3 0 R >> >> /Contents 5 0 R >>",
        stream(content), encrypt
    ], b"/Encrypt 6 0 R /ID [<" + doc_id.hex().encode() + b"><" + doc_id.hex().encode() + b">]")


def complex_pdf() -> bytes:
    commands = []
    for n in range(60000):
        commands.append(f"0.2 0.7 0.4 rg {n % 300} {(n // 300) % 300} 3 3 re f\n".encode())
    return pdf([
        b"<< /Type /Catalog /Pages 2 0 R >>", b"<< /Type /Pages /Count 1 /Kids [3 0 R] >>",
        b"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 300 300] /Contents 4 0 R >>",
        stream(b"".join(commands))
    ])


def mixed_pdf() -> bytes:
    return pdf([
        b"<< /Type /Catalog /Pages 2 0 R >>", b"<< /Type /Pages /Count 2 /Kids [4 0 R 6 0 R] >>",
        b"<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>",
        b"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 300 300] /Contents 5 0 R >>",
        stream(b"0.2 0.7 0.4 rg 40 200 220 80 re f"),
        b"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 300 300] /Resources << /Font << /F1 3 0 R >> >> /Contents 7 0 R >>",
        stream(b"BT /F1 20 Tf 40 240 Td (Searchable later page) Tj ET")
    ])


def expensive_pdf() -> bytes:
    commands = [b"q /GS gs\n"]
    for n in range(30000):
        commands.append(b"0.2 0.7 0.4 rg 0 0 300 300 re f\n" if n % 2 else b"0.7 0.2 0.4 rg 0 0 300 300 re f\n")
    commands.append(b"Q")
    return pdf([
        b"<< /Type /Catalog /Pages 2 0 R >>", b"<< /Type /Pages /Count 1 /Kids [3 0 R] >>",
        b"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 300 300] /Resources << /ExtGState << /GS 5 0 R >> >> /Contents 4 0 R >>",
        stream(b"".join(commands)), b"<< /Type /ExtGState /ca 0.2 /CA 0.2 >>"
    ])


def unicode_pdf() -> bytes:
    # Embedded font comes from the test machine; this PDF stays in ignored artifacts.
    font = Path("C:/Windows/Fonts/arial.ttf").read_bytes()
    count = struct.unpack_from(">H", font, 4)[0]
    tables = {}
    for i in range(count):
        tag, _, offset, size = struct.unpack_from(">4sIII", font, 12 + 16 * i)
        tables[tag] = (offset, size)
    base = tables[b"cmap"][0]
    num = struct.unpack_from(">H", font, base + 2)[0]
    sub = None
    for i in range(num):
        platform, encoding, rel = struct.unpack_from(">HHI", font, base + 4 + 8 * i)
        pos = base + rel
        if platform == 3 and encoding in (1, 10) and struct.unpack_from(">H", font, pos)[0] == 4:
            sub = pos
            break
    assert sub is not None
    segments = struct.unpack_from(">H", font, sub + 6)[0] // 2
    end_base, start_base = sub + 14, sub + 16 + 2 * segments
    delta_base, range_base = start_base + 2 * segments, start_base + 4 * segments

    def glyph(code: int) -> int:
        for s in range(segments):
            end = struct.unpack_from(">H", font, end_base + 2 * s)[0]
            start = struct.unpack_from(">H", font, start_base + 2 * s)[0]
            if start <= code <= end:
                delta = struct.unpack_from(">h", font, delta_base + 2 * s)[0]
                offset = struct.unpack_from(">H", font, range_base + 2 * s)[0]
                if not offset:
                    return (code + delta) & 65535
                found = struct.unpack_from(">H", font, range_base + 2 * s + offset + 2 * (code - start))[0]
                return (found + delta) & 65535 if found else 0
        return 0

    text = "QuietShare tiếng Việt: lời nhắc"
    chars = list(dict.fromkeys(text))
    ids = {char: i + 1 for i, char in enumerate(chars)}
    mapping = b"\0\0" + b"".join(struct.pack(">H", glyph(ord(c))) for c in chars)
    cmap = b"/CIDInit /ProcSet findresource begin 12 dict begin begincmap\n/CIDSystemInfo << /Registry (Adobe) /Ordering (UCS) /Supplement 0 >> def\n/CMapName /QuietShareTest def /CMapType 2 def\n1 begincodespacerange <0000> <FFFF> endcodespacerange\n"
    cmap += f"{len(chars)} beginbfchar\n".encode()
    cmap += b"".join(f"<{ids[c]:04X}> <{ord(c):04X}>\n".encode() for c in chars) + b"endbfchar\nendcmap CMapName currentdict /CMap defineresource pop end end"
    encoded = "".join(f"{ids[c]:04X}" for c in text)
    return pdf([
        b"<< /Type /Catalog /Pages 2 0 R >>", b"<< /Type /Pages /Count 1 /Kids [3 0 R] >>",
        b"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 600 300] /Resources << /Font << /F1 5 0 R >> >> /Contents 4 0 R >>",
        stream(f"BT /F1 20 Tf 20 200 Td <{encoded}> Tj ET".encode()),
        b"<< /Type /Font /Subtype /Type0 /BaseFont /Arial /Encoding /Identity-H /DescendantFonts [6 0 R] /ToUnicode 9 0 R >>",
        b"<< /Type /Font /Subtype /CIDFontType2 /BaseFont /Arial /CIDSystemInfo << /Registry (Adobe) /Ordering (Identity) /Supplement 0 >> /FontDescriptor 7 0 R /CIDToGIDMap 10 0 R /DW 600 >>",
        b"<< /Type /FontDescriptor /FontName /Arial /Flags 32 /FontBBox [-1000 -1000 3000 3000] /ItalicAngle 0 /Ascent 1000 /Descent -300 /CapHeight 700 /StemV 80 /FontFile2 8 0 R >>",
        stream(font, f"/Length1 {len(font)}".encode()), stream(cmap), stream(mapping)
    ])


out = Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
(out / "one.pdf").write_bytes(simple(1))
(out / "many.pdf").write_bytes(simple(120))
(out / "textless.pdf").write_bytes(simple(1, True))
(out / "scanned.pdf").write_bytes(scanned_pdf())
(out / "password.pdf").write_bytes(password_pdf())
(out / "complex.pdf").write_bytes(complex_pdf())
(out / "mixed.pdf").write_bytes(mixed_pdf())
(out / "expensive.pdf").write_bytes(expensive_pdf())
(out / "unicode.pdf").write_bytes(unicode_pdf())
(out / "invalid.pdf").write_bytes(b"%PDF-1.7\nnot a valid document\n")
