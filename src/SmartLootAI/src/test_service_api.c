#define main old_test_main
#include "test_policy.c"
#undef main
static unsigned char binding[0x30],script[0x70],tank[0x80];
static int fired,moves;
static LootPoint lastMove;
static int fakeNone(void *ship,void *unused){return 0;}
static void fakeMove(void *ship,const LootPoint *p){++moves;lastMove=*p;}
static void fakeFire(void *ship,void *weapon,void *item,int film){
    int i,n=INT(testItems,8);assert(!weapon);assert(serviceMember(ship));
    for(i=0;i<n;++i)if(itemPointers[i]==item)break;assert(i<n);
    for(;i<n-1;++i)itemPointers[i]=itemPointers[i+1];INT(testItems,8)=n-1;++fired;
}
static void setupService(int n){
    int i;reset(n);memset(binding,0,sizeof(binding));memset(script,0,sizeof(script));memset(tank,0,sizeof(tank));
    PTR(testShip,0x3cc)=binding;PTR(binding,4)=script;PTR(script,8)=L"Script.Mod_SmartLootServiceV2";
    PTR(testShip,0xfc)=tank;BYTE(tank,0x68)=90;initialized=1;serviceFire=fakeFire;noneOrder=fakeNone;moveOrder=fakeMove;fired=moves=0;
    for(i=0;i<n;++i)BYTE(itemData[i],0xc)=4;
}
int main(void){
    setupService(12);assert(SL_Prepare(testShip));assert(BYTE(testShip,0x498)==1 && BYTE(testShip,0x499)==1 && BYTE(testShip,0x44c)==1);assert(INT(tank,0x64)==90);
    assert(SL_Clean(testShip,100)==10 && fired==10 && INT(testItems,8)==2);
    setupService(1);FLOAT(itemData[0],0x10)=400;assert(SL_Clean(testShip,10)==1);
    setupService(1);FLOAT(itemData[0],0x10)=401;assert(SL_Clean(testShip,10)==0);assert(SL_Move(testShip));assert(moves==1 && lastMove.x>100 && lastMove.x<102 && BYTE(testShip,0x44c)==1);
    setupService(1);BYTE(itemData[0],0xc)=71;assert(SL_Clean(testShip,10)==0);
    setupService(1);PTR(script,8)=L"Script.Unrelated";assert(!SL_Prepare(testShip) && !SL_Target(testShip) && !SL_Clean(testShip,10));
    /* Janitors already in a save with cleanup off: no work, no targets, no shots; their stock targets cleared. */
    setupService(12);activeJunkCleanup=0;
    assert(!SL_CountWork(PTR(testShip,0x24)) && !SL_Target(testShip) && SL_Clean(testShip,10)==0 && fired==0);
    PTR(testWeapon,0x70)=itemData[0];assert(!allowCleanupShot(testShip,testWeapon,itemData[0]) && !PTR(testWeapon,0x70));
    PTR(testWeapon,0x70)=itemData[0];afterAssign(testShip);assert(!PTR(testWeapon,0x70));
    activeJunkCleanup=1;
    puts("PASS service API: janitors idle with cleanup off; script identity; NoDrop/NoTarget/order ownership; refuel; cap10; exact400/401; approach; micromodules; unrelated ships untouched.");return 0;
}
