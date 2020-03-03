#ifndef sp_Test_h
#define sp_Test_h
#ifdef sp_TESTS
#include "sp_Promise.h"
#include "sp_Common.h"

#define sp_assert(COND, P) do {                     \
    if(!(COND))                                     \
        (P)->cancel((P), &(sp_Error){               \
            .tag = "ASSERT_FAILURE",                \
            .msg = "Assertion (" #COND ") failed",  \
            .src = SRC_LOCATION                     \
        });                                         \
} while(0)

void spTest_MemPool(sp_Action* a, sp_Promise* p);
void spTest_SymPool(sp_Action* a, sp_Promise* p);
void spTest_MethodBuilder(sp_Action* a, sp_Promise* p);

#endif // sp_TESTS
#endif // sp_Test_h