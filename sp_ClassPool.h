#ifndef sp_ClassPool_h
#define sp_ClassPool_h
#include <stdint.h>
#include "sp_Class.h"

typedef struct sp_ClassPool sp_ClassPool;
typedef struct sp_MemPool sp_MemPool;
typedef struct sp_ObjPool sp_ObjPool;

sp_ClassPool* sp_createClassPool(sp_MemPool* mp, sp_ObjPool* op);

void sp_destroyClassPool(sp_ClassPool* cp);

sp_Class* sp_createClass(sp_ClassPool* cp);

#endif // sp_ClassPool