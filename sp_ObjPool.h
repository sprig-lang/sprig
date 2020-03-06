#ifndef sp_ObjPool_h
#define sp_ObjPool_h
#include "sp_Promise.h"
#include "sp_Visitor.h"
#include "sp_Object.h"
#include "sp_MemPool.h"

typedef struct sp_Class   sp_Class;
typedef struct sp_ObjPool sp_ObjPool;
typedef struct sp_Anchor  sp_Anchor;

// Type for GCable object pointers, sp_FakeType_ObjData isn't a real type,
// not defined anywhere in the code, the name is just used as a tag
// to distinguish from other struct pointers
typedef struct sp_FakeType_ObjData* sp_Ref;

struct sp_ObjPool {
    sp_Ref (*alloc)(sp_ObjPool* op, sp_Class* cls, sp_Promise* p);
    void   (*sweep)(sp_ObjPool* from, sp_ObjPool* to, sp_Promise* p);
    void   (*addAnchor)(sp_ObjPool* op, sp_Anchor* anc);
    void   (*remAnchor)(sp_ObjPool* op, sp_Anchor* anc);
    void   (*traverse)(sp_ObjPool* op, sp_Visitor* vis);
    void*  (*destroy)(sp_ObjPool* op);
    void   (*lock)(sp_ObjPool* op, sp_Promise* p);
    void   (*unlock)(sp_ObjPool* op, sp_Promise* p);
};

struct sp_Anchor {
    sp_Anchor*      next;
    sp_Anchor**     link;
    sp_Ref volatile obj;
};

sp_ObjPool* sp_createGlobalHeap(sp_MemPool* mp, sp_Promise* p);
sp_ObjPool* sp_createTaskHeap(sp_ObjPool* op, sp_Promise* p);


#endif // sp_ObjPool_h