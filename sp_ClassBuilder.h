#ifndef sp_ClassBuilder_h
#define sp_ClassBuilder_h

typedef struct sp_ClassBuilder sp_ClassBuilder;

sp_ClassBuilder* sp_createClassBuilder(sp_Compiler* com, sp_Method* init, sp_SrcLoc* loc, sp_Promise* p);
void             sp_destroyClassBuilder(sp_ClassBuilder* bdr);
void             sp_classBuilderAddMethod(sp_ClassBuilder* bdr, unsigned slot, sp_Method* mth, sp_Promise* p);
void             sp_classBuilderAddConst(sp_ClassBuilder* bdr, unsigned slot, sp_Ref val, sp_Promise* p);
sp_Class*        sp_classBuilderProduce(sp_ClassBuilder* bdr);


#endif // sp_ClassBuilder_h