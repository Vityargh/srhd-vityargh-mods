/* RScript library ABI is cdecl: EC_Expression removes argument words. */
typedef void (__cdecl *FireItemCall)(void*,void*,void*,int);
static FireItemCall serviceFire;
static int serviceSafeTarget(void *ship,void *x) {
    void *star=PTR(ship,0x24),*ships;int i;
    if(!star || !allowCleanupShot(ship,0,x))return 0;
    if(!(BYTE(x,0xc)==72 || BYTE(x,0xc)==70 || BYTE(x,0xc)==75 || quest(x)))return 1;
    ships=PTR(star,0x2c);
    for(i=0;i<listCount(ships);++i){void *other=listAt(ships,i);double dx,dy;if(other==ship || !inSpace(other,0))continue;
        dx=(double)FLOAT(other,0x14)-FLOAT(x,0x10);dy=(double)FLOAT(other,0x18)-FLOAT(x,0x14);
        if(dx*dx+dy*dy<=explosionRadiusSquared)return 0;
    }return 1;
}
static int serviceMember(void *ship) {
    void *binding,*script;const wchar_t *key;
    if(!ship)return 0;binding=PTR(ship,0x3cc);if(!binding)return 0;
    script=PTR(binding,4);if(!script)return 0;key=PTR(script,8);
    return key && !wcscmp(key,L"Script.Mod_SmartLootServiceV2");
}
__declspec(dllexport) int __cdecl SL_Ready(void){return initialized;}
__declspec(dllexport) int __cdecl SL_CountWork(void *star){
    unsigned char marker[0x4d0]={0};Candidate *c;int n,i,result=0;
    if(!initialized || !star)return 0;
    PTR(marker,8)=(void*)L"SmartLootAI - Janitor";BYTE(marker,0x10)=4;
    PTR(marker,0x24)=star;
    c=collectCandidates(marker,PTR(star,0x30),listCount(PTR(star,0x30)),&n);
    for(i=0;i<n;++i)if(serviceSafeTarget(marker,c[i].item))++result;
    free(c);return result;
}
static void *serviceTarget(void *ship,int withinRadius) {
    void *star=PTR(ship,0x24),*best=0;Candidate *c;int i,n,priority=0x7fffffff;double nearest=1e30;
    if(!star)return 0;c=collectCandidates(ship,PTR(star,0x30),listCount(PTR(star,0x30)),&n);
    for(i=0;i<n;++i){void *x=c[i].item;double dx=(double)FLOAT(x,0x10)-FLOAT(ship,0x14),dy=(double)FLOAT(x,0x14)-FLOAT(ship,0x18),d=dx*dx+dy*dy;
        if(withinRadius && d>160000)continue;
        if(!serviceSafeTarget(ship,x))continue;
        if(c[i].priority<priority || (c[i].priority==priority && d<nearest)){best=x;nearest=d;priority=c[i].priority;}
    }free(c);return best;
}
__declspec(dllexport) void *__cdecl SL_Target(void *ship){return initialized && serviceMember(ship)?serviceTarget(ship,0):0;}
__declspec(dllexport) int __cdecl SL_Edge(void *star){return initialized && star?(*(unsigned short*)((char*)star+0x1c))*9/10:0;}
__declspec(dllexport) int __cdecl SL_Prepare(void *ship){
    int j;void *tank;
    if(!initialized || !serviceMember(ship))return 0;
    BYTE(ship,0x498)=1;BYTE(ship,0x499)=1;BYTE(ship,0x49b)=1;
    BYTE(ship,0x44c)=1; /* Keep script-owned orders; suppress native AI upgrades. */
    PTR(ship,0x3f4)=0; /* No native EnemyShip. */
    for(j=0;j<BYTE(ship,0x12c) && j<5;++j){void *w=PTR(ship,0x118+j*4);if(w)PTR(w,0x70)=0;}
    tank=PTR(ship,0xfc);if(tank)INT(tank,0x64)=BYTE(tank,0x68);
    INT(ship,0xc8)=0x7fffff;return 1;
}
__declspec(dllexport) int __cdecl SL_Clean(void *ship,int budget){
    int used=0;
    if(!initialized || !serviceMember(ship) || !inSpace(ship,0) || !serviceFire)return 0;
    if(budget<0)return 0;if(budget>10)budget=10;
    while(used<budget){void *x=serviceTarget(ship,1),*star=PTR(ship,0x24);int i,unsafe=0;
        if(!x || !allowCleanupShot(ship,0,x))break;
        if(BYTE(x,0xc)==72 || BYTE(x,0xc)==70 || BYTE(x,0xc)==75 || quest(x)) {
            void *ships=PTR(star,0x2c);
            for(i=0;i<listCount(ships);++i){void *other=listAt(ships,i);double dx,dy;if(other==ship || !inSpace(other,0))continue;
                dx=(double)FLOAT(other,0x14)-FLOAT(x,0x10);dy=(double)FLOAT(other,0x18)-FLOAT(x,0x14);
                if(dx*dx+dy*dy<=explosionRadiusSquared){unsafe=1;break;}
            }
        }
        if(unsafe)break;
        ++used;serviceFire(ship,0,x,BYTE(star,0xf0)!=0);
    }return used;
}
static FireItemCall bridgeServiceFire(void *target) {
    unsigned char *p=allocCode(48);unsigned char code[]={0x8b,0x44,0x24,4,0x8b,0x54,0x24,8,0x8b,0x4c,0x24,12,0xff,0x74,0x24,16};
    memcpy(p,code,sizeof(code));rel32(p+sizeof(code),0xe8,target);p[sizeof(code)+5]=0xc3;return (FireItemCall)p;
}
__declspec(dllexport) int __cdecl SL_Move(void *ship) {
    void *item,*star;LootPoint point;double dx,dy,d;
    if(!initialized || !serviceMember(ship) || !inSpace(ship,0))return 0;
    star=PTR(ship,0x24);item=serviceTarget(ship,0);
    if(!star)return 0;
    BYTE(ship,0x44c)=0;
    noneOrder(ship,0);
    if(!item){point.x=SL_Edge(star);point.y=0;moveOrder(ship,&point);BYTE(ship,0x44c)=1;return 0;}
    dx=(double)FLOAT(item,0x10)-FLOAT(ship,0x14);dy=(double)FLOAT(item,0x14)-FLOAT(ship,0x18);d=sqrt(dx*dx+dy*dy);
    if(d<=400){BYTE(ship,0x44c)=1;return 1;}
    point.x=FLOAT(ship,0x14)+(float)(dx*(d-300)/d);point.y=FLOAT(ship,0x18)+(float)(dy*(d-300)/d);
    moveOrder(ship,&point);BYTE(ship,0x44c)=1;return 1;
}
