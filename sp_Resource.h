#ifndef sp_Resource_h
#define sp_Resource_h
#include "sp_Promise.h"

typedef struct sp_Resource sp_Resource;
typedef struct sp_MemPool  sp_MemPool;

struct sp_Resource {
    void* (*into)(sp_Resource* r, void** dst);
    void (*finl)(sp_Resource* r);
};

typedef struct {
    sp_Resource r;
    void* value;
} sp_AutoResource;
void sp_AutoResource_into(sp_Resource* r, void** dst);
#define sp_autoResource(VAL) (sp_Resource*)&(sp_AutoResource){.r = {.into = sp_AutoResource_into, .finl = NULL}, .value = (VAL)}


typedef struct {
    sp_Defer d;
    sp_Resource* r;
} sp_FinlDefer;
void sp_FinlDefer_execute(sp_Defer* d);
#define sp_finlDefer(RES) (sp_Defer*)&(sp_FinlDefer){.d = {.execute = sp_FinlDefer_execute}, .r = (RES)}

#endif // sp_Resource