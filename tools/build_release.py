"""Build player-ready archives for Vityargh mods.

Outputs (S:\\SR\\release\\out):
  Vityargh-SmartLootAI-2.0.1-ApplyCrashFix-1.0.zip  - unpack into the game folder; includes XenoNativeLoader
  Vityargh-BureauAffordableDesign-1.0.zip           - plain mod, no loader needed
  ApplyCrashFix-1.0-submission.zip                  - for Xenomorphchyma (mod + source + test + write-ups)
Paths inside archives use forward slashes; every archive is re-read and compared with its sources.
"""
from pathlib import Path
import hashlib, json, zipfile, time

SR = Path(r'S:\SR')
OUT = SR / 'release' / 'out'
LOADER = SR / 'vendor' / 'XenoNativeLoader-0.7.2'
# The config players run with: no loader-side patches, no auto restart (tested setup).
LOADER_INI = '[Core]\r\nEnabled=1\r\nApplyPatches=0\r\nAutoRestartNativeMods=0\r\n'
LOADER_SHA = {'dsound.dll': 'f09e417efc0931241cda290c85843896b7e26d31f8898aa629232d3e43bc1ea5',
              'XenoCore.dll': '029a5c294b9a5a368d74e1c0b712fc7f98b38778f4793173d0a82debff6e0bfd',
              'LICENSE': '1c62c2f95964a009ea7710042749d8b4fba5199328bfc26ebe03ef986d4c65ca'}
SKIP = {'Lang.txt'}  # editor sources, not needed by the game

sha = lambda b: hashlib.sha256(b).hexdigest()

def tree(src: Path, prefix: str):
    for p in sorted(src.rglob('*')):
        if p.is_file() and p.name not in SKIP:
            yield prefix + '/' + p.relative_to(src).as_posix(), p.read_bytes()

def text(s: str) -> bytes:  # UTF-8 with BOM + CRLF so Windows Notepad shows Cyrillic correctly
    return b'\xef\xbb\xbf' + s.replace('\r\n', '\n').replace('\n', '\r\n').encode('utf-8')

def write(name, entries):
    path = OUT / name
    seen = set()
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for arc, data in entries:
            assert arc not in seen, arc; seen.add(arc)
            info = zipfile.ZipInfo(arc, date_time=(2026, 10, 28, 12, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED; info.external_attr = 0o644 << 16
            z.writestr(info, data)
    with zipfile.ZipFile(path) as z:  # verify
        assert z.testzip() is None
        back = {i.filename: z.read(i) for i in z.infolist()}
    want = dict(entries)
    assert back.keys() == want.keys() and all(back[k] == want[k] for k in want), name
    return dict(archive=name, size=path.stat().st_size, sha256=sha(path.read_bytes()),
                files={k: dict(size=len(v), sha256=sha(v)) for k, v in sorted(want.items())})

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for n, h in LOADER_SHA.items():
        assert sha((LOADER / n).read_bytes()) == h, f'loader file changed: {n}'
    sl = SR / 'SmartLootAI-v2' / 'RuntimeMod'
    acf = SR / 'ApplyCrashFix' / 'RuntimeMod'
    bad = SR / 'BureauAffordableDesign'
    assert b'2.0.1' in (sl / 'ModuleInfo.txt').read_bytes()
    bundle = [('dsound.dll', (LOADER / 'dsound.dll').read_bytes()),
              ('XenoCore.dll', (LOADER / 'XenoCore.dll').read_bytes()),
              ('XenoNative.ini', LOADER_INI.encode('ascii')),
              ('XenoNativeLoader-LICENSE.txt', (LOADER / 'LICENSE').read_bytes()),
              ('ПРОЧТИ.txt', text((SR / 'release' / 'PROCHTI.txt').read_text(encoding='utf-8')))]
    bundle += list(tree(sl, 'Mods/OtherMods/SmartLootAI'))
    bundle += list(tree(acf, 'Mods/OtherMods/ApplyCrashFix'))
    sub = SR / 'ApplyCrashFix' / 'submission'
    submission = list(tree(acf, 'ApplyCrashFix-1.0/ApplyCrashFix'))
    submission += [('ApplyCrashFix-1.0/README_EN.md', (sub / 'README_EN.md').read_bytes()),
                   ('ApplyCrashFix-1.0/README_RU.md', (sub / 'README_RU.md').read_bytes()),
                   ('ApplyCrashFix-1.0/source/applyfix.c', (SR / 'ApplyCrashFix/src/applyfix.c').read_bytes()),
                   ('ApplyCrashFix-1.0/source/build.py', (SR / 'ApplyCrashFix/build.py').read_bytes()),
                   ('ApplyCrashFix-1.0/source/tests/pin_test.c', (SR / 'ApplyCrashFix/tests/pin_test.c').read_bytes())]
    report = [write('Vityargh-SmartLootAI-2.0.1-ApplyCrashFix-1.0.zip', bundle),
              write('Vityargh-BureauAffordableDesign-1.0.zip', list(tree(bad, 'Mods/OtherMods/BureauAffordableDesign'))),
              write('ApplyCrashFix-1.0-submission.zip', submission)]
    (OUT / 'release-manifest.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    for r in report:
        print(f"{r['archive']}: {r['size']} bytes, {len(r['files'])} files, verified")

if __name__ == '__main__':
    main()
