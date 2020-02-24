#include "sp_Test.h"
#include "sp_Promise.h"
#include <stdio.h>

#ifdef sp_TESTS

static fnoreturn void runAllTests(sp_Action* a, sp_Promise* p){
    spTest_MemPool(a, p);
    spTest_SymPool(a, p);
    p->complete(p, NULL);
}

int main(void){
    sp_Action a = {.execute = runAllTests};
    void* r;
    if(sp_try(&a, &r)){
        printf("PASSED\n");
        return 0;
    }
    else{
        printf("FAILED at %s (%s)\n", ((sp_Error*)r)->src, ((sp_Error*)r)->msg);
        return 1;
    }
}

#endif // sp_TESTS