/* Simulates a different Rangers.exe build: every signed site placed at a new, shuffled address with all
   masked bytes (absolute addresses, rel32 operands) changed. The resolver must find every site at its new
   address; a changed instruction byte, a missing site or a duplicate must stop it. */
#include "../src/smartloot.c"
#include <assert.h>
#include <stdio.h>
#define BUF (1u<<20)
static unsigned char buf[BUF];
static size_t place[SITE_COUNT];
static void build(unsigned seed) {
    unsigned i,k; size_t pos=64;
    srand(seed); for(i=0;i<BUF;++i) buf[i]=(unsigned char)(0xcc ^ (i*7));  /* filler that is not code */
    for(i=0;i<SITE_COUNT;++i) {
        unsigned j=(i*13+seed)%SITE_COUNT;           /* shuffled order = moved code */
        (void)j;
        pos+=128+(unsigned)(rand()%512);
        place[i]=pos;
        for(k=0;k<sites[i].length;++k) buf[pos+k]=sites[i].m[k]?sites[i].b[k]:(unsigned char)rand();
        pos+=sites[i].length;
    }
    assert(pos<BUF);
}
int main(void) {
    unsigned i; int bad,n; uint32_t fakeData[SITE_COUNT];
    for(unsigned seed=1;seed<=5;++seed) {
        build(seed);
        for(i=0;i<SITE_COUNT;++i) if(sites[i].kind==1) {           /* operand of a data site -> fake global */
            uint32_t a=(uint32_t)(uintptr_t)&fakeData[i]; memcpy(buf+place[i]+sites[i].offset,&a,4);
        }
        bad=resolveSites(buf,BUF,&n); assert(bad<0);
        for(i=0;i<SITE_COUNT;++i) {
            if(sites[i].kind==0) assert(siteAt[i]==(uintptr_t)(buf+place[i]+sites[i].offset));
            else assert(siteAt[i]==(uintptr_t)&fakeData[i]);
        }
    }
    /* An instruction byte differs (changed code or struct offset): refused. */
    build(7); for(i=0;i<SITE_COUNT;++i){unsigned k=0;while(!sites[i].m[k])++k;buf[place[i]+k]^=0xff;
        bad=resolveSites(buf,BUF,&n);assert(bad==(int)i && n==0);buf[place[i]+k]^=0xff;}
    /* Ambiguous: the same code twice. */
    build(8); memcpy(buf+BUF-4096,buf+place[3],sites[3].length); bad=resolveSites(buf,BUF,&n); assert(bad==3 && n==2);
    puts("PASS signatures: all 32 sites found after code moved and addresses/rel32 changed (5 layouts); changed instruction byte refused per site; duplicate refused.");
    return 0;
}
