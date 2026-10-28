#define SERVICE_MAX_SHIPS 15
#define SERVICE_MAX_SYSTEM 2
#define SERVICE_REGISTRY_SCHEMA 2
enum {SERVICE_WORKING=1,SERVICE_TRANSIT=2,SERVICE_WAITING=3};
typedef struct {
    unsigned shipId,systemId,destinationId;
    int variant,state,quotaTurn,quotaUsed;
} ServiceSlot;
typedef struct {unsigned schema;ServiceSlot slots[SERVICE_MAX_SHIPS];} ServiceRegistry;
static int fleetFind(const ServiceRegistry *r,unsigned id) {
    int i;if(!id)return -1;
    for(i=0;i<SERVICE_MAX_SHIPS;++i)if(r->slots[i].shipId==id)return i;
    return -1;
}
static int fleetCount(const ServiceRegistry *r) {
    int i,n=0;for(i=0;i<SERVICE_MAX_SHIPS;++i)if(r->slots[i].shipId)++n;return n;
}
static int fleetCommitted(const ServiceRegistry *r,unsigned systemId,unsigned excluding) {
    int i,n=0;if(!systemId)return 0;
    for(i=0;i<SERVICE_MAX_SHIPS;++i){const ServiceSlot *s=r->slots+i;
        if(!s->shipId || s->shipId==excluding)continue;
        /* Reserve both origin and arrival until native transit really starts. */
        if(s->systemId==systemId || s->destinationId==systemId)++n;
    }return n;
}
static int fleetRegister(ServiceRegistry *r,unsigned id,unsigned systemId,int variant,int turn) {
    int i;
    if(!id || !systemId || variant<1 || variant>3 || r->schema!=SERVICE_REGISTRY_SCHEMA)return -1;
    i=fleetFind(r,id);if(i>=0)return i;
    if(fleetCommitted(r,systemId,0)>=SERVICE_MAX_SYSTEM)return -1;
    for(i=0;i<SERVICE_MAX_SHIPS;++i)if(!r->slots[i].shipId){
        ServiceSlot *s=r->slots+i;s->shipId=id;s->systemId=systemId;s->destinationId=0;
        s->variant=variant;s->state=SERVICE_WORKING;s->quotaTurn=turn;s->quotaUsed=0;return i;
    }return -1;
}
static int fleetAssign(ServiceRegistry *r,unsigned id,unsigned destination) {
    int i=fleetFind(r,id);
    if(i<0 || !destination || fleetCommitted(r,destination,id)>=SERVICE_MAX_SYSTEM)return 0;
    r->slots[i].destinationId=destination;r->slots[i].state=SERVICE_TRANSIT;return 1;
}
static int fleetArrive(ServiceRegistry *r,unsigned id,unsigned actualSystem) {
    int i=fleetFind(r,id);ServiceSlot *s;
    if(i<0 || !actualSystem)return 0;s=r->slots+i;
    if(s->destinationId && s->destinationId!=actualSystem)return 0;
    if(fleetCommitted(r,actualSystem,id)>=SERVICE_MAX_SYSTEM)return 0;
    s->systemId=actualSystem;s->destinationId=0;s->state=SERVICE_WORKING;return 1;
}
static int fleetReserveShot(ServiceRegistry *r,unsigned id,int turn,double squaredDistance) {
    int i=fleetFind(r,id);ServiceSlot *s;
    if(i<0 || squaredDistance<0 || squaredDistance>160000)return 0;
    s=r->slots+i;if(s->state!=SERVICE_WORKING)return 0;
    if(s->quotaTurn>turn)return 0; /* Invalid future state must not grant credit. */
    if(s->quotaTurn!=turn){s->quotaTurn=turn;s->quotaUsed=0;}
    if(s->quotaUsed<0 || s->quotaUsed>=10)return 0;
    ++s->quotaUsed;return 1;
}
