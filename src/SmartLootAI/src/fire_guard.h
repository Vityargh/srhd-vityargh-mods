static int explosionRadiusSquared=40000;
/* Runs at actual fire time, after earlier guns may have removed equipment. */
static int __cdecl allowCleanupShot(void *ship,void *weapon,void *target) {
    void *star,*items;Candidate *c;int i,j,n,allowed=0,explosive;
    if(ship==(void*)player(0,0))return 1;
    if(!activeJunkCleanup && !serviceMember(ship))return 1; /* Passive: stock fire logic, no filtering. */
    star=PTR(ship,0x24);if(!star)return 0;items=PTR(star,0x30);
    if(!contains(items,target))return 0; /* Do not dereference stale item pointers. */
    c=collectCandidates(ship,items,listCount(items),&n);
    for(i=0;i<n;++i)if(c[i].item==target){allowed=1;break;}
    explosive=BYTE(target,0xc)==72 || BYTE(target,0xc)==70 || BYTE(target,0xc)==75 || quest(target);
    if(allowed && explosive) {
        for(i=0;i<listCount(items);++i) {
            void *x=listAt(items,i);double dx,dy;
            if(x==target)continue;
            dx=(double)FLOAT(x,0x10)-FLOAT(target,0x10);dy=(double)FLOAT(x,0x14)-FLOAT(target,0x14);
            if(dx*dx+dy*dy>explosionRadiusSquared)continue;
            for(j=0;j<n;++j)if(c[j].item==x)break;
            if(j==n){allowed=0;break;} /* Includes artifacts and retained 50 gear. */
        }
    }
    free(c);if(!allowed && weapon && PTR(weapon,0x70)==target)PTR(weapon,0x70)=0;
    return allowed;
}
/* Stock AI asks this routine whether an item may be shot. With cleanup on the mod decides
   (answer "no" to the stock AI); with cleanup off the original routine runs unchanged. */
static void *emitDetonateGate(void *original) {
    unsigned char *w;
    if(!activeJunkCleanup) return original;
    w=allocCode(16);w[0]=0x31;w[1]=0xc0;w[2]=0xc3;return w;
}
static void *emitFireGuard(void *original) {
    unsigned char *w=allocCode(80);unsigned n=0,branch;
    w[n++]=0x9c;w[n++]=0x60;w[n++]=0xfc; /* flags and all registers */
    w[n++]=0x51;w[n++]=0x52;w[n++]=0x50;
    rel32(w+n,0xe8,allowCleanupShot);n+=5;
    w[n++]=0x83;w[n++]=0xc4;w[n++]=12;
    w[n++]=0x85;w[n++]=0xc0;w[n++]=0x74;branch=n++;
    w[n++]=0x61;w[n++]=0x9d;rel32(w+n,0xe9,original);n+=5;
    w[branch]=(unsigned char)(n-branch-1);
    w[n++]=0x61;w[n++]=0x9d;w[n++]=0xc2;w[n++]=4;w[n++]=0;
    return w;
}
