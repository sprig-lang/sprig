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

typedef struct sp_MemPool sp_MemPool;

enum {
    sp_ALIGN_DEFAULT = 0
};

sp_MemPool* sp_createMemPool(sp_Promise* p);

void sp_destroyMemPool(sp_MemPool* mp);

void* sp_memAlloc(sp_MemPool* mp, size_t sz, unsigned alignment, sp_Promise* p);
void  sp_memFree(sp_MemPool* mp, void* mem);


#endif // sp_MemPool_h