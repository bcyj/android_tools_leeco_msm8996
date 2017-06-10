/*============================================================================
  @file sns_sam_ind.c

  @brief
  All indication processing (both incoming and outgoing) for the SAM
  Framework.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include "sns_sam_ped_v01.h"
#include "sns_sam_ped_v01.h"
#include "sns_smgr_api_v01.h"
#include "sns_smgr_internal_api_v02.h"
#include "sns_smgr_common_v01.h"
#include "sns_init.h"
#include "sns_sam_algo_api.h"
#include "sns_sam.h"
#include "sns_sam_client.h"
#include "sns_sam_service.h"
#include "sns_sam_req.h"
#include "sns_sam_cb.h"
#include "sns_sam_ind.h"
#include "sns_sam_pm.h"
#include "sns_sam_memmgr.h"
#include "sns_profiling.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

/*============================================================================
  Type Declarations
  ===========================================================================*/

/**
 * Used to limit the max size to which the input queue for an
 * algo can grow before triggering a flush of the elements.
 */
#define SAM_ALGO_MAX_INPUT_Q_SIZE 1000

/**
 * Used to store lists of client requests whose report or batch timers have
 * fired and need to be handled.
 */
struct sns_sam_client_event
{
  sns_q_link_s qLink;

  sam_client_req *clientReq;
};
typedef struct sns_sam_client_event sns_sam_client_event;

/**
 * Used to store lists of algorithm instances whose duty cycle timers have
 * fired and need to be handled.
 */
struct sns_sam_algo_event
{
  sns_q_link_s qLink;

  sns_sam_algo_inst *algoInst;
};
typedef struct sns_sam_algo_event sns_sam_algo_event;

/*============================================================================
  Static Data
  ===========================================================================*/
/* Report Timers that have been started*/
sns_q_s samReportTimersQ SNS_SAM_UIMAGE_DATA;

/* Client Requests, whose Report Timers have fired (and need to be handled) */
sns_q_s samReportQ SNS_SAM_UIMAGE_DATA;
OS_EVENT *samReportQMutex SNS_SAM_UIMAGE_DATA;

/* Client Requests, whose Batch Timers have fired (and need to be handled) */
sns_q_s samBatchQ SNS_SAM_UIMAGE_DATA;
OS_EVENT *samBatchQMutex SNS_SAM_UIMAGE_DATA;

/* Algorithm Instances, whose Duty Cycle Timers have fired. */
static sns_q_s samDutyCycleQ;
static OS_EVENT *samDutyCycleQMutex;

/*============================================================================
  Static Function Definitions
  ===========================================================================*/

/**
 * Remove the oldest and unused output data from an algorithm instance. Search
 * through all associated client requests, and check if any of them are still
 * using this data.
 *
 * @param[i] algoInst Algorithm instance to prune
 *
 * @return PENDING
 */

SNS_SAM_UIMAGE_CODE STATIC void
sns_sam_prune_output_head( sns_sam_algo_inst *algoInst )
{
  sns_q_link_s *qCurr;
  sns_sam_algo_output *outputData;

  while( NULL != ( outputData =
        (sns_sam_algo_output*)sns_q_check( &algoInst->algoOutputQ ) ) )
  {
    bool found = false;

    for( qCurr = sns_q_check( &algoInst->clientRequests ); NULL != qCurr;
         qCurr = sns_q_next( &algoInst->clientRequests, qCurr ) )
    {
      sam_client_req *clientReq = (sam_client_req*)qCurr;

      sns_sam_algo_output const *tempData =
        (sns_sam_algo_output*)sns_q_check( &clientReq->outputDataQ );
      if( NULL != tempData &&
          tempData->data == outputData->data)
      {
        found = true;
        break;
      }
    }

    if( found )
    {
      break;
    }
    else
    {
      sns_sam_free_io( algoInst, (intptr_t) outputData );
    }
  }
}

SNS_SAM_UIMAGE_CODE STATIC sns_sam_err
sns_sam_alloc_ind_msg( sns_sam_sensor_algo const *algo, struct sns_sam_msg *message )
{
  if( NULL != algo &&
      0 < algo->qmiIndSize )
  {
    message->msg.buf = (intptr_t)SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD,
      algo->qmiIndSize);
    if(message->msg.buf == (intptr_t)NULL)
    {
      return SAM_ENOMEM;
    }
    SNS_OS_MEMZERO( (void*)message->msg.buf, algo->qmiIndSize );
    message->msg.bufSize = algo->qmiIndSize;

    message->msgID = SNS_SAM_ALGO_REPORT_IND;
    message->sensorUID = &algo->sensor.sensorUID;
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_0( samModule, "Sensor lookup failure" );
    return SAM_ETYPE;
  }

  return SAM_ENONE;
}

/**
 * Create and send a new SAM report indication message.
 *
 * @param[i] clientReq Destination client
 * @param[i] outputData Data to be used to generated the report
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate indication buffer
 *         SAM_EFAILED Error generating or sending the indication message
 */
SNS_SAM_UIMAGE_CODE STATIC sns_sam_err
sns_sam_create_ind( sam_client_req const *clientReq,
    sns_sam_algo_output const *outputData )
{
  sns_sam_err errAlgo;
  sns_sam_err errFW = SAM_ENONE;
  sns_sam_ind_msg indMsg;
  sns_sam_ped_report_ind_msg_v01 *msg;

  sns_profiling_log_qdss( SNS_SAM_ALGO_REPORT_ENTER, 2,
    (uint32_t)clientReq->algoInstance->algorithm->sensor.sensorUID,
    (uint32_t)clientReq->algoInstance );

  errFW = sns_sam_alloc_ind_msg( clientReq->algorithm, &indMsg );
  if( SAM_ENONE == errFW )
  {
    msg = (sns_sam_ped_report_ind_msg_v01*)indMsg.msg.buf;
    msg->instance_id = clientReq->extInstanceID;

    errAlgo = clientReq->algorithm->algoMsgAPI->sns_sam_algo_gen_ind(
        &clientReq->algoInstance->cbFuncs, outputData,
        clientReq->extInstanceID, &indMsg );

    if( SAM_ENONE != errAlgo )
    {
      SNS_PRINTF_STRING_ERROR_2( samModule,
        "Error generating indication for %x, error: %i",
        clientReq->algorithm->sensor.sensorUID, errAlgo );
      errFW = SAM_EFAILED;
    }
    else
    {
      errFW = sns_sam_service_send_ind( &indMsg, clientReq );
      if( SAM_ENONE != errFW )
      {
        SNS_PRINTF_STRING_ERROR_2( samModule,
          "Error sending indication for %x, error %i",
          clientReq->algorithm->sensor.sensorUID, errFW );
        errFW = SAM_EFAILED;
      }
    }

    SNS_SAM_MEM_FREE( (void*)indMsg.msg.buf );
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
        "Not enough memory to allocate indication message for %x",
        clientReq->algorithm->sensor.sensorUID );
    errFW = SAM_ENOMEM;
  }
  sns_profiling_log_qdss( SNS_SAM_ALGO_REPORT_EXIT, 3,
    (uint32_t)clientReq->algoInstance->algorithm->sensor.sensorUID,
    (uint32_t)clientReq->algoInstance, 1 );

  return errFW;
}

/**
 * Copy an output data and add it to a client's queue.  Will typically be
 * used to copy the last output from an algorithm instance into the output
 * queue of the client request.
 *
 * @note Only the output data meta data is copied, the data itself is
 * maintained by the algorithm instance, and will only be freed when all
 * client requests no longer need it.
 *
 * @param[i] clientReq Client whose output queue the copied data will be added
 * @param[i] outputData Output data to copy
 *
 * @return true if the MAX BATCH SIZE has been reached, false otherwise
 */
SNS_SAM_UIMAGE_CODE STATIC bool
sns_sam_copy_output( sam_client_req *clientReq, sns_sam_algo_output const *outputData )
{
  sns_sam_algo_output *outputCopy = NULL;

  if(SNS_SAM_UIMAGE_ALGOINST_ISSUPPORTED(clientReq->algoInstance))
  {
    outputCopy = SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD, sizeof(sns_sam_algo_output) );
  }
  else
  {
    sns_sam_uimage_exit(SNS_SAM_UIMAGE_BLOCK_ALGO);
    outputCopy = SNS_OS_MALLOC( SNS_SAM_DBG_MOD, sizeof(sns_sam_algo_output) );
  }
  SNS_ASSERT(NULL != outputCopy);

  SNS_OS_MEMCOPY( outputCopy, outputData, sizeof(sns_sam_algo_output) );

  sns_q_link( outputCopy, (sns_q_link_s*)outputCopy );
  sns_q_put( &clientReq->outputDataQ, (sns_q_link_s*)outputCopy );

  if(false == SNS_SAM_UIMAGE_ALGOINST_ISSUPPORTED(clientReq->algoInstance))
  {
    sns_sam_uimage_vote_enter(SNS_SAM_UIMAGE_BLOCK_ALGO);
  }

  return (unsigned int)sns_q_cnt( &clientReq->outputDataQ ) >=
    clientReq->algorithm->sensor.attributes[ SAM_ALGO_ATTR_MAX_BATCH ].attrValue;
}

/**
 * Handle adding a new piece of output data to a client request.  May involve
 * flushing the buffer or dropping an output.
 *
 * @param[i] clientReq Client request in question
 * @param[i] outputData New output data to add to the client
 */
SNS_SAM_UIMAGE_CODE STATIC void
sns_sam_handle_batch_output( sam_client_req *clientReq,
  sns_sam_algo_output *outputData )
{
  sns_sam_algo_inst *algoInst = clientReq->algoInstance;
  sns_sam_algo_output const *lastOutput =
    (sns_sam_algo_output*)sns_q_last_check( &algoInst->algoOutputQ );

  if( NULL == lastOutput || lastOutput->data != outputData->data )
  {
    sns_q_put( &algoInst->algoOutputQ, &outputData->qLink );
  }

  if( sns_sam_copy_output( clientReq, outputData) )
  {
    if( !sns_sam_pm_ap_suspend() || clientReq->wuffEnabled )
    {
      sns_sam_handle_batch( clientReq );
    }
    else
    {
      sns_sam_algo_output *temp = sns_q_get( &clientReq->outputDataQ );
      SNS_SAM_MEM_FREE( temp );
      sns_sam_prune_output_head( algoInst );
    }
  }
}

/**
 * Handle the duty cycle timer firing event for a particular client request
 * report.
 *
 * @param[i] algoInst The Algorithm Instance whose duty cycle state must change
 */
STATIC void
sns_sam_handle_dc( sns_sam_algo_inst *algoInst )
{
  algoInst->dcState = !algoInst->dcState;

  SNS_PRINTF_STRING_LOW_1( samModule,
        "Changing duty cycle state to %i", algoInst->dcState );
  // PEND: Disable dependent streams as necessary
  sns_sam_register_dc_timer( algoInst );
}

/**
 * Check all buffered input and output data for the given algorithm instance.
 * If no memory in DDR found, set imageMode field to UIMAGE.
 *
 * @param[i] algoInst The Algorithm Instance to check
 */
STATIC void
sns_sam_check_algo_uimage_status( sns_sam_algo_inst *algoInst )
{
  if( SNS_SAM_ALGOINST_NOMEM_UIMAGE == algoInst->imageMode )
  {
    sns_q_link_s *qCurr;
    bool ddrPtrFound = false;

    for( qCurr = sns_q_check( (sns_q_s *)&algoInst->algoInputQ );
         NULL != qCurr && !ddrPtrFound;
         qCurr = sns_q_next( (sns_q_s *)&algoInst->algoInputQ, qCurr ) )
    {
      if( !SNS_OS_IS_UHEAP_PTR(qCurr) )
      {
        ddrPtrFound = true;
      }
    }

    for( qCurr = sns_q_check( &algoInst->algoOutputQ );
         NULL != qCurr && !ddrPtrFound;
         qCurr = sns_q_next( &algoInst->algoOutputQ, qCurr ) )
    {
      if( !SNS_OS_IS_UHEAP_PTR(qCurr) )
      {
        ddrPtrFound = true;
      }
    }

    if( !ddrPtrFound )
    {
      algoInst->imageMode = SNS_SAM_ALGOINST_UIMAGE;
    }
  }
}

/**
 * Whether to send an indication to this client, based on its location, the
 * current state of the AP, and its preferences as specified in its enable
 * request.
 *
 * @param[i] clientReq Potential indication destination
 *
 * @return True if an indication should be send; false otherwise
 */
STATIC bool
sns_sam_can_send_ind( sam_client_req const *clientReq )
{
  return SNS_PROC_APPS_V01 != clientReq->clientAttr.notifySuspend.proc_type ||
    clientReq->clientAttr.notifySuspend.send_indications_during_suspend ||
    !sns_sam_pm_ap_suspend();
}

/*============================================================================
  Public Function Definitions
  ===========================================================================*/

SNS_SAM_UIMAGE_CODE void
sns_sam_handle_report( sam_client_req *clientReq )
{
  sns_sam_algo_output *outputData;
  sns_sam_err err;
  bool freeOutput = false;
  sns_sam_algo_inst *algoInst = clientReq->algoInstance;

  if( 0 != clientReq->batchPeriod || sns_sam_can_send_ind( clientReq ) )
  {
    err = sns_sam_generate_output( algoInst, &outputData );
    if( SAM_ENONE != err )
    {
      SNS_PRINTF_STRING_LOW_0( samModule, "No available output" );
    }
    else if( 0 == clientReq->batchPeriod )
    {
      err = sns_sam_create_ind( clientReq, outputData );
      if( SAM_ENONE != err )
      {
        SNS_PRINTF_STRING_ERROR_2( samModule,
          "Unable to generate indication for %x, error %i",
          clientReq->algorithm->sensor.sensorUID, err );
      }
      freeOutput = true;
    }
    else
    {
      sns_sam_handle_batch_output( clientReq, outputData );
    }

    if( freeOutput )
    {
      sns_sam_free_io( algoInst, (intptr_t)outputData );
    }
  }
  else
  {
    SNS_PRINTF_STRING_LOW_0( samModule, "AP in suspend, not sending ind" );
  }
}

SNS_SAM_UIMAGE_CODE sns_sam_err
sns_sam_process_output( sns_sam_algo_inst *algoInst,
  sns_sam_algo_output *outputData )
{
  sns_sam_err err = SAM_ENONE;
  sns_q_link_s *qCurr;
  bool freeOutput = true;

  qCurr = sns_q_check( &algoInst->clientRequests );
  while( NULL != qCurr )
  {
    sam_client_req *clientReq = (sam_client_req*)qCurr;
    qCurr = sns_q_next( &algoInst->clientRequests, qCurr );

    if( SNS_SAM_REPORT_MODE_PERIODIC != clientReq->clientAttr.reportMode )
    {
      if( 0 != clientReq->batchPeriod )
      {
        freeOutput = false;
        sns_sam_handle_batch_output( clientReq, outputData );
      }
      else if( sns_sam_can_send_ind( clientReq ) )
      {
        err = sns_sam_create_ind( clientReq, outputData );
        if( SAM_ENOMEM == err )
        {
          SNS_PRINTF_STRING_ERROR_1( samModule,
            "Not enough memory to send ind for %x",
            algoInst->algorithm->sensor.sensorUID );
          err = SAM_ENOMEM;
        }
        else if( SAM_ENONE != err )
        {
          SNS_PRINTF_STRING_ERROR_2( samModule,
            "Error sending indication for %, error %i",
            clientReq->algorithm->sensor.sensorUID, err );
          err = SAM_EFAILED;
        }
      }
    }

    if( SNS_SAM_REPORT_MODE_ONESHOT == clientReq->clientAttr.reportMode )
    {
      sns_sam_remove_client_req( clientReq, NULL );
    }
  }

  if( freeOutput )
  {
    sns_sam_free_io( algoInst, (intptr_t) outputData );
  }

  return err;
}

/**
 * Allocate a message buffer.  The size
 * allocated will be the message size per qmi_idl_get_message_c_struct_len.
 *
 * @param[i] serviceObj QMI Service object associated with the sensor
 * @param[i] msgID Message we wish to create
 * @param[i] messageType Whether the msg is a request, response, or indication
 * @param[o] message Message object to be allocated and initialized
 *
 * @return SAM_ENONE
 *         SAM_ETYPE Error determining message size; invalid msgID or SvcObj
 *         SAM_ENOMEM Not enough memory to message buffer
 */
SNS_SAM_UIMAGE_CODE STATIC sns_sam_err
sns_sam_alloc_msg( qmi_idl_service_object_type const *serviceObj, uint32_t msgID,
    qmi_idl_type_of_message_type messageType, struct sns_sam_msg *message )
{
  uint32_t c_struct_len;
  int32_t errQMI;

  errQMI = qmi_idl_get_message_c_struct_len( *serviceObj, messageType, msgID,
      &c_struct_len );

  if( QMI_IDL_LIB_NO_ERR == errQMI )
  {
    message->msg.buf = (intptr_t)SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD,
      c_struct_len);
    if(message->msg.buf == (intptr_t)NULL)
    {
      return SAM_ENOMEM;
    }
    SNS_OS_MEMZERO( (void*)message->msg.buf, c_struct_len );
    message->msg.bufSize = c_struct_len;
    message->msgID = msgID;
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_2( samModule,
      "QMI length lookup error: %i, for msgID %i", errQMI, msgID );
    return SAM_ETYPE;
  }

  return SAM_ENONE;
}

SNS_SAM_UIMAGE_CODE void
sns_sam_handle_batch( sam_client_req *clientReq )
{
  sns_sam_err errFW;
  sns_sam_err errAlgo;
  sns_sam_algo_inst *algoInst = clientReq->algoInstance;
  sns_sam_ind_msg indMsg;
  uint32_t dataCount = 1, i;
  sns_sam_ped_batch_ind_msg_v01 *msg;
  sns_sam_batch_ind_type indType;

  sns_profiling_log_qdss( SNS_SAM_ALGO_REPORT_ENTER, 2,
    (uint32_t)algoInst->algorithm->sensor.sensorUID, (uint32_t)algoInst );

  if( !sns_sam_pm_ap_suspend() ||
      ( SNS_PROC_APPS_V01 == clientReq->clientAttr.notifySuspend.proc_type &&
        clientReq->clientAttr.notifySuspend.send_indications_during_suspend ) )
  {
    if(!SNS_SAM_UIMAGE_ALGOINST_ISSUPPORTED(algoInst))
    {
      sns_sam_uimage_exit(SNS_SAM_UIMAGE_BLOCK_ALGO);
    }

    //sns_em_get_timestamp64(&clientReq->nextBatchTS);
    // uncommenting this line causes alternaate batch timeouts to be skipped
    // say batch timer is setup to fire at T = 0, 1, 2....
    // say we are at T = 0, this line causes the nextBatchTs to be set to 1.1 instead of 1
    // Subsequent batch timer at T = 1 is skipped and this function is re-entered at T = 2

    clientReq->nextBatchTS += sns_sam_pm_ap_suspend()
      ? sns_em_convert_sec_in_q16_to_localtick( clientReq->batchPeriod )
      : sns_em_convert_sec_in_q16_to_localtick( clientReq->batchPeriodActive );

    SNS_PRINTF_STRING_LOW_1( samModule,
      "Setting next batch event to occur at %i", clientReq->nextBatchTS );

    indType = (uint64_t)sns_q_cnt(&clientReq->outputDataQ) >
      algoInst->algorithm->sensor.attributes[ SAM_ALGO_ATTR_MAX_REPORTS_PER_IND ].attrValue
        ? SAM_BATCH_FIRST_IND_V01 : SAM_BATCH_ONLY_IND_V01;

    while( 0 != dataCount && 0 != sns_q_cnt( &clientReq->outputDataQ ) )
    {
      errFW = sns_sam_alloc_msg( &algoInst->algorithm->sensor.serviceObj,
          SNS_SAM_ALGO_BATCH_IND, QMI_IDL_INDICATION, &indMsg );
      indMsg.sensorUID = &algoInst->algorithm->sensor.sensorUID;

      if( SAM_ENONE == errFW )
      {
        msg = (sns_sam_ped_batch_ind_msg_v01*)indMsg.msg.buf;
        msg->instance_id = clientReq->extInstanceID;

        dataCount = 0;
        errAlgo = algoInst->algorithm->algoMsgAPI->sns_sam_algo_gen_batch_ind(
            &algoInst->cbFuncs, clientReq->extInstanceID, indType,
            &clientReq->outputDataQ, &dataCount, &indMsg );

        for( i = 0; i < dataCount; i++ )
        {
          sns_sam_algo_output *curr = (sns_sam_algo_output*)sns_q_get( &clientReq->outputDataQ );
          SNS_SAM_MEM_FREE( curr );
        }

        indType = (uint64_t)sns_q_cnt(&clientReq->outputDataQ) >
          algoInst->algorithm->sensor.attributes[ SAM_ALGO_ATTR_MAX_REPORTS_PER_IND ].attrValue
            ? SAM_BATCH_INTERMEDIATE_IND_V01 : SAM_BATCH_LAST_IND_V01;

        if( SAM_ESTATE == errAlgo )
        {
          SNS_PRINTF_STRING_ERROR_1( samModule,
            "Dropping invalid output for %x",
            clientReq->algorithm->sensor.sensorUID );
        }
        else if( SAM_ENONE != errAlgo )
        {
          SNS_PRINTF_STRING_ERROR_2( samModule,
            "Unable to generate indication for %x, error %i",
            clientReq->algorithm->sensor.sensorUID, errAlgo );
        }
        else
        {
          errFW = sns_sam_service_send_ind( &indMsg, clientReq );

          if( SAM_ENONE != errFW )
          {
            SNS_PRINTF_STRING_ERROR_2( samModule,
              "Error sending indication for %x, error %i",
              clientReq->algorithm->sensor.sensorUID, errFW );
          }
        }

        SNS_SAM_MEM_FREE( (void*)indMsg.msg.buf );
      }
    }
    if(!SNS_SAM_UIMAGE_ALGOINST_ISSUPPORTED(algoInst))
    {
      sns_sam_uimage_vote_enter(SNS_SAM_UIMAGE_BLOCK_ALGO);
    }

    sns_sam_prune_output_head( algoInst );
  }
  sns_profiling_log_qdss( SNS_SAM_ALGO_REPORT_EXIT, 3,
     (uint32_t)algoInst->algorithm->sensor.sensorUID,
     (uint32_t)algoInst, dataCount );
}

sns_sam_err
sns_sam_ind_init_fw()
{
  const uint8_t priority0 = 0;
  uint8_t errOS = 0;

  samReportQMutex = sns_os_mutex_create_uimg( priority0, &errOS );
  if( 0 != errOS )
  {
    SNS_PRINTF_STRING_FATAL_1( samModule, "Cannot create mutex %i", errOS );
    return SAM_EFAILED;
  }

  samBatchQMutex = sns_os_mutex_create_uimg( priority0, &errOS );
  if( 0 != errOS )
  {
    SNS_PRINTF_STRING_FATAL_1( samModule, "Cannot create mutex %i", errOS );
    return SAM_EFAILED;
  }

  samDutyCycleQMutex = sns_os_mutex_create( priority0, &errOS );
  if( 0 != errOS )
  {
    SNS_PRINTF_STRING_FATAL_1( samModule, "Cannot create mutex %i", errOS );
    return SAM_EFAILED;
  }

  sns_q_init( &samDutyCycleQ );
  sns_q_init( &samReportQ );
  sns_q_init(&samReportTimersQ);
  sns_q_init( &samBatchQ );

  return SAM_ENONE;
}

SNS_SAM_UIMAGE_CODE void
sns_sam_algo_run( sns_sam_ind *indMsg, sns_sam_algo_inst *algoInst )
{
  // PEND: Need to come-up with a policy for allowed dataCount
  uint32_t dataCount = 200;
  sns_sam_err errAlgo;

  sns_profiling_log_qdss( SNS_SAM_ALGO_RUN_ENTER, 2,
    (uint32_t)algoInst->algorithm->sensor.sensorUID, (uint32_t)algoInst );

  if( !SNS_SAM_UIMAGE_ALGOINST_ISSUPPORTED(algoInst) )
  {
    sns_sam_uimage_exit(SNS_SAM_UIMAGE_BLOCK_ALGO);
  }

  algoInst->lastInputPtr = (sns_q_link_s *)sns_q_check((sns_q_s *)&algoInst->algoInputQ);

  sns_profiling_log_qdss( SNS_SAM_PROCESS_IND_ENTER, 2,
    (uint32_t)algoInst->algorithm->sensor.sensorUID, (uint32_t)algoInst );

  errAlgo = algoInst->algorithm->algoMsgAPI->sns_sam_algo_process_ind(
      &algoInst->cbFuncs, &indMsg->msg, (sns_sam_input_q *) &algoInst->algoInputQ, &dataCount );

  sns_profiling_log_qdss( SNS_SAM_PROCESS_IND_EXIT, 2,
    (uint32_t)algoInst->algorithm->sensor.sensorUID, (uint32_t)algoInst );

  if( SAM_ENONE == errAlgo && 0 < dataCount )
  {
    uint32_t processedCnt;
    sns_sam_algo_input *inputData;
    uint32_t j = 0;

    sns_profiling_log_qdss( SNS_SAM_ALGO_UPDATE_ENTER, 2,
      (uint32_t)algoInst->algorithm->sensor.sensorUID, (uint32_t)algoInst );

    errAlgo = algoInst->algorithm->algoAPI->sns_sam_algo_update(
        &algoInst->algoStateData, &algoInst->cbFuncs,
        (sns_sam_input_q *)&algoInst->algoInputQ, &algoInst->algorithm->persistData,
        &processedCnt );

    sns_profiling_log_qdss( SNS_SAM_ALGO_UPDATE_EXIT, 3,
      (uint32_t)algoInst->algorithm->sensor.sensorUID,
      (uint32_t)algoInst, processedCnt );

    if( SAM_ENONE != errAlgo )
    {
      SNS_PRINTF_STRING_ERROR_2( samModule,
        "Error for %x in algo update %i",
        algoInst->algorithm->sensor.sensorUID, errAlgo );
    }

    while( j < processedCnt &&
           NULL != ( inputData = sns_q_get( (sns_q_s *)&algoInst->algoInputQ ) ) )
    {
      sns_sam_free_io( algoInst, (intptr_t) inputData );
      j++;
    }

    if( algoInst->algoInputQ.inputData.cnt > SAM_ALGO_MAX_INPUT_Q_SIZE )
    {
      uint32_t queueLength = algoInst->algoInputQ.inputData.cnt;

      SNS_PRINTF_STRING_ERROR_3( samModule,
        "Flushing input queue for algo %x, queue cnt %i exceeds max limit:%i",
        algoInst->algorithm->sensor.sensorUID,
        algoInst->algoInputQ.inputData.cnt,
        SAM_ALGO_MAX_INPUT_Q_SIZE);

      for( j = 0; j < queueLength &&
           NULL != ( inputData = sns_q_get( &algoInst->algoInputQ.inputData ) );
           j++ )
      {
          sns_sam_free_io( algoInst, (intptr_t) inputData );
      }

      SNS_PRINTF_STRING_ERROR_1( samModule,"After flush input queue cnt:%i",
                                 algoInst->algoInputQ.inputData.cnt);

      // reset stats
      SNS_OS_MEMZERO( (void*)(algoInst->algoInputQ.input_stat),
                      sizeof(sns_sam_input_stat) * SNS_SAM_MAX_DEPENDENT_SENSOR );
    }

  }
  else
  {
    SNS_PRINTF_STRING_HIGH_2( samModule,
      "Error processing ind for %x, error %i",
      algoInst->algorithm->sensor.sensorUID, errAlgo );
  }

  if( !SNS_SAM_UIMAGE_ALGOINST_ISSUPPORTED(algoInst) )
  {
    sns_sam_check_algo_uimage_status( algoInst );
    sns_sam_uimage_vote_enter( SNS_SAM_UIMAGE_BLOCK_ALGO );
  }

  sns_profiling_log_qdss( SNS_SAM_ALGO_RUN_EXIT, 2,
    (uint32_t)algoInst->algorithm->sensor.sensorUID, algoInst );
}

SNS_SAM_UIMAGE_CODE void
sns_sam_handle_ind( sns_sam_ind *indMsg )
{
  sns_q_link_s *qCurr = sns_q_check( &algoInstQ );
  uint32_t i;

  while( NULL != qCurr )
  {
    sns_sam_algo_inst *algoInst = (sns_sam_algo_inst*)qCurr;
    qCurr = sns_q_next( &algoInstQ, qCurr );

    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      if( algoInst->sensorRequests[ i ] == indMsg->sensorReq )
      {
        SNS_PRINTF_STRING_LOW_2( samModule, "Feeding Indication from %x to %x",
          indMsg->sensorReq->sensor->sensorUID,
          algoInst->algorithm->sensor.sensorUID);
        sns_sam_algo_run( indMsg, algoInst );
      }
    }
  }
}

SNS_SAM_UIMAGE_CODE void
sns_sam_handle_batch_timer( void )
{
  sns_q_link_s *qCurr;
  uint8_t errOS;
  sns_sam_client_event *clientEvent;

  sns_sam_pm_adjust_mips( );
  do
  {
    sns_os_mutex_pend( samBatchQMutex, 0, &errOS );
    qCurr = sns_q_get( &samBatchQ );
    clientEvent = (sns_sam_client_event*)qCurr;
    sns_os_mutex_post( samBatchQMutex );

    if( NULL != clientEvent )
    {
      sns_sam_handle_batch( clientEvent->clientReq );
      SNS_SAM_MEM_FREE( clientEvent );
    }
  } while( NULL != clientEvent );
}

void
sns_sam_handle_dc_timer( void )
{
  sns_q_link_s *qCurr;
  uint8_t errOS;
  sns_sam_algo_event *algoEvent;

  do
  {
    sns_os_mutex_pend( samDutyCycleQMutex, 0, &errOS );
    qCurr = sns_q_get( &samDutyCycleQ );
    algoEvent = (sns_sam_algo_event*)qCurr;
    sns_os_mutex_post( samDutyCycleQMutex );

    if( NULL != algoEvent )
    {
      sns_sam_handle_dc( algoEvent->algoInst );
      SNS_SAM_MEM_FREE( algoEvent );
    }
  } while( NULL != algoEvent );
}

SNS_SAM_UIMAGE_CODE void
sns_sam_handle_report_timer( void )
{
  sns_q_link_s *qCurr;
  uint8_t errOS;
  sns_sam_client_event *clientEvent;

  SNS_PRINTF_STRING_LOW_1( samModule,
    "Handle Report Timer %i", sns_q_cnt( &samReportQ ) );

  sns_sam_pm_adjust_mips( );
  do
  {
    sns_os_mutex_pend( samReportQMutex, 0, &errOS );
    qCurr = sns_q_get( &samReportQ );
    clientEvent = (sns_sam_client_event*)qCurr;
    sns_os_mutex_post( samReportQMutex );

    if( NULL != clientEvent )
    {
      sns_q_link_s *qCurr;
      bool clientReqFound = false;

      for( qCurr = sns_q_check( &samReportTimersQ ); NULL != qCurr;
           qCurr = sns_q_next( &samReportTimersQ, qCurr ) )
      {
        sns_sam_algo_report_timers const *timer =
          (sns_sam_algo_report_timers*)qCurr;
        if( NULL != timer && timer->clientReq == clientEvent->clientReq )
        {
          clientReqFound = true;
          break;
        }
      }

      if( !clientReqFound )
      {
        SNS_PRINTF_STRING_MEDIUM_0( samModule,
            "Client req not found in list of active timers" );
        return;
      }
      else
      {
        sns_sam_handle_report( clientEvent->clientReq );
      }

      SNS_SAM_MEM_FREE( clientEvent );
    }
  } while( NULL != clientEvent );
}

SNS_SAM_UIMAGE_CODE void
sns_sam_timer_cb_dc( sns_sam_algo_inst *algoInst )
{
  uint8_t errOS;
  sns_sam_algo_event *algoEvent = SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD,
    sizeof(sns_sam_algo_event) );
  SNS_ASSERT(NULL != algoEvent);

  SNS_PRINTF_STRING_LOW_0( samModule, "sns_sam_timer_cb_dc" );

  algoEvent->algoInst = algoInst;
  sns_q_link( algoEvent, (sns_q_link_s*)algoEvent );

  sns_os_mutex_pend( samDutyCycleQMutex, 0, &errOS );
  sns_q_put( &samDutyCycleQ, (sns_q_link_s*)algoEvent );
  sns_os_mutex_post( samDutyCycleQMutex );

  sns_os_sigs_post( sns_sam_sig_event,
    SNS_SAM_DC_TIMER_SIG, OS_FLAG_SET, &errOS );
}

SNS_SAM_UIMAGE_CODE void
sns_sam_client_timer_cb( sam_client_req *clientReq )
{
  uint8_t errOS;
  uint64_t timestamp;

   SNS_PRINTF_STRING_LOW_0( samModule, "sns_sam_timer_cb_report" );

  if( SNS_SAM_REPORT_MODE_PERIODIC == clientReq->clientAttr.reportMode )
  {
     sns_sam_client_event *clientEvent = SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD,
       sizeof(sns_sam_client_event) );
     SNS_ASSERT(NULL != clientEvent);
     clientEvent->clientReq = clientReq;
     sns_q_link( clientEvent, (sns_q_link_s*)clientEvent );

    sns_os_mutex_pend( samReportQMutex, 0, &errOS );
    sns_q_put( &samReportQ, (sns_q_link_s*)clientEvent );
    sns_os_mutex_post( samReportQMutex );
    // PEND: Race condition regarding a freed client request during callback?
    SNS_PRINTF_STRING_LOW_0( samModule, "Set report signal" );
    sns_os_sigs_post( sns_sam_sig_event,
      SNS_SAM_REPORT_TIMER_SIG, OS_FLAG_SET, &errOS );
  }

  sns_em_get_timestamp64(&timestamp);
  if( 0 != clientReq->batchPeriod && timestamp >= clientReq->nextBatchTS )
  {
     sns_sam_client_event *clientEvent = SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD,
       sizeof(sns_sam_client_event) );
    SNS_ASSERT(NULL != clientEvent);
    clientEvent->clientReq = clientReq;
    sns_q_link( clientEvent, (sns_q_link_s*)clientEvent );

    sns_os_mutex_pend( samBatchQMutex, 0, &errOS );
    sns_q_put( &samBatchQ, (sns_q_link_s*)clientEvent );
    sns_os_mutex_post( samBatchQMutex );

    SNS_PRINTF_STRING_LOW_1( samModule,
        "Set batch signal (ts: %i)", timestamp );
    sns_os_sigs_post( sns_sam_sig_event,
      SNS_SAM_BATCH_TIMER_SIG, OS_FLAG_SET, &errOS );
  }
}
