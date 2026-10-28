from pathlib import Path
import sys
sys.path.insert(0, r'S:\SR')
from modinfo_format import format_file
r = Path(__file__).parent
lines = [
 'Name=ApplyCrashFix',
 'Author=Vityargh',
 'NoNormalResources=1',
 'Conflict=',
 'Dependence=',
 'Priority=100',
 'Section=Моды Vityargh',
 'SectionEng=Vityargh Mods',
 'Languages=Rus',
 'SmallDescription=Версия 1.0. Игра больше не вылетает после кнопки Применить в списке модов.',
 'FullDescription=Что исправляет<br>',
 'FullDescription=При применении модов игра выгружала движок планетарных боёв, но его окно оставалось и вызывало вылет. Мод оставляет движок в памяти до закрытия игры.<br><br>',
 'FullDescription=Что не меняется<br>',
 'FullDescription=Код игры и других модов не изменяется. Работает с любыми наборами модов, сохранения не затрагивает.<br><br>',
 'FullDescription=Как включить<br>',
 'FullDescription=Включите мод и полностью перезапустите игру. Требуется XenoNativeLoader.<br><br>',
 'SmallDescriptionEng=Version 1.0. No more crash after pressing Apply in the mod list.',
 'FullDescriptionEng=What it fixes<br>',
 'FullDescriptionEng=Applying mods unloaded the planetary battles engine while its window stayed alive and crashed the game. The mod keeps the engine loaded until the game exits.<br><br>',
 'FullDescriptionEng=What stays the same<br>',
 'FullDescriptionEng=No game or mod code is changed. Works with any mod set and does not touch saves.<br><br>',
 'FullDescriptionEng=Getting started<br>',
 'FullDescriptionEng=Enable the mod and fully restart the game. Requires XenoNativeLoader.<br><br>',
]
p = r / 'RuntimeMod' / 'ModuleInfo.txt'
p.write_bytes(('\r\n'.join(lines) + '\r\n').encode('cp1251'))
print(format_file(p))
