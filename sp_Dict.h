#ifndef sp_Dict_h
#define sp_Dict_h

#include "sp_Common.h"
#include "sp_Promise.h"

typedef struct sp_Dict sp_Dict;

sp_Dict* sp_newDict(sp_Engine* eng, sp_Promise* p);
void     sp_dictSetByStr(sp_Dict* dict, char const* key, void* val, sp_Promise* p);
void*    sp_dictGetByStr(sp_Dict* dict, char const* key);
void     sp_dictSetBySym(sp_Dict* dict, sp_Sym key, void* val, sp_Promise* p);
void*    sp_dictGetBySym(sp_Dict* dict, sp_Sym key);
#endif