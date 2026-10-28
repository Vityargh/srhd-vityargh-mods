"""Generate signatures.h: masked byte signatures for every game code site SmartLootAI hooks, calls or reads,
one set per supported Rangers.exe code layout.

At startup the plugin scans the game's code section with each set in turn and uses the first set whose
every site is found exactly once. Masked bytes: absolute addresses (relocation entries) and rel32 operands of
call/jmp/jcc, which change whenever code moves. Everything else - instructions, stack frames, struct field
offsets inside the instructions - must match exactly, so code that differs at these sites is refused (the
plugin then stays off) instead of being guessed at.

Layouts:
  redux   - Space Rangers HD 2.1.2500 built 2026-10-06 (Universe/Redux), optimized ($O+) code
  steam   - Space Rangers HD 2.1.2500 built 2025-10-13 (Steam/GOG release), unoptimized ($O-) code
Site addresses per layout come from pakompom/SpaceRangersHD_decomp (main and build-2025-10-13 branches):
the same routine names, so the same game functions (TList.Add 0x4161c0 is RTL code, identical in both). Both layouts have identical class sizes
(TGalaxy 0x1DC, TStar 0x114, TShip 0x4D0, TKling 0x4DC).
"""
import hashlib, json, pathlib, sys
sys.path.insert(0, r'S:\SR\python-tools')
import capstone, pefile
ROOT = pathlib.Path(__file__).parent
REFDIR = pathlib.Path(r'S:\SR\reference')
NAMES_HOOK = ['Kling', 'Warrior', 'Pirate', 'Transport', 'Ranger', 'Ruins', 'Tranclucator', 'DetonateGate', 'FireItem',
              'FollowServiceGate', 'HoleServiceGate', 'ServiceFuelRange', 'ServiceJumpRange', 'ServiceTargetGate',
              'ServiceDamageGate', 'ServiceHeatGate']
NAMES_CALL = ['usable', 'rangeOf', 'inSpace', 'relation', 'valueOf', 'player', 'OrderMove', 'OrderNone', 'spawn',
              'listAdd', 'takeoff']
NAMES_DATA = ['galaxySlot', 'explosionRadius']
BUILDS = [
    dict(id='redux', label='2.1.2500 2026-10-06 (Universe/Redux)', exe='Rangers-redux-20261006.exe',
         sha='e43a4b15503c705eebe2d8ce5778c2a2fb090db515a3fd86d319e5c5aa692ebf',
         hooks=[0x5b7284, 0x5a5880, 0x59d248, 0x5addc8, 0x71931c, 0x69dbc4, 0x5b9e14, 0x6d4098, 0x6d1b2c, 0x6e2e30,
                0x6e2828, 0x6d6e00, 0x6d6e6c, 0x7535b8, 0x6cd5e0, 0x6d2c58],
         calls=[0x6d6768, 0x6d9f3c, 0x6cbf48, 0x6d42a4, 0x6f5fb4, 0x747300, 0x6e2560, 0x6e2534, 0x738034, 0x4161c0, 0x6e2968],
         data=[0x81d2a0, 0x81bb2c]),
    dict(id='steam', label='2.1.2500 2025-10-13 (Steam/GOG)', exe='Rangers-steam.exe',
         sha='83300344af802bc51e64389c58f047e5afdf195c133048098be3881fae29ed98',
         hooks=[0x5cc88c, 0x5b9168, 0x5afd24, 0x5c2570, 0x759760, 0x6ce5ac, 0x5cfc84, 0x70bbb0, 0x7090b0, 0x71ce50,
                0x71c75c, 0x70f190, 0x70f230, 0x799f0c, 0x704490, 0x70a428],
         calls=[0x70e934, 0x712a24, 0x7029a4, 0x70be0c, 0x731f74, 0x78c078, 0x71c43c, 0x71c3f0, 0x77b8b8, 0x4161c0, 0x71c8fc],
         data=[0x88c288, 0x889f2c]),
]
MAXLEN = 1024  # Pirate and Transport AI share their first ~534 bytes
MINLEN = {'OrderMove': 96, 'OrderNone': 48}  # cover the register ABI the bridges rely on
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32); md.detail = True

class Image:
    def __init__(self, path, sha):
        raw = path.read_bytes()
        assert hashlib.sha256(raw).hexdigest() == sha, f'{path.name}: unexpected build'
        pe = pefile.PE(data=raw)
        self.base = pe.OPTIONAL_HEADER.ImageBase
        text = next(s for s in pe.sections if s.Characteristics & 0x20)
        self.text_va = self.base + text.VirtualAddress
        self.code = text.get_data()[:text.Misc_VirtualSize]
        self.relocs = set()
        for block in pe.DIRECTORY_ENTRY_BASERELOC:
            for e in block.entries:
                if e.type == 3:
                    for k in range(4): self.relocs.add(self.base + e.rva + k)
        self.pe = pe
    def at(self, va, n): return self.code[va - self.text_va:va - self.text_va + n]
    def mask_for(self, start, length):
        m = [1] * length
        for va in range(start, start + length):
            if va in self.relocs: m[va - start] = 0
        for ins in md.disasm(self.at(start, length + 16), start):
            if ins.address >= start + length: break
            if (ins.group(capstone.CS_GRP_CALL) or ins.group(capstone.CS_GRP_JUMP)) and ins.size >= 5 and \
               ins.operands and ins.operands[0].type == capstone.x86.X86_OP_IMM:
                for k in range(ins.size - 4, ins.size):
                    p = ins.address - start + k
                    if p < length: m[p] = 0
        return m
    def count(self, sig, m, limit=2):
        code = self.code; first = next(i for i, x in enumerate(m) if x); n = pos = 0
        while True:
            pos = code.find(bytes([sig[first]]), pos)
            if pos < 0: return n
            s = pos - first
            if s >= 0 and s + len(sig) <= len(code) and all(not m[k] or code[s + k] == sig[k] for k in range(len(sig))):
                n += 1
                if n >= limit: return n
            pos += 1
    def cut(self, name, start, kind, offset):
        for length in list(range(MINLEN.get(name, 48), 256, 8)) + list(range(256, MAXLEN + 1, 32)):
            sig = self.at(start, length); m = self.mask_for(start, length)
            if sum(m) >= 32 and self.count(sig, m) == 1:
                return dict(name=name, start=start, kind=kind, offset=offset, sig=sig, mask=m)
        raise SystemExit(f'no unique signature for {name}')
    def data_site(self, name, addr):
        needle = addr.to_bytes(4, 'little'); pos = 0
        while True:
            pos = self.code.find(needle, pos)
            assert pos >= 0, f'no code reference to {name}'
            opva = self.text_va + pos
            if opva in self.relocs:
                for back in range(1, 8):  # the instruction that owns this operand
                    ins = next(md.disasm(self.code[pos - back:pos - back + 16], opva - back), None)
                    if ins and ins.size >= back + 4:
                        try: return self.cut(name, opva - back, 1, back)
                        except SystemExit: break
            pos += 4

out = []
for b in BUILDS:
    img = Image(REFDIR / b['exe'], b['sha'])
    patches, sites = [], []
    for name, va in zip(NAMES_HOOK, b['hooks']):
        blob = img.at(va, 32); size = 0
        for i in md.disasm(blob, va):
            assert not i.group(capstone.CS_GRP_JUMP) and not i.group(capstone.CS_GRP_CALL), f'{b["id"]} {name}: branch in prologue'
            size += i.size
            if size >= 5: break
        assert not any(va + k in img.relocs for k in range(size)), f'{b["id"]} {name}: relocated byte in prologue'
        patches.append((name, size, blob[:size])); sites.append(img.cut(name, va, 0, 0))
    for name, va in zip(NAMES_CALL, b['calls']):
        sites.append(img.cut(name, va, 0, 0))
    for name, addr in zip(NAMES_DATA, b['data']):
        sites.append(img.data_site(name, addr))
    abi = {n: ' | '.join(f'{i.mnemonic} {i.op_str}' for i in md.disasm(img.at(va, 64), va))
           for n, va in (('OrderNone', b['calls'][7]), ('OrderMove', b['calls'][6]))}
    assert 'dl' in abi['OrderNone'], f'{b["id"]}: OrderNone does not read DL'
    assert 'edx' in abi['OrderMove'] and ('ecx' in abi['OrderMove'] or ', cl' in abi['OrderMove']), f'{b["id"]}: OrderMove ABI'
    for s, addr in zip(sites[-2:], b['data']): s['reference'] = addr
    for s in sites[:-2]: s['reference'] = s['start'] + s['offset']
    out.append(dict(build=b, patches=patches, sites=sites, img=img))

# A layout's signatures must not resolve completely in another layout (otherwise the order would decide).
for a in out:
    for o in out:
        if o is a: continue
        found = sum(o['img'].count(s['sig'], s['mask'], 2) == 1 for s in a['sites'])
        assert found < len(a['sites']), f"{a['build']['id']} signatures fully match {o['build']['id']}"

h = ['/* Generated by prepare.py. Do not edit. One signature set per supported Rangers.exe code layout. */']
h.append('typedef struct { const wchar_t *name; unsigned char kind,offset; unsigned short length; unsigned reference; const unsigned char *b,*m; } Site;')
h.append('typedef struct { unsigned char len; unsigned char b[16]; } PatchHead;')
names = [s['name'] for s in out[0]['sites']]
for x in out:
    v = x['build']['id']
    for s in x['sites']:
        h.append('static const unsigned char sig_%s_%s[]={%s};' % (v, s['name'], ','.join(hex(c) for c in s['sig'])))
        h.append('static const unsigned char mask_%s_%s[]={%s};' % (v, s['name'], ','.join(str(c) for c in s['mask'])))
    h.append('static const Site sites_%s[] = {' % v)
    for s in x['sites']:
        h.append('{L"%s",%d,%d,%d,0x%xu,sig_%s_%s,mask_%s_%s},' % (s['name'], s['kind'], s['offset'], len(s['sig']), s['reference'], v, s['name'], v, s['name']))
    h.append('};')
    h.append('static const PatchHead heads_%s[] = {' % v)
    for name, size, blob in x['patches']:
        h.append('{%d,{%s}}, /* %s */' % (size, ','.join(hex(c) for c in blob), name))
    h.append('};')
h.append('typedef struct { const wchar_t *label; const Site *sites; const PatchHead *heads; } Layout;')
h.append('static const Layout layouts[] = {')
for x in out:
    h.append('{L"%s",sites_%s,heads_%s},' % (x['build']['label'], x['build']['id'], x['build']['id']))
h.append('};')
h.append('#define LAYOUT_COUNT (sizeof(layouts)/sizeof(layouts[0]))')
h.append('#define SITE_COUNT %d' % len(names))
h.append('#define PATCH_COUNT %d' % len(NAMES_HOOK))
h.append('static Patch patches[PATCH_COUNT];')
h.append('enum { ' + ', '.join('S_' + n for n in names) + ' };')
(ROOT / 'signatures.h').write_text('\n'.join(h) + '\n')
(ROOT / 'binary-evidence.json').write_text(json.dumps([dict(build=x['build']['label'], sha256=x['build']['sha'], sites=[dict(
    name=s['name'], start=hex(s['start']), kind=s['kind'], length=len(s['sig']), fixed_bytes=sum(s['mask'])) for s in x['sites']])
    for x in out], indent=2))
print('Layouts: ' + '; '.join(f"{x['build']['id']} {len(x['patches'])} hooks / {len(x['sites'])} signatures" for x in out)
      + '; no layout matches another.')
