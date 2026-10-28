/* Loads Rangers.exe and initializes several Xeno plugins in the given order, like XenoNativeLoader does.
   Usage: multi_harness.exe <Rangers.exe> <game root> <plugin.dll> <plugin.ini> [<plugin.dll> <plugin.ini> ...]
   Reports each Initialize result, then who owns the JMP at each of SmartLootAI's 16 hook sites.
   Supported layouts are recognised by Rangers.exe size (see src/prepare.py): Redux/Universe 2026-10-06 and
   Steam/GOG 2025-10-13. Ownership is only claimed when it can be proven: the JMP lands inside a loaded plugin,
   or lands in allocated memory whose stub calls/jumps into exactly one loaded plugin. Anything else is UNKNOWN. */
#include <windows.h>
#include <stdio.h>
typedef void (WINAPI *LogFn)(const wchar_t*,const wchar_t*);
typedef BOOL (WINAPI *CfgBool)(const wchar_t*,const wchar_t*,const wchar_t*,BOOL);
typedef struct { unsigned size,api; HMODULE game; const wchar_t *root,*exe,*plugin,*config; LogFn log; void *b,*i,*s; } Host;
typedef DWORD (WINAPI *InitFn)(const Host*);
static void WINAPI logger(const wchar_t *id,const wchar_t *t){ wprintf(L"  [%ls] %ls\n",id,t); }
static BOOL WINAPI cfgBool(const wchar_t *f,const wchar_t *s,const wchar_t *k,BOOL d){ return GetPrivateProfileIntW(s,k,d,f)!=0; }
/* Host API v1 configGetInt: (path, section, key, default, minimum, maximum). */
static int WINAPI cfgInt(const wchar_t *f,const wchar_t *s,const wchar_t *k,int d,int lo,int hi){
    int v=GetPrivateProfileIntW(s,k,d,f); return v<lo?lo:v>hi?hi:v; }
static DWORD WINAPI cfgStr(const wchar_t *f,const wchar_t *s,const wchar_t *k,const wchar_t *d,wchar_t *o,DWORD n){ return GetPrivateProfileStringW(s,k,d,o,n,f); }
static LONG CALLBACK veh(EXCEPTION_POINTERS *e){
    if(e->ExceptionRecord->ExceptionCode==0x406D1388) return EXCEPTION_CONTINUE_SEARCH; /* thread naming */
    printf("EXCEPTION %08lx at %p\n",e->ExceptionRecord->ExceptionCode,e->ExceptionRecord->ExceptionAddress);
    fflush(stdout); ExitProcess(3); return 0;
}
static const unsigned redux[16]={0x5b7284,0x5a5880,0x59d248,0x5addc8,0x71931c,0x69dbc4,0x5b9e14,0x6d4098,0x6d1b2c,0x6e2e30,0x6e2828,0x6d6e00,0x6d6e6c,0x7535b8,0x6cd5e0,0x6d2c58};
static const unsigned steam[16]={0x5cc88c,0x5b9168,0x5afd24,0x5c2570,0x759760,0x6ce5ac,0x5cfc84,0x70bbb0,0x7090b0,0x71ce50,0x71c75c,0x70f190,0x70f230,0x799f0c,0x704490,0x70a428};
static HMODULE mods[16]; static int n;
static int moduleOf(const void *p){ MEMORY_BASIC_INFORMATION m; int j;
    if(!VirtualQuery(p,&m,sizeof m)) return -1;
    for(j=0;j<n;++j) if((HMODULE)m.AllocationBase==mods[j]) return j;
    return -1; }
static int readable(const unsigned char *p,unsigned len){ MEMORY_BASIC_INFORMATION m;
    return VirtualQuery(p,&m,sizeof m) && m.State==MEM_COMMIT && !(m.Protect&(PAGE_NOACCESS|PAGE_GUARD)) &&
           p+len<=(unsigned char*)m.BaseAddress+m.RegionSize; }
/* Owner of a stub in allocated memory: the one plugin its rel32 CALL/JMP (or JMP [abs]) instructions reach.
   The scan stops at the stub's end (RET, or an unconditional JMP), so a neighbouring stub is never read.
   Returns -2 if none or more than one plugin is reached. */
static int stubOwner(const unsigned char *t){ int found=-2,k;
    if(!readable(t,64)) return -2;
    for(k=0;k<59;++k){ const unsigned char *dest=0; int w,end=0;
        if(t[k]==0xC3 || t[k]==0xC2) break;
        if(t[k]==0xE8||t[k]==0xE9){ dest=t+k+5+*(const int*)(t+k+1); end=t[k]==0xE9; }
        else if(t[k]==0xFF && t[k+1]==0x25 && readable(*(unsigned char* const*)(t+k+2),4)){ dest=**(unsigned char* const* const*)(t+k+2); end=1; }
        if(dest){ w=moduleOf(dest);
            if(w>=0){ if(found>=0 && found!=w) return -2; found=w; } }
        if(end) break;
        if(dest) k+=4; }
    return found; }
/* A stub with no plugin call of its own (e.g. a 3-byte replacement) is attributed to the plugin that owns other,
   proven stubs in the same VirtualAlloc allocation; otherwise it stays UNKNOWN. */
static void *allocBase(const void *p){ MEMORY_BASIC_INFORMATION m; return VirtualQuery(p,&m,sizeof m)?m.AllocationBase:0; }
static const wchar_t *base(const wchar_t *p){ const wchar_t *s=wcsrchr(p,L'\\'); return s?s+1:p; }
int wmain(int argc,wchar_t**argv){
    HMODULE game; int i; unsigned k; const unsigned *va=0; const wchar_t *names[16]; WIN32_FILE_ATTRIBUTE_DATA fa;
    if(argc<5) return 2;
    if(GetFileAttributesExW(argv[1],GetFileExInfoStandard,&fa)){
        if(fa.nFileSizeLow==4533760) { va=redux; wprintf(L"layout: Redux/Universe 2026-10-06\n"); }
        else if(fa.nFileSizeLow==4995584) { va=steam; wprintf(L"layout: Steam/GOG 2025-10-13\n"); } }
    if(!va){ printf("unsupported Rangers.exe layout: hook sites unknown\n"); return 2; }
    game=LoadLibraryExW(argv[1],NULL,DONT_RESOLVE_DLL_REFERENCES); if(!game) return 2;
    if((UINT_PTR)game!=0x400000) printf("note: image at %p, sites are rebased\n",(void*)game);
    { typedef PVOID (WINAPI *AVEH)(ULONG,PVECTORED_EXCEPTION_HANDLER);
      ((AVEH)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"AddVectoredExceptionHandler"))(1,veh); }
    for(i=3;i+1<argc && n<16;i+=2){
        Host h={0}; DWORD r; InitFn init; HMODULE p=LoadLibraryW(argv[i]);
        wprintf(L"== init %ls\n",base(argv[i]));
        if(!p){ printf("  LoadLibrary failed %lu\n",GetLastError()); continue; }
        names[n]=base(argv[i]); mods[n++]=p;
        h.size=sizeof h; h.api=1; h.game=game; h.root=argv[2]; h.exe=argv[1]; h.plugin=argv[i]; h.config=argv[i+1];
        h.log=logger; h.b=(void*)cfgBool; h.i=(void*)cfgInt; h.s=(void*)cfgStr;
        init=(InitFn)GetProcAddress(p,"XenoPlugin_Initialize");
        if(!init){ printf("  no XenoPlugin_Initialize\n"); continue; }
        r=init(&h); printf("  Initialize returned %lu\n",r); fflush(stdout);
    }
    { unsigned counts[17]={0},unknown=0,none=0; int owner[16]; void *alloc[16];
      for(k=0;k<16;++k){ unsigned char *e=(unsigned char*)game+(va[k]-0x400000); unsigned char *t;
        owner[k]=-3; alloc[k]=0; if(*e!=0xE9) continue;
        t=e+5+*(int*)(e+1); alloc[k]=allocBase(t);
        owner[k]=moduleOf(t); if(owner[k]<0) owner[k]=stubOwner(t); }
      for(k=0;k<16;++k) if(owner[k]==-2 && alloc[k]){ unsigned j; int via=-2;
          for(j=0;j<16;++j) if(j!=k && owner[j]>=0 && alloc[j]==alloc[k] && moduleOf(alloc[j])<0){
              if(via>=0 && via!=owner[j]){ via=-2; break; } via=owner[j]; }
          if(via>=0){ owner[k]=via; wprintf(L"  (site 0x%x: stub has no plugin call; same allocation as proven %ls stubs)\n",va[k],names[via]); } }
      for(k=0;k<16;++k){
        if(owner[k]==-3){ ++none; printf("  site 0x%x: no JMP\n",va[k]); }
        else if(owner[k]<0){ ++unknown; printf("  site 0x%x: owner UNKNOWN\n",va[k]); }
        else { ++counts[owner[k]]; wprintf(L"  site 0x%x: %ls\n",va[k],names[owner[k]]); } }
      printf("== SmartLootAI hook sites (16):");
      for(i=0;i<n;++i) if(counts[i]) wprintf(L" %ls %u,",names[i],counts[i]);
      printf(" UNKNOWN %u, no JMP %u\n",unknown,none); }
    return 0;
}
