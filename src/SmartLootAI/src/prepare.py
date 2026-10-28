"""Generate signatures.h: masked byte signatures for every game code site SmartLootAI hooks, calls or reads.

The plugin finds these sites at startup by scanning the game's code section, so it is not tied to one
Rangers.exe build. Masked bytes: absolute addresses (relocation entries) and rel32 operands of call/jmp/jcc,
which change whenever code moves. Everything else - instructions, stack frames, struct field offsets inside
the instructions - must match exactly, so a build whose code or object layout differs at these sites is
refused (the plugin then stays off) instead of being guessed at.
The reference build below is only the build the signatures were cut from.
"""
import hashlib, json, pathlib, sys
sys.path.insert(0, r'S:\SR\python-tools')
import capstone, pefile
ROOT = pathlib.Path(__file__).parent
EXE = pathlib.Path(r'C:\Program Files (x86)\Steam\steamapps\common\Space Rangers HD A War Apart\Rangers.exe')
REFERENCE_SHA = 'e43a4b15503c705eebe2d8ce5778c2a2fb090db515a3fd86d319e5c5aa692ebf'
raw = EXE.read_bytes()
assert hashlib.sha256(raw).hexdigest() == REFERENCE_SHA, 'signatures are cut from the reference build'
pe = pefile.PE(data=raw)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32); md.detail = True
base = pe.OPTIONAL_HEADER.ImageBase
text = next(s for s in pe.sections if s.Name.rstrip(b'\0') == b'.text')
text_va = base + text.VirtualAddress
code = text.get_data()[:text.Misc_VirtualSize]
relocs = set()
for block in pe.DIRECTORY_ENTRY_BASERELOC:
    for e in block.entries:
        if e.type == 3:
            for k in range(4): relocs.add(base + e.rva + k)

HOOKS = [('Kling',0x5b7284),('Warrior',0x5a5880),('Pirate',0x59d248),('Transport',0x5addc8),('Ranger',0x71931c),
         ('Ruins',0x69dbc4),('Tranclucator',0x5b9e14),('DetonateGate',0x6d4098),('FireItem',0x6d1b2c),('FollowServiceGate',0x6e2e30),
         ('HoleServiceGate',0x6e2828),('ServiceFuelRange',0x6d6e00),('ServiceJumpRange',0x6d6e6c),
         ('ServiceTargetGate',0x7535b8),('ServiceDamageGate',0x6cd5e0),('ServiceHeatGate',0x6d2c58)]
CALLS = [('usable',0x6d6768),('rangeOf',0x6d9f3c),('inSpace',0x6cbf48),('relation',0x6d42a4),('valueOf',0x6f5fb4),
         ('player',0x747300),('OrderMove',0x6e2560),('OrderNone',0x6e2534),('spawn',0x738034),('listAdd',0x4161c0),
         ('takeoff',0x6e2968)]
DATA = [('galaxySlot',0x81d2a0),('explosionRadius',0x81bb2c)]
MAXLEN = 1024  # Pirate and Transport AI share their first 534 bytes
MINLEN = {'OrderMove': 96, 'OrderNone': 48}  # cover the register ABI the bridges rely on

def mask_for(start_va, length):
    """1 = byte must match. Decodes from start_va (an instruction boundary)."""
    m = [1] * length
    for va in range(start_va, start_va + length):
        if va in relocs: m[va - start_va] = 0
    off = start_va - text_va
    for ins in md.disasm(code[off:off + length + 16], start_va):
        if ins.address >= start_va + length: break
        rel32 = (ins.group(capstone.CS_GRP_CALL) or ins.group(capstone.CS_GRP_JUMP)) and ins.size >= 5 and \
            ins.operands and ins.operands[0].type == capstone.x86.X86_OP_IMM
        if rel32:
            for k in range(ins.size - 4, ins.size):
                p = ins.address - start_va + k
                if p < length: m[p] = 0
    return m

def matches(sig, m):
    first = next(i for i, x in enumerate(m) if x)
    n, pos = 0, 0
    while True:
        pos = code.find(bytes([sig[first]]), pos)
        if pos < 0: return n
        s = pos - first
        if s >= 0 and s + len(sig) <= len(code) and all(not m[k] or code[s + k] == sig[k] for k in range(len(sig))):
            n += 1
            if n > 1: return n
        pos += 1

def cut(name, start_va, kind, offset):
    for length in list(range(MINLEN.get(name, 48), 256, 8)) + list(range(256, MAXLEN + 1, 32)):
        sig = code[start_va - text_va:start_va - text_va + length]
        m = mask_for(start_va, length)
        if sum(m) >= 32 and matches(sig, m) == 1:
            return dict(name=name, start=start_va, kind=kind, offset=offset, sig=sig, mask=m)
    raise SystemExit(f'no unique signature for {name}')

sites = []
patches = []
for name, va in HOOKS:
    blob = code[va - text_va:va - text_va + 32]; size = 0
    for i in md.disasm(blob, va):
        assert not i.group(capstone.CS_GRP_JUMP) and not i.group(capstone.CS_GRP_CALL)
        size += i.size
        if size >= 5: break
    assert not any(va + k in relocs for k in range(size)), f'{name}: relocated byte in prologue'
    patches.append((name, size, blob[:size]))
    sites.append(cut(name, va, 0, 0))
for name, va in CALLS:
    sites.append(cut(name, va, 0, 0))
for name, addr in DATA:
    needle = addr.to_bytes(4, 'little'); pos = 0; site = None
    while site is None:
        pos = code.find(needle, pos)
        assert pos >= 0, f'no code reference to {name}'
        opva = text_va + pos
        if opva in relocs:
            for back in range(1, 8):  # find the instruction that owns this operand
                ins = next(md.disasm(code[pos - back:pos - back + 16], opva - back), None)
                if ins and ins.size >= back + 4:
                    try: site = cut(f'{name}', opva - back, 1, back)
                    except SystemExit: site = None
                    break
        pos += 4
    sites.append(site)

# The plugin must resolve exactly these addresses on the reference build.
REF = {s['name']: s['start'] + s['offset'] if s['kind'] == 0 else addr for s, addr in
       [(s, dict(DATA).get(s['name'])) for s in sites]}

h = ['/* Generated by prepare.py from the reference build %s. Do not edit. */' % REFERENCE_SHA[:16]]
h.append('static Patch patches[] = {')
for name, size, b in patches:
    h.append('{0,%d,{%s}}, /* %s */' % (size, ','.join(hex(x) for x in b), name))
h.append('};\n#define PATCH_COUNT (sizeof(patches)/sizeof(patches[0]))')
h.append('typedef struct { const wchar_t *name; unsigned char kind,offset; unsigned short length; unsigned reference; const unsigned char *b,*m; } Site;')
for s in sites:
    h.append('static const unsigned char sig_%s[]={%s};' % (s['name'], ','.join(hex(x) for x in s['sig'])))
    h.append('static const unsigned char mask_%s[]={%s};' % (s['name'], ','.join(str(x) for x in s['mask'])))
h.append('static const Site sites[] = {')
for s in sites:
    h.append('{L"%s",%d,%d,%d,0x%xu,sig_%s,mask_%s},' % (s['name'], s['kind'], s['offset'], len(s['sig']), REF[s['name']], s['name'], s['name']))
h.append('};\n#define SITE_COUNT (sizeof(sites)/sizeof(sites[0]))')
names = [s['name'] for s in sites]
h.append('enum { ' + ', '.join('S_' + n for n in names) + ' };')
(ROOT / 'signatures.h').write_text('\n'.join(h) + '\n')
(ROOT / 'binary-evidence.json').write_text(json.dumps(dict(reference_sha256=REFERENCE_SHA, sites=[dict(
    name=s['name'], start=hex(s['start']), kind=s['kind'], length=len(s['sig']), fixed_bytes=sum(s['mask'])) for s in sites]), indent=2))

abi = {}
for name, addr, length in [('OrderNone', 0x6e2534, 41), ('OrderMove', 0x6e2560, 94)]:
    blob = code[addr - text_va:addr - text_va + length]
    abi[name] = [f'{i.mnemonic} {i.op_str}' for i in md.disasm(blob, addr)]
assert 'mov esi, edx' in abi['OrderMove'] and 'mov ebx, ecx' in abi['OrderMove'] and 'test dl, dl' in abi['OrderNone']
print(f'Reference build verified; {len(patches)} hook prologues; {len(sites)} unique masked signatures '
      f'({min(sum(s["mask"]) for s in sites)}-{max(sum(s["mask"]) for s in sites)} fixed bytes each).')
