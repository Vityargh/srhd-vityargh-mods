/* Conditional native gates; ordinary ships always execute original routines. */
static void *emitServiceGate(void *original,int targetInEdx,int resultValue,int popBytes) {
    unsigned char *w=allocCode(80);unsigned n=0,branch;
    w[n++]=0x9c;w[n++]=0x60;w[n++]=0xfc;w[n++]=targetInEdx?0x52:0x50;
    rel32(w+n,0xe8,serviceMember);n+=5;w[n++]=0x83;w[n++]=0xc4;w[n++]=4;
    w[n++]=0x85;w[n++]=0xc0;w[n++]=0x74;branch=n++;
    w[n++]=0x61;w[n++]=0x9d;
    if(popBytes==12){ /* ApplyDamage's out DamageColor is the second stack word. */
        unsigned char clear[]={0x8b,0x54,0x24,8,0xc7,0x02,0,0,0,0};memcpy(w+n,clear,sizeof(clear));n+=sizeof(clear);
    }
    w[n++]=0xb8;*(int*)(w+n)=resultValue;n+=4;
    if(popBytes){w[n++]=0xc2;w[n++]=(unsigned char)popBytes;w[n++]=0;}else w[n++]=0xc3;
    w[branch]=(unsigned char)(n-branch-1);w[n++]=0x61;w[n++]=0x9d;
    rel32(w+n,0xe9,original);return w;
}
