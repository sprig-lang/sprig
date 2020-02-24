#ifndef sp_Compiler_h
#define sp_Compiler_h
#include "sp_MemPool.h"
#include "sp_SymPool.h"
#include "sp_ObjPool.h"

typedef struct sp_Compiler sp_Compiler;

struct sp_Compiler {
    sp_SymPool* sp;
    sp_MemPool* mp;
    sp_ObjPool* op;
};



#endif // sp_Compiler_h