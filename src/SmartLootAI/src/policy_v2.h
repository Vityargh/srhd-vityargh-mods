/* Explicit categories: never infer permission from an unknown numeric type. */
static int knownSpecialKey(void *item) {
    static const wchar_t *keys[]={L"Usl_30",L"Usl_63",L"Usl_70",L"Usl_19",L"Usl_Cactus",L"ShuBounty_ElementPamyati",L"Usl_40",L"Usl_PTQn66",L"ShuBounty_EscapePod",L"ShuBounty_EscapePodStations"};
    const wchar_t *key=PTR(item,0x38);unsigned i;
    if(!key)return 0;
    for(i=0;i<sizeof(keys)/sizeof(keys[0]);++i)if(!wcscmp(key,keys[i]))return 1;
    return 0;
}
static int knownCategory(void *item) {
    int t=BYTE(item,0xc);
    if(t<=7)return 1;
    if(t>=43 && t<=67)return !named(PTR(item,0x38));
    if(t==69)return !named(PTR(item,0x38));
    if(t==70 || t==75)return knownSpecialKey(item);
    if(t==72)return !named(PTR(item,0x38));
    return 0;
}
static int protectedItem(void *item) {
    int t=BYTE(item,0xc),weight=INT(item,0x18),cost;
    if(t>=8 && t<=41)return 1; /* All built-in artifact variants. */
    if(t==71 || t==42 || !knownCategory(item))return 1;
    if(t==69 && weight>=30 && weight<=1000)return 1;
    if(quest(item) || (t>=43 && t<=67 && !ordinary(item)) || t==69 || t==70 || t==75) {
        cost=valueOf(item,0);
        if(weight<=0 || cost<0 || (int64_t)cost >= (int64_t)weight*10000)return 1;
    }
    return 0;
}
static int equipmentCounted(void *item) {
    int t=BYTE(item,0xc);
    return t>=43 && t<=67 && knownCategory(item) && ordinary(item);
}
static Candidate *collectCandidates(void *ship,void *items,int n,int *outCount) {
    Candidate *result,*equipment;void *keep[3]={0,0,0};int i,j,count=0,eqCount=0;
    int janitor=isJanitor(ship);
    *outCount=0;if(!activeJunkCleanup || n<1 || n>100000)return 0;
    result=malloc(n*sizeof(*result));equipment=malloc(n*sizeof(*equipment));
    if(!result || !equipment){free(result);free(equipment);return 0;}
    if(janitor)for(i=0;i<n;++i) {
        void *x=listAt(items,i);
        if(BYTE(x,0xc)!=72 || protectedItem(x))continue;
        for(j=0;j<3;++j)if(fuller(x,keep[j])){
            int k;for(k=2;k>j;--k)keep[k]=keep[k-1];keep[j]=x;break;
        }
    }
    for(i=0;i<n;++i) {
        void *x=listAt(items,i);int t=BYTE(x,0xc),priority=3;
        Candidate c;c.item=x;c.id=INT(x,8);c.score=valueOf(x,0);c.priority=3;
        if(equipmentCounted(x)){equipment[eqCount++]=c;continue;}
        if(protectedItem(x))continue;
        if(t==4 && !cleanupMinerals)continue;
        if(t<=7 && t!=4 && !cleanupTradeGoods)continue;
        if(t==72){if(janitor && (x==keep[0] || x==keep[1] || x==keep[2]))continue;priority=0;c.score=INT(x,0x60);}
        else if(capsule(x))priority=1;
        else if(stationSalvage(x))priority=2;
        c.priority=priority;result[count++]=c;
    }
    /* Every caller sees the same cheapest excess set. No per-ship budget race. */
    qsort(equipment,eqCount,sizeof(*equipment),compare);
    for(i=0;i<eqCount-50;++i){equipment[i].priority=4;result[count++]=equipment[i];}
    free(equipment);
    qsort(result,count,sizeof(*result),compare);*outCount=count;return result;
}
