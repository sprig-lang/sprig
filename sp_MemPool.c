#include "sp_MemPool.h"
#include "sp_Test.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#define ALLOC_ERROR (sp_Error) {                        \
    .tag = "MEMORY_ALLOC",                              \
    .msg = "Memory allocation error",                   \
    .src = SRC_LOCATION                                 \
}

sp_MemPool* sp_createMemPool(sp_Promise* p){
    // dummy return value, for now we just use the standard memory pool
    return (sp_MemPool*)1;
}

void sp_destroyMemPool(sp_MemPool* mp){
    // NADA
}


void* sp_memAlloc(sp_MemPool* mp, size_t sz, unsigned alignment, sp_Promise* p) {
    void* mem = NULL;
    if(alignment == 0){
        mem = malloc(sz);
    }
    else{
        size_t pad = alignment - sz%alignment;
        mem = aligned_alloc(alignment, pad + sz);
    }
    if(mem == NULL){
        p->cancel(p, &ALLOC_ERROR);
    }
    return mem;
}

void sp_memFree(sp_MemPool* mp, void* mem) {
    free(mem);
}

#ifdef sp_TESTS

typedef struct {
    sp_Defer d;
    sp_MemPool* mp;
} TestDefer;
void TestDefer_execute(sp_Defer* d){
    TestDefer* td = (TestDefer*)d;
    sp_destroyMemPool(td->mp);
}

typedef struct {
    sp_Action actionHdr;
    sp_MemPool* mp;
    TestDefer   td;
} TestAction;

static void spTest_MemPool_creation(sp_Action* a, sp_Promise* p){
    TestAction* ta = (TestAction*)a;
    ta->mp = sp_createMemPool(p);
    ta->td = (TestDefer){.d = {.execute = TestDefer_execute}, .mp = ta->mp};
    p->onCancel(p, &ta->td);
}

static void spTest_MemPool_destruction(sp_Action* a, sp_Promise* p){
    TestAction* ta = (TestAction*)a;
    p->cancelDefer(p, &ta->td);
    sp_destroyMemPool(ta->mp);
}

static void spTest_MemPool_alloc(sp_Action* a, sp_Promise* p){
    TestAction* ta = (TestAction*)a;
    void* mem = sp_memAlloc(ta->mp, 32, 0, p);
    sp_memFree(ta->mp, mem);
}

static void spTest_MemPool_allocAligned(sp_Action* a, sp_Promise* p){
    TestAction* ta = (TestAction*)a;
    void* m32 = sp_memAlloc(ta->mp, 123, 32, p);
    sp_assert((uintptr_t)m32%32 == 0, p);
    sp_memFree(ta->mp, m32);

    void* m64 = sp_memAlloc(ta->mp, 123, 64, p);
    sp_assert((uintptr_t)m64%64 == 0, p);
    sp_memFree(ta->mp, m64);

    void* m128 = sp_memAlloc(ta->mp, 123, 128, p);
    sp_assert((uintptr_t)m128%128 == 0, p);
    sp_memFree(ta->mp, m128);

    void* m1024 = sp_memAlloc(ta->mp, 123, 1024, p);
    sp_assert((uintptr_t)m1024%1024 == 0, p);
    sp_memFree(ta->mp, m1024);
}

static fnoreturn void spTest_MemPool_all(sp_Action* a, sp_Promise* p){
    spTest_MemPool_creation(a, p);
    spTest_MemPool_alloc(a, p);
    spTest_MemPool_allocAligned(a, p);
    spTest_MemPool_destruction(a, p);
    p->complete(p, NULL);
}

void spTest_MemPool(sp_Action* a, sp_Promise* p) {
    TestAction ta = {.actionHdr = { .execute = spTest_MemPool_all } };
    void* r;
    if(sp_try((sp_Action*)&ta, &r)){
        p->complete(p, r);
    }
    else {
        p->cancel(p, r);
    }
}

#endif // sp_TESTS