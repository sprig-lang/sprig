#include "sp_ObjPool.h"
#include "sp_MemPool.h"


// This is a temporary heap/gc implementation designed to be functional
// but not robust.  I'll need to do some more reasearch before deciding
// on a final design for the GC, so this implementation is just a placeholder
// to allow me to get on with implementing the rest of the engine.

typedef struct GlobalAlloc GlobalAlloc;
typedef struct GlobalHeap  GlobalHeap;
typedef struct TaskHeap    TaskHeap;

typedef struct {
    sp_Class* cls;

    uint64_t mark : 1;  // Object has been marked during GC pass
    uint64_t part : 1;  // Object has been allocated but not committed, will not be scanned for references
    uint64_t mut  : 1;  // Object is mutable
    uint64_t next : 61; // Pointer to next object in the heap
    #define RealObject_NEXT_MASK (0x1FFFFFFFFFFFFFFF)
} RealObject;

struct sp_ObjPool {
    mtx_t       lock;
    sp_MemPool* mp;

    thrd_t    gcThread;
    cnd_t     gcCond;
    bool      gcContinue;
    sp_Error* gcError;

    size_t memAllocated;
    size_t memToAllocateBeforeNextGc;

    sp_Anchor*  ancs;
    sp_Worker*  wkrs;
    RealObject* objs;
};

#define SYNC_ERROR &(sp_Error){                         \
    .tag = "OBJ_POOL_SYNC",                             \
    .src = SRC_LOCATION,                                \
    .msg = "Thread synchronization error"               \
}

static inline size_t calcMemToAllocateBeforeNextGc(size_t currentlyAllocated) {
    if(currentlyAllocated == 0)
        return 1024;
    else
        return currentlyAllocated*2;
}

#define GC_SCAN_FRAME_SIZE (64)
typedef struct {
    sp_Visitor  vis;
    sp_ObjPool* op;
    RealObject* frame[GC_SCAN_FRAME_SIZE];
    unsigned    top;
} ScanVisitor;

static void ScanVisitor_visit(sp_Visitor* vis, void* pptr);

static void scanFrame(sp_ObjPool* op, RealObject* root) {
    root->mark = true;
    if(root == NULL || root->part)
        return;
    
    ScanVisitor vis = {
        .vis = { .visit = ScanVisitor_visit },
        .op  = op,
        .top = 0
    };
    
    sp_Class* cls = root->cls;
    cls->travInstance(cls, sp_objToPtr((sp_Object*)root), (sp_Visitor*)&vis);

    while(vis.top > 0) {
        root = vis.frame[--vis.top];
        if(!root->part)
            cls->travInstance(cls, sp_objToPtr((sp_Object*)root), (sp_Visitor*)&vis);
    }
}

static void ScanVisitor_visit(sp_Visitor* vis, void* pptr) {
    sp_Ptr ptr = *((sp_Ptr*)pptr);
    RealObject* obj = (RealObject*)sp_ptrToObj(ptr);
    ScanVisitor* sv = (ScanVisitor*)vis;

    if(obj == NULL || obj->mark)
        return;

    if(sv->top >= GC_SCAN_FRAME_SIZE){
        scanFrame(sv->op, obj);
        return;
    }
    
    obj->mark = true;
    sv->frame[sv->top++] = obj;
}



static void doGc(sp_ObjPool* op){

    // Scan everything, starting at roots
    sp_Anchor* ancIt = op->ancs;
    while(ancIt){
        scanFrame(op, (RealObject*)sp_ptrToObj(*ancIt->ptr));
        ancIt = ancIt->next;
    }

    // Separate garbage
    RealObject* live = NULL;
    RealObject* objIt = op->objs;
    while(objIt) {
        RealObject* obj = objIt;
        objIt = (RealObject*)(uintptr_t)objIt->next;

        if(obj->mark) {
            obj->next = (uintptr_t)live & RealObject_NEXT_MASK;
            live = obj;
        }
        else {
            sp_Class* cls = obj->cls;
            cls->finlInstance(cls, sp_objToPtr((sp_Object*)obj));
            sp_memFree(op->mp, obj);
            op->memAllocated -= cls->fullObjectSize;
        }
    }
    op->objs = live;
}

static int gcThread(void* arg) {
    sp_ObjPool* op = arg;
    while(true){
        if(mtx_lock(&op->lock) != thrd_success){
            op->gcError = SYNC_ERROR;
            return 0;
        }
        if(cnd_wait(&op->gcCond, &op->lock) != thrd_success){
            op->gcError = SYNC_ERROR;
            return 0;
        }
        if(!op->gcContinue)
            break;

        doGc(op);

        if(mtx_unlock(&op->lock) != thrd_success){
            op->gcError = SYNC_ERROR;
            return 0;
        }
    }
    return 0;
}

sp_ObjPool* sp_createObjPool(sp_MemPool* mp, sp_Promise* p) {
    sp_ObjPool* op = sp_memAlloc(mp, sizeof(sp_ObjPool), 0, p);
    p->onAbort(p, sp_deferredFree(mp, op));
    
    op->memAllocated = 0;
    op->memToAllocateBeforeNextGc = calcMemToAllocateBeforeNextGc(op->memAllocated);

    op->ancs = NULL;
    op->wkrs = NULL;
    op->objs = NULL;

    #define INIT_ERROR &(sp_Error){                         \
        .tag = "OBJ_POOL_INIT",                             \
        .src = SRC_LOCATION,                                \
        .msg = "Error initializing ObjPool"                 \
    }

    if(mtx_init(&op->lock, mtx_plain) != thrd_success)
        p->abort(p, INIT_ERROR);
    if(cnd_init(&op->gcCond) != thrd_success)
        p->abort(p, INIT_ERROR);
    
    op->gcContinue = true;
    if(thrd_create(&op->gcThread, gcThread, op) != thrd_success)
        p->abort(p, INIT_ERROR);
    
    return op;
}

void sp_destroyObjPool(sp_ObjPool* op, sp_Promise* p) {

    #define FINL_ERROR &(sp_Error){                         \
        .tag = "OBJ_POOL_FINL",                             \
        .src = SRC_LOCATION,                                \
        .msg = "Error finalizing ObjPool"                   \
    }

    // Tell the GC thread to stop when ready
    op->gcContinue = false;

    // Signal the GC thread to make sure it sees the flag
    if(cnd_signal(&op->gcCond) != thrd_success)
        p->abort(p, FINL_ERROR);
    
    // Wait for GC thread to finish up
    if(thrd_join(op->gcThread, NULL) != thrd_success)
        p->abort(p, FINL_ERROR);
    
    // Finalize all remaining objects
    RealObject* it = op->objs;
    while(it){
        RealObject* obj = it;
        it = (RealObject*)(uintptr_t)it->next;

        sp_Class* cls = obj->cls;
        cls->finlInstance(cls, sp_objToPtr((sp_Object*)obj));
        sp_memFree(op->mp, obj);
    }

    // Finalize the pool itself
    mtx_destroy(&op->lock);
    cnd_destroy(&op->gcCond);
    sp_memFree(op->mp, op);
}

static sp_Worker* unlockCurrentWorker(sp_ObjPool* op, sp_Promise* p) {
    #define WORKER_ERROR &(sp_Error){                       \
        .tag = "OBJ_POOL_WORKER_REG",                       \
        .src = SRC_LOCATION,                                \
        .msg = "Current thread not registered as worker"    \
    }

    thrd_t currentThread = thrd_current();
    sp_Worker* wkr = op->wkrs;
    while(wkr) {
        if(thrd_equal(wkr->thread, currentThread)){
            // Unlock this worker's execution lock to allow for GC to proceed
            if(mtx_unlock(&wkr->lock) != thrd_success)
                p->abort(p, SYNC_ERROR);
            break;
        }
        
        wkr = wkr->next;
    }
    if(wkr == NULL)
        p->abort(p, WORKER_ERROR);
    
    return wkr;
}

static void triggerGc(sp_ObjPool* op, size_t need, sp_Promise* p){
        sp_Worker* wkr = unlockCurrentWorker(op, p);

        // Start a GC cycle
        if(mtx_unlock(&op->lock) != thrd_success)
            p->abort(p, SYNC_ERROR);
        
        if(cnd_signal(&op->gcCond) != thrd_success)
            p->abort(p, SYNC_ERROR);
        
        // Wait for GC to finish
        if(mtx_lock(&op->lock) != thrd_success)
            p->abort(p, SYNC_ERROR);

        // Re-lock execution lock
        if(mtx_lock(&wkr->lock) != thrd_success)
            p->abort(p, SYNC_ERROR);

        if(op->gcError)
            p->abort(p, op->gcError);

        // Adjust GC trigger
        op->memToAllocateBeforeNextGc = (op->memAllocated + need)*2;
}

sp_Ptr sp_allocObj(sp_ObjPool* op, sp_Class* cls, sp_Promise* p) {
    if(mtx_lock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
    if(op->gcError)
        p->abort(p, op->gcError);
    
    size_t need = op->memAllocated + cls->fullObjectSize;
    if(need  >= op->memToAllocateBeforeNextGc) {
        triggerGc(op, need, p);
    }

    RealObject* obj = sp_memAlloc(op->mp, cls->fullObjectSize, 0, p);
    obj->cls = cls;
    obj->mark = 0;
    obj->mut  = cls->isMutable;
    obj->part = true;
    obj->next = (uintptr_t)op->objs & RealObject_NEXT_MASK;
    op->objs = obj;

    if(mtx_unlock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);

    return sp_objToPtr((sp_Object*)obj);
}

void sp_forceGc(sp_ObjPool* op, sp_Promise* p) {
    if(mtx_lock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
    if(op->gcError)
        p->abort(p, op->gcError);

    triggerGc(op, 0, p);

    if(mtx_unlock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
}

void sp_commitObj(sp_ObjPool* op, sp_Ptr ptr, sp_Promise* p) {
    if(mtx_lock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
    if(op->gcError)
        p->abort(p, op->gcError);
    
    RealObject* obj = (RealObject*)sp_ptrToObj(ptr);
    obj->part = false;

    if(mtx_unlock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
}

void sp_linkAnchor(sp_ObjPool* op, sp_Anchor* anc, sp_Promise* p) {

    if(mtx_lock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
    if(op->gcError)
        p->abort(p, op->gcError);
    
    NL_LIST_LINK(anc, &op->ancs);

    if(mtx_unlock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
}

void sp_unlinkAnchor(sp_ObjPool* op, sp_Anchor* anc, sp_Promise* p) {
    if(mtx_lock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
    if(op->gcError)
        p->abort(p, op->gcError);
    
    NL_LIST_UNLINK(anc);

    if(mtx_unlock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
}

void sp_linkWorker(sp_ObjPool* op, sp_Worker* wkr, sp_Promise* p) {
    if(mtx_lock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
    if(op->gcError)
        p->abort(p, op->gcError);
    
    NL_LIST_LINK(wkr, &op->wkrs);

    if(mtx_unlock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
}

void sp_unlinkWorker(sp_ObjPool* op, sp_Worker* wkr, sp_Promise* p) {
    if(mtx_lock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
    if(op->gcError)
        p->abort(p, op->gcError);
    
    NL_LIST_UNLINK(wkr);

    if(mtx_unlock(&op->lock) != thrd_success)
        p->abort(p, SYNC_ERROR);
}

void sp_UnlinkAnchorDefer_execute(sp_Defer* d) {
    sp_UnlinkAnchorDefer* ad = (sp_UnlinkAnchorDefer*)d;
    sp_unlinkAnchor(ad->op, ad->anc, ad->p);
}

