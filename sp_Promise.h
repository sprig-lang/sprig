#ifndef sp_Promise_h
#define sp_Promise_h
#include "sp_Common.h"

typedef struct sp_Promise sp_Promise;
typedef struct sp_Error sp_Error;
typedef struct sp_Action sp_Action;
typedef struct sp_Defer sp_Defer;

struct sp_Error {
    char const* tag;
    char const* msg;
    char const* src;

    void (*destroy)(sp_Error* e);
};

struct sp_Action {
    pnoreturn void (*execute)(sp_Action*, sp_Promise*);
};

struct sp_Defer {
    sp_Defer** link;
    sp_Defer*  next;
    void (*execute)(sp_Defer*);
};

struct sp_Promise {
    pnoreturn void (*complete)(sp_Promise* p, void* v);
    pnoreturn void (*abort)(sp_Promise* p, sp_Error* e);

    void (*onComplete)(sp_Promise* p, sp_Defer* d);
    void (*onAbort)(sp_Promise* p, sp_Defer* d);
    void (*beforeExit)(sp_Promise* p, sp_Defer* d);
    void (*cancelDefer)(sp_Promise* p, sp_Defer* d);
};



bool sp_try(sp_Action* action, void** out);

#endif // sp_Promise_h