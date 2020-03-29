#ifndef sp_Resource_h
#define sp_Resource_h
#include "sp_Promise.h"

typedef struct sp_Resource sp_Resource;
typedef struct sp_MemPool  sp_MemPool;

struct sp_Resource {
    void* (*getValue)(sp_Resource*);
    void (*finl)(sp_Resource*);
};

typedef struct {
    sp_Resource r;
    void* value;
} sp_AutoResource;
void* sp_AutoResource_getValue(sp_Resource* r);
void sp_AutoResource_finl(sp_Resource* r);
#define sp_autoResource(VAL) (sp_Resource*)&(sp_AutoResource){.r = {.getValue = sp_AutoResource_getValue, .finl = sp_AutoResource_finl}, .value = (VAL)}

typedef struct {
    sp_Resource r;
    void* value;
    sp_MemPool* mp;
} sp_HeapResource;
void* sp_HeapResource_getValue(sp_Resource* r);
void sp_HeapResource_finl(sp_Resource* r);
#define sp_heapResource(MP, VAL) (sp_Resource*)&(sp_HeapResource){.r = {.getValue = sp_HeapResource_getValue, .finl = sp_HeapResource_finl}, .mp = (MP), .value = (VAL)}

typedef struct {
    sp_Defer d;
    sp_Resource* r;
} sp_FinlDefer;
void sp_FinlDefer_execute(sp_Defer* d);
#define sp_finlDefer(RES) (sp_Defer*)&(sp_FinlDefer){.d = {.execute = sp_FinlDefer_execute}, .r = (RES)}

#endif // sp_Resource