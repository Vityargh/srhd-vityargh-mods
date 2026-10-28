# ApplyCrashFix 1.0 — XenoNativeLoader plugin for Space Rangers HD: A War Apart

**Author:** Vityargh · **License:** same terms as XenoMods (PolyForm Noncommercial 1.0.0), free to include in XenoStable.

## The bug

Pressing **Apply** in the in-game mod manager makes the game reload mods inside the running process. During that reload it calls `FreeLibrary` on `MatrixGame.dll` (the planetary battles engine). The window that MatrixGame created earlier (`RegisterClassExA` / `CreateWindowExA`) is not destroyed, so the next message sent to it jumps into memory that is no longer mapped, and the game crashes.

Evidence from a real crash dump (`Rangers.exe`, build 2.1.2500, Steam):

- exception `0xC000041D` (fatal user callback exception) with `EIP = 0x18ADCC40`;
- that address is `MatrixGame.dll + 0xFCC40`, a module that had already been unloaded;
- the crash happens right after Apply, before any planetary battle starts.

## The fix

The plugin pins `MatrixGame.dll` for the lifetime of the process with `LdrAddRefDll(LDR_ADDREF_DLL_PIN)`:

- if MatrixGame is already loaded at plugin start, it is pinned immediately;
- otherwise the plugin registers `LdrRegisterDllNotification` and pins the module the moment it loads.

A pinned module ignores `FreeLibrary`, so the orphaned window procedure stays valid. The engine simply stays in memory until the game exits (it is loaded at the first planetary battle anyway).

**What it does not touch:** no game code or data is patched, no other module is pinned, saves and mod sets are unaffected. If the ntdll loader API is unavailable, the plugin logs `BLOCKED` and changes nothing.

## Verification

`tests/pin_test.c` reproduces the Apply sequence (load MatrixGame.dll → `FreeLibrary` → check whether the crash offset is still mapped), twice:

| Run | Result |
|---|---|
| without the plugin | `UNMAPPED (would crash)` both rounds, exit 1 |
| with the plugin | `pinned`, `STILL MAPPED (safe)` both rounds, exit 0 |

The DLL is a 4 KB x86 PE built with TinyCC from `src/applyfix.c` (`build.py`); a rebuild is byte-identical to the shipped binary (SHA-256 `4A3A1C21…4D35CC6C`).

## Install (players)

Copy `ApplyCrashFix` into `Mods\OtherMods\`, enable it in the mod manager and restart the game once. Requires XenoNativeLoader.

## Suggested integration into XenoStable

The fix is global and independent of any mod, so it fits XenoStable as `XenoNative\Plugins\ApplyCrashFix.XenoPlugin.dll` with its `.ini`. Plugin id `ApplyCrashFix`, Host API v1, exports `XenoPlugin_Query` / `XenoPlugin_Initialize` only. Rename freely if you prefer your own naming.
