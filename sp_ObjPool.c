#include "sp_ObjPool.h"
#include "sp_MemPool.h"

#include <tinycthread.h>

typedef struct GlobalAlloc GlobalAlloc;
typedef struct GlobalHeap  GlobalHeap;
typedef struct TaskHeap    TaskHeap;

struct GlobalAlloc {
    uint64_t dead : 1;  // Object is dead, has already been finalized
    uint64_t mark : 1;  // Object has been marked during GC pass
    uint64_t part : 1;  // Object has been allocated but not fully initialized, can't be GCed
    uint64_t mut  : 1;  // Object is mutable
    uint64_t next : 60; // Pointer to next object in the heap
    char     data[];
};

struct GlobalHeap {
    mtx_t      lock;
    thread_t   thread;
    sp_Object* objs;
};