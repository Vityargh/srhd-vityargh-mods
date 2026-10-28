/* Durable values belong to the saved script registry, never process globals.
   Reserve BEFORE firing; delayed hits cannot overspend a ship's turn budget. */
typedef struct {unsigned shipId;int turn,used;} ServiceBudget;
static int reserveServiceShot(ServiceBudget *b,unsigned shipId,int turn,double distanceSquared) {
    if(!shipId || distanceSquared<0 || distanceSquared>400.0*400.0)return 0;
    if(b->shipId!=shipId){b->shipId=shipId;b->turn=turn;b->used=0;}
    if(b->turn!=turn){b->turn=turn;b->used=0;}
    if(b->used<0 || b->used>=10)return 0;
    ++b->used;return 1;
}
static int canAssignServiceDestination(int galaxyCount,int present,int enRoute) {
    return galaxyCount>=0 && galaxyCount<=15 && present>=0 && enRoute>=0 && present+enRoute<2;
}
static int canSpawnService(int galaxyCount,int present,int enRoute) {
    return galaxyCount<15 && canAssignServiceDestination(galaxyCount,present,enRoute);
}
