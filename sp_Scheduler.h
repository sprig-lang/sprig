#ifndef sp_Scheduler_h
#define sp_Scheduler_h
#include "sp_Promise.h"
#include "sp_Resource.h"
#include "sp_Engine.h"

typedef struct sp_Scheduler sp_Scheduler;
typedef struct sp_Task sp_Task;
typedef clock_t sp_TimeStamp;
typedef clock_t sp_TimeSpan;

sp_Scheduler* sp_createScheduler(sp_Engine* eng, sp_Promise* p);
void          sp_destroyScheduler(sp_Scheduler* sch, sp_Promise* p);

sp_Future*  sp_schedule(sp_Scheduler* sch, sp_Task* task, sp_TimeSpan delay, sp_Promise* p);
sp_Future*  sp_scheduleWithResourceArg(sp_Scheduler* sch, sp_Task* task, sp_Resource* arg, sp_TimeSpan delay, sp_Promise* p);
sp_Future*  sp_scheduleWithFutureArg(sp_Scheduler* sch, sp_Task* task, sp_Future* arg, sp_TimeSpan delay, sp_Promise* p);
sp_TimeStamp sp_now(sp_Scheduler* sch);
sp_TimeSpan  sp_secs(unsigned secs);
sp_TimeSpan  sp_msecs(unsigned msecs);
sp_TimeSpan  sp_nsecs(unsigned nsecs);
#endif