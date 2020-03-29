#include "sp_MethodBuilder.h"
#include "sp_List.h"

#define T sp_Sym
    #include "generic/sp_List.t.h"
    #include "generic/sp_List.t.c"
#undef T

#define T sp_Instr
    #include "generic/sp_List.t.h"
    #include "generic/sp_List.t.c"
#undef T

struct sp_MethodBuilder {
    sp_Compiler* com;
    sp_Sym       name;
    sp_SrcLoc    loc;

    sp_ListT(sp_Sym)* callParams;
    sp_ListT(sp_Sym)* activationParams;
    sp_ListT(sp_Instr)* instructions;
};

typedef struct {
    sp_Defer d;
    sp_ListT(sp_Sym)* list;
} DestroySymListDefer;
static void DestroySymListDefer_execute(sp_Defer* d){
    DestroySymListDefer* dd = (DestroySymListDefer*)d;
    sp_ListF(sp_Sym, destroy)(dd->list);
}

sp_MethodBuilder* sp_createMethodBuilder(sp_Compiler* com, sp_Sym name, sp_SrcLoc* loc, sp_Promise* p) {
    sp_MethodBuilder* bdr = sp_memAlloc(com->mp, sizeof(sp_MethodBuilder), 0, p);
    sp_Defer* freeBdrDefer = sp_deferredFree(com->mp, bdr);
    p->onAbort(p, freeBdrDefer);

    sp_ListT(sp_Sym)* callParams = sp_ListF(sp_Sym, create)(com->mp, p);
    sp_Defer* destroyCallParamsDefer = (sp_Defer*)&(DestroySymListDefer){
        .d = {.execute = DestroySymListDefer_execute},
        .list = callParams
    };

    sp_ListT(sp_Sym)* activationParams = sp_ListF(sp_Sym, create)(com->mp, p);
    sp_Defer* destroyActivationParamsDefer = (sp_Defer*)&(DestroySymListDefer){
        .d = {.execute = DestroySymListDefer_execute},
        .list = activationParams
    };

    sp_ListT(sp_Instr)* instructions = sp_ListF(sp_Instr, create)(com->mp, p);
    
    p->cancelDefer(p, freeBdrDefer);
    p->cancelDefer(p, destroyCallParamsDefer);
    p->cancelDefer(p, destroyActivationParamsDefer);

    bdr->com = com;
    bdr->name = name;
    bdr->loc = *loc;
    bdr->callParams = callParams;
    bdr->activationParams = activationParams;
    bdr->instructions = instructions;
    return bdr;
}

void sp_destroyMethodBuilder(sp_MethodBuilder* bdr) {
    sp_ListF(sp_Sym, destroy)(bdr->callParams);
    sp_ListF(sp_Sym, destroy)(bdr->activationParams);
    sp_ListF(sp_Instr, destroy)(bdr->instructions);
    sp_memFree(bdr->com->mp, bdr);
}

unsigned sp_methodBuilderAddCallParam(sp_MethodBuilder* bdr, sp_Sym name, sp_Promise* p) {
    unsigned idx = sp_ListF(sp_Sym, count)(bdr->callParams);
    sp_ListF(sp_Sym, append)(bdr->callParams, name, p);
    return idx;
}

unsigned sp_methodBuilderAddActivationParam(sp_MethodBuilder* bdr, sp_Sym name, sp_Promise* p) {
    unsigned idx = sp_ListF(sp_Sym, count)(bdr->activationParams);
    sp_ListF(sp_Sym, append)(bdr->activationParams, name, p);
    return idx;
}

unsigned sp_methodBuilderAddInstr(sp_MethodBuilder* bdr, sp_Instr instr, sp_Promise* p) {
    unsigned idx = sp_ListF(sp_Instr, count)(bdr->instructions);
    sp_ListF(sp_Instr, append)(bdr->instructions, instr, p);
    return idx;
}

static void Method_destroy(sp_Method* mth, sp_Compiler* com){
    sp_memFree(com->mp, mth->cParamNames);
    sp_memFree(com->mp, mth->aParamNames);
    sp_memFree(com->mp, mth->instrArray);
    sp_memFree(com->mp, mth);
}

sp_Method* sp_methodBuilderProduce(sp_MethodBuilder* bdr, sp_Promise* p) {
    sp_Method* mth = sp_memAlloc(bdr->com->mp, sizeof(sp_Method), 0, p);
    sp_Defer* freeMthDefer = sp_deferredFree(bdr->com->mp, mth);
    p->onAbort(p, freeMthDefer);

    unsigned  cParamCount = sp_ListF(sp_Sym, count)(bdr->callParams);
    sp_Sym*   cParamArray = sp_memAlloc(bdr->com->mp, sizeof(sp_Sym)*cParamCount, 0, p);
    sp_Defer* cFreeDefer  = sp_deferredFree(bdr->com->mp, cParamArray);
    p->onAbort(p, cFreeDefer);

    for(unsigned i = 0 ; i < cParamCount ; i++)
        cParamArray[i] = sp_ListF(sp_Sym, get)(bdr->callParams, i, p);
    
    unsigned  aParamCount = sp_ListF(sp_Sym, count)(bdr->activationParams);
    sp_Sym*   aParamArray = sp_memAlloc(bdr->com->mp, sizeof(sp_Sym)*cParamCount, 0, p);
    sp_Defer* aFreeDefer  = sp_deferredFree(bdr->com->mp, aParamArray);
    p->onAbort(p, aFreeDefer);
    for(unsigned i = 0 ; i < aParamCount ; i++)
        aParamArray[i] = sp_ListF(sp_Sym, get)(bdr->activationParams, i, p);
    

    unsigned  instrCount = sp_ListF(sp_Instr, count)(bdr->instructions);
    sp_Instr* instrArray = sp_memAlloc(bdr->com->mp, sizeof(sp_Instr)*instrCount, 0, p);
    for(unsigned i = 0 ; i < instrCount ; i++)
        instrArray[i] = sp_ListF(sp_Instr, get)(bdr->instructions, i, p);
    

    p->cancelDefer(p, freeMthDefer);
    p->cancelDefer(p, cFreeDefer);
    p->cancelDefer(p, aFreeDefer);

    mth->name = bdr->name;
    mth->loc  = bdr->loc;

    // For now nothing is cacheable
    mth->cachable = false;

    // Sub-method if the name contains '$'
    char const* nameStr = sp_getStr(bdr->com->sp, bdr->name, p);
    mth->submethod = (strchr(nameStr, '$') != NULL);

    // Destructor
    mth->destroy = Method_destroy;
    return mth;
}

#ifdef sp_TESTS
#include "sp_Test.h"

typedef struct {
    sp_Action actionHdr;
} TestAction;

typedef struct {
    sp_Defer d;
    sp_MemPool* mp;
} DestroyMemPoolDefer;
static void DestroyMemPoolDefer_execute(sp_Defer* d){
    DestroyMemPoolDefer* dm = (DestroyMemPoolDefer*)d;
    sp_destroyMemPool(dm->mp);
}

typedef struct {
    sp_Defer d;
    sp_SymPool* sp;
} DestroySymPoolDefer;
static void DestroySymPoolDefer_execute(sp_Defer* d){
    DestroySymPoolDefer* dd = (DestroySymPoolDefer*)d;
    sp_destroySymPool(dd->sp);
}

typedef struct {
    sp_Defer d;
    sp_MethodBuilder* mb;
} DestroyMethodBuilderDefer;
static void DestroyMethodBuilderDefer_execute(sp_Defer* d){
    DestroyMethodBuilderDefer* dm = (DestroyMethodBuilderDefer*)d;
    sp_destroyMethodBuilder(dm->mb);
}

typedef struct {
    sp_Defer d;
    sp_Method* mth;
    sp_Compiler* com;
} DestroyMethodDefer;
static void DestroyMethodDefer_execute(sp_Defer* d){
    DestroyMethodDefer* dd = (DestroyMethodDefer*)d;
    dd->mth->destroy(dd->mth, dd->com);
}

static fnoreturn void spTest_MethodBuilder_build(sp_Action* a, sp_Promise* p){
    sp_MemPool* mp = sp_createMemPool(p);
    sp_Defer* destroyMemPoolDefer = (sp_Defer*)&(DestroyMemPoolDefer){
        .d = { .execute = DestroyMemPoolDefer_execute },
        .mp = mp
    };
    p->onAbort(p, destroyMemPoolDefer);

    sp_SymPool* sp = sp_createSymPool(mp, p);
    sp_Defer* destroySymPoolDefer = (sp_Defer*)&(DestroySymPoolDefer){
        .d = { .execute = DestroySymPoolDefer_execute },
        .sp = sp
    };
    p->onAbort(p, destroySymPoolDefer);

    // TODO
    sp_ObjPool* op = NULL;

    sp_Sym      name = sp_getSym(sp, "foo", p);
    sp_Compiler com = { .mp = mp, .sp = sp, .op = op };
    sp_SrcLoc   loc = { .file = sp_getSym(sp, "foo.sp", p), .line = 0, .column = 0 };
    sp_MethodBuilder* bdr = sp_createMethodBuilder(&com, name, &loc, p);
    sp_Defer* destroyMethodBuilderDefer = (sp_Defer*)&(DestroyMethodBuilderDefer){
        .d = { .execute = DestroyMethodBuilderDefer_execute },
        .mb = bdr
    };
    p->onAbort(p, destroyMethodBuilderDefer);

    for(unsigned i = 0; i < 20 ; i++){
        char pname[4] = {'a', '0' + i/10, '0' + i%10 , '\0'};
        unsigned ap = sp_methodBuilderAddActivationParam(bdr, sp_getSym(sp, pname, p), p);
        sp_assert(ap == i, p);
    }

    for(unsigned i = 0; i < 20 ; i++){
        char pname[4] = {'c', '0' + i/10, '0' + i%10 , '\0'};
        unsigned ap = sp_methodBuilderAddCallParam(bdr, sp_getSym(sp, pname, p), p);
        sp_assert(ap == i, p);
    }

    for(unsigned i = 0 ; i < 100 ; i++){
        unsigned loc = sp_methodBuilderAddInstr(bdr, sp_instr(sp_OpCode_NOP, 0), p);
        sp_assert(loc == i, p);
    }

    sp_methodBuilderAddInstr(bdr, sp_instr(sp_OpCode_ACTIVATE, 3), p);
    sp_methodBuilderAddInstr(bdr, sp_instr(sp_OpCode_INVOKE, 4), p);

    sp_Method* mth = sp_methodBuilderProduce(bdr, p);
    sp_Defer* destroyMethodDefer = (sp_Defer*)&(DestroyMethodDefer){
        .d = { .execute = DestroyMethodDefer_execute },
        .mth = mth,
        .com = &com
    };
    p->onAbort(p, destroyMethodDefer);


    sp_assert(mth->aParamCount == 100, p);
    sp_assert(mth->aParamNames[0] == sp_getSym(sp, "a00", p), p);

    sp_assert(mth->cParamCount == 100, p);
    sp_assert(mth->cParamNames[0] == sp_getSym(sp, "c00", p), p);

    sp_assert(mth->instrCount == 102, p);
    sp_assert(mth->instrArray[mth->instrCount-1] == sp_instr(sp_OpCode_INVOKE, 4), p);

    mth->destroy(mth, &com);
    p->cancelDefer(p, destroyMethodDefer);

    sp_destroyMethodBuilder(bdr);
    p->cancelDefer(p, destroyMethodBuilderDefer);

    sp_destroySymPool(sp);
    p->cancelDefer(p, destroySymPoolDefer);

    sp_destroyMemPool(mp);
    p->cancelDefer(p, destroyMemPoolDefer);

    p->complete(p, NULL);
}

void spTest_MethodBuilder(sp_Action* a, sp_Promise* p) {
    TestAction ta = {.actionHdr = { .execute = spTest_MethodBuilder_build } };
    void* r;
    if(sp_try((sp_Action*)&ta, &r)){
        p->complete(p, r);
    }
    else {
        p->abort(p, r);
    }
}

#endif // sp_TESTS