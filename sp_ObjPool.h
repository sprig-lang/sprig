#ifndef sp_ObjPool_h
#define sp_ObjPool_h
#include "sp_Object.h"

typedef struct sp_ObjPool sp_ObjPool;
typedef struct sp_Anchor  sp_Anchor;

struct sp_ObjPool {
    sp_Ref (*alloc)(sp_ObjPool* op, sp_Class* cls, sp_Promise* p);
    void   (*addAnchor)(sp_ObjPool* op, sp_Anchor* anc);
    void   (*remAnchor)(sp_ObjPool* op, sp_Anchor* anc);
    void   (*accept)(sp_ObjPool* op, sp_Visitor* vis);
    void*  (*finl)(sp_ObjPool* op);
};

sp_ObjPool* sp_createMasterHeap(sp_MemPool* mp, sp_Promise* p);
sp_ObjPool* sp_createIncubator(sp_ObjPool* op, sp_Promise* p);

#endif // sp_ObjPool_h