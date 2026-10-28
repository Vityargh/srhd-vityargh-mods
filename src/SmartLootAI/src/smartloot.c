#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* Exact Delphi x86 layouts from pakompom/SpaceRangersHD_decomp, target 2026-10-06.
   No object lifetime, damage, item flags, inventory or save format changes. */
#define PTR(o,n) (*(void**)((char*)(o)+(n)))
#define INT(o,n) (*(int*)((char*)(o)+(n)))
#define BYTE(o,n) (*(unsigned char*)((char*)(o)+(n)))
#define FLOAT(o,n) (*(float*)((char*)(o)+(n)))
typedef int (__cdecl *RegCall)(void*,void*);
typedef int (WINAPI *ReallocWide)(wchar_t**,const wchar_t*);
static ReallocWide reallocateWide;
typedef void (WINAPI *LogFn)(const wchar_t*,const wchar_t*);
typedef struct { unsigned size,api; HMODULE game; const wchar_t *root,*exe,*plugin,*config; LogFn log; void *b,*i,*s; } Host;
typedef struct { unsigned size,api; wchar_t id[64],version[32],description[160]; uint64_t capabilities; } Info;
typedef struct { unsigned va,len; unsigned char expected[16]; void *entry,*original,*replacement; DWORD protect; } Patch;
#include "signatures.h"
typedef struct { void *item; int score; unsigned id; int priority; } Candidate;
typedef BOOL (WINAPI *ConfigBool)(const wchar_t*,const wchar_t*,const wchar_t*,BOOL);
static int activeJunkCleanup=1,cleanupMinerals=1,cleanupTradeGoods=1,cleanupOnlyOutsideCombat=1;
static RegCall usable, rangeOf, safeDetonate, inSpace, relation, standard, valueOf, player, infoOf;
static unsigned char *arena;
static unsigned used;
static LogFn logger;
static int initialized;
static void interruptJanitor(void *ship);
static void *preferredJanitorItem(void *ship);
static int ordinary(void *item);
static int isJanitor(void *ship);
static int serviceMember(void *ship);
static const wchar_t pluginId[] = L"SmartLootAI";
static void Log(const wchar_t *text) { if(logger) logger(pluginId,text); }
#define ARENA_SIZE 8192u
static int arenaOverflow;
static void *allocCode(unsigned n) { void *r; if(used+n>ARENA_SIZE-16){arenaOverflow=1;return arena+ARENA_SIZE-16;} r=arena+used; used=(used+n+15)&~15u; return r; }
static void rel32(unsigned char *p, unsigned char opcode, void *target) {
    p[0]=opcode; *(int*)(p+1)=(char*)target-(char*)p-5;
}
/* cdecl -> Delphi register ABI (Self=EAX, argument=EDX), normalize byte results. */
static RegCall bridge(void *target,int booleanResult) {
    unsigned char *p=allocCode(32); unsigned n=0;
    unsigned char load[]={0x8b,0x44,0x24,0x04,0x8b,0x54,0x24,0x08};
    memcpy(p,load,8); n=8; rel32(p+n,0xe8,target); n+=5;
    if(booleanResult) { p[n++]=0x0f;p[n++]=0xb6;p[n++]=0xc0; }
    p[n]=0xc3; return (RegCall)p;
}
static int listCount(void *list) { return list ? INT(list,8) : 0; }
static void *listAt(void *list,int i) { return ((void**)PTR(list,4))[i]; }
static int contains(void *list,void *object) {
    int i;for(i=0;i<listCount(list);++i) if(listAt(list,i)==object) return 1;return 0;
}
static int readBool(const Host *host,const wchar_t *key) {
    if(host->size>=36 && host->b) return ((ConfigBool)host->b)(host->config,L"SmartLootAI",key,TRUE)!=FALSE;
    return !host->config || GetPrivateProfileIntW(L"SmartLootAI",key,1,host->config)!=0;
}
static void readSettings(const Host *host) {
    activeJunkCleanup=readBool(host,L"ActiveJunkCleanup");
    cleanupMinerals=readBool(host,L"CleanupMinerals");
    cleanupTradeGoods=readBool(host,L"CleanupTradeGoods");
    cleanupOnlyOutsideCombat=readBool(host,L"CleanupOnlyOutsideCombat");
}
static int quest(void *item) { return PTR(item,0x2c)!=0 || BYTE(item,0x34)!=0; }
/* CreateQuestItem also creates ordinary ambient salvage. These two exact
   ShuBounty configurations are escape pods, not a general quest exemption. */
static int capsule(void *item) {
    const wchar_t *key;void *binding,*script;
    if(BYTE(item,0xc)!=70 || BYTE(item,0x34))return 0;
    key=PTR(item,0x38);if(!key)return 0;
    if(wcscmp(key,L"ShuBounty_EscapePod") && wcscmp(key,L"ShuBounty_EscapePodStations"))return 0;
    binding=PTR(item,0x2c);
    /* Named/unsellable bindings can belong to a mission using the same image. */
    if(binding && (PTR(binding,0x28)!=item || !BYTE(binding,0x2c) ||
       (PTR(binding,4) && *(wchar_t*)PTR(binding,4))))return 0;
    if(binding){script=PTR(binding,0x60);if(!script || !PTR(script,8) ||
        wcscmp(PTR(script,8),L"Script.Mod_ShuBounty"))return 0;}
    return 1;
}
static int stationSalvage(void *item) {
    static const wchar_t *keys[]={L"Usl_30",L"Usl_63",L"Usl_70",L"Usl_19",L"Usl_Cactus",
        L"ShuBounty_ElementPamyati",L"Usl_40",L"Usl_PTQn66"};
    const wchar_t *key;unsigned i;
    if(BYTE(item,0xc)!=70 || quest(item) || PTR(item,0x60) || BYTE(item,0x65) ||
       (PTR(item,0x24) && *(wchar_t*)PTR(item,0x24)))return 0;
    key=PTR(item,0x38);if(!key)return 0;
    for(i=0;i<sizeof(keys)/sizeof(keys[0]);++i)if(!wcscmp(key,keys[i]))return 1;
    return 0;
}
static int special(void *item) {
    int t=BYTE(item,0xc);
    if(quest(item))return 1;
    if(t>=43 && t<=67) return !ordinary(item);
    return t>=8 && t!=42 && t!=72;
}
static int protectedSpecial(void *item) {
    int weight=INT(item,0x18),cost=valueOf(item,0);
    return special(item) && (weight<=0 || cost<0 || (int64_t)cost >= (int64_t)weight*10000);
}
static int overflowTier(void *item) {
    if(protectedSpecial(item))return -1;
    if(capsule(item))return 0;
    if(stationSalvage(item))return 1;
    if(ordinary(item))return 2;
    if(special(item))return 3;
    return -1;
}
static int valuable(void *item) { int t=BYTE(item,0xc); return t>=8 && t!=42 && t!=72; }
static int named(void *s) { return s && *(wchar_t*)s; }
static int ordinary(void *item) {
    int t=BYTE(item,0xc);
    if(t<43 || t>67 || quest(item)) return 0;
    if(named(PTR(item,0x24)) || named(PTR(item,0x3c)) || INT(item,0x50) || INT(item,0x54) || listCount(PTR(item,0x58))) return 0;
    return standard(item,0);
}
static int compare(const void *a,const void *b) {
    const Candidate *x=a,*y=b;
    if(x->priority!=y->priority) return x->priority<y->priority ? -1:1;
    if(x->score!=y->score) return x->score<y->score ? -1:1;
    return x->id<y->id ? -1 : x->id!=y->id;
}
static int fuller(void *x,void *y) {
    return !y || INT(x,0x60)>INT(y,0x60) || (INT(x,0x60)==INT(y,0x60) && (unsigned)INT(x,8)<(unsigned)INT(y,8));
}
#include "policy_v2.h"
#include "fire_guard.h"
static int combat(void *ship,void *star) {
    void *ships=PTR(star,0x2c); int i,j,n=listCount(ships);
    if(BYTE(star,0x48)) return 1;
    for(i=0;i<n;++i) {
        void *x=listAt(ships,i);
        if(x==ship || !inSpace(x,0)) continue;
        if(PTR(ship,0x3f4)==x || PTR(x,0x3f4)==ship || relation(ship,x)<=1) return 1;
        /* Real attacks can exist without EnemyShip, hostile relations or Star.Battle. */
        for(j=0;j<BYTE(x,0x12c) && j<5;++j) {
            void *w=PTR(x,0x118+4*j);
            if(w && PTR(w,0x70)==ship) return 1;
        }
    }
    return 0;
}
/* Any hostile in reach of any usable gun reserves the ship for combat. */
static int enemyInRange(void *ship,void *star,int radius) {
    void *ships=PTR(star,0x2c);int i,j,k;
    for(i=0;i<listCount(ships);++i) {
        void *x=listAt(ships,i);int enemy=0;double dx,dy;
        if(x==ship || !inSpace(x,0))continue;
        if(PTR(ship,0x3f4)==x || PTR(x,0x3f4)==ship || relation(ship,x)<=1)enemy=1;
        for(j=0;!enemy && j<BYTE(x,0x12c) && j<5;++j){void *w=PTR(x,0x118+4*j);if(w && PTR(w,0x70)==ship)enemy=1;}
        for(k=0;!enemy && k<BYTE(ship,0x12c) && k<5;++k){void *w=PTR(ship,0x118+4*k);if(w && PTR(w,0x70)==x)enemy=1;}
        if(!enemy)continue;
        dx=(double)FLOAT(ship,0x14)-FLOAT(x,0x14);dy=(double)FLOAT(ship,0x18)-FLOAT(x,0x18);
        if(dx*dx+dy*dy<=(double)radius*radius)return 1;
    }
    return 0;
}
/* Called only after the original assignment routine, before any shots occur. */
static void __cdecl afterAssign(void *ship) {
    void *star,*items,*weapons[5]; Candidate *candidates;
    if(serviceMember(ship)){int si;for(si=0;si<BYTE(ship,0x12c) && si<5;++si){void *sw=PTR(ship,0x118+4*si);if(sw)PTR(sw,0x70)=0;}return;}
    int count,i,j,n,candidateCount=0,eligible=0,realTarget=0;
    if(ship==(void*)player(0,0) || !(star=PTR(ship,0x24)) || !inSpace(ship,0)) return;
    count=BYTE(ship,0x12c); if(count<1 || count>5) return;
    items=PTR(star,0x30); n=listCount(items); if(n<1 || n>100000) return;
    /* Fail closed if explicit script targets survived the stock routine. */
    for(j=0;j<count;++j) {
        void *target;
        weapons[j]=PTR(ship,0x118+j*4); if(!weapons[j]) return;
        target=PTR(weapons[j],0x70);
        if(target) for(i=0;i<n;++i) if(listAt(items,i)==target && BYTE(target,0xc)!=42) { PTR(weapons[j],0x70)=0; break; }
        target=PTR(weapons[j],0x70);
        if(target && !contains(PTR(star,0x28),target)) realTarget=1;
    }
    if(realTarget || combat(ship,star)) {
        interruptJanitor(ship);
    }
    /* Active cleanup may replace asteroid mining, never a ship/missile target.
       Cache weapon eligibility/range once, before constructing item candidates. */
    { int canClean[5]={0,0,0,0,0},ranges[5],maxRange=0;
    for(j=0;j<count;++j) {
        void *w=weapons[j],*target=PTR(w,0x70),*wi;int shot;
        if(!usable(ship,w)) continue;
        ranges[j]=rangeOf(ship,w);if(ranges[j]<0) continue;
        if(ranges[j]>maxRange)maxRange=ranges[j];
        if(target && (!activeJunkCleanup || !contains(PTR(star,0x28),target))) continue;
        wi=(void*)infoOf(w,0);if(!wi) continue;
        shot=BYTE(wi,0x35);if(shot>=5 && shot<=7) continue;
        if(enemyInRange(ship,star,ranges[j]))continue;
        canClean[j]=1;++eligible;
    }
    if(!eligible) return;
    candidates=collectCandidates(ship,items,n,&candidateCount);if(!candidates)return;
    { void *preferred=preferredJanitorItem(ship);for(i=0;i<candidateCount;++i)if(candidates[i].item==preferred)candidates[i].priority=-1; }
    if(candidateCount>1)qsort(candidates,candidateCount,sizeof(Candidate),compare);
    /* At most five linear range passes. No per-item fuel scan, no background loop. */
    for(j=0;j<count;++j) {
        void *w=weapons[j];if(!canClean[j])continue;
        for(i=0;i<candidateCount;++i) {
            void *item=candidates[i].item;int k;float dx,dy;
            for(k=0;k<j;++k) if(PTR(weapons[k],0x70)==item) break;
            if(k<j) continue;
            dx=FLOAT(ship,0x14)-FLOAT(item,0x10);dy=FLOAT(ship,0x18)-FLOAT(item,0x14);
            if(dx*dx+dy*dy>(double)ranges[j]*ranges[j] || !safeDetonate(ship,item)) continue;
            PTR(w,0x70)=item;break;
        }
    }
    free(candidates);
    }
}
#include "janitors.h"
#include "service_api.h"
#include "service_hooks.h"
/* Verifies every code site the plugin calls or whose data it reads, in memory.
   Header-only differences (4GB/LAA flag, timestamps) no longer block the plugin;
   any change to the actual code still does. Returns the index of the first mismatch or -1. */
static int verifyAnchors(uintptr_t delta) {
    unsigned i;
    for(i=0;i<ANCHOR_COUNT;++i) if(memcmp((void*)(anchors[i].va+delta),anchors[i].b,16)) return (int)i;
    return -1;
}
__declspec(dllexport) BOOL WINAPI XenoPlugin_Query(Info *info) {
    if(!info || info->size<520) return FALSE;
    info->api=1; wcscpy(info->id,pluginId);wcscpy(info->version,L"0.9");
    wcscpy(info->description,L"Smart loot targeting: immediate cleanup, special value protection, nearby enemies first");
    if(info->size>=sizeof(Info)) info->capabilities=0;
    return TRUE;
}
__declspec(dllexport) DWORD WINAPI XenoPlugin_Initialize(const Host *host) {
    unsigned i; int bad; uintptr_t delta;
    if(!host || host->size<32 || host->api<1 || !host->game) return ERROR_INVALID_PARAMETER;
    if(initialized) return ERROR_SUCCESS;
    logger=host->log;
    delta=(uintptr_t)host->game-0x400000u;
    if((bad=verifyAnchors(delta))>=0) {
        static const wchar_t head[]=L"BLOCKED: game code differs at 0x",tail[]=L" (unsupported Rangers.exe version); no hooks installed.";
        wchar_t msg[160];unsigned k,va=anchors[bad].va;
        wcscpy(msg,head);for(k=0;k<8;++k)msg[32+k]=L"0123456789ABCDEF"[(va>>(28-4*k))&15];msg[40]=0;wcscat(msg,tail);
        Log(msg);return ERROR_REVISION_MISMATCH;
    }
    if(memcmp((void*)(0x6e2560u+delta),moveEntry,sizeof(moveEntry)) || memcmp((void*)(0x6e2534u+delta),noneEntry,sizeof(noneEntry)) ||
       memcmp((void*)(0x738034u+delta),spawnEntry,sizeof(spawnEntry)) ||
       memcmp((void*)(0x4161c0u+delta),listAddEntry,sizeof(listAddEntry)) ||
       memcmp((void*)(0x6e2968u+delta),takeoffEntry,sizeof(takeoffEntry))) {
        Log(L"BLOCKED: OrderMove/OrderNone entry changed; no hooks installed.");return ERROR_REVISION_MISMATCH;
    }
    for(i=0;i<PATCH_COUNT;++i) { Patch *p=patches+i;p->entry=(void*)(p->va+delta);if(memcmp(p->entry,p->expected,p->len)) { Log(L"BLOCKED: hook bytes changed by another plugin; no hooks installed.");return ERROR_REVISION_MISMATCH; } }
    readSettings(host);
    explosionRadiusSquared=*(int*)(0x81bb2cu+delta);
    reallocateWide=(ReallocWide)GetProcAddress(LoadLibraryW(L"oleaut32.dll"),"SysReAllocString");
    if(!reallocateWide)return ERROR_PROC_NOT_FOUND;
    arena=VirtualAlloc(0,ARENA_SIZE,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);if(!arena) return GetLastError();
    for(i=0;i<PATCH_COUNT;++i) {
        Patch *p=patches+i;unsigned char *t=allocCode(32);
        memcpy(t,p->entry,p->len);rel32(t+p->len,0xe9,(char*)p->entry+p->len);p->original=t;
        if(i==16){p->replacement=emitServiceGate(t,1,0,0);}
        else if(i==17){p->replacement=emitServiceGate(t,0,0,12);}
        else if(i==18){p->replacement=emitServiceGate(t,0,0,0);}
        else if(i==12){p->replacement=emitServiceGate(t,1,0,4);}
        else if(i==13){p->replacement=emitServiceGate(t,0,0,0);}
        else if(i==14 || i==15){p->replacement=emitServiceGate(t,0,1000000,0);}
        else if(i==11) {p->replacement=emitFireGuard(t);}
        else if(i<7 || i>=9) {
            unsigned char *w=allocCode(64); unsigned n=0;
            w[n++]=0x50;rel32(w+n,0xe8,t);n+=5;w[n++]=0x58;w[n++]=0x9c;w[n++]=0x60;w[n++]=0xfc;w[n++]=0x50;
            rel32(w+n,0xe8,i<7 ? afterAssign:onStarDay);n+=5;w[n++]=0x83;w[n++]=0xc4;w[n++]=4;w[n++]=0x61;w[n++]=0x9d;w[n++]=0xc3;
            p->replacement=w;
        } else if(i==7) { unsigned char *w=allocCode(16);w[0]=0x31;w[1]=0xc0;w[2]=0xc3;p->replacement=w; }
        else {
            unsigned char *w=allocCode(64);unsigned n=0;
            void *callbacks[]={beforeMovement,onStarDay,onStarDay};
            w[n++]=0x9c;w[n++]=0x60;w[n++]=0xfc;w[n++]=0x50;
            rel32(w+n,0xe8,callbacks[i-8]);n+=5;w[n++]=0x83;w[n++]=0xc4;w[n++]=4;w[n++]=0x61;w[n++]=0x9d;
            rel32(w+n,0xe9,t);p->replacement=w;
        }
    }
    usable=bridge((void*)(0x6d6768u+delta),1);rangeOf=bridge((void*)(0x6d9f3cu+delta),0);
    safeDetonate=bridge(patches[7].original,1);inSpace=bridge((void*)(0x6cbf48u+delta),1);
    relation=bridge((void*)(0x6d42a4u+delta),1);valueOf=bridge((void*)(0x6f5fb4u+delta),0);player=bridge((void*)(0x747300u+delta),0);
    noneOrder=bridge((void*)(0x6e2534u+delta),0);moveOrder=bridgeMove((void*)(0x6e2560u+delta));galaxySlot=(void**)(0x81d2a0u+delta);
    spawnJanitor=bridge((void*)(0x738034u+delta),0);
    addShip=bridge((void*)(0x4161c0u+delta),0);takeoffOrder=bridge((void*)(0x6e2968u+delta),0);
    /* Virtual HasStandardStats must dispatch per equipment subtype. */
    { unsigned char *b=allocCode(32);unsigned char code[]={0x8b,0x44,0x24,4,0x8b,0x08,0xff,0x51,0x3c,0x0f,0xb6,0xc0,0xc3};memcpy(b,code,sizeof(code));standard=(RegCall)b; }
    { unsigned char *b=allocCode(32);unsigned char code[]={0x8b,0x44,0x24,4,0x8b,0x08,0xff,0x51,0x50,0xc3};memcpy(b,code,sizeof(code));infoOf=(RegCall)b; }
    if(arenaOverflow) { Log(L"BLOCKED: code arena too small; no hooks installed.");return ERROR_NOT_ENOUGH_MEMORY; }
    for(i=0;i<PATCH_COUNT;++i) if(!VirtualProtect(patches[i].entry,patches[i].len,PAGE_EXECUTE_READWRITE,&patches[i].protect)) {
        unsigned k;DWORD old;for(k=0;k<i;++k) VirtualProtect(patches[k].entry,patches[k].len,patches[k].protect,&old);
        Log(L"BLOCKED: VirtualProtect failed before mutation.");return ERROR_ACCESS_DENIED;
    }
    /* Several hooks share a page (12/13, 14/15): write every hook first, then restore
       protections in reverse order so a shared page is not re-protected mid-install. */
    for(i=0;i<PATCH_COUNT;++i) { Patch *p=patches+i;rel32(p->entry,0xe9,p->replacement);memset((char*)p->entry+5,0x90,p->len-5); }
    for(i=PATCH_COUNT;i-->0;) { DWORD old;VirtualProtect(patches[i].entry,patches[i].len,patches[i].protect,&old); }
    serviceFire=bridgeServiceFire((void*)(0x6d1b2cu+delta));
    FlushInstructionCache(GetCurrentProcess(),0,0);initialized=1;
    Log(L"ACTIVE 0.9: 19 verified hooks; immediate cleanup; special-only value protection; nearby enemies first.");
    Log(activeJunkCleanup?L"ActiveJunkCleanup=1":L"ActiveJunkCleanup=0");
    Log(cleanupMinerals?L"CleanupMinerals=1":L"CleanupMinerals=0");
    Log(cleanupTradeGoods?L"CleanupTradeGoods=1":L"CleanupTradeGoods=0");
    Log(L"Combat policy: cleanup allowed only with no enemy in any usable weapon range; legacy CleanupOnlyOutsideCombat ignored.");
    return ERROR_SUCCESS;
}
