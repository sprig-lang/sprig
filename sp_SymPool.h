/**
 * The SymPool interns strings into symbols, which makes hashing and comparison
 * more efficient.
 */

#ifndef sp_SymPool_h
#define sp_SymPool_h
#include <stdint.h>
#include "sp_MemPool.h"

typedef uint32_t sp_Sym;
typedef struct sp_SymPool sp_SymPool;

#define SYM_MAX UINT32_MAX

sp_SymPool* sp_createSymPool(sp_MemPool* mp, sp_Promise* p);
void        sp_destroySymPool(sp_SymPool* sp);
sp_Sym      sp_getSym(sp_SymPool* sp, char const* str, sp_Promise* p);
char const* sp_getStr(sp_SymPool* sp, sp_Sym sym, sp_Promise* p);

#endif // sp_SymPool_h