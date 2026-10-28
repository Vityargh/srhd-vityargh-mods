# Моды Vityargh для «Космических рейнджеров HD: Революция»

Моды для Space Rangers HD: A War Apart (2.1.2500, Steam).

## Что дают моды

**SmartLootAI — космос без свалки.** К середине партии системы забиты брошенными минералами, товарами и хламом: карта в мусоре, нужное теряется. С модом обычные корабли сами расстреливают мусор свободными орудиями, а по галактике летает служба мусорщиков — неуязвимые корабли «Работяга», «Скат» и «Косточка», которые чистят системы и с которыми можно поболтать о тяжёлой жизни уборщика. Ценное не пропадает: артефакты, микромодули, ноды, корпуса, квестовые вещи и предметы из других модов не трогаются никогда, а оборудования в каждой системе остаётся до 50 штук.

**ApplyCrashFix — больше никаких вылетов после «Применить».** Включили или выключили мод в списке, нажали «Применить» — и игра падала. Теперь нет.

**BureauAffordableDesign — своё КБ по карману.** Проекты собственных корпусов, оружия и оборудования в конструкторском бюро стоят в 10 раз меньше кредитов. Например, 17 295 000 превращаются в 1 729 500. Ресурсы и сроки разработки прежние.

> **Тестовая версия.** Моды проверены автоматическими тестами, но ещё не прошли полную проверку в игре. Сделайте копию сохранения перед установкой.

## Моды

| Мод | Версия | Что делает | Нужен загрузчик |
|---|---|---|---|
| [SmartLootAI](mods/SmartLootAI/README_RU.md) ([EN](mods/SmartLootAI/README_EN.md)) | 2.0.1 | Уборка лишнего лута и служба мусорщиков, RU/EN | да |
| [ApplyCrashFix](mods/ApplyCrashFix/README_RU.md) | 1.0 | Игра не вылетает после кнопки «Применить» в списке модов | да |
| [BureauAffordableDesign](mods/BureauAffordableDesign/README_RU.md) | 1.0 | Проекты в конструкторском бюро в 10 раз дешевле по кредитам | нет |

## Как установить

**Скачивайте готовые архивы со страницы [Releases](../../releases)**, а не кнопкой «Code»: в архиве уже лежит всё нужное, включая загрузчик.

1. Закройте игру.
2. Распакуйте архив целиком в папку игры (туда, где лежит `Rangers.exe`).
   Steam: Библиотека → правый клик по игре → Управление → Просмотреть локальные файлы.
3. Если архиватор спросит, заменить ли `dsound.dll`, `XenoCore.dll` или `XenoNative.ini`, нажмите **«Пропустить»**: загрузчик у вас уже стоит.
4. В игре откройте список модов, в разделе «Моды Vityargh» включите нужные, нажмите «Применить».
5. Полностью перезапустите игру.

Перед первым запуском сделайте копию сохранения.

## Загрузчик

SmartLootAI и ApplyCrashFix работают через [XenoNativeLoader](https://github.com/Xenomorphchyma/XenoMods) от Xenomorphchyma. Архив релиза включает его с разрешения автора, по лицензии PolyForm Noncommercial 1.0.0. В этом репозитории загрузчика нет.

## Что внутри репозитория

```
mods/    готовые папки модов, как они лежат в Mods\OtherMods
src/     исходники: нативные плагины (C), скрипты (RScript), генераторы и тесты
tools/   сборка архивов релиза
```

Как собрать моды из исходников — в [src/BUILD.md](src/BUILD.md).

## Лицензия

Код и файлы модов — [MIT](LICENSE): пользуйтесь и меняйте свободно, всё предоставляется «как есть», без гарантий и ответственности автора. Загрузчик и файлы игры под эту лицензию не попадают.

## Сообщить о проблеме

Создайте Issue и приложите файл `XenoNative.log` из папки игры, а если игра вылетела — последний файл из `Документы\SpaceRangersHD\Errors`.

---

**English**

**SmartLootAI — space without the landfill.** By mid-game, star systems are buried in abandoned minerals, goods and junk. With this mod, ordinary ships shoot junk with their free weapons, and a janitor service of invulnerable ships — Elbow Grease, Skate and Wishbone — cleans up the galaxy (and they're happy to chat about the hard life of a space janitor). Nothing valuable is lost: artifacts, micromodules, nodes, hulls, quest items and items from other mods are never touched, and each system keeps up to 50 pieces of equipment.

**ApplyCrashFix — no more crashes after Apply.** Toggling a mod and pressing Apply in the mod list used to crash the game. Not anymore.

**BureauAffordableDesign — a design bureau you can afford.** Your own hull, weapon and equipment projects cost 10 times fewer credits. Resources and development time stay the same.

Mods for Space Rangers HD: A War Apart 2.1.2500. Download the archive from Releases, unpack it into the game folder (skip replacing `dsound.dll` / `XenoCore.dll` / `XenoNative.ini` if asked), enable the mods in the in-game mod list, press Apply and restart the game.
