#ifndef sp_Promise_h
#define sp_Promise_h
#include "sp_Common.h"
#include "sp_Object.h"

typedef struct sp_Promise sp_Promise;
typedef struct sp_Future sp_Future;
typedef struct sp_Result sp_Result;
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

struct sp_Result {
    enum {
        sp_RESULT_VAL,
        sp_RESULT_ERR
    } type;
    union {
        void*     raw;
        sp_Ptr    obj;
        sp_Error* err;
    } value;
};
#define sp_result_raw(PTR) (sp_Result)

struct sp_Future {
    enum {
        sp_FUTURE_PENDING,
        sp_FUTURE_FINISHED
    } volatile status;

    sp_Result* (*getResult)(sp_Future*);
    void (*finl)(sp_Future*);
};

struct sp_Defer {
    sp_Defer** link;
    sp_Defer*  next;
    void (*execute)(sp_Defer*);
};

struct sp_Promise {
    pnoreturn void (*yield)(sp_Promise* p,  void* v);
    pnoreturn void (*abort)(sp_Promise* p, sp_Error* e);

    sp_Future* (*getFuture)(sp_Promise* p);

    void (*onComplete)(sp_Promise* p, sp_Defer* d);
    void (*onAbort)(sp_Promise* p, sp_Defer* d);
    void (*beforeExit)(sp_Promise* p, sp_Defer* d);
    void (*cancelDefer)(sp_Promise* p, sp_Defer* d);

    void (*finl)(sp_Promise* p);
};



bool sp_try(sp_Action* action, void** res, sp_Error** err);

#endif // sp_Promise_h