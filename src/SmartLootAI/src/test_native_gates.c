#define main old_main
#include "test_policy.c"
#undef main
static unsigned char bindingData[0x30],scriptData[0x70];
typedef int (__cdecl *DamageInvoke)(void*,void*,int,float,unsigned*,int);
int main(void){
    unsigned char *normal,*gate,*call;unsigned color=123;RegCall simple;
    reset(1);PTR(testShip,0x3cc)=bindingData;PTR(bindingData,4)=scriptData;PTR(scriptData,8)=L"Script.Mod_SmartLootServiceV2";
    arena=VirtualAlloc(0,8192,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);assert(arena);used=0;
    normal=allocCode(16);normal[0]=0xb8;*(int*)(normal+1)=77;normal[5]=0xc3;
    gate=emitServiceGate(normal,0,1000000,0);simple=bridge(gate,0);
    assert(simple(testShip,0)==1000000);assert(simple(testOther,0)==77);
    gate=emitServiceGate(normal,1,0,0);simple=bridge(gate,0);
    assert(simple(testOther,testShip)==0);assert(simple(testOther,testOther)==77);
    normal=allocCode(16);normal[0]=0xb8;*(int*)(normal+1)=77;normal[5]=0xc2;normal[6]=12;normal[7]=0;
    gate=emitServiceGate(normal,0,0,12);
    call=allocCode(64);
    {unsigned char code[]={0x8b,0x44,0x24,4,0x8b,0x54,0x24,8,0x8b,0x4c,0x24,12,0xff,0x74,0x24,24,0xff,0x74,0x24,24,0xff,0x74,0x24,24};memcpy(call,code,sizeof(code));rel32(call+sizeof(code),0xe8,gate);call[sizeof(code)+5]=0xc3;}
    assert(((DamageInvoke)call)(testShip,0,100,0,&color,0)==0 && color==0);
    color=123;assert(((DamageInvoke)call)(testOther,0,100,0,&color,0)==77 && color==123);
    puts("PASS native x86 gates executed: service jump range; target exclusion; ordinary fallback; ApplyDamage RET12, out-color zero and stack preserved.");return 0;
}
