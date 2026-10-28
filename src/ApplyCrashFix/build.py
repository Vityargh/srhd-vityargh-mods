import pathlib, subprocess, sys, shutil
sys.path.insert(0, r'S:\SR\python-tools')
import pefile
r = pathlib.Path(__file__).parent
tcc = r'S:\SR\toolchain\tcc\tcc.exe'
dll = r / 'src' / 'ApplyCrashFix.XenoPlugin.dll'
subprocess.run([tcc, '-shared', '-Wall', '-o', str(dll), str(r / 'src/applyfix.c'), '-lkernel32'], check=True)
# TinyCC decorates stdcall exports; Host API needs exact undecorated names.
data = bytearray(dll.read_bytes()); pe = pefile.PE(data=data)
for e in pe.DIRECTORY_ENTRY_EXPORT.symbols:
    if e.name and e.name.startswith(b'_XenoPlugin_') and b'@' in e.name:
        name = e.name[1:e.name.index(b'@')]
        data[e.name_offset:e.name_offset + len(e.name) + 1] = name + b'\0' * (len(e.name) + 1 - len(name))
dll.write_bytes(bytes(data)); pe = pefile.PE(data=bytes(data))
names = {e.name for e in pe.DIRECTORY_ENTRY_EXPORT.symbols}
assert {b'XenoPlugin_Query', b'XenoPlugin_Initialize'} <= names, names
assert pe.FILE_HEADER.Machine == 0x14c
shutil.copyfile(dll, r / 'RuntimeMod/Native/ApplyCrashFix.XenoPlugin.dll')
(r / 'RuntimeMod/Native/ApplyCrashFix.XenoPlugin.ini').write_text('[Plugin]\nEnabled=1\nDll=ApplyCrashFix.XenoPlugin.dll\nLegacy=0\n', encoding='ascii')
subprocess.run([tcc, '-o', str(r / 'tests/pin_test.exe'), str(r / 'tests/pin_test.c'), '-lkernel32'], check=True)
print('built', sorted(n.decode() for n in names))
