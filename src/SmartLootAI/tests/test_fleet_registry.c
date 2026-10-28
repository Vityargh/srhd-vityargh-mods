#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/fleet_registry.h"
int main(void){
    ServiceRegistry r={0},saved;int i;r.schema=SERVICE_REGISTRY_SCHEMA;
    assert(fleetRegister(&r,1,100,1,50)==0);assert(fleetRegister(&r,1,100,1,50)==0);assert(fleetCount(&r)==1);
    assert(fleetRegister(&r,2,100,2,50)==1);assert(fleetRegister(&r,3,100,3,50)<0);
    assert(fleetRegister(&r,3,200,3,50)==2);
    assert(fleetAssign(&r,1,300));assert(fleetAssign(&r,2,300));assert(!fleetAssign(&r,3,300));
    assert(!fleetReserveShot(&r,1,50,0));assert(fleetArrive(&r,1,300));
    for(i=0;i<10;++i)assert(fleetReserveShot(&r,1,50,160000));
    saved=r;memset(&r,0,sizeof(r));r=saved;assert(!fleetReserveShot(&r,1,50,0));
    assert(fleetReserveShot(&r,1,51,0));assert(!fleetReserveShot(&r,1,50,0));assert(!fleetReserveShot(&r,1,51,160001));
    for(i=4;i<=15;++i)assert(fleetRegister(&r,i,1000+i,i%3+1,51)>=0);
    assert(fleetCount(&r)==15);assert(fleetRegister(&r,16,5000,1,51)<0);
    puts("PASS fleet registry: stable IDs, duplicate registration, 3 variants, max2 including reservations, max15, transit/arrival, shared turn budget, save-state roundtrip, no future-turn credit.");return 0;
}
