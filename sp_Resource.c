#include "sp_Resource.h"
#include "sp_MemPool.h"

void sp_AutoResource_into(sp_Resource* r, void** dst) {
    *dst = ((sp_AutoResource*)r)->value;
}

void sp_FinlDefer_execute(sp_Defer* d) {
    sp_FinlDefer* fd = (sp_FinlDefer*)d;
    fd->r->finl(fd->r);
}