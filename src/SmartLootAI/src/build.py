import pathlib, subprocess, sys
sys.path.insert(0,r'S:\SR\python-tools')
import pefile
root=pathlib.Path(__file__).parent
subprocess.run([sys.executable,str(root/'prepare.py')],check=True)
subprocess.run([r'S:\SR\toolchain\tcc\tcc.exe','-shared','-Wall','-o',str(root/'SmartLootAI.XenoPlugin.dll'),str(root/'smartloot.c'),'-lkernel32'],check=True)
# TinyCC emits stdcall decoration. Host API uses exact undecorated export names.
path=root/'SmartLootAI.XenoPlugin.dll'
data=bytearray(path.read_bytes()); pe=pefile.PE(data=data)
for e in pe.DIRECTORY_ENTRY_EXPORT.symbols:
    if e.name and e.name.startswith(b'_XenoPlugin_') and e.name.endswith(b'@4'):
        name=e.name[1:-2]; offset=e.name_offset
        data[offset:offset+len(e.name)+1]=name+b'\0'*(len(e.name)+1-len(name))
path.write_bytes(data)
pe=pefile.PE(data=data)
assert pe.FILE_HEADER.Machine==0x14c and pe.OPTIONAL_HEADER.Magic==0x10b
assert {b'XenoPlugin_Query',b'XenoPlugin_Initialize',b'SL_Ready',b'SL_Target',b'SL_CountWork',b'SL_Clean',b'SL_Prepare',b'SL_Edge'} <= {e.name for e in pe.DIRECTORY_ENTRY_EXPORT.symbols}
print('Built x86 PE32 DLL with exact Xeno Host API exports.')
