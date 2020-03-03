#ifndef sp_Object_h
#define sp_Object_h
#include <stdint.h>

typedef struct sp_Object sp_Object;
typedef struct sp_Class sp_Class;

// Type for GCable object pointers, sp_ObjData isn't a real type,
// not defined anywhere in the code, the name is just used as a tag
// to distinguish from other struct pointers
typedef struct sp_ObjData* sp_Ref;

struct sp_Object {
    // Class reference
    sp_Class* cls;

    // Extra flags available to the memory manager, meaning varies
    // depending on which pool/heap the object is currently in
    unsigned flags: 8;

    // Object content follows
};

#endif // sp_Object_h