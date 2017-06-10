/*============================================================================
  @file sns_sam_cb.c

  @brief
  Implements the Algorithm callback functions.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include <string.h>
#include "log.h"
#include "sns_sam_ped_v01.h"
#include "sns_init.h"
#include "sns_usmr.h"
#include "sns_sam.h"
#include "sns_sam_algo_api.h"
#include "sns_sam_req.h"
#include "sns_sam_reg.h"
#include "sns_sam_ind.h"
#include "sns_sam_cb.h"
#include "sns_sam_client.h"
#include "sns_sam_pm.h"
#include "sns_sam_dep.h"
#include "sns_sam_memmgr.h"

#if !defined(SNS_DSPS_BUILD)
#include "sns_debug_str.h"
#define SAM_ROLLOVER_THRESH UINT64_MAX

SNS_SAM_UIMAGE_CODE STATIC bool sns_sam_island_status() { return false; }
#else
#include "sns_profiling.h"
#include "msg.h"
#define BUFSZ 1024

/* SAM_ROLLOVER_THRESH defines the minimum difference between timestamps to be present for
    a timestamp rollover to have occoured between the two timestamps. */
#define SAM_ROLLOVER_THRESH (UINT32_MAX - (DSPS_SLEEP_CLK * 60 * 5))

typedef struct
{
  char const *filename;
  uint16_t lineNum;
  uint8_t numParamsValid;
  intptr_t param1;
  intptr_t param2;
  intptr_t param3;
} debug_params_s;

SNS_SAM_UIMAGE_CODE STATIC bool sns_sam_island_status()
{
  return qurt_island_get_status();
}

/**
 * Prints out the debug string.
 *
 * @param[i] moduleID Module identifier
 * @param[i] priority Priority of the message string
 * @param[i] debugStr Debug string
 * @param[i] formatParams Format parameter structure
 */
STATIC void sns_debug_printf_string( sns_debug_module_id_e moduleID,
  uint8_t priority, const char *debugStr, const debug_params_s *formatParams )
{
  UNREFERENCED_PARAMETER(moduleID);
  /*char buf[ BUFSZ ];
  int index = 0;
  int strsz;

  if( NULL == formatParams || NULL == debugStr || priority > SNS_MSG_ERROR )
  {
    return;
  }

  strsz = snprintf( buf + index, BUFSZ - index, "%s(%d):",
    formatParams->filename, formatParams->lineNum ) ;

  if( strsz > 0 && strsz <= BUFSZ )
  {
    index = strsz;
  }

  switch( formatParams->numParamsValid )
  {
    case 0:
      snprintf( buf + index, BUFSZ - index, "%s", debugStr );
      break;
    case 1:
      snprintf( buf + index, BUFSZ - index, debugStr,
        formatParams->param1 );
      break;
    case 2:
      snprintf( buf + index, BUFSZ - index, debugStr,
        formatParams->param1, formatParams->param2 );
      break;
    case 3:
      snprintf( buf + index, BUFSZ - index, debugStr,
        formatParams->param1, formatParams->param2, formatParams->param3 );
      break;
  }

  MSG_SPRINTF_1( MSG_SSID_SNS, DBG_LOW_PRIO, "%s", buf );*/
  MSG_SPRINTF_FMT_VAR_3( MSG_SSID_SNS, DBG_LOW_PRIO, debugStr, formatParams->param1, formatParams->param2, formatParams->param3 );
}
#endif

/*============================================================================
  Type Declarations
  ===========================================================================*/

typedef void (*sns_sam_timer_cb)(void *);

/**
 * If an timer event occurs due to an algorithm timer, the event is stored in
 * this object, and processed asychronously later, within the SAM main thread.
 */
struct sns_sam_algo_timer_event
{
  /* Data fields necessary to add this object to a SAM list */
  sns_q_link_s qLink;
  /* The algorithm instance whose timer has fired */
  struct sns_sam_algo_inst *algoInst;
};
typedef struct sns_sam_algo_timer_event sns_sam_algo_timer_event;

/*============================================================================
  Static Data
  ===========================================================================*/

/* Contains all unprocessed algorithm timer callback events.  Each item
 * has item has type sns_sam_algo_timer_event. */
static sns_q_s algoTimerQ;
static OS_EVENT *algoTimerQMutex = NULL;

/*============================================================================
  Static Function Definitions
  ===========================================================================*/

/**
 * Send a write request to the Sensors Registry.  Processed asynchronously.
 *
 * @param[i] algoInst This algorithm instance
 * @param[io] regData Registry data to write
 *
 * @return
 *  SAM_ENONE
 *  SAM_ETYPE Invalid Registry ID or wrong data length
 *  SAM_EFAILED Error occurred sending the request; no response expected
 */
STATIC sns_sam_err
sns_sam_algo_reg_req( struct sns_sam_algo_inst const *algoInst,
    sns_sam_reg_data *regData )
{
  UNREFERENCED_PARAMETER(algoInst);
  sns_sam_err err;

  SNS_PRINTF_STRING_LOW_0( samModule, "sns_sam_algo_reg_req" );

  err = sns_sam_reg_req( regData );

  return ( SAM_ENONE == err ) ? SAM_ENONE : SAM_EFAILED;
}

/**
 * Log a message string generated by the algorithm.  The SAM Framework may
 * do nothing with the message, or log it to one of several buffers.
 *
 * @param[i] sensor This algorithm instance
 * @param[i] debug_str Debug string
 * @param[i] fmt_params_ptr Parameters to the debug string
 *
 * @return
 *  SAM_ENONE
 *  SAM_EFAILED Error occurred during log processing
 *  SAM_ETYPE The string was improperly formatted
 *  SAM_ENOT_AVAILABLE Logging is not available at this time
 */
STATIC sns_sam_err
sns_sam_algo_log( struct sns_sam_algo_inst const *algoInst,
    char const *debugStr, sns_sam_algo_debug_params const *formatParams )
{
  UNREFERENCED_PARAMETER(algoInst);

  if(sns_sam_island_status())
  {
    return SAM_ENOT_AVAILABLE;
  }

  sns_debug_printf_string( samModule, SNS_MSG_MEDIUM, debugStr,
    (debug_params_s*)formatParams );

  return SAM_ENONE;
}

/**
 * Indicates to the SAM Framework that the list or qualities of the
 * algorithm's dependent sensors has changes.  The framework should
 * then call sns_sam_algo_gen_req() for the sensor.
 *
 * @param[i] algoInst This algorithm instance
 * @param[i] sensorUID Dependent sensor that has been changed/added/removed
 *
 * @return
 *  SAM_ENONE
 *  SAM_ETYPE Invalid or unknown sensor ID
 *  SAM_EFAILED An error occured; the sensor change was not recorded
 */
SNS_SAM_UIMAGE_CODE STATIC sns_sam_err
sns_sam_algo_sensor_change( sns_sam_algo_inst const *algoInst,
    sns_sam_sensor_uid const *sensorUID )
{
  sns_sam_err err;
  sns_sam_uimage_exit(SNS_SAM_UIMAGE_BLOCK_SAM_BUSY);
  err = sns_sam_handle_sensor_change(
    (sns_sam_algo_inst*)algoInst, sensorUID );
  sns_sam_uimage_vote_enter(SNS_SAM_UIMAGE_BLOCK_SAM_BUSY);
  return err;
}
/**
 * Inserts an input object into the algorithm's input queue such that all the
 * items are in chronological sequence. The timestamp field present in
 * sns_sam_algo_input is used as the input object's timestamp when
 * determining the current input's position in the input queue.
 *
 * @param[i] algoInst This algorithm instance
 * param[i] inputData Data to be added to the input queue
 *
 */
SNS_SAM_UIMAGE_CODE STATIC void
sns_sam_algo_insert_input( sns_sam_algo_inst const *algoInstConst, sns_sam_algo_input *inputData)
{
  sns_sam_algo_inst *algoInst = (sns_sam_algo_inst *)algoInstConst;
  sns_sam_algo_input *lastInputPtr = (sns_sam_algo_input *) algoInst->lastInputPtr;

  while(NULL != lastInputPtr)
  {
    if(inputData->timestamp >= lastInputPtr->timestamp)
    {
      // In case existing queue started after rollover, we are before rollover
      if( SAM_ROLLOVER_THRESH < inputData->timestamp - lastInputPtr->timestamp )
      {
        sns_q_insert(&inputData->qLink, (sns_q_link_s*)lastInputPtr );
        break;
      }
      else
      {
        lastInputPtr = (sns_sam_algo_input *)sns_q_next((sns_q_s *)&algoInst->algoInputQ,
          (sns_q_link_s*)lastInputPtr);
      }
    }
    else
    {
      // In case existing queue started after rollover, we are before rollover
      if( SAM_ROLLOVER_THRESH < lastInputPtr->timestamp - inputData->timestamp )
      {
        lastInputPtr = (sns_sam_algo_input *)sns_q_next((sns_q_s *)&algoInst->algoInputQ,
          (sns_q_link_s*)lastInputPtr);
      }
      else
      {
        sns_q_insert(&inputData->qLink, (sns_q_link_s*)lastInputPtr );
        break;
      }
    }
  }
  if(NULL == lastInputPtr)
  {
    sns_q_put((sns_q_s *)&algoInst->algoInputQ, (sns_q_link_s *) inputData);
  }
  algoInst->lastInputPtr = (sns_q_link_s *)inputData;
}

/**
 * Allocates memory for a single input data, adds that item to the algorithm
 * instance input queue, and copies the supplies contents onto the buffer.
 *
 * @note inputObj Must have the length as supplied in sns_sam_algo_mem_req.
 *
 * @param[i] sensor This algorithm instance
 * @param[i] inputObj Data to be added to the input queue
 * @param[i] addToHead Whether to add the new input to the head or tail of
 *                     the queue.  The tail is the typical choice.
 *
 * @return
 *  SAM_ENONE
 *  SAM_EFAILED No more memory is presently available on the system
 *  SAM_EMAX The algorithm is not allowed any more input allocations
 */
SNS_SAM_UIMAGE_CODE STATIC sns_sam_err
sns_sam_algo_add_input( struct sns_sam_algo_inst const *algoInstConst,
  intptr_t inputObj, sns_sam_timestamp timestamp )
{
  sns_sam_algo_inst *algoInst = (sns_sam_algo_inst *)algoInstConst;
  sns_sam_algo_input *inputData = sns_sam_alloc_input( algoInst );;

  sns_profiling_log_qdss( SNS_SAM_ALGO_CB_ADD_INPUT_ENTER, 2,
      (uint32_t)algoInstConst->algorithm->sensor.sensorUID, (uint32_t)algoInstConst );
  if( NULL != inputData )
  {
    SNS_OS_MEMCOPY((void*)inputData->data, (void*)inputObj, algoInst->bufferSizes.inputDataSize);
    inputData->timestamp = timestamp;
    sns_sam_algo_insert_input(algoInstConst, inputData);
  }
  else
  {
    SNS_PRINTF_STRING_HIGH_0( samModule, "Unable to allocate input data" );
  }
  sns_profiling_log_qdss( SNS_SAM_ALGO_CB_ADD_INPUT_EXIT, 2,
      (uint32_t)algoInstConst->algorithm->sensor.sensorUID, (uint32_t)algoInstConst );

  return SAM_ENONE;
}

/**
  * Allocates memory for a single output data, adds that item to the algorithm
  * instance output queue, and copies the supplies contents onto the buffer.
  *
  * @note outputData Must have the length as supplied in sns_sam_algo_mem_req.
  *
  * @param[i] sensor This algorithm instance
  * @param[i] outputObj Data to be added to the output queue
  *
  * @return
  *  SAM_ENONE
  *  SAM_EFAILED No more memory is presently available on the system
  *  SAM_EMAX The algorithm is not allowed any more output allocations
  */
SNS_SAM_UIMAGE_CODE STATIC sns_sam_err
sns_sam_algo_add_output( struct sns_sam_algo_inst const *algoInstConst,
  intptr_t outputObj )
{
  sns_sam_algo_inst *algoInst = (sns_sam_algo_inst *)algoInstConst;
  sns_sam_algo_output *outputData = sns_sam_alloc_output( algoInst );

  sns_profiling_log_qdss( SNS_SAM_ALGO_CB_ADD_OUTPUT_ENTER, 2,
      (uint32_t)algoInstConst->algorithm->sensor.sensorUID, (uint32_t)algoInstConst );
  if( NULL != outputData )
  {
    sns_sam_err err;

    SNS_OS_MEMCOPY((void*)outputData->data, (void*)outputObj, algoInst->bufferSizes.outputDataSize);

    err = sns_sam_process_output( algoInst, outputData );
    if( SAM_ENONE != err )
    {
      SNS_PRINTF_STRING_ERROR_2( samModule,
        "Error processing output for %x, error %i",
        algoInst->algorithm->sensor.sensorUID, err );
    }
  }
  sns_profiling_log_qdss( SNS_SAM_ALGO_CB_ADD_OUTPUT_EXIT, 2,
      (uint32_t)algoInstConst->algorithm->sensor.sensorUID, (uint32_t)algoInstConst );

  return SAM_ENONE;
}

/**
 * Submits a log packet to DIAG.  Packet will not be saved by framework, and
 * hence is safe on the stack.
 *
 * @param[i] algoInst This algorithm instance
 * @param[i] logPktID Log Packet ID
 * @param[i] logPktSize sizeof the log packet
 * @param[i] logPkt Packet packet to submit
 *
 * @return
 *  SAM_ENONE
 *  SAM_EFAILED Unable to sucessfully submit the log packet
 *  SAM_ENOT_AVAILABLE Logging is not available at this time
 */
SNS_SAM_UIMAGE_CODE STATIC sns_sam_err
sns_sam_algo_log_submit( struct sns_sam_algo_inst const *algoInst,
  uint32_t logPktID, uint32_t logPktSize, PACK(void*) logPkt )
{
  bool result;

  sns_profiling_log_qdss( SNS_SAM_ALGO_CB_LOG_SUBMIT_ENTER, 3,
      (uint32_t)algoInst->algorithm->sensor.sensorUID,
      (uint32_t)algoInst, logPktID );

  if(sns_sam_island_status())
  {
    return SAM_ENOT_AVAILABLE;
  }

  log_set_length( logPkt, logPktSize );
  log_set_code( logPkt, logPktID );
  //log_set_timestamp( logPkt );
  result = log_submit( logPkt );

  sns_profiling_log_qdss( SNS_SAM_ALGO_CB_LOG_SUBMIT_EXIT, 3,
      (uint32_t)algoInst->algorithm->sensor.sensorUID,
      (uint32_t)algoInst, logPktID );

  return result ? SAM_ENONE : SAM_EFAILED;
}

/**
  * Indicate to the SAM Framework that the algorithm would prefer that the
  * associated sensor stream be batched.
  *
  * @param[i] algoInst This algorithm instance
  * @param[i] sensorUID Dependent sensor that was already enabled
  * @param[i] batchPeriod Max time (in seconds Q16) between
  *       batched indications
  * @param[i] flushFull True: Wake client to flush full buffer;
  *                     False: Replace oldest if buffer full & client in suspend
  *
  * @return
  *  SAM_ENONE
  *  SAM_ETYPE No active stream is found with this sensor; not a SAM sensor
  */
STATIC sns_sam_err
sns_sam_algo_req_batch( struct sns_sam_algo_inst const *algoInst,
  sns_sam_sensor_uid const *sensorUID, uint32_t batchPeriod, bool flushFull )
{
  UNREFERENCED_PARAMETER(flushFull);
  sns_sam_err err;

  SNS_PRINTF_STRING_LOW_2( samModule,
        "Algorithm %x requested batched %x",
        algoInst->algorithm->sensor.sensorUID, *sensorUID );

  sns_sam_uimage_exit(SNS_SAM_UIMAGE_BLOCK_ALGO);
  err = sns_sam_set_batch_period( (sns_sam_algo_inst*)algoInst, sensorUID,
    batchPeriod );
  sns_sam_uimage_vote_enter(SNS_SAM_UIMAGE_BLOCK_ALGO);

  return SAM_ETYPE == err ? SAM_ETYPE : SAM_ENONE;
}

/**
  * Callback function to be registered with the EM service.  Will create a
  * timer event object on the queue, and signal the SAM main thread for
  * processing.
  *
  * @note This algorithm instance may not be valid, if the algorithm instance
  *       is removed within this function.
  *
  * @param[i] algoInst The algorithm instance which requested the timer
  */
STATIC void
sns_sam_timer_cb_algo( sns_sam_algo_inst *algoInst )
{
  uint8_t errOS;
  sns_sam_algo_timer_event *timerEvent = SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD,
    sizeof(sns_sam_algo_timer_event) );
  SNS_ASSERT(NULL != timerEvent);

  timerEvent->algoInst = algoInst;

  SNS_PRINTF_STRING_LOW_0( samModule, "sns_sam_timer_cb_algo" );

  sns_q_link( timerEvent, (sns_q_link_s*)timerEvent );

  sns_os_mutex_pend( algoTimerQMutex, 0, &errOS );
  sns_q_put( &algoTimerQ, (sns_q_link_s*)timerEvent );
  sns_os_mutex_post( algoTimerQMutex );

  sns_os_sigs_post( sns_sam_sig_event,
    SNS_SAM_ALGO_TIMER_SIG, OS_FLAG_SET, &errOS );
}

/**
  * Register a timer callback.  Only one timer is active per algorithm
  * instance; subsequent requsts update or disable the existing timer.
  *
  * @note If a timer is disabled by the algorithm, it is guaranteed to not
  *       receive the timeout event.
  *
  * @param[i] algoInst This algorithm instance
  * @param[i] timeout Nanoseconds until the algorithm will be alerted
  *                   0 to disable the current active timer.
  * @param[i] timerCB Callback function to register for this timer event.
  *
  * @return
  *  SAM_ENONE
  *  SAM_EFAILED Unable to successfully register the timer
  */
STATIC sns_sam_err
sns_sam_algo_timer_reg( struct sns_sam_algo_inst const *algoInstConst,
  uint64_t timeout, sns_sam_algo_timer_cb timerCB )
{
  sns_sam_algo_inst *algoInst = (sns_sam_algo_inst *)algoInstConst;

  // Converting from nanoseconds to microseconds
  timeout = timeout/1000;

  if( 0 == timeout )
  {
    if( NULL != algoInst->algoTimerCB )
    {
      uint8_t errOS;
      sns_em_delete_timer_obj( algoInst->algoTimer );

      sns_os_mutex_pend( algoTimerQMutex, 0, &errOS );
      algoInst->algoTimerCB = NULL;
      sns_os_mutex_post( algoTimerQMutex );
      sns_sam_clear_algo_timer( algoInst );
    }
  }
  else
  {
    uint32_t timeoutTick = sns_em_convert_usec_to_localtick( timeout );
    sns_err_code_e err;

    if( NULL == algoInst->algoTimerCB )
    {
      err = sns_em_create_timer_obj( (sns_sam_timer_cb)&sns_sam_timer_cb_algo,
        algoInst, SNS_EM_TIMER_TYPE_ONESHOT, &algoInst->algoTimer );
      if( SNS_SUCCESS != err )
      {
        SNS_PRINTF_STRING_ERROR_1( samModule,
          "Error creating Algo timer %i", err );
        return SAM_EFAILED;
      }
    }

    algoInst->algoTimerCB = timerCB;
    err = sns_em_register_timer( algoInst->algoTimer, timeoutTick );

    if( SNS_SUCCESS != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
          "Error registering Algo timer %i", err );
      return SAM_EFAILED;
    }
  }

  return SAM_ENONE;
}

/**
 * Log QDSS events for algorithm update per input
 *
 * @param[in] algoInst This algorithm instance.
 * @param[in] qdssEventId QDSS SW event
 * @param[in] inputType Specify the input type for algo depending on inputs from multiple sensors
 *                      For algo depending on only one kind of sensor input, use 0.
 *
 * @return
 * SAM_ENONE -- Operation was successful. \n
 *
 * @dependencies
 * None.
 */
SNS_SAM_UIMAGE_CODE static sns_sam_err
sns_sam_algo_qdss_algo_update( struct sns_sam_algo_inst const *algoInst,
  sns_tracer_event qdssEventId, uint32_t inputType )
{
  sns_profiling_log_qdss( qdssEventId, 3,
    (uint32_t)algoInst->algorithm->sensor.sensorUID, (uint32_t)algoInst, inputType );
  return SAM_ENONE;
}
/*============================================================================
  Function Definitions
  ===========================================================================*/

void
sns_sam_clear_algo_timer( sns_sam_algo_inst const *algoInst )
{
  uint8_t errOS;
  sns_q_link_s *qCurr;

  sns_os_mutex_pend( algoTimerQMutex, 0, &errOS );
  for( qCurr = sns_q_check( &algoTimerQ ); NULL != qCurr;
       qCurr = sns_q_next( &algoTimerQ, qCurr ) )
  {
    sns_sam_algo_timer_event *timerEvent =
      (sns_sam_algo_timer_event*)qCurr;
    // Confirm that the algorithm instance has not been removed
    if( algoInst == timerEvent->algoInst )
    {
       sns_q_delete( &timerEvent->qLink );
       SNS_SAM_MEM_FREE( timerEvent );
       break;
    }
  }
  sns_os_mutex_post( algoTimerQMutex );
}

void
sns_sam_handle_algo_timer()
{
  struct sns_sam_algo_timer_event *timerEvent = NULL;
  uint8_t errOS;

  sns_os_mutex_pend( algoTimerQMutex, 0, &errOS );
  while( NULL != ( timerEvent = sns_q_get( &algoTimerQ ) ) )
  {
    sns_sam_algo_inst *algoInst = timerEvent->algoInst;
    sns_sam_algo_timer_cb timerCB = NULL;
    sns_q_link_s *qCurr;

    for( qCurr = sns_q_check( &algoInstQ ); NULL != qCurr;
         qCurr = sns_q_next( &algoInstQ, qCurr ) )
    {
      sns_sam_algo_inst const *tempAI = (sns_sam_algo_inst*)qCurr;
      // Confirm that the algorithm instance has not been removed
      if( tempAI == algoInst )
      {
        timerCB = algoInst->algoTimerCB;
      }
    }

    if( NULL != timerCB )
    {
      sns_os_mutex_post( algoTimerQMutex );
      timerCB( algoInst, &algoInst->cbFuncs, &algoInst->algoStateData );
      sns_os_mutex_pend( algoTimerQMutex, 0, &errOS );
    }
    SNS_SAM_MEM_FREE( timerEvent );
  }
  sns_os_mutex_post( algoTimerQMutex );
}

void
sns_sam_init_cb( sns_sam_algo_inst const *algoInst,
  sns_sam_algo_callback *cbFunctions )
{
  uint8_t errOS = 0;
  cbFunctions->structSize = sizeof(sns_sam_algo_callback);
  cbFunctions->algoInst = algoInst;

  cbFunctions->sns_sam_algo_reg_req = &sns_sam_algo_reg_req;
  cbFunctions->sns_sam_algo_log = &sns_sam_algo_log;
  cbFunctions->sns_sam_algo_sensor_change = &sns_sam_algo_sensor_change;
  cbFunctions->sns_sam_algo_add_input = &sns_sam_algo_add_input;
  cbFunctions->sns_sam_algo_add_output = &sns_sam_algo_add_output;
  cbFunctions->sns_sam_algo_log_submit = &sns_sam_algo_log_submit;
  cbFunctions->sns_sam_algo_req_batch = &sns_sam_algo_req_batch;
  cbFunctions->sns_sam_algo_timer_reg = &sns_sam_algo_timer_reg;
  cbFunctions->sns_sam_algo_qdss_algo_update = &sns_sam_algo_qdss_algo_update;

  if( NULL == algoTimerQMutex )
  {
    algoTimerQMutex = sns_os_mutex_create( 0, &errOS );
    if( 0 != errOS )
    {
      SNS_PRINTF_STRING_FATAL_1( samModule, "Cannot create mutex %i", errOS );
      return;
    }
    sns_q_init( &algoTimerQ );
  }
}
