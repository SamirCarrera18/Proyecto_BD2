#include "ttree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
static unsigned rng=20260911u;
static unsigned next_random(void) { rng^=rng<<13; rng^=rng>>17; rng^=rng<<5; return rng; }
static void check(TTree *t) { char error[150]; if (!tt_validate(t,error,sizeof error)) { fprintf(stderr,"%s; size=%zu\n",error,t->size); abort(); } }
typedef struct Walk { int64_t previous; size_t count; } Walk;
static bool sorted(int32_t k,void *ctx) { Walk *w=ctx; assert(k>w->previous); w->previous=k; w->count++; return true; }
static void scenario(int mode) {
    enum { N=30000 };
    int32_t *keys=malloc(N*sizeof(*keys)); TTree t; int i; Walk w={ (int64_t)INT32_MIN-1,0 };
    assert(keys); tt_init(&t,NULL,NULL,NULL); check(&t);
    for(i=0;i<N;i++) keys[i]=i*3-45000;
    if(mode==1) for(i=0;i<N/2;i++) { int32_t x=keys[i]; keys[i]=keys[N-1-i]; keys[N-1-i]=x; }
    if(mode==2) for(i=N-1;i>0;i--) { unsigned j=next_random()%(i+1); int32_t x=keys[i]; keys[i]=keys[j]; keys[j]=x; }
    for(i=0;i<N;i++) { assert(tt_insert(&t,keys[i])==TT_INSERTED); if(i<500 || i%200==0) check(&t); }
    check(&t);
    for(i=0;i<N;i++) { assert(tt_contains(&t,keys[i])); assert(!tt_contains(&t,keys[i]+1)); assert(tt_insert(&t,keys[i])==TT_DUPLICATE); }
    assert(tt_insert(&t,INT32_MIN)==TT_INSERTED); assert(tt_insert(&t,INT32_MAX)==TT_INSERTED); check(&t);
    assert(tt_visit_range(&t,INT32_MIN,INT32_MAX,sorted,&w)); assert(w.count==N+2);
    w.previous=-1; w.count=0; assert(tt_visit_range(&t,0,30,sorted,&w)); assert(w.count==11);
    { int32_t bad[]={1,1}; assert(tt_build(&t,bad,2)==TT_DUPLICATE); assert(t.size==N+2); check(&t); }
    assert(tt_build(&t,keys,N)==TT_INSERTED); check(&t); assert(t.size==N);
    tt_clear(&t); tt_clear(&t); check(&t); free(keys);
}
typedef struct Alloc { int remaining; size_t live; } Alloc;
static void *limited(size_t n,void *ctx) { Alloc *a=ctx; void *p; if(a->remaining==0)return NULL; if(a->remaining>0)a->remaining--; p=malloc(n); if(p)a->live++;return p; }
static void release(void *p,void *ctx) { Alloc *a=ctx; a->live--;free(p); }
static void allocation_failure(void) {
    TTree t; Alloc a={-1,0}; int i; tt_init(&t,limited,release,&a);
    for(i=0;i<TT_CAPACITY;i++) assert(tt_insert(&t,i*2)==TT_INSERTED);
    a.remaining=0;
    assert(tt_insert(&t,1)==TT_OOM); assert(t.size==TT_CAPACITY); check(&t);
    for(i=0;i<TT_CAPACITY;i++) assert(tt_contains(&t,i*2));
    a.remaining=1;
    { int32_t keys[TT_CAPACITY+1]; for(i=0;i<=TT_CAPACITY;i++)keys[i]=i;
      assert(tt_build(&t,keys,TT_CAPACITY+1)==TT_OOM);check(&t); }
    tt_clear(&t);assert(a.live==0);
}
int main(void) { int i; for(i=0;i<3;i++)scenario(i); allocation_failure(); printf("OK: capacidad=%d; orden ascendente, descendente, aleatorio, rangos, extremos, duplicados y fallos de memoria.\n",TT_CAPACITY); return 0; }
