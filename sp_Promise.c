#include "sp_Promise.h"
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
    sp_Promise promiseHdr;

    jmp_buf* jmp;
    void* result;
    bool  cancelled;

    sp_Defer* onCancelDefers;
    sp_Defer* onCompleteDefers;
    sp_Defer* beforeExitDefers;
} TryPromise;

static fnoreturn void complete(sp_Promise* p, void* v) {
    TryPromise* tp = (TryPromise*)p;
    tp->cancelled = false;
    tp->result    = v;
    jmp_buf* jmp  = tp->jmp;

    sp_Defer* it = tp->onCompleteDefers;
    while(it){
        sp_Defer* d = it;
        it = it->next;
        d->execute(d);
    }
    it = tp->beforeExitDefers;
    while(it){
        sp_Defer* d = it;
        it = it->next;
        d->execute(d);
    }

    longjmp(*jmp, 1);
}

static fnoreturn void cancel(sp_Promise* p, sp_Error* e){
    TryPromise* tp = (TryPromise*)p;
    tp->cancelled = true;
    tp->result    = e;
    jmp_buf* jmp  = tp->jmp;

    sp_Defer* it = tp->onCancelDefers;
    while(it){
        sp_Defer* d = it;
        it = it->next;
        d->execute(d);
    }
    it = tp->beforeExitDefers;
    while(it){
        sp_Defer* d = it;
        it = it->next;
        d->execute(d);
    }
    
    longjmp(*jmp, 1);
}

static void onComplete(sp_Promise* p, sp_Defer* d){
    TryPromise* tp = (TryPromise*)p;
    NL_LIST_LINK(d, &tp->onCompleteDefers);
}

static void onCancel(sp_Promise* p, sp_Defer* d){
    TryPromise* tp = (TryPromise*)p;
    NL_LIST_LINK(d, &tp->onCancelDefers);
}

static void beforeExit(sp_Promise* p, sp_Defer* d){
    TryPromise* tp = (TryPromise*)p;
    NL_LIST_LINK(d, &tp->beforeExitDefers);
}

static void cancelDefer(sp_Promise* p, sp_Defer* d){
    NL_LIST_UNLINK(d);
}

bool sp_try(sp_Action* action, void** out) {
    jmp_buf jmp;
    TryPromise tp = {
        .jmp = &jmp,
        .promiseHdr = {
            .complete = complete,
            .cancel = cancel,
            .onComplete = onComplete,
            .onCancel = onCancel,
            .beforeExit = beforeExit,
            .cancelDefer = cancelDefer
        }
    };
    if(setjmp(jmp)){
        *out = tp.result;
        return !tp.cancelled;
    }
    action->execute(action, (sp_Promise*)&tp);
    assert(false);
}