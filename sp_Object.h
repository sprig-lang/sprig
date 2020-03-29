#ifndef sp_Object_h
#define sp_Object_h
#include <stdint.h>

typedef struct sp_Object sp_Object;
typedef struct sp_Class sp_Class;

// Type for GCable object pointers, sp_FakeType_ObjData isn't a real type,
// not defined anywhere in the code, the name is just used as a tag
// to distinguish from other struct pointers
typedef struct sp_FakeType_ObjData* sp_Ptr;

struct sp_Object {
    // Class reference
    sp_Class* cls;

    // Extra state for memory manager
    uint64_t meta: 8;
};

#define sp_objToPtr(OBJ) (sp_Ptr)((char*)(OBJ) + sizeof(sp_Object))
#define sp_ptrToObj(PTR) (sp_Object*)((char*)(PTR) - sizeof(sp_Object))

#endif // sp_Object_h