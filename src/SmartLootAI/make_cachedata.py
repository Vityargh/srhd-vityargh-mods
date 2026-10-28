"""Create CFG/CacheData.dat registering the service script path (missing in 2.0 -> TDataEC.PathGetBuf crash)."""
from pathlib import Path
import subprocess, shutil, tempfile, time
r = Path(__file__).parent
bpe = r'S:\SR\toolchain\BlockParEditor\BlockParEditor.exe'
ref = Path(tempfile.gettempdir()) / 'slref' / 'CacheData.txt'  # decoded from Reflection\RefNodesCreating
raw = ref.read_bytes()
utf16 = raw[:2] == b'\xff\xfe'
text = raw.decode('utf-16') if utf16 else raw.decode('cp1251')
text = text.replace('Mod_RefNodesCreating=Mods\\Reflection\\RefNodesCreating\\DATA\\Script\\Mod_RefNodesCreating.scr',
                    'Mod_SmartLootServiceV2=Mods\\OtherMods\\SmartLootAI\\DATA\\Script\\Mod_SmartLootServiceV2.scr')
assert 'Mod_SmartLootServiceV2=' in text and 'RefNodes' not in text
tmp = Path(tempfile.mkdtemp(prefix='sl_cache_'))
(tmp / 'CacheData.txt').write_bytes(raw[:2] + text.lstrip('\ufeff').encode('utf-16-le') if utf16 else text.encode('cp1251'))
src = r / 'src' / 'CacheData.txt'; shutil.copyfile(tmp / 'CacheData.txt', src)
out = r / 'RuntimeMod' / 'CFG' / 'CacheData.dat'
if out.exists(): out.unlink()
subprocess.run([bpe, '--cli', '--convert', str(tmp / 'CacheData.txt'), str(out)], check=True)
for _ in range(40):
    if out.exists() and out.stat().st_size: break
    time.sleep(0.5)
# round-trip
chk = tmp / 'chk'; chk.mkdir(); shutil.copyfile(out, chk / 'CacheData.dat')
subprocess.run([bpe, '--cli', '--convert', str(chk / 'CacheData.dat'), str(chk / 'CacheData.txt')], check=True)
for _ in range(40):
    if (chk / 'CacheData.txt').exists(): break
    time.sleep(0.5)
b = (chk / 'CacheData.txt').read_bytes()
back = b.decode('utf-16') if b[:2] == b'\xff\xfe' else b.decode('cp1251')
print(back)
assert 'Mod_SmartLootServiceV2=Mods\\OtherMods\\SmartLootAI\\DATA\\Script\\Mod_SmartLootServiceV2.scr' in back
print('CacheData.dat created and verified, size', out.stat().st_size)
