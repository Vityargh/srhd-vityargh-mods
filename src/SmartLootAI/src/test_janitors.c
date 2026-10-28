#define main existing_policy_main
#include "test_policy.c"
#undef main
static unsigned char worldData[0x300],hullData[0x100],engineData[0x100],planetsData[0x20],planetData[0x200],bindingData[0x70],scriptData[0x70],rosterData[0x20];
static void *worldPointer,*planetPointers[1],*rosterPointers[2];
static int moves,nones,spawns,registrations,takeoffs;
static int fakeNone(void *ship,void *override){++nones;BYTE(ship,0x428)=0;PTR(ship,0x430)=0;return 0;}
static void fakeMove(void *ship,const LootPoint *p){++moves;BYTE(ship,0x428)=1;FLOAT(ship,0x434)=p->x;FLOAT(ship,0x438)=p->y;PTR(ship,0x430)=0;}
static int fakeSpawn(void *p,void *money){assert(p==planetData && (int)money==100);++spawns;BYTE(testOther,0x10)=4;INT(testOther,4)=120;PTR(testOther,0x24)=testStar;PTR(testOther,0x1c)=p;PTR(testOther,0xf8)=hullData;PTR(testOther,0x100)=engineData;rosterPointers[0]=testOther;INT(rosterData,8)=1;return (int)testOther;}
static int fakeAdd(void *list,void *ship){assert(list==testShips && ship==testOther);++registrations;shipPointers[1]=ship;INT(testShips,8)=2;return 1;}
static int fakeTakeoff(void *ship,void *unused){assert(ship==testOther);++takeoffs;BYTE(ship,0x428)=5;return 0;}
static int WINAPI fakeName(wchar_t **p,const wchar_t *name){*p=(wchar_t*)name;return 1;}
static void setup(int n,int janitor){
    clearJanitors();galaxySlot=0;reset(n);
    memset(worldData,0,sizeof(worldData));memset(hullData,0,sizeof(hullData));memset(engineData,0,sizeof(engineData));memset(planetsData,0,sizeof(planetsData));memset(planetData,0,sizeof(planetData));memset(bindingData,0,sizeof(bindingData));
    memset(rosterData,0,sizeof(rosterData));PTR(rosterData,4)=rosterPointers;PTR(planetData,0x114)=rosterData;
    janitorWorld=0;janitorLoad=janitorTurn=0;worldPointer=worldData;galaxySlot=&worldPointer;INT(worldData,0x4c)=100;INT(worldData,0x174)=1;
    INT(testStar,4)=20;INT(testShip,4)=21;BYTE(testShip,0x10)=4;BYTE(testShip,0x11)=2;
    if(janitor)PTR(testShip,8)=(void*)janitorName;
    INT(testShip,0xc4)=100;PTR(testShip,0xf8)=hullData;INT(hullData,0x18)=INT(hullData,0x60)=1000;
    PTR(testShip,0x100)=engineData;*(double*)(engineData+0x40)=100;
    BYTE(testShip,0x428)=1;FLOAT(testShip,0x434)=-400;FLOAT(testShip,0x438)=-300;
    PTR(testStar,0x24)=planetsData;PTR(planetsData,4)=planetPointers;INT(planetsData,8)=1;planetPointers[0]=planetData;INT(planetData,0x68)=1000;BYTE(planetData,0x74)=2;
    moveOrder=fakeMove;noneOrder=fakeNone;spawnJanitor=fakeSpawn;addShip=fakeAdd;takeoffOrder=fakeTakeoff;reallocateWide=fakeName;moves=nones=spawns=registrations=takeoffs=0;
    for(int i=0;i<n;++i){BYTE(itemData[i],0xc)=4;FLOAT(itemData[i],0x10)=1500+i*100;}
}
static void pod(int i){BYTE(itemData[i],0xc)=70;PTR(itemData[i],0x38)=L"ShuBounty_EscapePod";}
static int candidateHas(void *x){int n,i,result=0;Candidate *c=collectCandidates(testShip,testItems,listCount(testItems),&n);for(i=0;i<n;++i)if(c[i].item==x)result=1;free(c);return result;}
int main(void){int i;
    /* No normal NPC route, pointer or state is touched, even with junk far away. */
    setup(1,0);{unsigned char copy[sizeof(testShip)];memcpy(copy,testShip,sizeof(copy));beforeMovement(testShip);assert(!memcmp(copy,testShip,sizeof(copy)) && !moves && !nones);}
    setup(2,1);beforeMovement(testShip);assert(moves==1 && preferredJanitorItem(testShip)==itemData[0]);
    /* Do not switch to newly closer junk; do not replan several times per day. */
    FLOAT(itemData[1],0x10)=500;beforeMovement(testShip);assert(moves==1);++INT(worldData,0x4c);beforeMovement(testShip);assert(preferredJanitorItem(testShip)==itemData[0]);
    /* Actual Weapon.Target assignment after arrival, without direct deletion. */
    FLOAT(testShip,0x14)=1450;afterAssign(testShip);assert(target()==itemData[0] && !INT(itemData[0],0x30));
    /* Removed target is resolved by ID each time, then the next job is chosen. */
    itemPointers[0]=itemData[1];INT(testItems,8)=1;++INT(worldData,0x4c);PTR(testWeapon,0x70)=0;beforeMovement(testShip);assert(preferredJanitorItem(testShip)==itemData[1]);
    setup(1,1);beforeMovement(testShip);PTR(testWeapon,0x70)=testOther;afterAssign(testShip);assert(target()==testOther && !preferredJanitorItem(testShip) && BYTE(testShip,0x428)==0);
    for(i=0;i<8;++i){setup(1,1);if(i==0)PTR(testShip,0x3cc)=testOther;if(i==1)BYTE(testShip,0x43c)=1;if(i==2)BYTE(testShip,0x44c)=1;if(i==3)BYTE(testShip,0x3b4)=1;if(i==4)INT(hullData,0x60)=100;if(i==5)BYTE(engineData,0x48)=1;if(i==6)PTR(testShip,0x3fc)=testOther;if(i==7)BYTE(testShip,0x444)=1;beforeMovement(testShip);assert(!moves && !nones);}
    setup(1,1);BYTE(testInfo,0x35)=5;beforeMovement(testShip);assert(!moves);
    setup(1,1);beforeMovement(testShip);INT(worldData,0x4c)+=60;beforeMovement(testShip);assert(!preferredJanitorItem(testShip));
    setup(1,1);beforeMovement(testShip);++INT(worldData,0x174);syncJanitors();assert(!janitorState(testStar,0));beforeMovement(testShip);assert(preferredJanitorItem(testShip)==itemData[0]);
    /* New ships only, one per inhabited peaceful system; two native daily paths. */
    setup(1,0);onStarDay(testStar);assert(spawns==1 && isJanitor(testOther) && registrations==1 && takeoffs==1);onStarDay(testStar);assert(spawns==1);++INT(worldData,0x4c);onStarDay(testStar);assert(spawns==1);
    INT(testShips,8)=1;INT(worldData,0x4c)+=100;onStarDay(testStar);assert(spawns==1 && registrations==2 && takeoffs==2); /* Re-launch same docked janitor, never duplicate it. */
    for(i=0;i<5;++i){setup(1,0);if(i==0)BYTE(testStar,0x48)=1;if(i==1)BYTE(testStar,0x42)=1;if(i==2)BYTE(testShip,0x11)=5;if(i==3)INT(planetData,0x68)=0;if(i==4)activeJunkCleanup=0;onStarDay(testStar);assert(!spawns);}
    setup(1,0);onStarDay(testStar);INT(testShips,8)=1;INT(rosterData,8)=0;++INT(worldData,0x4c);onStarDay(testStar);assert(spawns==1);INT(worldData,0x4c)+=30;onStarDay(testStar);assert(spawns==2);
    /* Dominator/klissan owner marker: ambient scripted pods allowed, missions not. */
    setup(1,0);BYTE(testShip,0x10)=0;pod(0);FLOAT(itemData[0],0x10)=0;PTR(itemData[0],0x2c)=bindingData;PTR(bindingData,0x28)=itemData[0];BYTE(bindingData,0x2c)=1;PTR(bindingData,0x60)=scriptData;PTR(scriptData,8)=L"Script.Mod_ShuBounty";afterAssign(testShip);assert(target()==itemData[0]);
    PTR(bindingData,4)=L"StoryPod";INT(itemData[0],0x20)=10000;PTR(testWeapon,0x70)=0;afterAssign(testShip);assert(!target());PTR(bindingData,4)=0;BYTE(bindingData,0x2c)=0;afterAssign(testShip);assert(!target());
    BYTE(bindingData,0x2c)=1;PTR(scriptData,8)=L"Script.StoryMission";afterAssign(testShip);assert(!target());
    setup(1,0);BYTE(testShip,0x11)=5;pod(0);assert(candidateHas(itemData[0]));BYTE(itemData[0],0x34)=1;assert(candidateHas(itemData[0]));
    setup(1,0);BYTE(testShip,0x10)=0;pod(0);PTR(itemData[0],0x38)=L"Usl_CapsuleStukach";assert(candidateHas(itemData[0]));
    /* 50-item budget: station salvage before good gear, cards always protected. */
    setup(51,0);for(i=0;i<51;++i)BYTE(itemData[i],0xc)=44;BYTE(itemData[50],0xc)=70;PTR(itemData[50],0x38)=L"Usl_30";INT(itemData[50],0x18)=10;INT(itemData[50],0x20)=100000;assert(!candidateHas(itemData[50]) && candidateHas(itemData[0]));
    INT(itemData[50],0x20)=99999;assert(candidateHas(itemData[50]) && candidateHas(itemData[0]));
    PTR(itemData[50],0x38)=L"Usl_74";assert(candidateHas(itemData[50]) && candidateHas(itemData[0]));
    setup(51,0);for(i=0;i<51;++i)BYTE(itemData[i],0xc)=44;pod(50);assert(candidateHas(itemData[50]) && candidateHas(itemData[0]));
    BYTE(testShip,0x10)=0;assert(candidateHas(itemData[50]) && candidateHas(itemData[0]));
    /* Real system BSTR allocation/replace ABI, no Delphi heap ownership crossing. */
    {wchar_t *name=0;void (WINAPI *freeWide)(wchar_t*);HMODULE ole=LoadLibraryW(L"oleaut32.dll");ReallocWide reallocWide=(ReallocWide)GetProcAddress(ole,"SysReAllocString");freeWide=(void*)GetProcAddress(ole,"SysFreeString");assert(reallocWide(&name,L"Old"));assert(reallocWide(&name,janitorName));assert(!wcscmp(name,janitorName));freeWide(name);}
    /* Execute original TList.Add on a capacity-backed fixture (no heap growth). */
    {const wchar_t *exe=L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Space Rangers HD A War Apart\\Rangers.exe";
      HMODULE game=LoadLibraryExW(exe,0,DONT_RESOLVE_DLL_REFERENCES);void *vmt[4]={0},*slots[4]={0};unsigned char list[0x10]={0};RegCall add;
      assert(game);vmt[1]=(char*)game+0x41657c-0x400000;PTR(list,0)=vmt;PTR(list,4)=slots;INT(list,0xc)=4;
      arena=VirtualAlloc(0,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);assert(arena);used=0;add=bridge((char*)game+0x4161c0-0x400000,0);
      assert(add(list,(void*)123)==0 && INT(list,8)==1 && slots[0]==(void*)123);
      assert(add(list,(void*)456)==1 && INT(list,8)==2 && slots[1]==(void*)456);}
    clearJanitors();puts("PASS v1.3: normal NPC routes byte-identical; dedicated creation gates/duplicates/cooldown; target lock/arrival/Weapon.Target; combat and urgent preemption; no missiles; save-load recovery; capsule binding/quest safeguards; overflow tiers and credit-card protection; BSTR ABI.");return 0;
}
