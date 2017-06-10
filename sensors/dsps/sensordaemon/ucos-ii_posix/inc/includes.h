#ifndef INCLUDES_H  /* skip the rest if we're including from the assembler file */
#define INCLUDES_H
/*============================================================================
  @file includes.h

  @brief
  This contains definitions used by uCoS and the SNS OSA.

  <br><br>

  DEPENDENCIES: None.

  Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================

  INCLUDE FILES

  ============================================================================*/
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>


/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

#define OS_TASK_CREATE_EN 1
#define OS_TASK_CREATE_EXT_EN 1
#define OS_TASK_DEL_EN 1
#define OS_FLAG_EN 1
#define OS_MAX_FLAGS 100
#define OS_FLAG_DEL_EN 1
#define OS_MUTEX_EN 1
#define OS_MUTEX_DEL_EN 1
#define OS_SEM_EN 1
#define OS_SEM_DEL_EN 1

/*
*********************************************************************************************************
*                                         EVENT FLAGS
*********************************************************************************************************
*/
#define  OS_FLAG_WAIT_CLR_ALL           0u  /* Wait for ALL    the bits specified to be CLR (i.e. 0)   */
#define  OS_FLAG_WAIT_CLR_AND           0u

#define  OS_FLAG_WAIT_CLR_ANY           1u  /* Wait for ANY of the bits specified to be CLR (i.e. 0)   */
#define  OS_FLAG_WAIT_CLR_OR            1u

#define  OS_FLAG_WAIT_SET_ALL           2u  /* Wait for ALL    the bits specified to be SET (i.e. 1)   */
#define  OS_FLAG_WAIT_SET_AND           2u

#define  OS_FLAG_WAIT_SET_ANY           3u  /* Wait for ANY of the bits specified to be SET (i.e. 1)   */
#define  OS_FLAG_WAIT_SET_OR            3u

#define  OS_FLAG_CONSUME             0x80u  /* Consume the flags if condition(s) satisfied             */


#define  OS_FLAG_CLR                    0u
#define  OS_FLAG_SET                    1u


/*
*********************************************************************************************************
*       Possible values for 'opt' argument of OSSemDel(), OSMboxDel(), OSQDel() and OSMutexDel()
*********************************************************************************************************
*/
#define  OS_DEL_NO_PEND                 0u
#define  OS_DEL_ALWAYS                  1u


/*
*********************************************************************************************************
*                                        OS???Pend() OPTIONS
*
* These #defines are used to establish the options for OS???PendAbort().
*********************************************************************************************************
*/
#define  OS_PEND_OPT_NONE               0u  /* NO option selected                                      */
#define  OS_PEND_OPT_BROADCAST          1u  /* Broadcast action to ALL tasks waiting                   */

/*
*********************************************************************************************************
*                                     OS???PostOpt() OPTIONS
*
* These #defines are used to establish the options for OSMboxPostOpt() and OSQPostOpt().
*********************************************************************************************************
*/
#define  OS_POST_OPT_NONE            0x00u  /* NO option selected                                      */
#define  OS_POST_OPT_BROADCAST       0x01u  /* Broadcast message to ALL tasks waiting                  */
#define  OS_POST_OPT_FRONT           0x02u  /* Post to highest priority task waiting                   */
#define  OS_POST_OPT_NO_SCHED        0x04u  /* Do not call the scheduler if this option is selected    */

/*
*********************************************************************************************************
*                                 TASK OPTIONS (see OSTaskCreateExt())
*********************************************************************************************************
*/
#define  OS_TASK_OPT_NONE          0x0000u  /* NO option selected                                      */
#define  OS_TASK_OPT_STK_CHK       0x0001u  /* Enable stack checking for the task                      */
#define  OS_TASK_OPT_STK_CLR       0x0002u  /* Clear the stack when the task is create                 */
#define  OS_TASK_OPT_SAVE_FP       0x0004u  /* Save the contents of any floating-point registers       */
#define  OS_TASK_OPT_ISLAND        0x0008u  /* Task expected to run in contrained resources            */

/*
*********************************************************************************************************
*                                             ERROR CODES
*********************************************************************************************************
*/
#define OS_ERR_NONE                     0u

#define OS_ERR_EVENT_TYPE               1u
#define OS_ERR_PEND_ISR                 2u
#define OS_ERR_POST_NULL_PTR            3u
#define OS_ERR_PEVENT_NULL              4u
#define OS_ERR_POST_ISR                 5u
#define OS_ERR_QUERY_ISR                6u
#define OS_ERR_INVALID_OPT              7u
#define OS_ERR_ID_INVALID               8u
#define OS_ERR_PDATA_NULL               9u

#define OS_ERR_TIMEOUT                 10u
#define OS_ERR_EVENT_NAME_TOO_LONG     11u
#define OS_ERR_PNAME_NULL              12u
#define OS_ERR_PEND_LOCKED             13u
#define OS_ERR_PEND_ABORT              14u
#define OS_ERR_DEL_ISR                 15u
#define OS_ERR_CREATE_ISR              16u
#define OS_ERR_NAME_GET_ISR            17u
#define OS_ERR_NAME_SET_ISR            18u
#define OS_ERR_ILLEGAL_CREATE_RUN_TIME 19u

#define OS_ERR_MBOX_FULL               20u

#define OS_ERR_Q_FULL                  30u
#define OS_ERR_Q_EMPTY                 31u

#define OS_ERR_PRIO_EXIST              40u
#define OS_ERR_PRIO                    41u
#define OS_ERR_PRIO_INVALID            42u

#define OS_ERR_SCHED_LOCKED            50u
#define OS_ERR_SEM_OVF                 51u

#define OS_ERR_TASK_CREATE_ISR         60u
#define OS_ERR_TASK_DEL                61u
#define OS_ERR_TASK_DEL_IDLE           62u
#define OS_ERR_TASK_DEL_REQ            63u
#define OS_ERR_TASK_DEL_ISR            64u
#define OS_ERR_TASK_NAME_TOO_LONG      65u
#define OS_ERR_TASK_NO_MORE_TCB        66u
#define OS_ERR_TASK_NOT_EXIST          67u
#define OS_ERR_TASK_NOT_SUSPENDED      68u
#define OS_ERR_TASK_OPT                69u
#define OS_ERR_TASK_RESUME_PRIO        70u
#define OS_ERR_TASK_SUSPEND_IDLE       71u
#define OS_ERR_TASK_SUSPEND_PRIO       72u
#define OS_ERR_TASK_WAITING            73u

#define OS_ERR_TIME_NOT_DLY            80u
#define OS_ERR_TIME_INVALID_MINUTES    81u
#define OS_ERR_TIME_INVALID_SECONDS    82u
#define OS_ERR_TIME_INVALID_MS         83u
#define OS_ERR_TIME_ZERO_DLY           84u
#define OS_ERR_TIME_DLY_ISR            85u

#define OS_ERR_MEM_INVALID_PART        90u
#define OS_ERR_MEM_INVALID_BLKS        91u
#define OS_ERR_MEM_INVALID_SIZE        92u
#define OS_ERR_MEM_NO_FREE_BLKS        93u
#define OS_ERR_MEM_FULL                94u
#define OS_ERR_MEM_INVALID_PBLK        95u
#define OS_ERR_MEM_INVALID_PMEM        96u
#define OS_ERR_MEM_INVALID_PDATA       97u
#define OS_ERR_MEM_INVALID_ADDR        98u
#define OS_ERR_MEM_NAME_TOO_LONG       99u

#define OS_ERR_NOT_MUTEX_OWNER        100u

#define OS_ERR_FLAG_INVALID_PGRP      110u
#define OS_ERR_FLAG_WAIT_TYPE         111u
#define OS_ERR_FLAG_NOT_RDY           112u
#define OS_ERR_FLAG_INVALID_OPT       113u
#define OS_ERR_FLAG_GRP_DEPLETED      114u
#define OS_ERR_FLAG_NAME_TOO_LONG     115u

#define OS_ERR_PIP_LOWER              120u

#define OS_ERR_TMR_INVALID_DLY        130u
#define OS_ERR_TMR_INVALID_PERIOD     131u
#define OS_ERR_TMR_INVALID_OPT        132u
#define OS_ERR_TMR_INVALID_NAME       133u
#define OS_ERR_TMR_NON_AVAIL          134u
#define OS_ERR_TMR_INACTIVE           135u
#define OS_ERR_TMR_INVALID_DEST       136u
#define OS_ERR_TMR_INVALID_TYPE       137u
#define OS_ERR_TMR_INVALID            138u
#define OS_ERR_TMR_ISR                139u
#define OS_ERR_TMR_NAME_TOO_LONG      140u
#define OS_ERR_TMR_INVALID_STATE      141u
#define OS_ERR_TMR_STOPPED            142u
#define OS_ERR_TMR_NO_CALLBACK        143u

#define OS_ENTER_CRITICAL() /* This has no effect on Linux */
#define OS_EXIT_CRITICAL()  /* This has no effect on Linux */


/*============================================================================
  Type Declarations
  ============================================================================*/
typedef unsigned char      BOOLEAN;
typedef unsigned char      INT8U;
typedef signed   char      INT8S;
typedef unsigned short int     INT16U;
typedef signed   short int     INT16S;
typedef unsigned int      INT32U;
typedef signed   int      INT32S;
typedef float              FP32;
typedef INT32U         OS_STK;
typedef sigset_t       OS_CPU_SR;
typedef unsigned int   OS_FLAGS;
typedef unsigned short OS_PRIO;

typedef struct os_flag_grp {
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  volatile OS_FLAGS flags;
} OS_FLAG_GRP;

typedef union {
  pthread_mutex_t mutex;
  sem_t sem;
} OS_EVENT;

#endif // INCLUDES_H
