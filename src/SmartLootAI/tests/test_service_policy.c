#include <assert.h>
#include <stdio.h>
#include "../src/service_policy.h"
int main(void) {
    ServiceBudget b={0},saved;int i;
    assert(!reserveServiceShot(&b,1,42,401.0*401));
    for(i=0;i<10;++i)assert(reserveServiceShot(&b,1,42,400.0*400));
    assert(!reserveServiceShot(&b,1,42,0));
    saved=b;b=saved;assert(!reserveServiceShot(&b,1,42,0));
    assert(reserveServiceShot(&b,1,43,399.0*399));
    assert(canSpawnService(14,1,0));assert(!canSpawnService(15,0,0));
    assert(!canSpawnService(10,1,1));assert(!canAssignServiceDestination(15,2,0));
    assert(canAssignServiceDestination(15,0,1));
    puts("PASS service primitives: 400 inclusive; 10 reservations per ship/turn; restored budget cannot reset quota; max15 global and max2 including arrivals.");
    return 0;
}
