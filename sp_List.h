#ifndef sp_List_h
#define sp_List_h
#include "sp_Common.h"

#define sp_ListF(T, F) CCAT(sp_List__, CCAT(T, CCAT(__, F)))
#define sp_ListT(T) CCAT(sp_List__, CCAT(T, __Type))

#endif // sp_List_h