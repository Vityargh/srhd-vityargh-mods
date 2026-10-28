"""Check whether other native plugins reference the game code SmartLootAI hooks or relies on.
Looks for absolute VAs and RVAs (as 32-bit constants) and for the exact prologue bytes we patch."""
import struct, sys
from pathlib import Path

HOOKS = {0x5b7284: 'Kling', 0x5a5880: 'Warrior', 0x59d248: 'Pirate', 0x5addc8: 'Transport', 0x71931c: 'Ranger',
         0x69dbc4: 'Ruins', 0x5b9e14: 'Tranclucator', 0x6d4098: 'DetonateGate', 0x6e72b4: 'BeforeMovement',
         0x7e16e0: 'StarPrepare', 0x7f5698: 'StarDay', 0x6d1b2c: 'FireItem', 0x6e2e30: 'FollowGate',
         0x6e2828: 'HoleGate', 0x6d6e00: 'FuelRange', 0x6d6e6c: 'JumpRange', 0x7535b8: 'TargetGate',
         0x6cd5e0: 'DamageGate', 0x6d2c58: 'HeatGate'}
CALLS = {0x6d6768: 'usable', 0x6d9f3c: 'rangeOf', 0x6cbf48: 'inSpace', 0x6d42a4: 'relation', 0x6f5fb4: 'valueOf',
         0x747300: 'player', 0x6e2560: 'OrderMove', 0x6e2534: 'OrderNone', 0x738034: 'spawn', 0x4161c0: 'listAdd',
         0x6e2968: 'takeoff'}
DATA = {0x81d2a0: 'galaxySlot', 0x81bb2c: 'explosionRadius'}
ALL = {**{k: 'HOOK ' + v for k, v in HOOKS.items()}, **{k: 'call ' + v for k, v in CALLS.items()}, **{k: 'data ' + v for k, v in DATA.items()}}

def scan(path):
    b = Path(path).read_bytes()
    consts = {}
    for i in range(len(b) - 3):
        v = struct.unpack_from('<I', b, i)[0]
        if 0x401000 <= v < 0x900000 or 0x1000 <= v < 0x500000:
            consts.setdefault(v, i)
    hits = []
    for a, name in ALL.items():
        for v, kind in ((a, 'VA'), (a - 0x400000, 'RVA')):
            for d in range(0, 16):
                if v + d in consts:
                    hits.append(f'{name} @0x{a:x} {kind}+{d}')
                    break
    return hits

for p in sys.argv[1:]:
    h = scan(p)
    print(f'== {Path(p).name}: {"NO overlap" if not h else str(len(h)) + " hit(s)"}')
    for x in h: print('   ', x)
