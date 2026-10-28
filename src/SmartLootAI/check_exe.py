"""Check whether SmartLootAI can run on a given Rangers.exe, without starting the game.
Usage: python check_exe.py <path to Rangers.exe> [...]
Applies the same masked signatures the plugin uses at startup (src/signatures.h) to the file's code section."""
import re, sys, pathlib
sys.path.insert(0, r'S:\SR\python-tools')
import pefile
H = (pathlib.Path(__file__).parent / 'src' / 'signatures.h').read_text()
arrays = {m[1] + '_' + m[2]: bytes(int(x, 0) for x in m[3].split(',')) for m in re.finditer(r'static const unsigned char (sig|mask)_(\w+)\[\]=\{([^}]*)\}', H)}
sites = [(m[1], int(m[2]), int(m[3])) for m in re.finditer(r'\{L"(\w+)",(\d+),(\d+),\d+,0x[0-9a-f]+u,sig_\w+,mask_\w+\}', H)]
patches = [(m[2], bytes(int(x, 0) for x in m[1].split(','))) for m in re.finditer(r'\{0,\d+,\{([^}]*)\}\}, /\* (\w+) \*/', H)]
def check(path):
    pe = pefile.PE(path, fast_load=True)
    sec = next(s for s in pe.sections if s.Characteristics & 0x20)
    code = sec.get_data()[:sec.Misc_VirtualSize]; va0 = pe.OPTIONAL_HEADER.ImageBase + sec.VirtualAddress
    ok = True; found = {}
    for name, kind, off in sites:
        sig, m = arrays['sig_' + name], arrays['mask_' + name]
        first = next(i for i, x in enumerate(m) if x); hits = []; pos = 0
        while len(hits) < 2:
            pos = code.find(sig[first:first + 1], pos)
            if pos < 0: break
            s = pos - first
            if s >= 0 and s + len(sig) <= len(code) and all(not m[k] or code[s + k] == sig[k] for k in range(len(sig))): hits.append(s)
            pos += 1
        if len(hits) == 1: found[name] = hits[0] + off
        else: ok = False; print(f'  {name}: {"not found" if not hits else "several matches"}')
    for name, pro in patches:
        if name in found and code[found[name]:found[name] + len(pro)] != pro: ok = False; print(f'  {name}: hook prologue differs')
    print(f'{path}: {"SUPPORTED" if ok else "NOT SUPPORTED"} ({len(found)}/{len(sites)} sites)')
    return ok
if __name__ == '__main__':
    sys.exit(0 if all([check(p) for p in sys.argv[1:]]) else 1)
