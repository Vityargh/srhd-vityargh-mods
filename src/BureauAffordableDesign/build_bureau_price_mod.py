import sys, pathlib, json, hashlib
GAME = pathlib.Path(r'C:\Program Files (x86)\Steam\steamapps\common\Space Rangers HD A War Apart')
sys.path.insert(0, str(GAME))
from srhd_modkit.blockpar import load_blockpar, parse_blockpar
ROOT = pathlib.Path(r'S:\SR\BureauAffordableDesign')
ROOT.mkdir(exist_ok=True)
out = ROOT / 'CFG/Rus'
out.mkdir(parents=True, exist_ok=True)
source = pathlib.Path(r'S:\SR\BureauPrice.Lang.current.txt')
doc = load_blockpar(source)
overlay = parse_blockpar('DesignDepartmentModList ^{\n    CreateEquipment ^{\n    }\n}\n')
changes = []
def visit(node, path):
    for p in node.parameters:
        if p.key == 'Credits':
            old = int(p.value)
            new = (old + 5) // 10 if old >= 0 else -((-old + 5) // 10)
            dest = overlay.ensure_node(path)
            dest.set_parameter('Credits', str(new), create=True)
            changes.append({'path':path, 'before':old, 'after':new})
    for child in node.children:
        visit(child, path + '/' + child.name)
for category in ('Hull','Weapon','Equipments'):
    path = 'DesignDepartmentModList/CreateEquipment/' + category
    visit(doc.find_node(path), path)
assert len(changes) > 50
overlay.save(out / 'Lang.txt')
(ROOT / 'ModuleInfo.txt').write_text('Name=BureauAffordableDesign\nAuthor=Vityargh\nDependence=Cat_DesignDepartment\nPriority=100\nSection=Прочие моды\nLanguages=Rus\nSmallDescription=КБ: кредитная цена проектирования снижена в 10 раз.\nFullDescription=Новые проекты корпусов, оружия и оборудования стоят в 10 раз меньше кредитов. Ресурсы, сроки и проекты, уже принятые в работу, не меняются.\n', encoding='utf-8-sig')
(ROOT / 'README_RU.md').write_text('КБ: доступное проектирование\n\nКредитная стоимость НОВЫХ проектов корпусов, оружия и оборудования / 10.\nПример со скриншота: 17 295 000 -> 1 729 500 кредитов.\nРесурсы, сроки, чертежи, магазин и членские взносы не меняются.\nУже оформленные проекты сохраняют старую задолженность.\n\nУстановка: папку BureauAffordableDesign поместить в Mods/OtherMods, включить мод в менеджере модов игры вместе с Cat_DesignDepartment, пересобрать конфигурацию штатным менеджером и полностью перезапустить игру. Для открытого окна проектирования изменить любой параметр для пересчёта.\nОтключение: снять галочку мода и пересобрать конфигурацию.\n\nИзменяются только параметры Credits внутри CreateEquipment/Hull, Weapon, Equipments. Исполняемый файл и SCR не патчатся. Лимит денег игры не расширяется.\nПроверены структура и точный состав изменений, DAT round-trip. Проверка покупки в игре пока не выполнена.\n', encoding='utf-8-sig')
evidence = {'source_lang_sha256': hashlib.sha256((GAME/'Mods/Kotyanka/Cat_DesignDepartment/CFG/Rus/Lang.dat').read_bytes()).hexdigest(), 'changes':changes}
(ROOT.parent/'BureauAffordableDesign.changes.json').write_text(json.dumps(evidence,ensure_ascii=False,indent=2),encoding='utf-8')
# Exact overlay coverage: every design Credits value and nothing else.
def leaves(node, path):
    result = {(path+'/'+p.key):p.value for p in node.parameters}
    for c in node.children: result.update(leaves(c,path+'/'+c.name))
    return result
actual = leaves(overlay.roots[0], overlay.roots[0].name)
expected = {c['path']+'/Credits':str(c['after']) for c in changes}
assert actual == expected
print(f'PASS: {len(changes)} credit coefficients scaled exactly /10; overlay contains no other values')
