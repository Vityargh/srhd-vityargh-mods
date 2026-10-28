/* Reproduces XenoNativeLoader calling XenoPlugin_Initialize on a mapped Rangers.exe image. */
#include <windows.h>
#include <stdio.h>
typedef void (WINAPI *LogFn)(const wchar_t*,const wchar_t*);
typedef struct { unsigned size,api; HMODULE game; const wchar_t *root,*exe,*plugin,*config; LogFn log; void *b,*i,*s; } Host;
typedef DWORD (WINAPI *InitFn)(const Host*);
static HMODULE dll;
static void WINAPI logger(const wchar_t *id,const wchar_t *t){ wprintf(L"[log] %ls\n",t); }
static LONG CALLBACK veh(EXCEPTION_POINTERS *e){
    DWORD_PTR ip=(DWORD_PTR)e->ExceptionRecord->ExceptionAddress;
    printf("EXCEPTION %08lx at %p (dll+0x%lx) access=%p\n",e->ExceptionRecord->ExceptionCode,(void*)ip,
        (unsigned long)(ip-(DWORD_PTR)dll),(void*)e->ExceptionRecord->ExceptionInformation[1]);
    fflush(stdout); ExitProcess(3); return 0;
}
int wmain(int argc,wchar_t**argv){
    const wchar_t *exe=argv[1],*plugin=argv[2],*ini=argv[3];
    HMODULE game=LoadLibraryExW(exe,NULL,DONT_RESOLVE_DLL_REFERENCES);
    printf("game=%p (need 00400000)\n",(void*)game);
    if(!game) return 2;
    dll=LoadLibraryW(plugin); printf("plugin=%p\n",(void*)dll); if(!dll) return 2;
    { typedef PVOID (WINAPI *AVEH)(ULONG,PVECTORED_EXCEPTION_HANDLER);
      ((AVEH)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"AddVectoredExceptionHandler"))(1,veh); }
    Host h={0}; h.size=sizeof(Host); h.api=1; h.game=game; h.exe=exe; h.plugin=plugin; h.config=ini; h.log=logger;
    InitFn init=(InitFn)GetProcAddress(dll,"XenoPlugin_Initialize");
    DWORD r=init(&h);
    printf("Initialize returned %lu\n",r);
    { static const unsigned va[]={0x5b7284,0x5a5880,0x59d248,0x5addc8,0x71931c,0x69dbc4,0x5b9e14,0x6d4098,0x6d1b2c,0x6e2e30,0x6e2828,0x6d6e00,0x6d6e6c,0x7535b8,0x6cd5e0,0x6d2c58};
      unsigned k,ok=0,ro=0; MEMORY_BASIC_INFORMATION m;
      for(k=0;k<sizeof(va)/sizeof(va[0]);++k){ unsigned char *e=(unsigned char*)game+(va[k]-0x400000);
        if(*e==0xE9) ++ok; else printf("hook %u at %x NOT installed (%02x)\n",k,va[k],*e);
        VirtualQuery(e,&m,sizeof m); if(!(m.Protect&(PAGE_READWRITE|PAGE_EXECUTE_READWRITE|PAGE_WRITECOPY|PAGE_EXECUTE_WRITECOPY))) ++ro; }
      printf("hooks installed %u/16, pages re-protected %u/16\n",ok,ro); if(ok!=16) return 4; }
    return r==0?0:1;
}
