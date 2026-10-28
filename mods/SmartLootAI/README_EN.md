# SmartLootAI 0.9 (Beta) — Vityargh

Making space a cleaner place: unwanted loot is cleared by regular ships and a dedicated space cleanup service.

## Installation

1. Close the game completely.
2. Extract the archive into your game folder (where `Rangers.exe` is located). If your archiver asks whether to overwrite `dsound.dll`, `XenoCore.dll`, or `XenoNative.ini`, choose **Skip**: you already have the loader installed.
3. Launch the game, find SmartLootAI under **"Vityargh Mods"** in the mod list, enable it, and click **Apply**.
4. Restart the game completely.

**Back up your save before launching the mod for the first time.**

## What Gets Cleaned Up

* **Minerals and trade goods** are removed immediately, regardless of quantity.
* **Equipment and weapons** are allowed to accumulate up to 50 items per star system. Anything above that limit is removed, starting with the cheapest items.

## What Is Never Touched

* Artifacts and micromodules.
* Dominator nodes (standard ones, weighing 30–1000 units).
* Unrecognized items from other mods.
* Quest and special items worth at least 10,000 credits per unit of weight.

## Regular Ships

Regular ships fire at junk only with available weapons and only when no enemies are within those weapons' range. Weapons already engaged in combat continue fighting normally, ship routes remain unchanged, and missiles are never wasted on junk.

**The mod does not take control of your ship.**

## Space Cleanup Service

* Three ships: SCV-01 **"Elbow Grease"**, SCV-02 **"Skate"**, and SCV-03 **"Wishbone"**.
* They are invulnerable, never engage in combat, drop no loot, and have unlimited fuel.
* They automatically travel to systems containing junk, including Dominator-controlled systems. They do not travel through black holes.
* Up to 10 cleanup ships can operate across the galaxy, with no more than two in any single system.
* You can talk to their crews, with **34 conversation topics** available.

New cleanup ships do not appear immediately. The service first performs a one-time scan of the galaxy. This is normal.

## Configuration

Configuration file:

`Mods\OtherMods\SmartLootAI\Native\SmartLootAI.XenoPlugin.ini`

Section: `[SmartLootAI]`

Set a value to `1` to enable an option or `0` to disable it. Restart the game after making changes.

| Setting | Description |
|---|---|
| `ActiveJunkCleanup` | Allows regular ships to actively clear junk using available weapons |
| `CleanupMinerals` | Enables mineral cleanup |
| `CleanupTradeGoods` | Enables trade goods cleanup |

## Troubleshooting

* **You see the message "cleanup service not started".** The loader is missing or your game version is unsupported. Extract the entire archive into the game folder (`dsound.dll` and `XenoCore.dll` should be next to `Rangers.exe`), then restart the game.
* **Detailed information** is written to `XenoNative.log` in the game folder. The entry `ACTIVE 0.9` means the mod is running correctly. An entry containing `BLOCKED` explains why the mod could not start.

## Compatibility

* Tested on **Space Rangers HD: A War Apart 2.1.2500 (Steam)**, the latest game release.
* On other game versions, the mod automatically disables itself if the game's code differs from the supported version and reports the reason. Your save file will remain unaffected.
* No conflicts are expected with mods that do not modify loot cleanup or ship behavior.

## Uninstalling / Rolling Back

Disable the mod and apply the changes. However, the cleanup service state already stored in your save file will not be removed.

For a complete rollback, restore a save made before installing the mod.

## Future Plans

This is still an early version, and there's plenty more I want to add:

* **Unique ships for each cleanup crew.** Right now, Elbow Grease, Skate, and Wishbone all fly identical ships. The plan is to give each one its own design: Elbow Grease will be an old, battered industrial collector with cargo containers and manipulators; Skate will be modern and compact; and Wishbone will be a small utility craft.
* **The "Quiet Harbor" station.** A home base for the cleanup service, featuring repair facilities, junk sorting, and a space workers' tavern where crews can relax between shifts.
* **Crew portraits** during conversations.
* **In-game settings**, without having to edit configuration files.

I'm developing the code and models with AI assistance, and I don't have much experience creating 3D models for games yet.

Any help would be greatly appreciated — whether it's 3D models, testing, ideas, translations, or your own forks of the project.

I also have original sketches from *Planetes* (2003), concept drawings, early 3D models, and character portraits that can serve as references.
