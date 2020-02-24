#ifndef sp_MethodBuilder_h
#define sp_MethodBuilder_h
#include "sp_Common.h"
#include "sp_Promise.h"
#include "sp_Compiler.h"
#include "sp_Class.h"

typedef struct sp_MethodBuilder sp_MethodBuilder;

sp_MethodBuilder* sp_createMethodBuilder(sp_Compiler* com, sp_Sym name, sp_SrcLoc* loc, sp_Promise* p);
void              sp_destroyMethodBuilder(sp_MethodBuilder* bdr);
unsigned          sp_methodBuilderAddCallParam(sp_MethodBuilder* bdr, sp_Sym name, sp_Promise* p);
unsigned          sp_methodBuilderAddActivationParam(sp_MethodBuilder* bdr, sp_Sym name, sp_Promise* p);
unsigned          sp_methodBuilderAddInstr(sp_MethodBuilder* bdr, sp_Instr instr, sp_Promise* p);
sp_Method*        sp_methodBuilderProduce(sp_MethodBuilder* bdr, sp_Promise* p);

#endif // sp_MethodBuilder_h