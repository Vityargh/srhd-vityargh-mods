#define main historical_test_main
#include "test_policy.c"
#undef main
int main(void) {
    void *stale=(void*)0x1234;
    reset(51);PTR(testWeapon,0x70)=itemData[0];assert(allowCleanupShot(testShip,testWeapon,itemData[0]));
    /* One earlier gun has removed the excess: the old target must be refused. */
    INT(testItems,8)=50;assert(!allowCleanupShot(testShip,testWeapon,itemData[0]));assert(!target());
    reset(1);assert(!allowCleanupShot(testShip,testWeapon,stale));
    reset(2);BYTE(itemData[0],0xc)=72;BYTE(itemData[1],0xc)=71;
    assert(!allowCleanupShot(testShip,testWeapon,itemData[0]));
    FLOAT(itemData[1],0x10)=201;assert(allowCleanupShot(testShip,testWeapon,itemData[0]));
    reset(2);BYTE(itemData[0],0xc)=72;BYTE(itemData[1],0xc)=250;assert(!allowCleanupShot(testShip,testWeapon,itemData[0]));
    reset(1);isPlayer=1;assert(allowCleanupShot(testShip,testWeapon,itemData[0]));
    puts("PASS fire-time guard: recomputes equipment excess; clears invalid weapon targets; rejects stale pointer without dereference; protects nearby micromodules and unknowns; player excluded.");return 0;
}
