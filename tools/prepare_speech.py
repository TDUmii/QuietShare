"""Build the optional CPU speech add-on. Models are never bundled or downloaded here.

Windows: python tools/prepare_speech.py --compiler-root "C:/path/to/mingw64/bin"
Requires MinGW-w64 GCC 13.1.0, CMake and Ninja. Downloads pinned upstream sources.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import urllib.request
import zipfile

ROOT = Path(__file__).resolve().parents[1]
COMMIT = 'f049fff95a089aa9969deb009cdd4892b3e74916'
SOURCE_HASH = '58347c4dc92142c47b6d6e5a2a7e1b00b501c9bf8cdaff7f089e49ae59eb3a44'
EXE_HASH = '96a4d1b1342e33d39ebf6cd38997330cc7ed869e81e4f1e215858f407132dd93'
README = '''# QuietShare optional speech runtime

whisper.cpp v1.9.1, source commit f049fff95a089aa9969deb009cdd4892b3e74916.
CPU only. QuietShare uses two background threads and supports cancellation.
The executable has only Windows system DLL imports. No model or audio is included.
Models download separately from the official ggerganov/whisper.cpp repository.
Audio stays on this computer. Review recognition results before using them.
See the QuietShare user manuals for installation, model selection and troubleshooting.

## Tiếng Việt

Bộ nhận dạng tùy chọn chạy trên CPU. QuietShare dùng hai luồng nền và cho phép dừng xử lý.
Gói này không chứa mô hình hoặc âm thanh. Mô hình được tải riêng khi người dùng chọn cài.
Âm thanh được xử lý trên máy. Hãy kiểm tra lại văn bản nhận dạng trước khi sử dụng.
Xem sách hướng dẫn QuietShare để biết cách cài, chọn mô hình và xử lý lỗi.
'''


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def fetch(url, path, expected):
    if not path.exists() or digest(path) != expected:
        with urllib.request.urlopen(url, timeout=120) as response, path.open('wb') as output:
            shutil.copyfileobj(response, output)
    if digest(path) != expected:
        raise ValueError(f'Pinned digest mismatch: {path.name}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler-root', required=True, type=Path)
    parser.add_argument('--reuse-executable', action='store_true', help='Verify and package an already built executable')
    args = parser.parse_args()
    work = ROOT / 'artifacts/speech-build'
    work.mkdir(parents=True, exist_ok=True)
    source = work / f'whisper.cpp-{COMMIT}'
    exe = work / 'build/bin/whisper-cli.exe'
    if not args.reuse_executable:
        fetch(f'https://github.com/ggml-org/whisper.cpp/archive/{COMMIT}.zip', work / 'source.zip', SOURCE_HASH)
        if not source.exists():
            with zipfile.ZipFile(work / 'source.zip') as archive:
                archive.extractall(work)
        env = os.environ.copy()
        env['PATH'] = str(args.compiler_root) + os.pathsep + env['PATH']
        env['GIT_CEILING_DIRECTORIES'] = str(work)
        flags = ['-DCMAKE_BUILD_TYPE=Release', '-DBUILD_SHARED_LIBS=OFF', '-DWHISPER_BUILD_TESTS=OFF',
                 '-DGGML_OPENMP=OFF', '-DGGML_NATIVE=OFF', '-DGGML_AVX=OFF', '-DGGML_AVX2=OFF',
                 '-DGGML_FMA=OFF', '-DGGML_F16C=OFF', '-DGGML_AVX512=OFF', '-DGGML_SSE42=OFF',
                 '-DGGML_BMI2=OFF', '-DCMAKE_EXE_LINKER_FLAGS=-static -static-libgcc -static-libstdc++',
                 '-DCMAKE_C_FLAGS=-D_WIN32_WINNT=0x0601', '-DCMAKE_CXX_FLAGS=-D_WIN32_WINNT=0x0601']
        subprocess.run([str(args.compiler_root / 'cmake.exe'), '-S', str(source), '-B', str(work / 'build'), '-G', 'Ninja', *flags], env=env, check=True)
        subprocess.run([str(args.compiler_root / 'cmake.exe'), '--build', str(work / 'build'), '--target', 'whisper-cli', '--parallel', '2'], env=env, check=True)
        subprocess.run([str(args.compiler_root / 'strip.exe'), str(exe)], check=True)
    if digest(exe) != EXE_HASH:
        raise ValueError('Executable differs from the reviewed release. Review build metadata and toolchain differences before updating release pins.')
    addon = ROOT / 'artifacts/speech-addon'
    addon.mkdir(exist_ok=True)
    shutil.copyfile(exe, addon / 'whisper-cli.exe')
    license_hash = '94f29bbed6a22c35b992c5c6ebf0e7c92f13b836b90f36f461c9cf2f0f1d010d'
    for name in ('LICENSE-whisper.cpp.txt', 'LICENSE-ggml.txt'):
        fetch(f'https://raw.githubusercontent.com/ggml-org/whisper.cpp/{COMMIT}/LICENSE', addon / name, license_hash)
    fetch('https://raw.githubusercontent.com/openai/whisper/v20250625/LICENSE', addon / 'LICENSE-model.txt',
          'b5d65a59060e68c4ff940e1eddfa6f94b2d68fdf58ed7f4dd57721c997e35e9d')
    (addon / 'README.md').write_bytes(README.encode('utf-8'))
    archive_path = ROOT / 'artifacts/QuietShare-Speech-CPU-x64.zip'
    names = ['LICENSE-ggml.txt', 'LICENSE-model.txt', 'LICENSE-whisper.cpp.txt', 'README.md', 'whisper-cli.exe']
    with zipfile.ZipFile(archive_path, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name in sorted(names):
            info = zipfile.ZipInfo(name, (2020, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o100644 << 16
            archive.writestr(info, (addon / name).read_bytes(), compresslevel=9)
    metadata = {'sourceCommit': COMMIT, 'sourceSha256': SOURCE_HASH, 'exeSha256': EXE_HASH,
                'archiveSha256': digest(archive_path), 'archiveBytes': archive_path.stat().st_size, 'files': names}
    (work / 'manifest.json').write_bytes((json.dumps(metadata, indent=2) + '\n').encode())
    print(json.dumps(metadata, indent=2))


if __name__ == '__main__':
    main()
