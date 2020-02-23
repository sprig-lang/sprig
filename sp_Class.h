#ifndef sp_Class_h
#define sp_Class_h
#include "sp_Object.h"
#include "sp_Dict.h"
#include "sp_Visitor.h"

typedef struct sp_Class sp_Class;

struct sp_Class {
    // Classes are objects
    sp_Object obj;

    // Allocation size of a complete sp_Object with this class
    size_t fullObjectSize;

    // Methods
    sp_Dict* methods;

    // Visitor callback
    void (*visit)(sp_Object* o, sp_Visitor* v);

    // Finalizer callback
    void (*finl)(sp_Object* o, sp_Engine* e);
};

#endif // sp_Class_h