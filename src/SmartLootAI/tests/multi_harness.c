/* Loads Rangers.exe and initializes several Xeno plugins in the given order, like XenoNativeLoader does.
   Usage: multi_harness.exe <Rangers.exe> <game root> <plugin.dll> <plugin.ini> [<plugin.dll> <plugin.ini> ...]
   Reports each Initialize result, then which of SmartLootAI's 16 hook sites carry a JMP and whose JMP it is. */
#include <windows.h>
#include <stdio.h>
typedef void (WINAPI *LogFn)(const wchar_t*,const wchar_t*);
typedef BOOL (WINAPI *CfgBool)(const wchar_t*,const wchar_t*,const wchar_t*,BOOL);
typedef struct { unsigned size,api; HMODULE game; const wchar_t *root,*exe,*plugin,*config; LogFn log; void *b,*i,*s; } Host;
typedef DWORD (WINAPI *InitFn)(const Host*);
static void WINAPI logger(const wchar_t *id,const wchar_t *t){ wprintf(L"  [%ls] %ls\n",id,t); }
static BOOL WINAPI cfgBool(const wchar_t *f,const wchar_t *s,const wchar_t *k,BOOL d){ return GetPrivateProfileIntW(s,k,d,f)!=0; }
static int WINAPI cfgInt(const wchar_t *f,const wchar_t *s,const wchar_t *k,int d){ return GetPrivateProfileIntW(s,k,d,f); }
static DWORD WINAPI cfgStr(const wchar_t *f,const wchar_t *s,const wchar_t *k,const wchar_t *d,wchar_t *o,DWORD n){ return GetPrivateProfileStringW(s,k,d,o,n,f); }
static LONG CALLBACK veh(EXCEPTION_POINTERS *e){
    if(e->ExceptionRecord->ExceptionCode==0x406D1388) return EXCEPTION_CONTINUE_SEARCH; /* thread naming */
    printf("EXCEPTION %08lx at %p\n",e->ExceptionRecord->ExceptionCode,e->ExceptionRecord->ExceptionAddress);
    fflush(stdout); ExitProcess(3); return 0;
}
int wmain(int argc,wchar_t**argv){
    static const unsigned va[]={0x5b7284,0x5a5880,0x59d248,0x5addc8,0x71931c,0x69dbc4,0x5b9e14,0x6d4098,0x6d1b2c,0x6e2e30,0x6e2828,0x6d6e00,0x6d6e6c,0x7535b8,0x6cd5e0,0x6d2c58};
    HMODULE game,mods[16]; int n=0,i; unsigned k;
    if(argc<5) return 2;
    game=LoadLibraryExW(argv[1],NULL,DONT_RESOLVE_DLL_REFERENCES); if(!game) return 2;
    { typedef PVOID (WINAPI *AVEH)(ULONG,PVECTORED_EXCEPTION_HANDLER);
      ((AVEH)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"AddVectoredExceptionHandler"))(1,veh); }
    for(i=3;i+1<argc;i+=2){
        Host h={0}; DWORD r; InitFn init; HMODULE p=LoadLibraryW(argv[i]);
        wprintf(L"== init %ls\n",wcsrchr(argv[i],L'\\')?wcsrchr(argv[i],L'\\')+1:argv[i]);
        if(!p){ printf("  LoadLibrary failed %lu\n",GetLastError()); continue; }
        mods[n++]=p;
        h.size=sizeof h; h.api=1; h.game=game; h.root=argv[2]; h.exe=argv[1]; h.plugin=argv[i]; h.config=argv[i+1];
        h.log=logger; h.b=(void*)cfgBool; h.i=(void*)cfgInt; h.s=(void*)cfgStr;
        init=(InitFn)GetProcAddress(p,"XenoPlugin_Initialize");
        if(!init){ printf("  no XenoPlugin_Initialize\n"); continue; }
        r=init(&h); printf("  Initialize returned %lu\n",r); fflush(stdout);
    }
    { unsigned ours=0,theirs=0,none=0;
      for(k=0;k<sizeof(va)/sizeof(va[0]);++k){ unsigned char *e=(unsigned char*)game+(va[k]-0x400000);
        if(*e!=0xE9){ ++none; continue; }
        { unsigned char *t=e+5+*(int*)(e+1); int j,who=-1; MEMORY_BASIC_INFORMATION m; VirtualQuery(t,&m,sizeof m);
          for(j=0;j<n;++j) if((HMODULE)m.AllocationBase==mods[j]) who=j;
          if(who<0) ++ours; /* our jumps go to a VirtualAlloc'd arena */ else { ++theirs; printf("  hook site 0x%x is owned by plugin #%d\n",va[k],who); } } }
      printf("== SmartLootAI hook sites: JMP to arena %u, JMP into another plugin %u, untouched %u\n",ours,theirs,none); }
    return 0;
}
