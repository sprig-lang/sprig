#include <string.h>
#include "sp_List.t.h"

#ifndef LIST_INDEX_OUT_OF_RANGE_ERROR
    #define LIST_INDEX_OUT_OF_RANGE_ERROR (sp_Error) {      \
        .tag = "OUT_OF_RANGE",                              \
        .msg = "List index out of range",                   \
        .src = SRC_LOCATION                                 \
    }
#endif


struct sp_ListT(T) {
    sp_MemPool* mp;
    unsigned cap;
    unsigned cnt;
    T*       buf;
};


static inline sp_ListT(T)* sp_ListF(T, create)(sp_MemPool* mp, sp_Promise* p) {
    sp_ListT(T)* ls = sp_memAlloc(mp, sizeof(sp_ListT(T)), 0, p);
    sp_Defer* d = sp_deferredFree(mp, ls);
    p->onAbort(p, d);

    unsigned cap = 7;
    T* buf = sp_memAlloc(mp, sizeof(T)*cap, 0, p);
    p->cancelDefer(p, d);

    ls->mp = mp;
    ls->cap = cap;
    ls->cnt = 0;
    ls->buf = buf;

    return ls;
}

static inline void sp_ListF(T, destroy)(sp_ListT(T)* ls) {
    sp_MemPool* mp = ls->mp;
    sp_memFree(mp, ls->buf);
    sp_memFree(mp, ls);
}

static inline void sp_ListF(T, append)(sp_ListT(T)* ls, T val, sp_Promise* p) {
    if(ls->cnt >= ls->cap){
        unsigned cap = ls->cap*2;
        T* buf = sp_memAlloc(ls->mp, sizeof(T)*cap, 0, p);
        memcpy(buf, ls->buf, sizeof(T)*ls->cnt);

        sp_memFree(ls->mp, ls->buf);
        ls->cap = cap;
        ls->buf = buf;
    }

    ls->buf[ls->cnt++] = val;
}

static inline T sp_ListF(T, get)(sp_ListT(T)* ls, unsigned idx, sp_Promise* p) {
    if(idx >= ls->cnt)
        p->abort(p, &LIST_INDEX_OUT_OF_RANGE_ERROR);
    return ls->buf[idx];
}

static inline void sp_ListF(T, set)(sp_ListT(T)* ls, unsigned idx, T val, sp_Promise* p) {
    if(idx >= ls->cnt)
        p->abort(p, &LIST_INDEX_OUT_OF_RANGE_ERROR);
    ls->buf[idx] = val;
}

static inline void sp_ListF(T, reserve)(sp_ListT(T)* ls, unsigned num, sp_Promise* p) {
    if(ls->cnt + num >= ls->cap){
        unsigned cap = ls->cnt + num;
        T* buf = sp_memAlloc(ls->mp, sizeof(T)*cap, 0, p);
        memcpy(buf, ls->buf, sizeof(T)*ls->cnt);

        sp_memFree(ls->mp, ls->buf);
        ls->cap = cap;
        ls->buf = buf;
    }
}

static inline unsigned sp_ListF(T, count)(sp_ListT(T)* ls) {
    return ls->cnt;
}


static inline void sp_ListF(T, accept)(sp_ListT(T)* ls, sp_Visitor* vis) {
    for(unsigned i = 0 ; i < ls->cnt ; i++)
        vis->visit(vis, &ls->buf[i]);
}