/* ApplyCrashFix - keeps MatrixGame.dll (planetary battles engine) mapped for the whole session.
   Why: on "Apply" in the mod menu the game reloads mods in-process and FreeLibrary()s MatrixGame.dll,
   but the window MatrixGame created (RegisterClassExA/CreateWindowExA) survives. Its next message jumps
   into unmapped memory -> crash (verified in a crash dump: EIP inside unloaded MatrixGame.dll).
   Fix: pin the module (LdrAddRefDll PIN) as soon as it is loaded, so its window procedure stays valid.
   No game code is patched. */
#include <windows.h>
#include <wchar.h>
typedef void (WINAPI *LogFn)(const wchar_t*,const wchar_t*);
typedef struct { unsigned size,api; HMODULE game; const wchar_t *root,*exe,*plugin,*config; LogFn log; void *b,*i,*s; } Host;
typedef struct { unsigned size,api; wchar_t id[64],version[32],description[160]; unsigned long long capabilities; } Info;
typedef struct { USHORT Length, MaximumLength; PWSTR Buffer; } USTR;
typedef struct { ULONG Flags; const USTR *FullDllName; const USTR *BaseDllName; PVOID DllBase; ULONG SizeOfImage; } LdrNotifyData;
typedef VOID (CALLBACK *LdrNotifyFn)(ULONG reason, const LdrNotifyData *data, PVOID ctx);
typedef LONG (NTAPI *RegNotify)(ULONG, LdrNotifyFn, PVOID, PVOID*);
typedef LONG (NTAPI *AddRefDll)(ULONG, PVOID);
#define LDR_ADDREF_DLL_PIN 1
static const wchar_t pluginId[] = L"ApplyCrashFix";
static LogFn logger; static AddRefDll addRef; static PVOID cookie; static volatile LONG pinned;
static void Log(const wchar_t *t) { if (logger) logger(pluginId, t); }

static int isTarget(const USTR *n) {
    static const wchar_t name[] = L"MatrixGame.dll";
    return n && n->Buffer && n->Length == (sizeof(name) - sizeof(wchar_t)) && !_wcsnicmp(n->Buffer, name, n->Length / 2);
}
static void pin(PVOID base, const wchar_t *how) {
    wchar_t msg[160];
    LONG st = addRef ? addRef(LDR_ADDREF_DLL_PIN, base) : -1;
    if (st >= 0) InterlockedIncrement(&pinned);
    _snwprintf(msg, 159, L"MatrixGame.dll %ls at %p: %ls", how, base, st >= 0 ? L"pinned" : L"pin FAILED"); msg[159] = 0;
    Log(msg);
}
static VOID CALLBACK onDll(ULONG reason, const LdrNotifyData *d, PVOID ctx) {
#ifdef PIN_DEBUG
    if (d && d->BaseDllName) { wchar_t b[128]; _snwprintf(b, 127, L"notify r=%lu len=%u %.*ls", reason, d->BaseDllName->Length, d->BaseDllName->Length/2, d->BaseDllName->Buffer); b[127]=0; Log(b); }
#endif
    if (reason == 1 && d && isTarget(d->BaseDllName)) pin(d->DllBase, L"loaded");
}
__declspec(dllexport) BOOL WINAPI XenoPlugin_Query(Info *info) {
    if (!info || info->size < 520) return FALSE;
    info->api = 1; wcscpy(info->id, pluginId); wcscpy(info->version, L"1.0.0");
    wcscpy(info->description, L"Keeps the planetary battles engine loaded so Apply in the mod menu does not crash");
    if (info->size >= sizeof(Info)) info->capabilities = 0;
    return TRUE;
}
__declspec(dllexport) DWORD WINAPI XenoPlugin_Initialize(const Host *host) {
    HMODULE nt = GetModuleHandleW(L"ntdll.dll"); RegNotify reg; HMODULE m;
    if (!host || host->size < 32 || host->api < 1) return ERROR_INVALID_PARAMETER;
    logger = host->log;
    addRef = (AddRefDll)GetProcAddress(nt, "LdrAddRefDll");
    reg = (RegNotify)GetProcAddress(nt, "LdrRegisterDllNotification");
    if (!addRef || !reg) { Log(L"BLOCKED: ntdll loader API missing; nothing changed."); return ERROR_PROC_NOT_FOUND; }
    m = GetModuleHandleW(L"MatrixGame.dll");
    if (m) pin(m, L"already loaded");
    if (reg(0, onDll, NULL, &cookie) < 0) { Log(L"BLOCKED: cannot watch DLL loads."); return ERROR_GEN_FAILURE; }
    Log(L"ACTIVE 1.0.0: watching for MatrixGame.dll; no game code patched.");
    return ERROR_SUCCESS;
}
BOOL WINAPI DllMain(HINSTANCE h, DWORD r, LPVOID p) { if (r == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(h); return TRUE; }
