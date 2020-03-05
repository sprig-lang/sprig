#ifndef sp_Index_h
#define sp_Index_h
#include "sp_Engine.h"
#include "sp_Promise.h"
#include "sp_SymPool.h"

typedef struct sp_Index sp_Index;

sp_Index* sp_createIndex(sp_Engine* eng, sp_Promise* p);
void      sp_destroyIndex(sp_Index* idx);
unsigned  sp_index(sp_Index* idx, sp_Sym sym, sp_Promise* p);

#endif // sp_Index_h