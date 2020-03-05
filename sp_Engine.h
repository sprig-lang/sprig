#ifndef sp_Engine_h
#define sp_Engine_h
#include "sp_MemPool.h"
#include "sp_SymPool.h"
#include "sp_ObjPool.h"

typedef struct sp_Engine sp_Engine;

struct sp_Engine {
    sp_MemPool* mp;
    sp_SymPool* sp;
    sp_ObjPool* op;
};

#endif // sp_Engine_h