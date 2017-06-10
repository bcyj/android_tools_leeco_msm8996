/************************************************************************* */
/**
 * Scheduler.cpp
 * @brief implementation of Scheduler.
 *  Scheduler class manages the Stream Source task scheduling. It adds, removes and
 *  schedules the various Stream Source sub tasks. Internally it maintains a linked
 *  list of task function pointers and invokes the tasks in a round-robin
 *  fashion.
 *
 COPYRIGHT 2011-2013 QUALCOMM Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/Scheduler.cpp#17 $
$DateTime: 2013/05/20 11:35:02 $
$Change: 3790690 $

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "Scheduler.h"
#include "qtv_msg.h"
#include "string.h"

/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */
/**
 * constructor SchedulerTaskParamBase
 */
SchedulerTaskParamBase::SchedulerTaskParamBase(): ntaskID(0)
{
}

/**
 * @brief Scheduler::Scheduler
 * @param[in/out] result
 * @param piEnv
 *
 * The result will be 0 on success else failure
 */
Scheduler::Scheduler(int &result)
: m_taskTable(0),
  m_taskNumber(0),
  m_numberOfTasks(0),
  m_interruptSignal(false),
  m_bIsNewTaskAdded(false),
  m_SleepSignalQ(0),
  m_SleepSignal(0),
  m_taskTableLock(0)
{
  QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,"Scheduler Init");

  result = MM_CriticalSection_Create( &m_taskTableLock );

  if (0 == result)
  {
    result = MM_SignalQ_Create( &m_SleepSignalQ);
  }

  if (0 == result)
  {
    result = MM_Signal_Create( m_SleepSignalQ, 0, 0, &m_SleepSignal);
  }

  memset(m_schedulerTaskNodes, 0, sizeof(m_schedulerTaskNodes));

}

Scheduler::~Scheduler()
{
  m_taskTable = 0;
  if (m_taskTableLock)
  {
    MM_CriticalSection_Release( m_taskTableLock );
    m_taskTableLock = 0;
  }

  if ( m_SleepSignal )
  {
    (void)MM_Signal_Release( m_SleepSignal);
    m_SleepSignal = 0;
  }

  if ( m_SleepSignalQ )
  {
    (void)MM_SignalQ_Release( m_SleepSignalQ);
    m_SleepSignalQ = 0;
  }
}

/**
 * @brief Scheduler::addTask
 * @param task
 * @param parameter
 * @param param_release_required whether scheduler needs to release task's
 *                         parameter resource
 *
 * @return int : the task id, or 0 on failure
 */
int
Scheduler::AddTask
(
  int (*task)(void *parameter),
  void *parameter,
  bool paramReleaseRequired,
  bool markAsStaticTask
)
{
  QTV_NULL_PTR_CHECK(m_taskTableLock, 0);

  SchedulerTaskNode *t, *t_prev;

  MM_CriticalSection_Enter(m_taskTableLock);

  t_prev = m_taskTable;

  // search for the end of the task list
  while (t_prev != 0 && t_prev->pNextTask != 0)
  {
    t_prev = t_prev->pNextTask;
  }

  // add a new task node
  t = Allocate();
  if (t != 0)
  {
    m_bIsNewTaskAdded = true;

    t->nTaskId = ++m_taskNumber;   // Generate task id
    t->Task = task;
    t->bDeleted = false;
    t->pTaskParameter = parameter;
    t->bParamReleaseRequired = paramReleaseRequired;
    t->bStaticTask = markAsStaticTask;
    t->pNextTask = 0;

    if (t_prev == 0)
    {
      QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
                    "Added first entry in task table, sch 0x%p",(void *) this);
      m_taskTable = t;
    }
    else
    {
      t_prev->pNextTask = t;
    }
    ++m_numberOfTasks; // Increase the task count
  }

  MM_CriticalSection_Leave(m_taskTableLock);

  if(t != 0)
  {
     QTV_MSG_PRIO4(QTVDIAG_GENERAL, QTVDIAG_PRIO_DEBUG,
              "Added task with id: %d, param %p Total No of tasks: %d, sch %p",
              t->nTaskId, (void *)t->pTaskParameter, m_numberOfTasks, (void *)this);
     return t->nTaskId;
  }
  else
  {
     QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
             "Failed to add a new task. Total No of tasks: %d, sch %p",
             m_numberOfTasks, (void *)this);
     return 0;
  }
}

/**
 * @brief  Scheduler::deleteTask
 * @param task_id
 *
 * @return bool
 */
bool
Scheduler::DeleteTask(int taskId)
{
  SchedulerTaskNode *t;
  bool found_it = false;
  bool deleted = false;

  t = m_taskTable;
  while (t != 0)
  {
    if (t->nTaskId == taskId)
    {
      if (t->bDeleted)
      {
        deleted = true;
      }
      found_it = true;
      t->bDeleted = true;
      break;
    }
    t = t->pNextTask;
  }

  if (deleted)
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
           "Already marked for deletion, task id %d sch %p",
           t->nTaskId,(void *)this);
  }
  else if(found_it)
  {
    --m_numberOfTasks;
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_DEBUG,
           "Marked task with id %d for deletion sch %p",
           t->nTaskId, (void *)this);
  }
  else
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
             "Find failed, when attempted to mark task id %d for deletion on sch %p",
             taskId, (void *)this);

  }

  QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_DEBUG,
           "Remaining No of tasks: %d",
           m_numberOfTasks);

  return found_it;
}

/**
 * @brief Scheduler::start
 * This blocking call executes all the tasks Duration is the
 * total run time (in msec) of the scheduler
 */
bool
Scheduler::Start()
{
  SchedulerTaskNode *t;
  int count = 0;
  bool bStop = m_interruptSignal;

  QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
               "Scheduler %p Start", (void *)this);

  while (!bStop)
  {
    RunTasks();

    MM_CriticalSection_Enter(m_taskTableLock);
    bStop = m_interruptSignal;

    MM_CriticalSection_Leave(m_taskTableLock);

    // give up CPU to other threads
    if (++count > GREEDY_LEVEL)
    {
      count = 0;
      Sleep(SCHEDULER_SLEEP_TIME);
    }
  }

  QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
               "Scheduler %p END !!!", (void *)this);

  MM_CriticalSection_Enter(m_taskTableLock);

  SchedulerTaskNode *t_tmp;
  t = m_taskTable;
  while (t != 0)
  {
    // Delete the task from the scheduler
    t_tmp = t->pNextTask;
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_DEBUG,
             "Delete task with id %d from scheduler %p",
             t->nTaskId, (void *)this);
    t->bDeleted = true;
    Deallocate(t);
    t = t_tmp;
  }
  m_taskTable = 0;

  m_interruptSignal = false;

  MM_CriticalSection_Leave(m_taskTableLock);

  return true;
}

/**
 * Get the pointer to the tail node.
 */
SchedulerTaskNode *Scheduler::FindTailNode()
{
  SchedulerTaskNode *pTailNode = NULL;

  // find the tail node
  SchedulerTaskNode *t = m_taskTable;

  while (t != 0)
  {
    pTailNode = t;
    t = t->pNextTask;
  }

  return pTailNode;
}

/**
 * @brief
 *  Execute all tasks in this this scheduler. Any task that has
 *  been marked deleted will be purged as well.
 */
void Scheduler::RunTasks()
{
  MM_CriticalSection_Enter(m_taskTableLock);
  m_bIsNewTaskAdded = false; // will run through all tasks anyways.
  SchedulerTaskNode *pCurTask = m_taskTable;
  SchedulerTaskNode *pTailNode = FindTailNode();
  MM_CriticalSection_Leave(m_taskTableLock);

  bool bIsATaskDeleted = false;
  bool bLoop = true;

  while (bLoop)
  {
    bLoop = false;

    if (pTailNode)
    {
      // run tasks
      while (pCurTask != 0)
      {
        if (!pCurTask->bDeleted)
        {
          (void)pCurTask->Task(pCurTask->pTaskParameter);
        }
        else
        {
          bIsATaskDeleted = true;
        }

        if (pTailNode == pCurTask)
        {
          break;
        }

        pCurTask = pCurTask->pNextTask;
      }
    }

    MM_CriticalSection_Enter(m_taskTableLock);
    if (m_bIsNewTaskAdded)
    {
      m_bIsNewTaskAdded = false;

      if (pCurTask)
      {
        pCurTask = pCurTask->pNextTask;

        if (pCurTask)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Execute newly added taskID %u", pCurTask->nTaskId);
          bLoop = true;

          // get the pointer to the updated tail node.
          pTailNode = FindTailNode();
        }
      }
      else
      {
        // there were no tasks that were run.
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_DEBUG,
                     "Scheduler task - ok to run in next iteration");
      }
    }
    MM_CriticalSection_Leave(m_taskTableLock);
  }

  // delete all tasks that are marked deleted
  if (bIsATaskDeleted)
  {
    MM_CriticalSection_Enter(m_taskTableLock);

    SchedulerTaskNode *t = m_taskTable;
    SchedulerTaskNode *t_prev = NULL;

    while (t != 0)
    {
      if (t->bDeleted)
      {
        // remove the task from the schduler
        if (t_prev == 0)
        {
          m_taskTable = t->pNextTask;

          QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
                        "RunTasks Deleting task with id %d, sch %p",
                          t->nTaskId, (void *)this);

          Deallocate(t);

          t = m_taskTable;
        }
        else
        {
          t_prev->pNextTask = t->pNextTask;

          QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_DEBUG,
                        "Deleting task with id %d from scheduler %p",
                        t->nTaskId, (void *)this);

          Deallocate(t);
          t = t_prev->pNextTask;
        }
      }
      else
      {
        t_prev = t;
        t = t->pNextTask;
      }
    }

    MM_CriticalSection_Leave(m_taskTableLock);
  }
}

/**
 * @bruef Scheduler::stop
 * Stops the task scheduler
 */
void
Scheduler::Stop(bool force)
{
  SchedulerTaskNode *t;
  uint32 numStaticTasks = 0;

  QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,"Scheduler %p stop", (void *)this);

  MM_CriticalSection_Enter(m_taskTableLock);

  // remove all left over tasks
  t = m_taskTable;
  while (t != 0)
  {
    if (!force && t->bStaticTask)
    {
      //marked as static task, so do not delete
      t = t->pNextTask;
      numStaticTasks++;
      continue;
    }

    // set the task for delete from the scheduler
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_DEBUG,
             "Set the task with id %d for delete, sch %p",
             t->nTaskId, (void *)this);
    if (!t->bDeleted)
    {
      t->bDeleted = true;
      --m_numberOfTasks;
    }
    t = t->pNextTask;
  }

  if (!numStaticTasks)
  {
    QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Scheduler %p interrupt flag set", (void *)this);
    m_interruptSignal = true;
  }

  MM_CriticalSection_Leave(m_taskTableLock);
}

/**
 * @brief Scheduler::Sleep
 * Blocks the current thread for a specified time. The thread becomes
 * schedulable after the specified time has elapsed.
 * Time to sleep in milliSecs
 * Note: Delay must be less then 0x80000000 ,other wise delay is treated
 *       as zero by ISysTimer_SetDelay
 * @param ms
 */
void
Scheduler::Sleep(int ms)
{
  if ( m_SleepSignal && m_SleepSignalQ )
  {
    int nTimedOut;
    void *pUserArg =  NULL;
    (void)MM_SignalQ_TimedWait( m_SleepSignalQ, ms, &pUserArg, &nTimedOut );
  }
  return;
}

/**
 * @brief This method allocates the task node memory
 *
 * @param node
 */
SchedulerTaskNode *Scheduler::Allocate()
{
  SchedulerTaskNode* t = NULL;
  // look for any nodes avaliable in pre-allocated memory
  for (int i = 0; i < MAX_SCHEDULER_TASKS ; i++)
  {
    if (m_schedulerTaskNodes[i].bInUse == false)
    {
      m_schedulerTaskNodes[i].bInUse = true;
      t = &m_schedulerTaskNodes[i];
      break;
    }
  }
  // No space in pre-allocated memory call malloc
  if ( t == NULL )
  {
    t = (SchedulerTaskNode*)QTV_Malloc(sizeof(SchedulerTaskNode));
  }
  return t;
}
/**
 * @brief This method deallocates the task node memory
 *
 * @param node
 */
void Scheduler::Deallocate(SchedulerTaskNode *node)
{
  if (node)
  {
    if (node->pTaskParameter != 0
        && node->bParamReleaseRequired)
    {
      QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_DEBUG,
                    "Deallocate sch task param %p from sch %p",
                    (void *)node->pTaskParameter, (void *)this);
      QTV_Delete( (SchedulerTaskParamBase*)node->pTaskParameter );
      node->pTaskParameter = 0;
    }
    // is the node from pre-allocated memory
    for ( int i = 0; i < MAX_SCHEDULER_TASKS ; i++ )
    {
      if ( node == &m_schedulerTaskNodes[i] )
      {
        m_schedulerTaskNodes[i].bInUse = false;
        node = NULL;
        break;
      }
    }
    // did not find the node is pre-allocated memory call free
    if ( node != NULL )
    {
      QTV_Free(node);
      node = NULL;
    }
  }
}
