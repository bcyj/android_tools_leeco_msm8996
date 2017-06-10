/*============================================================================
  @file sns_sam_pm.c

  @brief
  Functions that will be used by the SAM Framework related to the power management.

  @note
  File line length should generally be limited to <= 80 columns.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

#include "sns_sam_pm.h"
#include "sns_sam.h"
#include "sns_common.h"
#include "sns_pm.h"
#include "sns_debug_str.h"
#include "sns_sam_memmgr.h"
#include "sns_profiling.h"

#define SNS_SAM_UIMAGE_WAIT_TIME_US 5000   // 500ms of wait time before entering uImage

/* Macros from sns_pm.h*/
#ifdef SNS_USES_ISLAND
#define SNS_SAM_PM_VOTE_MODE(handle, mode)    sns_pm_vote_img_mode(handle, mode)
#define SNS_SAM_PM_VOTE_MIPS(handle, mips)    sns_pm_vote_mips(handle, mips)
#define SNS_SAM_PM_VOTE_BW(handle, bw, core)  sns_pm_vote_bw(handle, bw, core)

#define SNS_SAM_PM_INIT(handle, suspendCb, name)    sns_pm_client_init(handle, suspendCb, name)
#else
inline sns_pm_err_code_e SNS_SAM_PM_VOTE_MODE(sns_pm_handle_t client_handle, sns_pm_img_mode_e mode)
{
  UNREFERENCED_PARAMETER(client_handle);
  UNREFERENCED_PARAMETER(mode);
  return SNS_PM_SUCCESS;
}
inline sns_pm_err_code_e SNS_SAM_PM_VOTE_MIPS( sns_pm_handle_t client_handle, uint32 mips )
{
  UNREFERENCED_PARAMETER(client_handle);
  UNREFERENCED_PARAMETER(mips);
  return SNS_PM_SUCCESS;
}
inline sns_pm_err_code_e SNS_SAM_PM_VOTE_BW( sns_pm_handle_t client_handle, uint32 bw,
  sns_pm_core_e core )
{
  UNREFERENCED_PARAMETER(client_handle);
  UNREFERENCED_PARAMETER(bw);
  UNREFERENCED_PARAMETER(core);
  return SNS_PM_SUCCESS;
}
inline sns_pm_err_code_e SNS_SAM_PM_INIT( sns_pm_handle_t *client_handle, suspend_cb_t callback, char *name)
{
  UNREFERENCED_PARAMETER(client_handle);
  UNREFERENCED_PARAMETER(callback);
  UNREFERENCED_PARAMETER(name);
  return SNS_PM_SUCCESS;
}
#endif /*USES_ISLAND*/

/**
*  SAM's power management information
*/
typedef struct
{
  /** Set to true when SAM wants to vote to change the image mode*/
  bool                       updateMode;
  /** The image mode SAM should vote for*/
  sns_sam_uimage_mode_type   modeVote;

  /** Mutex to protect samImageMode*/
  OS_EVENT                  *samStatusMutexPtr;
  /** The uImage state of SAM*/
  sns_sam_uimage_status      samImageMode;

  /** The current MIPS*/
  sns_sam_internal_mips_type current_mips;
  /** Handle required by PM*/
  sns_pm_handle_t            pmHandle;
  /** uImage mode is allowed; true = enabled*/
  bool uimage_enabled;
} sns_sam_pm_state;

static sns_sam_pm_state sns_sam_pm SNS_SAM_UIMAGE_DATA; // The internal state maintained by SAM
                                                 // related to power management

/*============================================================================
  External Objects
  ===========================================================================*/
/* Contains all unprocessed incoming indication messages */
extern sns_q_s samIndMsgQ;
extern OS_EVENT *samIndMsgQMutex;

/* Client Requests, whose Report Timers have fired (and need to be handled) */
extern sns_q_s samReportQ;
extern OS_EVENT *samReportQMutex;

/* Client Requests, whose Batch Timers have fired (and need to be handled) */
extern sns_q_s samBatchQ;
extern OS_EVENT *samBatchQMutex;

/* Contains all unprocessed incoming request messages */
extern sns_q_s samReqMsgQ;
extern OS_EVENT *samReqMsgQMutex;




/* Whether the AP is currently in suspend */
static bool apInSuspend SNS_SAM_UIMAGE_DATA = false;
static OS_EVENT *apInSuspendMutex SNS_SAM_UIMAGE_DATA;

/*==============================================================================
  FUNCTION:  sns_sam_pm_ap_suspend
==============================================================================*/
SNS_SAM_UIMAGE_CODE bool
sns_sam_pm_ap_suspend()
{
  bool retVal;
  uint8_t os_err;

  sns_os_mutex_pend(apInSuspendMutex, 0, &os_err);
  SNS_ASSERT(OS_ERR_NONE == os_err);
  retVal = apInSuspend;
  os_err = sns_os_mutex_post(apInSuspendMutex);
  SNS_ASSERT(OS_ERR_NONE == os_err);
  return retVal;
}

/*==============================================================================
  FUNCTION:  sns_sam_pm_mode_update
==============================================================================*/
/**
* Moves SAM to the specified image mode type. This function only updates SAM's state,
* a seperate call to sns_pm_vote_img_mode must be used to inform PM about the
* image mode required.
*
* @param[i] samState pointer to SAM's PM state structure
* @param[i] newMode  The image mode that SAM should change to
*
* @return none
*/
SNS_SAM_UIMAGE_CODE static
void sns_sam_pm_mode_update(sns_sam_pm_state *samState, sns_sam_uimage_mode_type newMode)
{
  sns_sam_timestamp current_timestamp;
  sns_sam_timestamp runlength;

  if(samState->samImageMode.sam_status == newMode)
  {
    return;
  }
  samState->samImageMode.sam_status = newMode;
  current_timestamp = sns_em_get_timestamp();

  if(samState->samImageMode.timestamp < current_timestamp)
  {
    runlength = current_timestamp - samState->samImageMode.timestamp;
  }
  else if(samState->samImageMode.timestamp != 0)
  {
    runlength = (UINT32_MAX - samState->samImageMode.timestamp)
      + current_timestamp;
  }
  else
  {
    runlength = current_timestamp;
  }
  //SNS_PRINTF_STRING_ERROR_3(SNS_SAM_DBG_MOD, "SAM: runlength: %d, now: %d, last: %d",
  //   runlength, current_timestamp, samState->samImageMode.timestamp);

  if(newMode == SNS_SAM_MODE_BIGIMAGE)
  {
    samState->samImageMode.uimage_avg_run_length = (samState->samImageMode.uimage_avg_run_length +
      runlength)/2;
    //SNS_PRINTF_STRING_ERROR_2(SNS_SAM_DBG_MOD, "SAM: avg_runlength: %d =  now %d",
    //   samState->samImageMode.uimage_avg_run_length, runlength);
  }
  else
  {
    samState->samImageMode.bigimage_avg_run_length = (samState->samImageMode.uimage_avg_run_length +
      runlength)/2;
    //SNS_PRINTF_STRING_ERROR_2(SNS_SAM_DBG_MOD, "SAM: avg_runlength: %d =  now %d",
    //  samState->samImageMode.uimage_avg_run_length, runlength);
  }
  samState->samImageMode.timestamp = current_timestamp;
}

/*==============================================================================
  FUNCTION:  sns_sam_uimage_enter
==============================================================================*/
static sns_sam_err sns_sam_uimage_enter(void)
{
  uint8_t os_err;
  sns_pm_err_code_e pm_err;
  sns_sam_err sam_err = SAM_ENONE;


  sns_os_mutex_pend(sns_sam_pm.samStatusMutexPtr, 0, &os_err);
  SNS_ASSERT(OS_ERR_NONE == os_err);
  SNS_PRINTF_STRING_LOW_0(SNS_SAM_DBG_MOD, "SAM: ENTERING uIMAGE");
  sns_profiling_log_qdss( SNS_SAM_UIMAGE_ENTER, 0 );
  sns_sam_pm_mode_update(&sns_sam_pm, SNS_SAM_MODE_UIMAGE);

  pm_err = SNS_SAM_PM_VOTE_MODE( sns_sam_pm.pmHandle,
    SNS_IMG_MODE_MICRO);

  if(pm_err != SNS_PM_SUCCESS)
  {
    sam_err = SAM_EFAILED;
  }


  os_err = sns_os_mutex_post(sns_sam_pm.samStatusMutexPtr);
  SNS_ASSERT(OS_ERR_NONE == os_err);

  return sam_err;
}

/*==============================================================================
  FUNCTION:  sns_sam_pm_eval_state
==============================================================================*/
/**
*  Evaluates the votes to enter uImage and determines if we can enter uImage.
*
*  @return SAM_ENONE
*/

SNS_SAM_UIMAGE_CODE static
sns_sam_err sns_sam_pm_eval_state(void)
{
  uint8_t i;
  bool  flags_clear = true;

  for(i = 0; i < SNS_SAM_UIMAGE_BLOCK_COUNT; i++)
  {
    //SNS_PRINTF_STRING_FATAL_2(SNS_SAM_DBG_MOD, "SAM: uImage blocks:: code: %d, count: %d",
    //i, sns_sam_pm.samImageMode.sns_sam_uimage_block[i]);
    if(sns_sam_pm.samImageMode.sns_sam_uimage_block[i] > 0)
    {
      flags_clear = false;
      //break;
    }
  }

  if(true == flags_clear)
  {
    //SNS_PRINTF_STRING_FATAL_0(SNS_SAM_DBG_MOD, "SAM: Vote is - uImage");
    sns_sam_pm.modeVote = SNS_SAM_MODE_UIMAGE;
  }
  else
  {
    //SNS_PRINTF_STRING_FATAL_0(SNS_SAM_DBG_MOD, "SAM: Vote is - BigImage");
    sns_sam_pm.modeVote = SNS_SAM_MODE_BIGIMAGE;
  }

  sns_sam_pm.updateMode = (sns_sam_pm.modeVote == sns_sam_pm.samImageMode.sam_status) ?
    false:true;

  return SAM_ENONE;
}


/*==============================================================================
  FUNCTION:  sns_sam_pm_process_state
==============================================================================*/
/**
* Checks if we can enter uImage mode.
*
* @return SAM_ENONE
*/
static sns_sam_err sns_sam_pm_process_state(void)
{
  uint8_t os_err;
  if(true == sns_sam_pm.uimage_enabled)
  {
    sns_os_mutex_pend(sns_sam_pm.samStatusMutexPtr, 0, &os_err);
    SNS_ASSERT(os_err == OS_ERR_NONE);

    sns_sam_pm_eval_state();

    os_err = sns_os_mutex_post(sns_sam_pm.samStatusMutexPtr);
    SNS_ASSERT(os_err == OS_ERR_NONE);

    if(true == sns_sam_pm.updateMode &&
      sns_sam_pm.modeVote == SNS_SAM_MODE_UIMAGE)
    {
         sns_sam_uimage_enter();
    }

  }

  return SAM_ENONE;
}

/*==============================================================================
  FUNCTION:  sns_sam_pm_suspend_cb
==============================================================================*/
/**
* Callback function for Apps suspend/wakeup
*/
SNS_SAM_UIMAGE_CODE static void sns_sam_pm_suspend_cb( bool InSuspend)
{
  uint8_t os_err;
  sns_os_mutex_pend(apInSuspendMutex, 0, &os_err);
  SNS_ASSERT(OS_ERR_NONE == os_err);
  apInSuspend = InSuspend;
  os_err = sns_os_mutex_post(apInSuspendMutex);
  SNS_ASSERT(OS_ERR_NONE == os_err);

  sns_os_sigs_post( sns_sam_sig_event, SNS_SAM_PM_SIG,
      OS_FLAG_SET, &os_err );
  SNS_ASSERT( OS_ERR_NONE == os_err );
}

/*==============================================================================
  FUNCTION:  sns_sam_uimage_vote_enter
==============================================================================*/
SNS_SAM_UIMAGE_CODE void
sns_sam_uimage_vote_enter( sns_sam_uimage_blocking_type blockType)
{
  uint8_t os_err;
  sns_os_mutex_pend(sns_sam_pm.samStatusMutexPtr, 0, &os_err);
  SNS_ASSERT(OS_ERR_NONE == os_err);
  sns_sam_pm.samImageMode.sns_sam_uimage_block[blockType] = (sns_sam_pm.samImageMode.sns_sam_uimage_block[blockType] == 0)?
    0 : sns_sam_pm.samImageMode.sns_sam_uimage_block[blockType] - 1;


  //SNS_PRINTF_STRING_LOW_2(SNS_SAM_DBG_MOD, "SAM: Unblocking for uImage:: code: %d, count: %d",
  //  (uint8_t)blockType, sns_sam_pm.samImageMode.sns_sam_uimage_block[blockType]);

  os_err = sns_os_mutex_post(sns_sam_pm.samStatusMutexPtr);
  SNS_ASSERT(OS_ERR_NONE == os_err);
  sns_sam_pm_process_state();
}

/*==============================================================================
  FUNCTION:  sns_sam_uimage_exit
==============================================================================*/
sns_sam_err sns_sam_uimage_exit( sns_sam_uimage_blocking_type blockType)
{
  uint8_t os_err;
  sns_pm_err_code_e pm_err;
  sns_sam_err sam_err = SAM_ENONE;

  sns_os_mutex_pend(sns_sam_pm.samStatusMutexPtr, 0, &os_err);
  SNS_ASSERT(OS_ERR_NONE == os_err);

  sns_sam_pm.samImageMode.sns_sam_uimage_block[blockType] = (sns_sam_pm.samImageMode.sns_sam_uimage_block[blockType] < 255)?
    (sns_sam_pm.samImageMode.sns_sam_uimage_block[blockType] + 1) : 255;
  if(sns_sam_pm.samImageMode.sns_sam_uimage_block[blockType] == 255)
  {
      SNS_PRINTF_STRING_LOW_1(SNS_SAM_DBG_MOD, "SAM: Blocktype %d count 255!", blockType);
  }
  sns_sam_pm_mode_update(&sns_sam_pm, SNS_SAM_MODE_BIGIMAGE);
  pm_err = SNS_SAM_PM_VOTE_MODE( sns_sam_pm.pmHandle, SNS_IMG_MODE_BIG);
  SNS_PRINTF_STRING_LOW_0(SNS_SAM_DBG_MOD, "SAM: Exited uImage");
  if(pm_err != SNS_PM_SUCCESS)
  {
    sam_err = SAM_EFAILED;
  }

  sns_profiling_log_qdss( SNS_SAM_UIMAGE_EXIT, 2, blockType, sns_sam_pm.samImageMode.sns_sam_uimage_block[blockType] );
  //SNS_PRINTF_STRING_LOW_2(SNS_SAM_DBG_MOD, "SAM: UImage exit:: code: %d, count: %d",
  //  blockType, sns_sam_pm.samImageMode.sns_sam_uimage_block[blockType]);

  os_err = sns_os_mutex_post(sns_sam_pm.samStatusMutexPtr);
  SNS_ASSERT(OS_ERR_NONE == os_err);
  return sam_err;
}

/*==============================================================================
  FUNCTION:  sns_sam_pm_enter_sleep
==============================================================================*/
SNS_SAM_UIMAGE_CODE void
sns_sam_pm_enter_sleep()
{
  SNS_SAM_PM_VOTE_MODE(sns_sam_pm.pmHandle, SNS_IMG_MODE_NOCLIENT);
}

/*==============================================================================
  FUNCTION:  sns_sam_pm_exit_sleep
==============================================================================*/
SNS_SAM_UIMAGE_CODE void
sns_sam_pm_exit_sleep()
{
   if(sns_sam_pm.samImageMode.sam_status  == SNS_SAM_MODE_UIMAGE)
    {
      SNS_SAM_PM_VOTE_MODE(sns_sam_pm.pmHandle, SNS_IMG_MODE_MICRO);
    }
   else
   {
      SNS_SAM_PM_VOTE_MODE(sns_sam_pm.pmHandle, SNS_IMG_MODE_BIG);
   }
}
/*==============================================================================
  FUNCTION:  sns_sam_pm_vote_mips
==============================================================================*/
void sns_sam_pm_vote_mips( sns_sam_internal_mips_type newMips )
{
  if(newMips != sns_sam_pm.current_mips)
  {
    sns_sam_uimage_exit(SNS_SAM_UIMAGE_BLOCK_SAM_BUSY);
    SNS_SAM_PM_VOTE_MIPS( sns_sam_pm.pmHandle, sns_sam_mips_table[newMips] );
    sns_sam_pm.current_mips = newMips;
    sns_sam_uimage_vote_enter(SNS_SAM_UIMAGE_BLOCK_SAM_BUSY);
  }
}

/*==============================================================================
  FUNCTION:  sns_sam_q_count
==============================================================================*/
SNS_SAM_UIMAGE_CODE
static uint32_t sns_sam_q_count( sns_q_s *samQ, OS_EVENT *samQMutex )
{
  uint32_t samQCnt;
  uint8_t errOS;

  sns_os_mutex_pend( samQMutex, 0, &errOS );
  samQCnt = sns_q_cnt(samQ);
  sns_os_mutex_post( samQMutex );

  return samQCnt;
}

/*==============================================================================
  FUNCTION:  sns_sam_pm_adjust_mips
==============================================================================*/
void sns_sam_pm_adjust_mips( void )
{
  uint32_t samReqQCnt, samReportQCnt, samBatchQCnt, samIndQCnt;

  // Get SAM queue counts
  samReqQCnt = sns_sam_q_count(&samReqMsgQ, samReqMsgQMutex);
  samReportQCnt = sns_sam_q_count(&samReportQ, samReportQMutex);
  samBatchQCnt = sns_sam_q_count(&samBatchQ, samBatchQMutex);
  samIndQCnt = sns_sam_q_count(&samIndMsgQ, samIndMsgQMutex);

  // If any of the SAM queues have > threshold items, vote for higher MIPS
  if( (samReqQCnt > SNS_SAM_Q_UPPER_THRESH) || (samReportQCnt > SNS_SAM_Q_UPPER_THRESH) ||
      (samBatchQCnt > SNS_SAM_Q_UPPER_THRESH) || (samIndQCnt > SNS_SAM_Q_UPPER_THRESH) )
  {
    sns_sam_pm_vote_mips( SNS_SAM_MIPS_TURBO );
  }
  // If all of the SAM queues have < threshold items, vote for normal MIPS
  else if( (samReqQCnt < SNS_SAM_Q_LOWER_THRESH) && (samReportQCnt < SNS_SAM_Q_LOWER_THRESH) &&
      (samBatchQCnt < SNS_SAM_Q_LOWER_THRESH) && (samIndQCnt < SNS_SAM_Q_LOWER_THRESH) )
  {
    sns_sam_pm_vote_mips( SNS_SAM_MIPS_NORMAL );
  }
}
/*==============================================================================
  FUNCTION:  sns_sam_pm_init
==============================================================================*/

sns_sam_err sns_sam_pm_init()
{
  uint32_t i;
  uint8_t os_err;
  sns_pm_err_code_e pm_err;
  sns_sam_pm.modeVote = false;
  sns_sam_pm.samImageMode.sam_status = SNS_SAM_MODE_BIGIMAGE;

  for(i = 0; i < SNS_SAM_UIMAGE_BLOCK_COUNT; i++)
  {
    sns_sam_pm.samImageMode.sns_sam_uimage_block[i] = false;
  }
  sns_sam_pm.updateMode = SNS_SAM_MODE_BIGIMAGE;
  sns_sam_pm.current_mips = SNS_SAM_MIPS_NORMAL;
  sns_sam_pm.samImageMode.uimage_avg_run_length = 0;
  sns_sam_pm.samImageMode.bigimage_avg_run_length = 0;
  sns_sam_pm.samImageMode.sam_status = SNS_SAM_MODE_BIGIMAGE;
  sns_sam_pm.samImageMode.timestamp = sns_em_get_timestamp();

  apInSuspendMutex = sns_os_mutex_create_uimg(0, &os_err);
  SNS_ASSERT(OS_ERR_NONE == os_err);

  sns_sam_pm.samStatusMutexPtr = sns_os_mutex_create_uimg(0, &os_err);
  if(OS_ERR_NONE != os_err)
  {
    sns_sam_pm.uimage_enabled = false;
    return SAM_ENONE;
  }

  pm_err = SNS_SAM_PM_INIT( &sns_sam_pm.pmHandle, (suspend_cb_t)&sns_sam_pm_suspend_cb, "SAM");
  if(SNS_PM_SUCCESS != pm_err)
  {
    return SAM_EFAILED;
  }

  pm_err = SNS_SAM_PM_VOTE_MIPS(sns_sam_pm.pmHandle, sns_sam_mips_table[sns_sam_pm.current_mips]);
  pm_err = SNS_SAM_PM_VOTE_MODE(sns_sam_pm.pmHandle, SNS_IMG_MODE_BIG);
  /* hack : preventing power collapse */
  //pm_err = sns_pm_set_latency(sns_sam_pm.pmHandle, 0);

  sns_sam_pm.uimage_enabled = true;
  return SAM_ENONE;
}

