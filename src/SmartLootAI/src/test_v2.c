#define main historical_test_main
#include "test_policy.c"
#undef main
static int has(void *ship,void *item) {
    int n,i,found=0;Candidate *c=collectCandidates(ship,testItems,listCount(testItems),&n);
    for(i=0;i<n;++i)if(c[i].item==item)found=1;
    free(c);return found;
}
int main(void) {
    int t,n;Candidate *c;
    for(t=8;t<=41;++t){reset(1);BYTE(itemData[0],0xc)=t;assert(!has(testShip,itemData[0]));}
    reset(1);BYTE(itemData[0],0xc)=71;assert(!has(testShip,itemData[0]));
    for(t=76;t<256;++t){reset(1);BYTE(itemData[0],0xc)=t;assert(!has(testShip,itemData[0]));}
    reset(1);PTR(itemData[0],0x38)=L"UnknownModEquipment";assert(!has(testShip,itemData[0]));
    reset(1);BYTE(itemData[0],0xc)=70;PTR(itemData[0],0x38)=L"UnknownQuestItem";assert(!has(testShip,itemData[0]));
    reset(1);BYTE(itemData[0],0xc)=70;PTR(itemData[0],0x38)=L"Usl_30";BYTE(itemData[0],0x34)=1;INT(itemData[0],0x20)=9999;assert(has(testShip,itemData[0]));INT(itemData[0],0x20)=10000;assert(!has(testShip,itemData[0]));
    reset(1);BYTE(itemData[0],0xc)=69;INT(itemData[0],0x18)=29;assert(has(testShip,itemData[0]));INT(itemData[0],0x18)=30;assert(!has(testShip,itemData[0]));INT(itemData[0],0x18)=1000;assert(!has(testShip,itemData[0]));INT(itemData[0],0x18)=1001;assert(has(testShip,itemData[0]));
    reset(49);c=collectCandidates(testShip,testItems,49,&n);assert(n==0);free(c);
    reset(50);c=collectCandidates(testShip,testItems,50,&n);assert(n==0);free(c);
    reset(51);c=collectCandidates(testShip,testItems,51,&n);assert(n==1 && c[0].item==itemData[0]);free(c);
    reset(60);c=collectCandidates(testShip,testItems,60,&n);assert(n==10);free(c);
    reset(51);BYTE(itemData[50],0xc)=4;c=collectCandidates(testShip,testItems,51,&n);assert(n==1 && c[0].item==itemData[50]);free(c);
    reset(51);INT(itemData[0],0x20)=900000;c=collectCandidates(testShip,testItems,51,&n);assert(n==1 && c[0].item==itemData[1]);free(c);
    /* Normal ships do not preserve fuel barrels. */
    reset(3);for(t=0;t<3;++t)BYTE(itemData[t],0xc)=72;c=collectCandidates(testShip,testItems,3,&n);assert(n==3);free(c);
    /* Legacy janitors preserve three; identity migration tested separately later. */
    reset(4);PTR(testShip,8)=L"SmartLootAI - Janitor";BYTE(testShip,0x10)=4;for(t=0;t<4;++t){BYTE(itemData[t],0xc)=72;INT(itemData[t],0x60)=40-t;}c=collectCandidates(testShip,testItems,4,&n);assert(n==1 && c[0].item==itemData[3]);free(c);
    /* Short idle gun cleans while long gun is reserved for reachable enemy. */
    reset(2);for(t=0;t<2;++t)BYTE(itemData[t],0xc)=4;
    BYTE(testShip,0x12c)=2;PTR(testShip,0x11c)=otherWeapon;secondRange=400;
    INT(testShips,8)=2;shipPointers[1]=testOther;hostile=1;FLOAT(testOther,0x14)=300;
    afterAssign(testShip);assert(target()==itemData[0] && !PTR(otherWeapon,0x70));
    PTR(testWeapon,0x70)=0;FLOAT(testOther,0x14)=200;afterAssign(testShip);assert(!target());
    FLOAT(testOther,0x14)=401;afterAssign(testShip);assert(target() && PTR(otherWeapon,0x70));
    puts("PASS v2 core: explicit unknown protection; 34 artifacts and micromodules; quest price boundary; nodes 29/30/1000/1001; equipment 49/50/51/60 and cheapest excess; normal/janitor barrels; independent gun ranges.");
    return 0;
}
