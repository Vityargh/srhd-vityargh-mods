"""Human-readable ModuleInfo for 2.0 (multi-line FullDescription like 1.3.4 / BureauAffordableDesign; no guillemets, no em dashes)."""
from pathlib import Path
import shutil
r = Path(__file__).parent
lines = [
 'Name=SmartLootAI',
 'Author=Vityargh',
 'NoNormalResources=1',
 'Conflict=',
 'Dependence=',
 'Priority=100',
 'Section=Моды Vityargh',
 'SectionEng=Vityargh Mods',
 'Languages=Rus,Eng',
 'SmallDescription=Версия 0.10 (бета). Служба мусорщиков и уборка лишнего лута в космосе.',
 'FullDescription=<color=0,80,170>Версия 0.10 (бета)</color><br>',
 'FullDescription=Космос становится чище: лишний лут убирают обычные корабли и отдельная служба мусорщиков.<br><br>',
 'FullDescription=<color=0,80,170>Что убирается</color><br>',
 'FullDescription=Минералы, товары и бесполезный хлам исчезают сразу. Оборудование и оружие копятся: в каждой системе остаётся до 50 штук, а лишнее убирается начиная с самого дешёвого.<br><br>',
 'FullDescription=<color=0,80,170>Что остаётся</color><br>',
 'FullDescription=Артефакты, микромодули, ноды и незнакомые предметы из других модов не трогаются никогда. Квестовые вещи сохраняются, если стоят от 10 000 кредитов за единицу веса.<br><br>',
 'FullDescription=<color=0,80,170>Корабли и мусорщики</color><br>',
 'FullDescription=Корабли стреляют по мусору только свободными орудиями и не отвлекаются от боя. Мусорщики Работяга, Скат и Косточка неуязвимы, ни с кем не воюют и сами перелетают между системами. С ними можно поговорить.<br><br>',
 'FullDescription=<color=0,80,170>Как включить</color><br>',
 'FullDescription=Распакуйте архив мода в папку игры целиком, включите мод, примените изменения и полностью перезапустите игру. Нужный загрузчик XenoNativeLoader уже лежит в архиве. Перед первым запуском лучше сделать копию сохранения.<br><br>',
 'FullDescription=<color=0,80,170>Что дальше</color><br>',
 'FullDescription=Это ранняя версия. В планах: свои модели для каждого мусорщика, станция "Тихая гавань" и настройки прямо в игре.<br><br>',
 'SmallDescriptionEng=Version 0.10 (beta). A janitor service and cleanup of unwanted loot in space.',
 'FullDescriptionEng=<color=0,80,170>Version 0.10 (beta)</color><br>',
 'FullDescriptionEng=Space gets cleaner: unwanted loot is removed by ordinary ships and a dedicated janitor service.<br><br>',
 'FullDescriptionEng=<color=0,80,170>What gets cleaned</color><br>',
 'FullDescriptionEng=Minerals, goods and useless junk go right away. Equipment and weapons pile up: each system keeps up to 50 of them, and the cheapest extras go first.<br><br>',
 'FullDescriptionEng=<color=0,80,170>What stays</color><br>',
 'FullDescriptionEng=Artifacts, micromodules, nodes and unknown items from other mods are never touched. Quest items stay if they are worth 10,000 credits or more per weight unit.<br><br>',
 'FullDescriptionEng=<color=0,80,170>Ships and janitors</color><br>',
 'FullDescriptionEng=Ships fire at junk only with free weapons and never leave a fight for it. Janitors Elbow Grease, Skate and Wishbone are invulnerable, never fight and travel between systems on their own. You can talk to them.<br><br>',
 'FullDescriptionEng=<color=0,80,170>Getting started</color><br>',
 'FullDescriptionEng=Unpack the whole archive into the game folder, enable the mod, apply the changes and fully restart the game. The required XenoNativeLoader is already included. Back up your save before the first launch.<br><br>',
 'FullDescriptionEng=<color=0,80,170>What is next</color><br>',
 'FullDescriptionEng=This is an early version. Planned: its own model for each janitor, the Quiet Harbor station and in-game settings.<br><br>',
]
text = '\r\n'.join(lines) + '\r\n'
assert '—' not in text and '«' not in text
(r / 'src' / 'ModuleInfo.txt').write_bytes(text.encode('cp1251'))
(r / 'RuntimeMod' / 'ModuleInfo.txt').write_bytes(text.encode('cp1251'))
# keep prepare_runtime.py from overwriting it on the next build
p = r / 'prepare_runtime.py'
s = p.read_text(encoding='utf-8')
i = s.find("(runtime/'ModuleInfo.txt').write_text(")
if i >= 0:
    j = s.find('\n', i)
    s = s[:i] + "shutil.copyfile(r/'src/ModuleInfo.txt',runtime/'ModuleInfo.txt')" + s[j:]
    p.write_text(s, encoding='utf-8')
print('ModuleInfo written; prepare_runtime now copies src/ModuleInfo.txt')


import sys; sys.path.insert(0, r'S:\SR'); from modinfo_format import format_file
format_file(r / 'src' / 'ModuleInfo.txt'); shutil.copyfile(r / 'src' / 'ModuleInfo.txt', r / 'RuntimeMod' / 'ModuleInfo.txt')
