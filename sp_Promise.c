#include "sp_Promise.h"
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
    sp_Promise promiseHdr;

    jmp_buf*  jmp;
    sp_Result res;

    sp_Defer* onAbortDefers;
    sp_Defer* onCompleteDefers;
    sp_Defer* beforeExitDefers;
} TryPromise;

static fnoreturn void yieldRaw(sp_Promise* p, void* v) {
    TryPromise* tp = (TryPromise*)p;
    tp->res.type = sp_RESULT_RAW;
    tp->res.value.raw  = v;
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

static fnoreturn void yieldObj(sp_Promise* p, sp_Ptr v) {
    TryPromise* tp = (TryPromise*)p;
    tp->res.type = sp_RESULT_OBJ;
    tp->res.value.obj  = v;
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

static fnoreturn void pabort(sp_Promise* p, sp_Error* e){
    TryPromise* tp = (TryPromise*)p;
    tp->res.type = sp_RESULT_ERR;
    tp->res.value.err  = e;
    jmp_buf* jmp  = tp->jmp;

    sp_Defer* it = tp->onAbortDefers;
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

static void onAbort(sp_Promise* p, sp_Defer* d){
    TryPromise* tp = (TryPromise*)p;
    NL_LIST_LINK(d, &tp->onAbortDefers);
}

static void beforeExit(sp_Promise* p, sp_Defer* d){
    TryPromise* tp = (TryPromise*)p;
    NL_LIST_LINK(d, &tp->beforeExitDefers);
}

static void cancelDefer(sp_Promise* p, sp_Defer* d){
    NL_LIST_UNLINK(d);
}

bool sp_tryRaw(sp_Action* action, void** res, sp_Error** err) {
    jmp_buf jmp;
    TryPromise tp = {
        .jmp = &jmp,
        .promiseHdr = {
            .yieldRaw = yieldRaw,
            .abort = pabort,
            .onComplete = onComplete,
            .onAbort = onAbort,
            .beforeExit = beforeExit,
            .cancelDefer = cancelDefer
        }
    };
    if(setjmp(jmp)){
        if(tp.res.type == sp_RESULT_RAW){
            *res = tp.res.value.raw;
            return true;
        }
        else {
            *err = tp.res.value.err;
            return false;
        }
    }
    action->execute(action, (sp_Promise*)&tp);
    assert(false);
}

bool sp_tryObj(sp_Action* action, sp_Ptr* res, sp_Error** err) {
    jmp_buf jmp;
    TryPromise tp = {
        .jmp = &jmp,
        .promiseHdr = {
            .yieldObj = yieldObj,
            .abort = pabort,
            .onComplete = onComplete,
            .onAbort = onAbort,
            .beforeExit = beforeExit,
            .cancelDefer = cancelDefer
        }
    };
    if(setjmp(jmp)){
        if(tp.res.type == sp_RESULT_OBJ){
            *res = tp.res.value.obj;
            return true;
        }
        else{
            *err = tp.res.value.err;
            return false;
        }
    }
    action->execute(action, (sp_Promise*)&tp);
    assert(false);
}