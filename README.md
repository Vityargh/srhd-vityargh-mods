# Моды Vityargh для «Космических рейнджеров HD: Революция»

[English version below](#english)

Привет всем, кто до сих пор летает в Космических рейнджерах.
Сделал несколько небольших модов, которых мне самому не хватало в игре. Делюсь — может, пригодятся и вам.

## Что они дают

** SmartLootAI ** — космос без свалки.

К середине партии системы завалены брошенными минералами, товарами и всяким хламом. С этим модом обычные корабли между делом расстреливают мусор свободными орудиями, а по галактике летает служба мусорщиков — «Работяга», «Скат» и «Косточка». Они неуязвимы, ни с кем не воюют и с удовольствием поболтают с вами о нелёгкой жизни космического уборщика. Всё ценное остаётся на месте: артефакты, микромодули, ноды, корпуса, квестовые вещи и предметы из других модов не трогаются никогда.
====================================================================
**ApplyCrashFix** — прощайте, вылеты после «Применить».
Включили мод, нажали «Применить» в списке модов — и игра падала. Больше не падает.
====================================================================
**BureauAffordableDesign** — своё КБ по карману.
Проекты собственных корпусов, оружия и оборудования в конструкторском бюро стоят в 10 раз меньше кредитов. Ресурсы и сроки разработки прежние.
====================================================================
> ⚠️⚠️⚠️ Это тестовая версия: моды прошли автоматические проверки, но в долгой игре пока обкатываются. Сделайте копию сохранения перед установкой — и, пожалуйста, расскажите, если что-то пойдёт не так ⚠️⚠️⚠️

| Мод | Версия | Нужен загрузчик | Подробнее |
|---|---|---|---|
| SmartLootAI | 0.9 (бета) | да | [README](mods/SmartLootAI/README_RU.md) |
| ApplyCrashFix | 1.0 | да | [README](mods/ApplyCrashFix/README_RU.md) |
| BureauAffordableDesign | 1.0 | нет, но нужен мод Cat_DesignDepartment | [README](mods/BureauAffordableDesign/README_RU.md) |

## Как установить

Скачивайте готовые архивы со страницы **[Releases](../../releases)**, а не кнопкой «Code»: в архивах уже лежит всё нужное, включая загрузчик.

1. Закройте игру.
2. Распакуйте архив целиком в папку игры — туда, где лежит `Rangers.exe`.
   В Steam: Библиотека → правый клик по игре → Управление → Просмотреть локальные файлы.
3. Если архиватор спросит, заменить ли `dsound.dll`, `XenoCore.dll` или `XenoNative.ini`, нажмите **«Пропустить»**: значит, загрузчик у вас уже стоит.
4. Запустите игру, в списке модов откройте раздел «Моды Vityargh», включите нужные и нажмите «Применить».
5. Перезапустите игру.

## Если что-то не так

Создайте [Issue](../../issues) и приложите файл `XenoNative.log` из папки игры. Если игра вылетела — ещё последний файл из `Документы\SpaceRangersHD\Errors`.

## Планы

SmartLootAI пока в ранней версии, и многое из задуманного ещё впереди:

- **Свой корабль для каждого мусорщика.** Сейчас Работяга, Скат и Косточка летают на одинаковых кораблях. Задумка такая: Работяга — старый потёртый промышленный сборщик с контейнерами и манипуляторами, Скат — современный и компактный, Косточка — маленький вспомогательный аппарат.
- **Станция «Тихая гавань».** База службы: ремонт, сортировка мусора и таверна космических работников, где мусорщики пережидают между сменами.
- **Портреты экипажа** в разговорах.
- **Настройки прямо в игре**, без правки файлов.

Версии растут постепенно: 0.9, 0.10, 0.11 и дальше — по мере того как появляются новые части.

## Хотите помочь?

Код и модели я делаю вместе с ИИ, а опыта в 3D-моделях для игры у меня пока мало. Поэтому буду рад любой посильной помощи — моделям кораблей и станции, тестам, идеям, переводам. И, конечно, вашим форкам: берите код и делайте по-своему.

## Для тех, кто хочет заглянуть внутрь

```
mods/    готовые папки модов, как они лежат в Mods\OtherMods
src/     исходники: нативные плагины (C), скрипты (RScript), генераторы и тесты
tools/   сборка архивов релиза
```

Как собрать моды из исходников — в [src/BUILD.md](src/BUILD.md). Код открыт по лицензии [MIT](LICENSE): пользуйтесь и меняйте свободно, всё предоставляется «как есть», без гарантий.

Проверено на Space Rangers HD: A War Apart 2.1.2500 (Steam). Тексты модов — на русском и английском.
Огромное спасибо Xenomorphchyma за загрузчик [XenoNativeLoader](https://github.com/Xenomorphchyma/XenoMods) (лицензия PolyForm Noncommercial 1.0.0). В репозитории его нет — он входит только в архивы релиза.

Удачи!

---

<a id="english"></a>

# Vityargh's mods for Space Rangers HD: A War Apart

Hi to everyone still flying in Space Rangers!
I made a few small mods that I personally missed in the game. Sharing them in case you find them useful too.

## What they do
====================================================================
**SmartLootAI** — space without the landfill.
By mid-game, star systems are buried in abandoned minerals, goods and all sorts of junk. With this mod, ordinary ships clear it with their free weapons, and a janitor service roams the galaxy: Elbow Grease, Skate and Wishbone. They're invulnerable, never fight anyone, and are always happy to chat about the hard life of a space janitor. Everything valuable stays put: artifacts, micromodules, nodes, hulls, quest items and items from other mods are never touched.
====================================================================
**ApplyCrashFix** — goodbye, crashes after Apply.
Enabling a mod and pressing Apply in the mod list used to crash the game. Not anymore.
====================================================================
**BureauAffordableDesign** — a design bureau you can afford.
Your own hull, weapon and equipment projects cost 10 times fewer credits. Resources and development time stay the same.
====================================================================
> ⚠️⚠️⚠️ This is a test release: the mods passed automated checks but are still being played through. Back up your save before installing, and please let me know if something goes wrong ⚠️⚠️⚠️

| Mod | Version | Needs the loader | Details |
|---|---|---|---|
| SmartLootAI | 0.9 (beta) | yes | [README](mods/SmartLootAI/README_EN.md) |
| ApplyCrashFix | 1.0 | yes | [README (RU)](mods/ApplyCrashFix/README_RU.md) |
| BureauAffordableDesign | 1.0 | no, but needs the Cat_DesignDepartment mod | [README (RU)](mods/BureauAffordableDesign/README_RU.md) |

## Installation

Download the ready-made archives from **[Releases](../../releases)**, not with the "Code" button: the archives already contain everything you need, including the loader.

1. Close the game.
2. Unpack the whole archive into the game folder, where `Rangers.exe` is.
   In Steam: Library → right-click the game → Manage → Browse local files.
3. If the archiver asks whether to replace `dsound.dll`, `XenoCore.dll` or `XenoNative.ini`, choose **Skip**: you already have the loader.
4. Start the game, open the "Vityargh Mods" section in the mod list, enable the mods you want and press Apply.
5. Restart the game.

## Something went wrong?

Open an [Issue](../../issues) and attach `XenoNative.log` from the game folder. If the game crashed, also attach the latest file from `Documents\SpaceRangersHD\Errors`.

## Plans

SmartLootAI is still an early version, and much of what I have in mind is still ahead:

- **A ship of its own for each janitor.** Right now Elbow Grease, Skate and Wishbone fly identical ships. The idea: Elbow Grease is an old, battered industrial collector with containers and manipulators, Skate is modern and compact, Wishbone is a small utility craft.
- **The Quiet Harbor station.** The service's base: repairs, junk sorting and a tavern for space workers where janitors wait between shifts.
- **Crew portraits** in conversations.
- **In-game settings**, no file editing needed.

Versions grow step by step: 0.9, 0.10, 0.11 and on, as new parts arrive.

## Want to help?

I build the code and the models together with AI, and I don't have much experience with 3D models for the game yet. So any help is very welcome — ship and station models, testing, ideas, translations. And of course your forks: take the code and make it your own.

## For those who want to look inside

```
mods/    ready-to-use mod folders, as they sit in Mods\OtherMods
src/     sources: native plugins (C), scripts (RScript), generators and tests
tools/   release archive builder
```

Build instructions are in [src/BUILD.md](src/BUILD.md) (in Russian). The code is open under the [MIT](LICENSE) license: use and change it freely, everything is provided as is, without warranty.

Tested on Space Rangers HD: A War Apart 2.1.2500 (Steam). The mods' texts are in Russian and English.
Huge thanks to Xenomorphchyma for [XenoNativeLoader](https://github.com/Xenomorphchyma/XenoMods) (PolyForm Noncommercial 1.0.0 license). It is not stored in this repository, only in the release archives.

Goodluck!
