from pathlib import Path
import sys
game=Path(r'C:\Program Files (x86)\Steam\steamapps\common\Space Rangers HD A War Apart')
sys.path.insert(0,str(game))
from srhd_modkit.blockpar import load_blockpar
root=Path(r'S:\SR\BureauAffordableDesign')
path=root/'CFG/Rus/Lang.txt'
doc=load_blockpar(path)
key='ShipInfo/AddInfo/CustomInfos/ArtiUniqueShipsPack_Crutch'
expected={'Description':'NoShow','Icon':'1','Name':'-'}
upstream=load_blockpar(game/'Mods/KotyankaPlus/ArtiUniqueShipsPack/CFG/Rus/Lang.txt').find_node(key)
assert {p.key:p.value for p in upstream.parameters}==expected
before=doc.canonical_semantic()
node=doc.ensure_node(key)
for k,v in expected.items(): node.set_parameter(k,v,create=True)
doc.save(path,encoding='cp1251')
check=load_blockpar(path)
assert {p.key:p.value for p in check.find_node(key).parameters}==expected
check.delete_node('ShipInfo')
assert check.canonical_semantic()==before
readme=root/'README_RU.md'
text=readme.read_text(encoding='utf-8-sig')
text+='\n\nВключён фикс вылета КБ: добавлена скрытая служебная метка ArtiUniqueShipsPack_Crutch, которую использует скрипт станции. Значения совпадают с оригинальным UniqueShipsPack. Исполняемый файл и SCR не меняются. Исправление также остаётся в ранее исправленном Cat_DesignDepartment.\n'
readme.write_text(text,encoding='utf-8-sig')
print('PASS: crashfix matches upstream exactly; all price parameters unchanged.')
