# ApplyCrashFix 1.0

Fixes the game crash after pressing **Apply** in the mod list.

## What the problem was

When you press Apply, the game reloads mods on the fly and unloads the planetary battles engine (`MatrixGame.dll`) from memory. But the window that engine had already created stays. As soon as Windows sends that window any message, the game calls code that is no longer loaded and crashes.

This is confirmed by a dump of a real crash: the crash address points inside the already unloaded `MatrixGame.dll`.

## What the mod does

It keeps the planetary battles engine from being unloaded until the game closes. Nothing else.

- The code of the game and of other mods is not changed.
- Saves are not affected.
- Works with any set of mods.
- The planetary battles engine (about 1.5 MB) stays in memory until you quit the game, even if there are no more planetary battles.

## Installation

1. Put the `ApplyCrashFix` folder into `Mods\OtherMods\`.
2. Enable the mod in the mod list.
3. Fully restart the game.

Requires XenoNativeLoader (`dsound.dll` and `XenoCore.dll` next to `Rangers.exe`). It is already included in the Vityargh mods archive (version 0.7.2).

The fix was tuned on the 2.1.2500 (6 Oct 2026) build (Redux / Space Rangers Universe), where this crash reproduces most reliably. It also works everywhere else the game loads `MatrixGame.dll` — including the original Steam/GOG — because it does not touch the game code.

## Testing

The test repeats what the game does on Apply: it loads the engine, unloads it and checks whether its code is still in memory.

- Without the mod: the code is unloaded; in the game, that is a crash.
- With the mod: the code stays in place, no crash.
