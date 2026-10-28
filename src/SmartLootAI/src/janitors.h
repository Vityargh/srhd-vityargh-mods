/* Dedicated utility ships: normal NPC orders are never modified.
   Spawn through the same TPlanet.BuyWarrior used by RScript BuyWarrior;
   native lifetime, combat, repairs, damage and serialization remain in use. */
#include <math.h>
typedef struct {float x,y;} LootPoint;
typedef void (__cdecl *MoveCall)(void*,const LootPoint*);
static MoveCall moveOrder;
static RegCall noneOrder,spawnJanitor,addShip,takeoffOrder;
static void **galaxySlot;
static const wchar_t janitorName[]=L"SmartLootAI - Janitor";
typedef struct JanitorState {
    struct JanitorState *next;
    void *star;unsigned starId,shipId,itemId;
    int checked,spawnAfter,planned,started,holding;
    unsigned failed[8],failedNext;
    LootPoint issued;
} JanitorState;
static JanitorState *janitors[128];
static void *janitorWorld;
static int janitorLoad,janitorTurn;
static int isJanitor(void *ship) {
    if(serviceMember(ship))return 1;
    const wchar_t *name=PTR(ship,8);
    return BYTE(ship,0x10)==4 && name && !wcscmp(name,janitorName);
}
static void clearJanitors(void) {
    int i;for(i=0;i<128;++i){JanitorState *s=janitors[i];while(s){JanitorState *next=s->next;free(s);s=next;}janitors[i]=0;}
}
static void *syncJanitors(void) {
    void *world=galaxySlot?*galaxySlot:0;int turn,load;
    if(!world)return 0;
    turn=INT(world,0x4c);load=INT(world,0x174);
    if(world!=janitorWorld || load!=janitorLoad || turn<janitorTurn){clearJanitors();janitorWorld=world;janitorLoad=load;}
    janitorTurn=turn;return world;
}
static JanitorState *janitorState(void *star,int create) {
    unsigned b=((uintptr_t)star>>4)&127;JanitorState **link=&janitors[b],*s;
    while((s=*link)!=0){if(s->star==star){if(s->starId==(unsigned)INT(star,4))return s;*link=s->next;free(s);break;}link=&s->next;}
    if(!create)return 0;
    s=calloc(1,sizeof(*s));if(!s)return 0;s->star=star;s->starId=INT(star,4);s->checked=s->planned=-1;s->next=janitors[b];janitors[b]=s;return s;
}
static int ownsCleanupOrder(JanitorState *s,void *ship) {
    return s->holding && s->shipId==(unsigned)INT(ship,4) && BYTE(ship,0x428)==1 &&
        !BYTE(ship,0x43c) && !BYTE(ship,0x44c) && !PTR(ship,0x430) &&
        FLOAT(ship,0x434)==s->issued.x && FLOAT(ship,0x438)==s->issued.y;
}
static void interruptJanitor(void *ship) {
    void *star;JanitorState *s;
    if(!isJanitor(ship) || !syncJanitors() || !(star=PTR(ship,0x24)))return;
    s=janitorState(star,0);if(!s)return;
    if(ownsCleanupOrder(s,ship))noneOrder(ship,0);
    s->holding=0;s->itemId=0;
}
static void *preferredJanitorItem(void *ship) {
    void *star,*items;JanitorState *s;int i;
    if(!isJanitor(ship) || !syncJanitors() || !(star=PTR(ship,0x24)))return 0;
    s=janitorState(star,0);if(!s || s->shipId!=(unsigned)INT(ship,4) || !s->itemId)return 0;
    items=PTR(star,0x30);for(i=0;i<listCount(items);++i){void *x=listAt(items,i);if((unsigned)INT(x,8)==s->itemId)return x;}return 0;
}
static int urgent(void *ship) {
    void *hull=PTR(ship,0xf8),*engine=PTR(ship,0x100);
    if(INT(ship,0xc4)<=0 || !inSpace(ship,0))return 1;
    if(BYTE(ship,0x3b4) || PTR(ship,0x3cc) || PTR(ship,0x3d0) || PTR(ship,0x3fc) ||
       BYTE(ship,0x43c) || BYTE(ship,0x44c) || BYTE(ship,0x444) || BYTE(ship,0x460) || BYTE(ship,0x428)>3)return 1;
    if(!hull || (double)INT(hull,0x60)<(double)INT(hull,0x18)*0.7)return 1;
    return !engine || BYTE(engine,0x48) || *(double*)((char*)engine+0x40)<20.0;
}
static int hasBattleTarget(void *ship,void *star) {
    int j,n=BYTE(ship,0x12c);if(n>5)return 1;
    for(j=0;j<n;++j){void *w=PTR(ship,0x118+4*j),*t=w?PTR(w,0x70):0;
        if(t && !contains(PTR(star,0x30),t) && !contains(PTR(star,0x28),t))return 1;}
    return combat(ship,star);
}
static int cheapWeaponRange(void *ship) {
    int j,n=BYTE(ship,0x12c),best=0;if(n>5)return 0;
    for(j=0;j<n;++j){void *w=PTR(ship,0x118+4*j),*info;int shot,r;
        if(!w || !usable(ship,w))continue;info=(void*)infoOf(w,0);if(!info)continue;
        shot=BYTE(info,0x35);if(shot>=5 && shot<=7)continue;
        r=rangeOf(ship,w);if(r>best)best=r;}
    return best;
}
static void __cdecl onStarDay(void *star) {
    return; /* V2 script owns fleet creation and migration. */
    void *world=syncJanitors(),*ships,*planets,*planet=0,*ship=0;JanitorState *s;int i,n,turn;Candidate *c;
    if(!world || !activeJunkCleanup)return;
    s=janitorState(star,1);if(!s)return;turn=INT(world,0x4c);
    if(s->checked==turn)return;s->checked=turn;
    ships=PTR(star,0x2c);for(i=0;i<listCount(ships);++i)if(isJanitor(listAt(ships,i))){ship=listAt(ships,i);break;}
    if(ship)return; /* Already registered, including native takeoff/repair states. */
    if(BYTE(star,0x48) || BYTE(star,0x42)!=0)return;
    for(i=0;i<listCount(ships);++i){void *x=listAt(ships,i);if(inSpace(x,0) && (BYTE(x,0x11)==5 || BYTE(x,0x11)==7))return;}
    planets=PTR(star,0x24);for(i=0;i<listCount(planets);++i){void *p=listAt(planets,i),*roster=PTR(p,0x114);int j;
        for(j=0;j<listCount(roster);++j)if(isJanitor(listAt(roster,j)))ship=listAt(roster,j);
        if(!planet && BYTE(p,0x74)<=4 && INT(p,0x68)>0)planet=p;
    }
    if(!planet)return;
    if(ship){
        void *hull=PTR(ship,0xf8),*engine=PTR(ship,0x100);
        /* Planet.NextDay removes peaceful docked garrison members from Ships.
           Re-register ONLY our healthy, idle janitor after that processing. */
        if(PTR(ship,0x24)!=star || !PTR(ship,0x1c) || PTR(ship,0x3cc) || PTR(ship,0x3d0) || PTR(ship,0x3fc) ||
           BYTE(ship,0x3b4) || BYTE(ship,0x43c) || BYTE(ship,0x44c) || BYTE(ship,0x460) || !hull || !engine ||
           (double)INT(hull,0x60)<INT(hull,0x18)*0.7 || BYTE(engine,0x48) || *(double*)((char*)engine+0x40)<20.0)return;
    }else if(turn<s->spawnAfter)return;
    /* Classification marker only; it is never passed into the game. */
    { unsigned char marker[0x18]={0};PTR(marker,8)=(void*)janitorName;BYTE(marker,0x10)=4;
      c=collectCandidates(marker,PTR(star,0x30),listCount(PTR(star,0x30)),&n);free(c); }
    if(!n)return;
    if(!ship){s->spawnAfter=turn+30;
        ship=(void*)spawnJanitor(planet,(void*)100);if(!ship)return;
        /* Delphi WideString is an owned Windows BSTR; no borrowed string stored. */
        if(!reallocateWide((wchar_t**)((char*)ship+8),janitorName))return;
        Log(L"JANITOR: created one native utility ship in a peaceful inhabited system.");
    }
    /* BuyWarrior creates a docked garrison member, not a Star.Ships member.
       Post-day callbacks run after planet processing has finished. */
    if(!contains(ships,ship))addShip(ships,ship);
    takeoffOrder(ship,0);
    s->shipId=INT(ship,4);s->itemId=0;s->holding=0;
}
static void __cdecl beforeMovement(void *ship) {
    return; /* V2 script owns service movement. */
    void *world,*star,*items,*item=0;JanitorState *s;Candidate *c;
    int i,k,n,r,turn,bestPriority=0x7fffffff;double nearest=1.0e30,dx,dy,d;
    if(!isJanitor(ship) || ship==(void*)player(0,0) || !(world=syncJanitors()) || !(star=PTR(ship,0x24)))return;
    if(!activeJunkCleanup || urgent(ship) || hasBattleTarget(ship,star)){interruptJanitor(ship);return;}
    s=janitorState(star,1);if(!s)return;turn=INT(world,0x4c);
    if(s->planned==turn)return;s->planned=turn;
    if(s->shipId!=(unsigned)INT(ship,4)){s->shipId=INT(ship,4);s->itemId=0;s->holding=0;memset(s->failed,0,sizeof(s->failed));}
    r=cheapWeaponRange(ship);if(!r){interruptJanitor(ship);return;}
    items=PTR(star,0x30);c=collectCandidates(ship,items,listCount(items),&n);
    for(i=0;i<n;++i)if(c[i].id==s->itemId && safeDetonate(ship,c[i].item))item=c[i].item;
    if(item && turn-s->started>=60){s->failed[(s->failedNext++)&7]=s->itemId;item=0;}
    if(!item){s->itemId=0;
        for(i=0;i<n;++i){int failed=0;void *x=c[i].item;
            for(k=0;k<8;++k)if(s->failed[k] && s->failed[k]==c[i].id){failed=1;break;}
            if(failed || !safeDetonate(ship,x))continue;
            dx=FLOAT(x,0x10)-FLOAT(ship,0x14);dy=FLOAT(x,0x14)-FLOAT(ship,0x18);d=dx*dx+dy*dy;
            if(c[i].priority<bestPriority || (c[i].priority==bestPriority && d<nearest)){item=x;nearest=d;bestPriority=c[i].priority;}
        }
        if(item){s->itemId=INT(item,8);s->started=turn;}
    }
    free(c);
    /* Utility ships stay in their assigned system when healthy and peaceful.
       Native AI retains combat, repairs, takeoff and scripted orders. */
    noneOrder(ship,0);s->holding=0;if(!item)return;
    dx=FLOAT(item,0x10)-FLOAT(ship,0x14);dy=FLOAT(item,0x14)-FLOAT(ship,0x18);d=sqrt(dx*dx+dy*dy);
    if(d<=r){s->issued.x=FLOAT(ship,0x14);s->issued.y=FLOAT(ship,0x18);}
    else{double fraction=(d-r*0.75)/d;s->issued.x=FLOAT(ship,0x14)+(float)(dx*fraction);s->issued.y=FLOAT(ship,0x18)+(float)(dy*fraction);}
    moveOrder(ship,&s->issued);s->holding=BYTE(ship,0x428)==1 && FLOAT(ship,0x434)==s->issued.x && FLOAT(ship,0x438)==s->issued.y;
}
static MoveCall bridgeMove(void *target) {
    unsigned char *p=allocCode(32);unsigned char load[]={0x8b,0x44,0x24,4,0x8b,0x54,0x24,8,0x31,0xc9};
    memcpy(p,load,sizeof(load));rel32(p+sizeof(load),0xe8,target);p[sizeof(load)+5]=0xc3;return (MoveCall)p;
}
