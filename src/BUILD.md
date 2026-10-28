# Сборка из исходников

Все шаги — Windows, Python 3, игра из Steam. Скрипты пока используют абсолютные пути рабочей машины (`S:\SR\...` и папку игры в Steam); их нужно поправить под себя.

## Инструменты

| Что | Зачем |
|---|---|
| TinyCC (`tcc.exe`) | сборка x86 DLL и тестов |
| Python-пакеты `pefile`, `capstone` | разбор `Rangers.exe`, генерация сигнатур |
| [SRHD-XenoModKit](https://github.com/Xenomorphchyma/SRHD-XenoModKit) (`python -m srhd_modkit`; склонировать и добавить папку в `PYTHONPATH`) | сборка SCR из RSON, аудит, линтер |
| RScript 4.15f | компилятор скриптов (вызывается из ModKit) |
| BlockParEditor 2.1 | преобразование DAT ↔ TXT |

## SmartLootAI

```
cd src/SmartLootAI
python src/build.py            # prepare.py (сигнатуры из Rangers.exe) + DLL через tcc
python check_exe.py <путь к Rangers.exe>  # подойдёт ли мод к другой сборке игры, без запуска игры
python run_checks.py           # 6 наборов x86-тестов
python generate_dialogue.py    # RSON скрипта службы + CrewLang.txt и CrewLang_Eng.txt
# SCR скрипта службы. --lang - выходной файл: RScript пишет туда фрагмент текстов (src/CrewFragment.txt).
# Фрагмент в игру не идёт: Lang.dat собирает make_lang.py из CrewLang*.txt.
# <toolchain> - папка, где лежит RScript (например S:\SR\toolchain).
python -m srhd_modkit script build src/Mod_SmartLootServiceV2.rson --scr <RuntimeMod>/DATA/Script/Mod_SmartLootServiceV2.scr --lang src/CrewFragment.txt --overwrite --tools-root <toolchain>
python make_lang.py            # CFG/Rus и CFG/Eng Lang.dat из CrewLang*.txt с проверкой
python make_moduleinfo.py
python prepare_runtime.py      # раскладка DLL, ini, ModuleInfo в папку мода
```

`generate_dialogue.py` берёт шаблон RSON из чужого мода (Kotyanka `Cat_DesignDepartment`), который в репозиторий не входит.

Перед установкой обязательно прогнать стенд загрузки: `tests/init_harness.c` грузит настоящий `Rangers.exe` и вызывает `XenoPlugin_Initialize` так же, как загрузчик. Ожидаемый результат — `hooks installed 19/19`.

## ApplyCrashFix

```
cd src/ApplyCrashFix
python build.py
tests/pin_test.exe <путь к MatrixGame.dll> none          # без фикса: UNMAPPED
tests/pin_test.exe <путь к MatrixGame.dll> <DLL плагина> # с фиксом: STILL MAPPED
```

## BureauAffordableDesign

`build_bureau_price_mod.py` берёт языковой файл `Cat_DesignDepartment`, делит параметры `Credits` у проектов на 10 и собирает `Lang.dat`. Точный список изменений — `changes.json`.

## Релиз

`tools/build_release.py` собирает два архива для игроков и сверяет каждый байт с исходными файлами. Загрузчик берётся из локальной папки `vendor`, его хеши сверяются с манифестом релиза XenoMods.
