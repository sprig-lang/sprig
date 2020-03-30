#include "sp_Scheduler.h"

typedef struct {
    sp_Future*   fArg;
    sp_Resource* rArg;
    sp_Task*     task;
} TaskQueueEntry;

typedef struct {
    unsigned first;
    unsigned last;
    unsigned size;
    TaskQueueEntry** ring;
} TaskQueue;

typedef struct {
    clock_t when;
    TaskQueueEntry* entry;
} TaskScheduleEntry;

typedef struct {
    unsigned last;
    unsigned size;
    TaskScheduleEntry* heap;
} TaskSchedule;

struct sp_Scheduler {
    unsigned   numWorkers;
    sp_Worker* workers;

};

sp_Scheduler* sp_createScheduler(sp_MemPool* mp, sp_ObjPool* op, unsigned numThreads, sp_Promise* p) {

}

void sp_destroyScheduler(sp_Scheduler* sch, sp_Promise* p) {

}

sp_Future*  sp_schedule(sp_Scheduler* sch, sp_Task* task, sp_TimeSpan delay, sp_Promise* p);
sp_Future*  sp_scheduleWithResourceArg(sp_Scheduler* sch, sp_Task* task, sp_Resource* arg, sp_TimeSpan delay, sp_Promise* p);
sp_Future*  sp_scheduleWithFutureArg(sp_Scheduler* sch, sp_Task* task, sp_Future* arg, sp_TimeSpan delay, sp_Promise* p);
sp_TimeStamp sp_now(sp_Scheduler* sch);
sp_TimeSpan  sp_secs(unsigned secs);
sp_TimeSpan  sp_msecs(unsigned msecs);
sp_TimeSpan  sp_nsecs(unsigned nsecs);


