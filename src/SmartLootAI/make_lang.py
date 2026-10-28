"""Build RuntimeMod/CFG/<Lang>/Lang.dat (Rus, Eng) from src/CrewLang*.txt with a round-trip check."""
from pathlib import Path
import subprocess, shutil, tempfile, time
r = Path(__file__).parent
bpe = r'S:\SR\toolchain\BlockParEditor\BlockParEditor.exe'
SOURCES = {'Rus': r / 'src' / 'CrewLang.txt', 'Eng': r / 'src' / 'CrewLang_Eng.txt'}

def conv(a, b):
    if b.exists(): b.unlink()
    subprocess.run([bpe, '--cli', '--convert', str(a), str(b)], check=True)
    for _ in range(60):
        if b.exists() and b.stat().st_size: time.sleep(0.3); return
        time.sleep(0.5)
    raise SystemExit('no output ' + str(b))

def dec(b):
    return b.decode('utf-16') if b[:2] == b'\xff\xfe' else b.decode('cp1251')

norm = lambda s: sorted(l.strip() for l in s.lstrip('\ufeff').splitlines() if l.strip())
keys = lambda s: sorted(l.split('=', 1)[0] for l in norm(s) if '=' in l)
built = {}
for lang, src in SOURCES.items():
    tmp = Path(tempfile.mkdtemp(prefix='sl_lang_'))
    shutil.copyfile(src, tmp / 'Lang.txt'); conv(tmp / 'Lang.txt', tmp / 'Lang.dat')
    (tmp / 'c').mkdir(); shutil.copyfile(tmp / 'Lang.dat', tmp / 'c' / 'Lang.dat'); conv(tmp / 'c' / 'Lang.dat', tmp / 'c' / 'Lang.txt')
    want, back = dec(src.read_bytes()), dec((tmp / 'c' / 'Lang.txt').read_bytes())
    assert norm(back) == norm(want), f'{lang}: round-trip mismatch'
    assert '\u00ab' not in back and '\u2014' not in back, f'{lang}: unsupported glyphs'
    built[lang] = (tmp / 'Lang.dat', keys(want))
assert built['Rus'][1] == built['Eng'][1], 'Rus/Eng key sets differ'
for lang, (dat, k) in built.items():
    out = r / 'RuntimeMod' / 'CFG' / lang / 'Lang.dat'
    out.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(dat, out)
    print(f'{lang}: Lang.dat built and verified, {out.stat().st_size} bytes, {len(k)} keys')
