#include "sp_List.h"
#include "sp_MemPool.h"
#include "sp_Promise.h"
#include "sp_Visitor.h"

typedef struct sp_ListT(T) sp_ListT(T);

static inline sp_ListT(T)* sp_ListF(T, create)(sp_MemPool* mp, sp_Promise* p);
static inline void sp_ListF(T, destroy)(sp_ListT(T)* ls);
static inline void sp_ListF(T, append)(sp_ListT(T)* ls, T val, sp_Promise* p);
static inline T sp_ListF(T, get)(sp_ListT(T)* ls, unsigned idx, sp_Promise* p);
static inline void sp_ListF(T, set)(sp_ListT(T)* ls, unsigned idx, T val, sp_Promise* p);
static inline void sp_ListF(T, reserve)(sp_ListT(T)* ls, unsigned num, sp_Promise* p);
static inline unsigned sp_ListF(T, count)(sp_ListT(T)* ls);
static inline void sp_ListF(T, accept)(sp_ListT(T)* ls, sp_Visitor* vis);

