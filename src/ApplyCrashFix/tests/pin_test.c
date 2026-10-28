/* Simulates the Apply sequence: load MatrixGame.dll, FreeLibrary it, check whether its code is still mapped.
   argv[1]=MatrixGame.dll path, argv[2]=plugin dll or "none". */
#include <windows.h>
#include <stdio.h>
typedef void (WINAPI *LogFn)(const wchar_t*,const wchar_t*);
typedef struct { unsigned size,api; HMODULE game; const wchar_t *root,*exe,*plugin,*config; LogFn log; void *b,*i,*s; } Host;
typedef DWORD (WINAPI *InitFn)(const Host*);
static void WINAPI logger(const wchar_t *id,const wchar_t *t){ wprintf(L"[%ls] %ls\n",id,t); }
int wmain(int argc, wchar_t **argv) {
    if (wcscmp(argv[2], L"none")) {
        HMODULE p = LoadLibraryW(argv[2]); Host h = {0}; DWORD r;
        if (!p) { puts("plugin load failed"); return 2; }
        h.size = sizeof h; h.api = 1; h.game = GetModuleHandleW(NULL); h.log = logger;
        r = ((InitFn)GetProcAddress(p, "XenoPlugin_Initialize"))(&h);
        printf("Initialize=%lu\n", r); if (r) return 3;
    }
    for (int round = 1; round <= 2; ++round) {
        HMODULE m = LoadLibraryExW(argv[1], NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        MEMORY_BASIC_INFORMATION mb; BYTE *code = (BYTE*)m + 0xFCC40; /* crash offset from the dump */
        printf("round %d: loaded at %p\n", round, (void*)m); if (!m) { printf("err=%lu\n", GetLastError()); return 2; }
        FreeLibrary(m);
        VirtualQuery(code, &mb, sizeof mb);
        printf("round %d: after FreeLibrary code page %s\n", round, mb.State == MEM_COMMIT ? "STILL MAPPED (safe)" : "UNMAPPED (would crash)");
        if (round == 2) return mb.State == MEM_COMMIT ? 0 : 1;
    }
    return 0;
}


