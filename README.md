# Моды Vityargh для «Космических рейнджеров HD: Революция»

Моды для Space Rangers HD: A War Apart (2.1.2500, Steam).

| Мод | Версия | Что делает | Нужен загрузчик |
|---|---|---|---|
| [SmartLootAI](mods/SmartLootAI/README_RU.md) | 2.0.1 | Уборка лишнего лута и служба мусорщиков | да |
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

**English:** mods for Space Rangers HD: A War Apart 2.1.2500. Download the archive from Releases, unpack it into the game folder (skip replacing `dsound.dll` / `XenoCore.dll` / `XenoNative.ini` if asked), enable the mods in the in-game mod list, press Apply and restart the game.
