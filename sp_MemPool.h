/**
 * The MemPool serves as an abstraction over whatever memory management system
 * we're using.  While multiple MemPool instances can be allocated, generally
 * we'll only use one pool per runtime instance.  The implementation must be
 * thread safe.
 */

#ifndef sp_MemPool_h
#define sp_MemPool_h
#include "sp_Common.h"
#include "sp_Promise.h"
#include "sp_Visitor.h"

typedef struct sp_MemPool     sp_MemPool;
typedef struct sp_FreeDefer   sp_FreeDefer;
typedef struct sp_FreeVisitor sp_FreeVisitor;

struct sp_FreeDefer {
    sp_Defer    d;
    sp_MemPool* mp;
    void*       mem;
};
void sp_FreeDefer_execute(sp_Defer* d);
#define sp_deferredFree(MP, MEM) (sp_Defer*)&(sp_FreeDefer){.d = {.execute = sp_FreeDefer_execute}, .mp = (MP), .mem = (MEM)}

struct sp_FreeVisitor {
    sp_Visitor  v;
    sp_MemPool* mp;
};
void sp_FreeVisitor_visit(sp_Visitor* v, void* val);
#define sp_visitFree(MP) (sp_Visitor*)&(sp_FreeVisitor){.v = {.visit = sp_FreeVisitor_visit}, .mp = (MP)}

enum {
    sp_ALIGN_DEFAULT = 0
};

sp_MemPool* sp_createMemPool(sp_Promise* p);

void sp_destroyMemPool(sp_MemPool* mp);

void* sp_memAlloc(sp_MemPool* mp, size_t sz, unsigned alignment, sp_Promise* p);
void  sp_memFree(sp_MemPool* mp, void* mem);


#endif // sp_MemPool_h