#include "sp_Resource.h"
#include "sp_MemPool.h"

void* sp_AutoResource_getValue(sp_Resource* r) {
    return ((sp_AutoResource*)r)->value;
}
void sp_AutoResource_finl(sp_Resource* r) {
    // NADA
}

void* sp_HeapResource_getValue(sp_Resource* r) {
    return ((sp_HeapResource*)r)->value;
}
void sp_HeapResource_finl(sp_Resource* r) {
    sp_HeapResource* hr = (sp_HeapResource*)r;
    sp_memFree(hr->mp, hr->value);
}

void sp_FinlDefer_execute(sp_Defer* d) {
    sp_FinlDefer* fd = (sp_FinlDefer*)d;
    fd->r->finl(fd->r);
}