#include "sp_Index.h"
#include "sp_Engine.h"
#include <limits.h>

struct sp_Index {
    sp_Engine* eng;
    sp_Sym*    keys;
    unsigned*  locs;
    unsigned   cnt;
    unsigned   cap;
};


sp_Index* sp_createIndex(sp_Engine* eng, sp_Promise* p) {
    sp_Index* idx = sp_memAlloc(eng->mp, sizeof(sp_Index), 0, p);
    sp_Defer* freeIdxDefer = sp_deferredFree(eng->mp, idx);
    p->onAbort(p, freeIdxDefer);

    unsigned cap = 7;

    sp_Sym* keys = sp_memAlloc(eng->mp, sizeof(sp_Sym)*cap, 0, p);
    sp_Defer* freeKeysDefer = sp_deferredFree(eng->mp, keys);
    p->onAbort(p, freeKeysDefer);
    for(unsigned i = 0 ; i < cap ; i++)
        keys[i] = SYM_MAX;

    unsigned* locs = sp_memAlloc(eng->mp, sizeof(unsigned)*cap, 0, p);
    sp_Defer* freeLocsDefer = sp_deferredFree(eng->mp, locs);
    p->onAbort(p, freeLocsDefer);


    idx->eng  = eng;
    idx->keys = keys;
    idx->locs = locs;
    idx->cnt  = 0;
    idx->cap  = cap;

    p->cancelDefer(p, freeIdxDefer);
    p->cancelDefer(p, freeKeysDefer);
    p->cancelDefer(p, freeLocsDefer);

    return idx;
}

void sp_destroyIndex(sp_Index* idx) {
    sp_Engine* eng = idx->eng;
    sp_memFree(eng->mp, idx->keys);
    sp_memFree(eng->mp, idx->locs);
    sp_memFree(eng->mp, idx);
}


// TODO: Need to find a good hash function at some point
static uint32_t hashSym(sp_Sym sym){
    uint32_t h1 = sym ^ (sym << 8);
    uint32_t h2 = h1 ^ (h1 << 16);
    uint32_t h3 = h2 ^ (h2 << 1);
    return h3;
}

static void growIdx(sp_Index* idx, sp_Promise* p){
    if(idx->cap >= UINT_MAX / 2){
        p->abort(p, &(sp_Error){
            .tag = "MAX_INDEX_CAPACITY_EXCEEDED",
            .msg = "Index has exceeded max capacity",
            .src = SRC_LOCATION
        });
    }

    unsigned cap = idx->cap*2;

    sp_Sym* keys = sp_memAlloc(idx->eng->mp, sizeof(sp_Sym)*cap, 0, p);
    sp_Defer* freeKeysDefer = sp_deferredFree(idx->eng->mp, keys);
    p->onAbort(p, freeKeysDefer);

    unsigned* locs = sp_memAlloc(idx->eng->mp, sizeof(unsigned)*cap, 0, p);
    sp_Defer* freeLocsDefer = sp_deferredFree(idx->eng->mp, locs);
    p->onAbort(p, freeLocsDefer);


    sp_memFree(idx->eng->mp, idx->keys);
    sp_memFree(idx->eng->mp, idx->locs);

    idx->keys = keys;
    idx->locs = locs;
    idx->cap  = cap;

    p->cancelDefer(p, freeKeysDefer);
    p->cancelDefer(p, freeLocsDefer);
}

unsigned sp_index(sp_Index* idx, sp_Sym sym, sp_Promise* p) {

    // If we're out of room then grow the index
    if(idx->cnt >= idx->cap)
        growIdx(idx, p);
    
    uint32_t h = hashSym(sym);
    unsigned i = (unsigned)h % idx->cap;

    // There will be at least one empty slot available
    while(true){
        if(idx->keys[i] == sym) {
            return idx->locs[i];
        }
        if(idx->keys[i] == SYM_MAX) {
            idx->keys[i] = sym;
            idx->locs[i] = idx->cnt++;
            return idx->locs[i];
        }
        i = (i+1) % idx->cap;
    }

    return UINT_MAX;
}