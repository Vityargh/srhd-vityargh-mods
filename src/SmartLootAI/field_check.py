"""Compare the struct-field displacements used inside each mapped routine in two builds.
Same source + same class sizes: the multiset of [reg+disp] displacements should match when layouts match."""
import sys, re, collections
sys.path.insert(0, r'S:\SR\python-tools')
import pefile, capstone
from map_build import SITES, owner, OTHER
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32); md.detail = True
def load(p):
    pe = pefile.PE(p, fast_load=True); return pe, pe.get_memory_mapped_image(), pe.OPTIONAL_HEADER.ImageBase
A = load(r'S:\SR\reference\Rangers-redux-20261006.exe'); B = load(r'S:\SR\reference\Rangers-steam.exe')
def disps(img, base, va, n=1500):
    c = collections.Counter()
    for ins in md.disasm(img[va - base:va - base + n], va):
        for op in ins.operands:
            if op.type == capstone.x86.X86_OP_MEM and op.mem.base not in (0, capstone.x86.X86_REG_EBP, capstone.x86.X86_REG_ESP) and 0 < op.mem.disp < 0x2000 and op.mem.index == 0:
                c[op.mem.disp] += 1
        if ins.mnemonic == 'ret': break
    return c
bad = 0
for name, va in SITES:
    o = owner(va)
    if not o or o[1] not in OTHER: continue
    a = disps(A[1], A[2], va); b = disps(B[1], B[2], OTHER[o[1]])
    sa, sb = set(a), set(b)
    only_a, only_b = sorted(sa - sb), sorted(sb - sa)
    flag = 'OK ' if not only_a and not only_b else 'DIFF'
    if flag == 'DIFF': bad += 1
    print(f'{flag} {name:18} fields={len(sa):3}  only_redux={[hex(x) for x in only_a][:8]}  only_steam={[hex(x) for x in only_b][:8]}')
print('routines with differing field sets:', bad)
