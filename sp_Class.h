#ifndef sp_Class_h
#define sp_Class_h
#include "sp_Object.h"
#include "sp_Visitor.h"
#include "sp_Index.h"

typedef struct   sp_Compiler sp_Compiler;
typedef struct   sp_Engine   sp_Engine;
typedef struct   sp_Class    sp_Class;
typedef struct   sp_Method   sp_Method;
typedef struct   sp_SrcLoc   sp_SrcLoc;
typedef uint32_t sp_Instr;
typedef enum     sp_OpCode sp_OpCode;


struct sp_SrcLoc {
    sp_Sym   file;
    unsigned line;
    unsigned column;
};

struct sp_Class {
    // Source location
    sp_SrcLoc loc;

    // Object mutability
    bool isMutable;

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
    sp_Ptr*  constArray;
    
    // Object creation, finalization, and traversal functions
    void (*makeInstance)(sp_Class* cls, sp_Ptr r);
    void (*finlInstance)(sp_Class* cls, sp_Ptr r);
    void (*travInstance)(sp_Class* cls, sp_Ptr r, sp_Visitor* vis);

    // Free this class
    void (*destroy)(sp_Class* cls, sp_Compiler* com);
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

    // Destroy this method
    void (*destroy)(sp_Method* mth, sp_Compiler* com);
};

enum sp_OpCode {
    sp_OpCode_NOP,
    sp_OpCode_ACTIVATE,
    sp_OpCode_INVOKE,
    sp_OpCode_LCL_LOAD,
    sp_OpCode_LCL_SAVE,
    sp_OpCode_OBJ_LOAD,
    sp_OpCode_OBJ_SAVE,
    sp_OpCode_PUSH_METHOD,
    sp_OpCode_PUSH_CONST,
    sp_OpCode_PUSH_NUMBER,
    sp_OpCode_PUSH_STRING,
    sp_OpCode_PUSH_SYMBOL,
    sp_OpCode_USE
};

#define sp_instr(OPCODE, OPERAND) ((sp_Instr)(OPERAND) << 8 | (sp_Instr)(OPCODE))
#define sp_getOpCode(INSTR)       ((INSTR) & 0xF)
#define sp_getOperand(INSTR)      ((INSTR) >> 8)

#endif // sp_Class_h