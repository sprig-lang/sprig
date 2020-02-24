#include "sp_MethodBuilder.h"
#include "sp_List.h"

#define T sp_Sym
    #include "sp_List.t.h"
    #include "sp_List.t.c"
#undef T

#define T sp_Instr
    #include "sp_List.t.h"
    #include "sp_List.t.c"
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
    sp_Defer* freeBdrDefer = sp_deferedFree(com->mp, bdr);
    p->onCancel(p, freeBdrDefer);

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
    sp_ListF(sp_Sym, append)(bdr->instructions, instr, p);
    return idx;
}

sp_Method* sp_methodBuilderProduce(sp_MethodBuilder* bdr, sp_Promise* p) {
    sp_Method* mth = sp_memAlloc(bdr->com->mp, sizeof(sp_Method), 0, p);
    sp_Defer* freeMthDefer = sp_deferedFree(bdr->com->mp, mth);
    p->onCancel(p, freeMthDefer);

    unsigned  cParamCount = sp_ListF(sp_Sym, count)(bdr->callParams);
    sp_Sym*   cParamArray = sp_memAlloc(bdr->com->mp, sizeof(sp_Sym)*cParamCount, 0, p);
    sp_Defer* cFreeDefer  = sp_deferedFree(bdr->com->mp, cParamArray);
    p->onCancel(p, cFreeDefer);

    for(unsigned i = 0 ; i < cParamCount ; i++)
        cParamArray[i] = sp_ListF(sp_Sym, get)(bdr->callParams, i, p);
    
    unsigned  aParamCount = sp_ListF(sp_Sym, count)(bdr->activationParams);
    sp_Sym*   aParamArray = sp_memAlloc(bdr->com->mp, sizeof(sp_Sym)*cParamCount, 0, p);
    sp_Defer* aFreeDefer  = sp_deferedFree(bdr->com->mp, aParamArray);
    p->onCancel(p, aFreeDefer);
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

    return mth;
}