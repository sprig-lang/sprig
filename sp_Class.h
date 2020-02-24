#ifndef sp_Class_h
#define sp_Class_h
#include "sp_Object.h"
#include "sp_Visitor.h"
#include "sp_Engine.h"
#include "sp_Index.h"

typedef struct   sp_Class  sp_Class;
typedef struct   sp_Method sp_Method;
typedef struct   sp_SrcLoc sp_SrcLoc;
typedef uint32_t sp_Instr;

struct sp_Class {
    // Classes are objects
    sp_Object obj;

    // Source location
    sp_SrcLoc loc;

    // Object mutability
    bool mutable;

    // Allocation size of a complete sp_Object with this class
    size_t fullObjectSize;

    // Initializer
    sp_Method* init;

    // Method dispatch
    sp_Index* dispatch;

    // Methods
    unsigned   methodCount;
    sp_Method* methodArray;

    // Constants
    unsigned constCount;
    sp_Ref*  constArray;

    // Visitor callback
    void (*accept)(sp_Ref r, sp_Visitor* v);

    // Finalizer callback
    void (*finl)(sp_Ref r, sp_Engine* e);
};

struct sp_SrcLoc {
    sp_Sym   file;
    unsigned line;
    unsigned column;
};

struct sp_Method {
    sp_Sym    name;
    sp_SrcLoc loc;
    bool cachable;
    bool submethod;

    // Call parameters
    unsigned cParamCount;
    sp_Sym*  cParamNames;

    // Activation parameters
    unsigned aParamCount;
    sp_Sym*  aParamNames;

    // Bytecode
    unsigned  instrCount;
    sp_Instr* instrArray;
};



#endif // sp_Class_h