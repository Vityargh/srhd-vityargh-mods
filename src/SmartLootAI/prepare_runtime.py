from pathlib import Path
import sys,shutil
r=Path(__file__).parent
sys.path.insert(0,r'C:\Program Files (x86)\Steam\steamapps\common\Space Rangers HD A War Apart')
from srhd_modkit.blockpar import parse_blockpar
runtime=r/'RuntimeMod'
api={'SL_Ready':[], 'SL_CountWork':['dword'],'SL_Target':['dword'],'SL_Edge':['dword'],'SL_Prepare':['dword'],'SL_Clean':['dword','int'],'SL_Move':['dword']}
main=parse_blockpar('Data ^{\n Script ^{\n Mod_SmartLootServiceV2=1,Script.Mod_SmartLootServiceV2\n }\n ScriptLibs ^{\n Mod_SmartLootServiceV2=SmartLootService\n SmartLootService ^{\n Path=Mods\\OtherMods\\SmartLootAI\\Native\\SmartLootAI.XenoPlugin.dll\n }\n }\n}\n')
lib=main.find_node('Data/ScriptLibs/SmartLootService')
for name,args in api.items():lib.set_parameter(name,','.join(['dword' if name=='SL_Target' else 'int',name]+args),create=True)
main.save(r/'src/Main.txt',encoding='cp1251')
shutil.copyfile(r/'src/SmartLootAI.XenoPlugin.dll',runtime/'Native/SmartLootAI.XenoPlugin.dll')
(runtime/'Native/SmartLootAI.XenoPlugin.ini').write_text('[Plugin]\nEnabled=1\nDll=SmartLootAI.XenoPlugin.dll\nLegacy=0\n\n[SmartLootAI]\nActiveJunkCleanup=1\nCleanupMinerals=1\nCleanupTradeGoods=1\n',encoding='ascii')
shutil.copyfile(r/'src/ModuleInfo.txt',runtime/'ModuleInfo.txt')
print('Runtime files and ScriptLibs signatures staged; installed game unchanged.')
