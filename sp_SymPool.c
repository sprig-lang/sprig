#include "sp_SymPool.h"
#include "sp_Test.h"
#include <string.h>

struct sp_SymPool {
    sp_MemPool* mp;

    char*  strBuf;
    size_t strBufTop;
    size_t strBufCap;
};

typedef struct {
    sp_Defer d;
    void* mp;
    void* mem;
} FreeDefer;

static void FreeDefer_execute(sp_Defer* d){
    FreeDefer* fd = (FreeDefer*)d;
    sp_memFree(fd->mp, fd->mem);
}

sp_SymPool* sp_createSymPool(sp_MemPool* mp, sp_Promise* p) {
    sp_SymPool* sp = sp_memAlloc(mp, sizeof(sp_SymPool), 0, p);
    FreeDefer d = {.d = {.execute = FreeDefer_execute }, .mp = mp, .mem = sp};
    p->onCancel(p, (sp_Defer*)&d);

    size_t cap = 256;
    char*  buf = sp_memAlloc(mp, cap, 0, p);

    sp->mp = mp;
    sp->strBuf = buf;
    sp->strBufTop = 0;
    sp->strBufCap = cap;

    p->cancelDefer(p, (sp_Defer*)&d);

    return sp;
}

void sp_destroySymPool(sp_SymPool* sp){
    sp_MemPool* mp = sp->mp;
    sp_memFree(mp, sp->strBuf);
    sp_memFree(mp, sp);
}


sp_Sym sp_getSym(sp_SymPool* sp, char const* str, sp_Promise* p) {
    for(size_t i = 0 ; i < sp->strBufTop ; i++){
        if(strcmp(str, &sp->strBuf[i]) == 0)
            return i;
    }

    size_t len = strlen(str) + 1;
    if(sp->strBufCap - sp->strBufTop < len){
        size_t cap = sp->strBufCap*2 + len;
        char*  buf = sp_memAlloc(sp->mp, cap, 0, p);
        memcpy(buf, sp->strBuf, sp->strBufTop);

        sp_memFree(sp->mp, sp->strBuf);
        sp->strBuf = buf;
        sp->strBufCap = cap;
    }

    memcpy(&sp->strBuf[sp->strBufTop], str, len);
    if(sp->strBufTop >= SYM_MAX){
        p->cancel(p, &(sp_Error){
            .tag = "SYM_MAX",
            .msg = "Symbol space is full",
            .src = SRC_LOCATION
        });
    }

    sp_Sym sym = (sp_Sym)sp->strBufTop;
    sp->strBufTop += len;

    return sym;
}

char const* sp_getStr(sp_SymPool* sp, sp_Sym sym, sp_Promise* p) {
    size_t loc = (size_t)sym;
    if(loc >= sp->strBufTop || (loc != 0 && sp->strBuf[loc] != '\0')){
        p->cancel(p, &(sp_Error){
            .tag = "INVALID_SYM",
            .msg = "Invalid symbol",
            .src = SRC_LOCATION
        });
    }

    return &sp->strBuf[loc];
}

#ifdef sp_TESTS

typedef struct {
    sp_Defer d;
    sp_SymPool* sp;
} TestDefer;
static void TestDefer_execute(sp_Defer* d){
    TestDefer* td = (TestDefer*)d;
    sp_destroySymPool(td->sp);
}

typedef struct {
    sp_Action a;
    sp_SymPool* sp;
    TestDefer   td;

    sp_Sym s1;
    sp_Sym s2;
    sp_Sym s3;
} TestAction;

static void spTest_SymPool_creation(sp_Action* a, sp_Promise* p){
    TestAction* ta = (TestAction*)a;
    sp_MemPool* mp = sp_createMemPool(p);
    sp_SymPool* sp = sp_createSymPool(mp, p);
    sp_assert(sp != NULL, p);

    ta->sp = sp;
    ta->td = (TestDefer){.d = {.execute = TestDefer_execute}, .sp = sp};
    p->onCancel(p, (sp_Defer*)&ta->td);
}

static void spTest_SymPool_destruction(sp_Action* a, sp_Promise* p){
    TestAction* ta = (TestAction*)a;
    p->cancelDefer(p, (sp_Defer*)&ta->td);
    sp_destroySymPool(ta->sp);
}

static void spTest_SymPool_getSym(sp_Action* a, sp_Promise* p){
    TestAction* ta = (TestAction*)a;
    sp_Sym s1 = sp_getSym(ta->sp, "sym1", p);
    sp_Sym s2 = sp_getSym(ta->sp, "sym2", p);
    sp_Sym s3 = sp_getSym(ta->sp, "sym3", p);

    ta->s1 = s1;
    ta->s2 = s2;
    ta->s3 = s3;

    sp_Sym c1 = sp_getSym(ta->sp, "sym1", p);
    sp_Sym c2 = sp_getSym(ta->sp, "sym2", p);
    sp_Sym c3 = sp_getSym(ta->sp, "sym3", p);

    sp_assert(c1 == s1, p);
    sp_assert(c2 == s2, p);
    sp_assert(c3 == s3, p);
}

static void spTest_SymPool_getStr(sp_Action* a, sp_Promise* p){
    TestAction* ta = (TestAction*)a;

    char const* str1 = sp_getStr(ta->sp, ta->s1, p);
    char const* str2 = sp_getStr(ta->sp, ta->s2, p);
    char const* str3 = sp_getStr(ta->sp, ta->s3, p);

    sp_assert(!strcmp(str1, "sym1"), p);
    sp_assert(!strcmp(str2, "sym2"), p);
    sp_assert(!strcmp(str3, "sym3"), p);
}

static fnoreturn void spTest_SymPool_all(sp_Action* a, sp_Promise* p){
    spTest_SymPool_creation(a, p);
    spTest_SymPool_getSym(a, p);
    spTest_SymPool_getStr(a, p);
    spTest_SymPool_destruction(a, p);
    p->complete(p, NULL);
}

void spTest_SymPool(sp_Action* a, sp_Promise* p) {
    TestAction ta = {.a = { .execute = spTest_SymPool_all } };
    void* r;
    if(sp_try((sp_Action*)&ta, &r)){
        p->complete(p, r);
    }
    else {
        p->cancel(p, r);
    }
}
#endif