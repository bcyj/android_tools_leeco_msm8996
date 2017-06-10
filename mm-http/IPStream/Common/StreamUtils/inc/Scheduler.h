/************************************************************************* */
/**
 * Scheduler.h
 * @brief Header file for Scheduler.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/Scheduler.h#10 $
$DateTime: 2013/05/20 11:35:02 $
$Change: 3790690 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#ifndef _PVSCHEDULER_H_
#define _PVSCHEDULER_H_

#include "MMSignal.h"
#include "MMCriticalSection.h"
#include "SourceMemDebug.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Forward declarations
** ----------------------------------------------------------------------- */
struct IEnv;

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

#if defined(_WIN32_WCE)
int const GREEDY_LEVEL = 2;     /* the larger, the greedier  */
#elif defined(WIN32)
int const GREEDY_LEVEL = 7;     /* the larger, the greedier  */
#else
int const GREEDY_LEVEL = 2;     /* the larger, the greedier  */
#endif

static const unsigned long SCHEDULER_SLEEP_TIME = 5;  // Milliseconds

int const MAX_SCHEDULER_TASKS  = 20; /* estimate of max concurrent tasks */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef struct TaskNode
{
    int nTaskId;                    /* task id starts at 1. */
    int (*Task)(void *pTaskParameter);
    void *pTaskParameter;
    bool bDeleted;
    bool bParamReleaseRequired;
    bool bStaticTask;
    struct TaskNode *pNextTask;
    bool bInUse;
} SchedulerTaskNode;

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */


/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/**
 * @ brief class SchedulerTaskParamBase
 */

class SchedulerTaskParamBase
{
public:
  SchedulerTaskParamBase();
  virtual ~SchedulerTaskParamBase() {}
  int ntaskID;

};

/**
 * @brief Class Scheduler
 */
class Scheduler
{
public:

    Scheduler(int &result);
    ~Scheduler();

    // task maintainance methods
    int AddTask(int (*task)(void *parameter), void *parameter,
                     bool paramReleaseRequired = true,
                     bool markAsStaticTask = false);
    bool DeleteTask(int taskId);
    bool Start();
    void Stop(bool force = true);
    void Sleep(int ms);

    SchedulerTaskNode *FindTailNode();

    void RunTasks();

private:

    // task allocation/deallocation methods
    SchedulerTaskNode *Allocate();
    void Deallocate(SchedulerTaskNode *node);

    SchedulerTaskNode m_schedulerTaskNodes[MAX_SCHEDULER_TASKS];
    SchedulerTaskNode *m_taskTable;
    int m_taskNumber; // Used for generating task Ids
    int m_numberOfTasks; // No of active+inactive tasks present in task table
    bool m_interruptSignal;
    bool m_bIsNewTaskAdded;

    MM_HANDLE m_SleepSignalQ;
    MM_HANDLE m_SleepSignal;
    MM_HANDLE m_taskTableLock;
};

#endif
