"""Map SmartLootAI's game sites from the reference (Redux 2026-10-06) build to another build by routine
name, using the two pakompom/SpaceRangersHD_decomp trees (each annotates routines with their address)."""
import re, sys, pathlib
def routines(src):
    out = {}
    for f in pathlib.Path(src).rglob('*.pas'):
        for m in re.finditer(r'\{ @routine \$([0-9A-F]+) (\w+) \}', f.read_text(encoding='utf-8', errors='replace')):
            out[m[2]] = int(m[1], 16)
    return out
REF = routines(r'S:\SR\SpaceRangersHD_decomp\source')
OTHER = routines(r'S:\SR\decomp-steam\source')
ref_sorted = sorted((a, n) for n, a in REF.items())
def owner(va):
    best = None
    for a, n in ref_sorted:
        if a <= va: best = (a, n)
        else: break
    return best
SITES = [('Kling',0x5b7284),('Warrior',0x5a5880),('Pirate',0x59d248),('Transport',0x5addc8),('Ranger',0x71931c),
         ('Ruins',0x69dbc4),('Tranclucator',0x5b9e14),('DetonateGate',0x6d4098),('FireItem',0x6d1b2c),('FollowServiceGate',0x6e2e30),
         ('HoleServiceGate',0x6e2828),('ServiceFuelRange',0x6d6e00),('ServiceJumpRange',0x6d6e6c),
         ('ServiceTargetGate',0x7535b8),('ServiceDamageGate',0x6cd5e0),('ServiceHeatGate',0x6d2c58),
         ('usable',0x6d6768),('rangeOf',0x6d9f3c),('inSpace',0x6cbf48),('relation',0x6d42a4),('valueOf',0x6f5fb4),
         ('player',0x747300),('OrderMove',0x6e2560),('OrderNone',0x6e2534),('spawn',0x738034),('listAdd',0x4161c0),
         ('takeoff',0x6e2968)]
if __name__ == '__main__':
    for name, va in SITES:
        a, rname = owner(va)
        other = OTHER.get(rname)
        print(f'{name:18} ref {va:#x} = {rname}+{va-a:#x}  ->  steam {other + (va - a):#x}' if other is not None else f'{name:18} ref {va:#x} = {rname}+{va-a:#x}  ->  steam MISSING')
