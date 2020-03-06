#ifndef sp_Object_h
#define sp_Object_h
#include <stdint.h>

typedef struct sp_Object sp_Object;
typedef struct sp_Class sp_Class;

struct sp_Object {
    // Class reference
    sp_Class* cls;

    // Extra flags available to the memory manager, meaning varies
    // depending on which pool/heap the object is currently in
    unsigned flags: 8;

    // Object content follows
};

#endif // sp_Object_h