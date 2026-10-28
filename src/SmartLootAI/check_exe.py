"""Check whether SmartLootAI can run on a given Rangers.exe, without starting the game.
Usage: python check_exe.py <path to Rangers.exe> [...]
Applies the same masked signatures the plugin uses at startup (src/signatures.h), layout by layout."""
import re, sys, pathlib
sys.path.insert(0, r'S:\SR\python-tools')
import pefile
H = (pathlib.Path(__file__).parent / 'src' / 'signatures.h').read_text()
arr = {m[1] + '_' + m[2]: bytes(int(x, 0) for x in m[3].split(',')) for m in re.finditer(r'static const unsigned char (sig|mask)_(\w+)\[\]=\{([^}]*)\}', H)}
layouts = []
for m in re.finditer(r'static const Site sites_(\w+)\[\] = \{(.*?)\n\};', H, re.S):
    v = m[1]
    sites = [(s[1], int(s[2]), int(s[3])) for s in re.finditer(r'\{L"(\w+)",(\d+),(\d+),\d+,0x[0-9a-f]+u,sig_\w+,mask_\w+\}', m[2])]
    heads = re.search(r'static const PatchHead heads_%s\[\] = \{(.*?)\n\};' % v, H, re.S)[1]
    heads = [(h[2], bytes(int(x, 0) for x in h[1].split(','))) for h in re.finditer(r'\{\d+,\{([^}]*)\}\}, /\* (\w+) \*/', heads)]
    label = re.search(r'\{L"([^"]+)",sites_%s,heads_%s\}' % (v, v), H)[1]
    layouts.append((v, label, sites, heads))
def scan(code, v, name):
    sig, m = arr[f'sig_{v}_{name}'], arr[f'mask_{v}_{name}']
    first = next(i for i, x in enumerate(m) if x); hits = []; pos = 0
    while len(hits) < 2:
        pos = code.find(sig[first:first + 1], pos)
        if pos < 0: break
        s = pos - first
        if s >= 0 and s + len(sig) <= len(code) and all(not m[k] or code[s + k] == sig[k] for k in range(len(sig))): hits.append(s)
        pos += 1
    return hits
def check(path):
    pe = pefile.PE(path, fast_load=True)
    sec = next(s for s in pe.sections if s.Characteristics & 0x20)
    code = sec.get_data()[:sec.Misc_VirtualSize]
    report = []
    for v, label, sites, heads in layouts:
        found, missing = {}, []
        for name, kind, off in sites:
            hits = scan(code, v, name)
            if len(hits) == 1: found[name] = hits[0] + off
            else: missing.append(f'{name} ({"not found" if not hits else "several matches"})')
        for name, pro in heads:
            if name in found and code[found[name]:found[name] + len(pro)] != pro: missing.append(f'{name} (hook head differs)')
        if not missing:
            print(f'{path}: SUPPORTED, layout {label}'); return True
        report.append(f'  {label}: {len(found)}/{len(sites)} sites; first missing: {", ".join(missing[:4])}')
    print(f'{path}: NOT SUPPORTED'); print('\n'.join(report)); return False
if __name__ == '__main__':
    sys.exit(0 if all([check(p) for p in sys.argv[1:]]) else 1)
