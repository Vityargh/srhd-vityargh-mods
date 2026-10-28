#include "smartloot.c"
#include <assert.h>
#include <stdio.h>
static unsigned char testShip[0x1000],testStar[0x200],testWeapon[0x100],testOther[0x1000],testInfo[0x80];
static unsigned char itemData[100][0x100],testItems[0x20],testShips[0x20];
static unsigned char otherWeapon[0x100],testAsteroids[0x20];
static void *asteroidPointers[1];
static int secondRange;
static void *itemPointers[100],*shipPointers[2];
static int safe=1,weaponRange=200,hostile=0,standardFlag=1,isPlayer=0,rangeCalls=0,standardCalls=0;
static int yes(void*a,void*b){return 1;}
static int stdStats(void*a,void*b){++standardCalls;return standardFlag;}
static int rangeTest(void*a,void*b){++rangeCalls;return b==otherWeapon && secondRange>0?secondRange:weaponRange;}
static int safeTest(void*a,void*b){return safe;}
static int relationTest(void*a,void*b){return hostile?0:4;}
static int valueTest(void*a,void*b){return INT(a,0x20);}
static int infoTest(void*a,void*b){return (int)testInfo;}
static int playerTest(void*a,void*b){return isPlayer?(int)testShip:0;}
static BOOL WINAPI configTest(const wchar_t *path,const wchar_t *section,const wchar_t *key,BOOL fallback) {
    assert(!wcscmp(section,L"SmartLootAI"));return wcscmp(key,L"CleanupMinerals")!=0;
}
static void reset(int n) {
    int i;memset(testShip,0,sizeof(testShip));memset(testStar,0,sizeof(testStar));memset(testWeapon,0,sizeof(testWeapon));memset(testOther,0,sizeof(testOther));memset(testInfo,0,sizeof(testInfo));memset(itemData,0,sizeof(itemData));memset(testItems,0,sizeof(testItems));memset(testShips,0,sizeof(testShips));
    PTR(testShip,0x24)=testStar;PTR(testShip,0x118)=testWeapon;BYTE(testShip,0x12c)=1;
    PTR(testStar,0x30)=testItems;PTR(testStar,0x2c)=testShips;PTR(testItems,4)=itemPointers;INT(testItems,8)=n;
    PTR(testShips,4)=shipPointers;INT(testShips,8)=1;shipPointers[0]=testShip;
    for(i=0;i<n;++i) {itemPointers[i]=itemData[i];INT(itemData[i],8)=i+1;BYTE(itemData[i],0xc)=44;INT(itemData[i],0x18)=1;INT(itemData[i],0x20)=100+i;}
    memset(otherWeapon,0,sizeof(otherWeapon));memset(testAsteroids,0,sizeof(testAsteroids));
    safe=1;weaponRange=200;secondRange=0;hostile=0;standardFlag=1;isPlayer=0;rangeCalls=0;standardCalls=0;
    activeJunkCleanup=cleanupMinerals=cleanupTradeGoods=cleanupOnlyOutsideCombat=1;
    usable=yes;rangeOf=rangeTest;safeDetonate=safeTest;inSpace=yes;relation=relationTest;standard=stdStats;valueOf=valueTest;player=playerTest;infoOf=infoTest;
}
static void *target(void){return PTR(testWeapon,0x70);}
int main(void) {
    int i;Info qi;Host bad;
    reset(2);BYTE(itemData[0],0xc)=4;BYTE(itemData[1],0xc)=4;INT(testShips,8)=2;shipPointers[1]=testOther;hostile=1;FLOAT(testOther,0x14)=250;BYTE(testShip,0x12c)=2;PTR(testShip,0x11c)=otherWeapon;secondRange=300;afterAssign(testShip);assert(!target() && !PTR(otherWeapon,0x70));
    FLOAT(testOther,0x14)=301;afterAssign(testShip);assert(target() && PTR(otherWeapon,0x70));
    /* System battle alone is allowed; nearby hostile reserves every gun. */
    reset(1);BYTE(itemData[0],0xc)=4;BYTE(testStar,0x48)=1;INT(testShips,8)=2;shipPointers[1]=testOther;hostile=1;FLOAT(testOther,0x14)=201;afterAssign(testShip);assert(target()==itemData[0]);
    PTR(testWeapon,0x70)=0;FLOAT(testOther,0x14)=200;afterAssign(testShip);assert(!target());
    FLOAT(testOther,0x14)=199;afterAssign(testShip);assert(!target());
    FLOAT(testOther,0x14)=201;FLOAT(itemData[0],0x10)=201;afterAssign(testShip);assert(!target());
    reset(1);BYTE(itemData[0],0xc)=4;BYTE(testStar,0x48)=1;INT(testShips,8)=2;shipPointers[1]=testOther;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0x34)=1;INT(itemData[0],0x20)=9999;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0x34)=1;INT(itemData[0],0x20)=10000;afterAssign(testShip);assert(!target());
    reset(1);BYTE(itemData[0],0x34)=1;INT(itemData[0],0x20)=9999;BYTE(testStar,0x48)=1;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0x34)=1;INT(itemData[0],0x18)=2;INT(itemData[0],0x20)=19999;assert(overflowTier(itemData[0])==3);INT(itemData[0],0x20)=20000;assert(overflowTier(itemData[0])==-1);
    reset(1);BYTE(itemData[0],0xc)=4;INT(itemData[0],0x20)=10000;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=4;INT(itemData[0],0x20)=9999;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=4;INT(itemData[0],0x18)=2;INT(itemData[0],0x20)=19999;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=4;INT(itemData[0],0x18)=2;INT(itemData[0],0x20)=20000;afterAssign(testShip);assert(target()==itemData[0]);
    reset(51);for(i=0;i<51;++i)INT(itemData[i],0x20)=10000;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=4;INT(itemData[0],0x18)=0;afterAssign(testShip);assert(target()==itemData[0]);
    reset(50);afterAssign(testShip);assert(target()==itemData[0]);
    reset(51);afterAssign(testShip);assert(target()==itemData[0]);
    reset(51);INT(itemData[0],0x50)=1;afterAssign(testShip);assert(target()==itemData[1]);
    reset(51);PTR(itemData[0],0x24)=L"Unique";afterAssign(testShip);assert(target()==itemData[1]);
    reset(51);standardFlag=0;afterAssign(testShip);assert(target()==itemData[0]);
    reset(51);for(i=0;i<51;++i)BYTE(itemData[i],0xc)=71;afterAssign(testShip);assert(target()==itemData[0]);
    reset(51);for(i=0;i<51;++i)BYTE(itemData[i],0xc)=69;afterAssign(testShip);assert(target()==itemData[0]);
    reset(51);for(i=0;i<51;++i)BYTE(itemData[i],0xc)=24;afterAssign(testShip);assert(target()==itemData[0]);
    for(i=0;i<8;++i) {reset(1);BYTE(itemData[0],0xc)=i;afterAssign(testShip);assert(target()==itemData[0]);}
    reset(3);for(i=0;i<3;++i){BYTE(itemData[i],0xc)=72;INT(itemData[i],0x60)=10+i;}afterAssign(testShip);assert(!target());
    reset(4);for(i=0;i<4;++i){BYTE(itemData[i],0xc)=72;INT(itemData[i],0x60)=40-i*10;}afterAssign(testShip);assert(target()==itemData[3]);
    reset(6);for(i=0;i<6;++i){BYTE(itemData[i],0xc)=72;INT(itemData[i],0x60)=i*10;}afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=4;PTR(itemData[0],0x2c)=itemData[1];afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=4;BYTE(itemData[0],0x34)=1;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=42;afterAssign(testShip);assert(!target());
    reset(1);BYTE(itemData[0],0xc)=4;PTR(testWeapon,0x70)=testOther;afterAssign(testShip);assert(target()==testOther);
    reset(1);BYTE(itemData[0],0xc)=4;BYTE(testStar,0x48)=1;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=4;INT(testShips,8)=2;shipPointers[1]=testOther;hostile=1;afterAssign(testShip);assert(!target());
    reset(1);BYTE(itemData[0],0xc)=4;INT(testShips,8)=2;shipPointers[1]=testOther;PTR(testOther,0x3f4)=testShip;afterAssign(testShip);assert(!target());
    reset(1);BYTE(itemData[0],0xc)=4;safe=0;afterAssign(testShip);assert(!target());
    reset(1);BYTE(itemData[0],0xc)=4;FLOAT(itemData[0],0x10)=300;afterAssign(testShip);assert(!target());
    reset(1);BYTE(itemData[0],0xc)=4;BYTE(testInfo,0x35)=5;afterAssign(testShip);assert(!target());
    reset(51);PTR(testWeapon,0x70)=itemData[0];BYTE(testStar,0x48)=1;afterAssign(testShip);assert(target()==itemData[0]);
    /* v1.1: active search below native thresholds and independent switches. */
    reset(1);BYTE(itemData[0],0xc)=4;afterAssign(testShip);assert(target()==itemData[0] && standardCalls==0 && rangeCalls==1);
    reset(1);BYTE(itemData[0],0xc)=4;activeJunkCleanup=0;afterAssign(testShip);assert(!target());
    reset(51);activeJunkCleanup=0;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=4;cleanupMinerals=0;afterAssign(testShip);assert(!target());
    reset(1);BYTE(itemData[0],0xc)=0;cleanupMinerals=0;afterAssign(testShip);assert(target()==itemData[0]);
    for(i=0;i<8;++i) if(i!=4) {reset(1);BYTE(itemData[0],0xc)=i;cleanupTradeGoods=0;afterAssign(testShip);assert(!target());}
    reset(1);BYTE(itemData[0],0xc)=4;cleanupTradeGoods=0;afterAssign(testShip);assert(target()==itemData[0]);
    reset(4);activeJunkCleanup=0;for(i=0;i<4;++i)BYTE(itemData[i],0xc)=72;afterAssign(testShip);assert(!target());
    /* Replace an asteroid target only when active cleanup is enabled. */
    reset(1);BYTE(itemData[0],0xc)=4;PTR(testStar,0x28)=testAsteroids;PTR(testAsteroids,4)=asteroidPointers;INT(testAsteroids,8)=1;asteroidPointers[0]=testOther;PTR(testWeapon,0x70)=testOther;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=4;activeJunkCleanup=0;PTR(testStar,0x28)=testAsteroids;PTR(testAsteroids,4)=asteroidPointers;INT(testAsteroids,8)=1;asteroidPointers[0]=testOther;PTR(testWeapon,0x70)=testOther;afterAssign(testShip);assert(target()==testOther);
    /* A real attack with neutral relations, nil EnemyShip and Battle=0 blocks cleanup. */
    reset(1);BYTE(itemData[0],0xc)=4;INT(testShips,8)=2;shipPointers[1]=testOther;BYTE(testOther,0x12c)=1;PTR(testOther,0x118)=otherWeapon;PTR(otherWeapon,0x70)=testShip;afterAssign(testShip);assert(!target());
    cleanupOnlyOutsideCombat=0;afterAssign(testShip);assert(!target());
    FLOAT(testOther,0x14)=300;
    /* Never replace actual combat targets. */
    PTR(testWeapon,0x70)=testOther;afterAssign(testShip);assert(target()==testOther);
    /* Per-weapon priority: with restriction off, only the idle gun may clean. */
    BYTE(testShip,0x12c)=2;PTR(testShip,0x11c)=otherWeapon;PTR(otherWeapon,0x70)=0;
    afterAssign(testShip);assert(target()==testOther && PTR(otherWeapon,0x70)==itemData[0]);
    reset(2);BYTE(itemData[0],0xc)=4;BYTE(itemData[1],0xc)=0;BYTE(testShip,0x12c)=2;PTR(testShip,0x11c)=otherWeapon;
    afterAssign(testShip);assert(target()==itemData[0] && PTR(otherWeapon,0x70)==itemData[1] && rangeCalls==2);
    reset(4);for(i=0;i<4;++i){BYTE(itemData[i],0xc)=72;INT(itemData[i],0x60)=100;}afterAssign(testShip);assert(target()==itemData[3]);
    reset(1);BYTE(itemData[0],0xc)=4;BYTE(testStar,0x48)=1;cleanupOnlyOutsideCombat=0;afterAssign(testShip);assert(target()==itemData[0]);
    reset(1);BYTE(itemData[0],0xc)=4;isPlayer=1;PTR(testWeapon,0x70)=itemData[0];activeJunkCleanup=0;afterAssign(testShip);assert(target()==itemData[0]);
    for(i=5;i<=7;++i){reset(1);BYTE(itemData[0],0xc)=4;BYTE(testInfo,0x35)=i;afterAssign(testShip);assert(!target());}
    /* INI loading: defaults, all zeroes, each independent value, and Host API callback. */
    { Host h={0};const wchar_t *cfg=L"S:\\SR\\SmartLootAI\\test-settings.ini";
      h.size=sizeof(h);readSettings(&h);assert(activeJunkCleanup&&cleanupMinerals&&cleanupTradeGoods&&cleanupOnlyOutsideCombat);
      h.config=cfg;WritePrivateProfileStringW(L"SmartLootAI",L"ActiveJunkCleanup",L"0",cfg);WritePrivateProfileStringW(L"SmartLootAI",L"CleanupMinerals",L"0",cfg);WritePrivateProfileStringW(L"SmartLootAI",L"CleanupTradeGoods",L"0",cfg);WritePrivateProfileStringW(L"SmartLootAI",L"CleanupOnlyOutsideCombat",L"0",cfg);
      readSettings(&h);assert(!activeJunkCleanup&&!cleanupMinerals&&!cleanupTradeGoods&&!cleanupOnlyOutsideCombat);
      WritePrivateProfileStringW(L"SmartLootAI",L"CleanupMinerals",L"1",cfg);readSettings(&h);assert(!activeJunkCleanup&&cleanupMinerals&&!cleanupTradeGoods&&!cleanupOnlyOutsideCombat);
      h.b=(void*)configTest;readSettings(&h);assert(activeJunkCleanup&&!cleanupMinerals&&cleanupTradeGoods&&cleanupOnlyOutsideCombat);DeleteFileW(cfg);
    }
    /* Verify emitted cdecl -> Delphi ABI and byte return normalization on x86. */
    arena=VirtualAlloc(0,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);assert(arena);used=0;
    { unsigned char *f=allocCode(16);RegCall call;f[0]=0x01;f[1]=0xd0;f[2]=0xc3;call=bridge(f,0);assert(call((void*)12,(void*)30)==42);
      f[0]=0xb8;*(unsigned*)(f+1)=0x12340001;f[5]=0xc3;call=bridge(f,1);assert(call(0,0)==1); }
    memset(&qi,0,sizeof(qi));qi.size=sizeof(qi);assert(XenoPlugin_Query(&qi));assert(qi.api==1 && !wcscmp(qi.id,L"SmartLootAI"));
    memset(&bad,0,sizeof(bad));assert(XenoPlugin_Initialize(&bad)==ERROR_INVALID_PARAMETER);
    bad.size=sizeof(bad);bad.api=1;bad.game=GetModuleHandleW(0);assert(XenoPlugin_Initialize(&bad)==ERROR_REVISION_MISMATCH);
    puts("PASS v1.1: active target search; asteroid replacement; low item counts; 4 INI switches/defaults/Host API; actual incoming weapon targets; combat priority; player exclusion; no missiles; fuel 3/4/6; 50/51 overflow; protected/quest drops; range/safety; x86 ABI; unsupported EXE rejection.");
    return 0;
}
