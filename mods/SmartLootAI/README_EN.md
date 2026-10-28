# SmartLootAI 0.9 (beta) - Vityargh

Space gets cleaner: unwanted loot is removed by ordinary ships and a dedicated janitor service.

## Installation

1. Close the game.
2. Unpack the whole archive into the game folder (where Rangers.exe is). If the archiver asks to replace dsound.dll, XenoCore.dll or XenoNative.ini, choose "Skip": you already have the loader.
3. Start the game, enable SmartLootAI in the "Vityargh Mods" section of the mod list, press Apply.
4. Fully restart the game.

Back up your save before the first launch.

## What gets cleaned

* Minerals and goods - right away, in any amount.
* Equipment and weapons pile up: each system keeps up to 50 of them, the cheapest extras go first.

## Never touched

Artifacts, micromodules, dominator nodes, hulls, unknown items from other mods, and quest items worth 10,000 credits or more per weight unit.

## Ordinary ships

They shoot junk only with free weapons and only when no enemy is in range. Busy weapons keep fighting, routes don't change, no missiles are wasted on junk. Your own ship is never controlled.

## Janitor service

* SCV-01 "Elbow Grease", SCV-02 "Skate" and SCV-03 "Wishbone".
* Invulnerable, never fight, drop nothing, infinite fuel.
* Travel on their own to wherever there is junk, including dominator systems. Never through black holes.
* Up to 10 in the galaxy, no more than two per system.
* You can talk to the crew: 34 topics.

New janitors don't appear right away: the service first sweeps the galaxy once.

## Settings

File `Mods\OtherMods\SmartLootAI\Native\SmartLootAI.XenoPlugin.ini`, section `[SmartLootAI]`, 1 = on, 0 = off. Restart the game after changes.

| Setting | What it does |
|---|---|
| `ActiveJunkCleanup` | Ordinary ships clean junk with free weapons |
| `CleanupMinerals` | Clean up minerals |
| `CleanupTradeGoods` | Clean up trade goods |

## If it doesn't work

* **A message says the sanitation service is not running.** The loader is missing or the game version is not supported. Unpack the whole archive into the game folder and restart.
* **Details** are written to `XenoNative.log` in the game folder. `ACTIVE 0.9` means everything works; `BLOCKED` explains why the mod did not start.

## Compatibility

Tested on Space Rangers HD: A War Apart 2.1.2500 (Steam). On other versions the mod switches itself off if the game code differs, and tells you so. Your save is not affected.

## Rollback

Disable the mod and press Apply. The service state already written to a save stays there; for a full rollback use a save made before installing the mod.

## Plans

This is an early version, and much of what I have in mind is still ahead:

* **A ship of its own for each janitor.** Right now Elbow Grease, Skate and Wishbone fly identical ships. The idea: Elbow Grease is an old, battered industrial collector with containers and manipulators, Skate is modern and compact, Wishbone is a small utility craft.
* **The Quiet Harbor station.** The service's base: repairs, junk sorting and a tavern for space workers where janitors wait between shifts.
* **Crew portraits** in conversations.
* **In-game settings**, no file editing needed.

I build the code and the models together with AI, and I don't have much experience with 3D models for the game yet. Any help is very welcome - models, testing, ideas, translations - and so are your forks.
