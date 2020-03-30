#ifndef sp_ObjPool_h
#define sp_ObjPool_h
#include "sp_Promise.h"
#include "sp_Visitor.h"
#include "sp_Object.h"
#include "sp_Class.h"
#include "sp_MemPool.h"
#include <tinycthread.h>

typedef struct sp_Class   sp_Class;
typedef struct sp_ObjPool sp_ObjPool;
typedef struct sp_Anchor  sp_Anchor;
typedef struct sp_Worker  sp_Worker;


// Anchors are used to add root pointers to the GC system, they
// can be used as temporary or permanent roots.  Anchors are
// allocated externally to the GC system and simply linked into
// and unlinked from the ObjPool's anchor list.
struct sp_Anchor {
    sp_Anchor*       next;
    sp_Anchor**      link;
    sp_Ptr volatile  ptr;
};

// Workers represent external threads that will be accessing objects
// provided by this pool, and allow the pool to pause the thread to
// work on GC.
struct sp_Worker {
    sp_Worker*     next;
    sp_Worker**    link;
    
    thrd_t thread;
    mtx_t  lock;
};

// Only sp_objAlloc, sp_objCommit, sp_linkAnchor, sp_unlinkAnchor, and sp_forceGc
// are thread safe in this module, all other functions should only be called during
// engine initialization and finalization, and all from the same thread

sp_ObjPool* sp_createObjPool(sp_MemPool* mp, sp_Promise* p);
void sp_destroyObjPool(sp_ObjPool* op, sp_Promise* p);

sp_Ptr sp_objAlloc(sp_ObjPool* op, sp_Class* cls, sp_Promise* p);
void   sp_objCommit(sp_ObjPool* op, sp_Ptr obj, sp_Promise* p);

void sp_linkAnchor(sp_ObjPool* op, sp_Anchor* anc, sp_Promise* p);
void sp_unlinkAnchor(sp_ObjPool* op, sp_Anchor* anc, sp_Promise* p);

void sp_linkWorker(sp_ObjPool* op, sp_Worker* wkr, sp_Promise* p);
void sp_unlinkWorker(sp_ObjPool* op, sp_Worker* wkr, sp_Promise* p);

void sp_forceGc(sp_ObjPool* op, sp_Promise* p);

typedef struct {
    sp_Defer    d;
    sp_ObjPool* op;
    sp_Anchor*  anc;
    sp_Promise* p;
} sp_UnlinkAnchorDefer;
void sp_UnlinkAnchorDefer_execute(sp_Defer* d);
#define sp_deferredUnlinkAnchor(OP, ANC, P) (sp_Defer*)&(sp_UnlinkAnchorDefer){.d = {.execute = sp_UnlinkAnchorDefer_execute}, .op = (OP), .anc = (ANC), .p = (P)}




typedef struct {
    sp_Resource r;
    sp_Anchor anc;
    sp_ObjPool* op;
} sp_ObjResource;
sp_ObjResource* sp_ObjResource_init(sp_ObjResource* r);
void sp_ObjResource_into(sp_Resource* r, void** dst);
void sp_ObjResource_finl(sp_Resource* r);
#define sp_objResource(OP, OBJ) (sp_Resource*)sp_ObjResource_init(&(sp_ObjResource){.r = {.into = sp_ObjResource_into, .finl = sp_ObjResource_finl}, .anc = {.ptr = obj}})


#endif // sp_ObjPool_h