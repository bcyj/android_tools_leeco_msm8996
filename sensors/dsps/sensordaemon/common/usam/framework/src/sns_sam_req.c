/*============================================================================
  @file sns_sam_req.c

  @brief
  Handles processing of all incoming request messages to the SAM Framework.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include "sns_sam_ped_v01.h"
#include "sns_smgr_api_v01.h"
#include "sns_em.h"
#include "sns_init.h"
#include "sns_debug_str.h"
#include "sns_sam.h"
#include "sns_sam_algo_api.h"
#include "sns_sam_client.h"
#include "sns_sam_service.h"
#include "sns_sam_cb.h"
#include "sns_sam_req.h"
#include "sns_sam_resp.h"
#include "sns_sam_ind.h"
#include "sns_sam_pm.h"
#include "sns_sam_dep.h"
#include "sns_sam_memmgr.h"
#include "sns_profiling.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ===========================================================================*/
/* Maximum message ID supported by any SAM service */
#define SNS_SAM_MAX_MSG_ID SNS_SAM_ALGO_GET_ATTRIB_REQ

#define SNS_SAM_DC_ON_PERCENT_MAX UINT8_MAX

/*============================================================================
  External Data
  ===========================================================================*/

/* Static list of available uImage algorithms */
extern const sns_sam_sensor_uid samUImageAlgoSUIDs[];
/* Number of algorithms listed in samUImageAlgoSUIDs */
extern const uint32_t samUImageAlgoSUIDsSize;


/*============================================================================
  Type Declarations
  ===========================================================================*/

typedef void (*sns_sam_timer_cb)(void *);

/*============================================================================
  Static Function Definitions
  ===========================================================================*/

/**
 * Mark all matching clients as available, aka "not busy".  Follows
 * q_compare_func_t template.
 *
 * @param[i] itemPtr The current client request to inspect
 * @param[i] compareObj SMR service handle
 *
 * @return 0
 */
SNS_SAM_UIMAGE_CODE STATIC int
sns_sam_mark_client( void *itemPtr, void *compareObject )
{
  sam_client_req *clientReq = (sam_client_req*)itemPtr;
  smr_qmi_client_handle serviceHndl = (smr_qmi_client_handle)compareObject;

  if( clientReq->serviceHndl == serviceHndl )
  {
    clientReq->clientBusy = false;
  }

  return 0;
}

/**
 * Lookup a client request.
 *
 * @param[i] serviceHndl The SMR service handle for this external client
 * @param[i] instanceID The unique ID given to this particular client request
 *
 * @return The client request or NULL if not found
 */
STATIC sam_client_req*
sns_sam_client_req_lookup( smr_qmi_client_handle serviceHndl, uint32_t instanceID )
{
  sns_q_link_s *qCurrAI, *qCurrCR;

  for( qCurrAI = sns_q_check( &algoInstQ ); NULL != qCurrAI;
       qCurrAI = sns_q_next( &algoInstQ, qCurrAI ) )
  {
    sns_sam_algo_inst *algoInst = (sns_sam_algo_inst*)qCurrAI;

    for( qCurrCR = sns_q_check( &algoInst->clientRequests ); NULL != qCurrCR;
         qCurrCR = sns_q_next( &algoInst->clientRequests, qCurrCR ) )
    {
      sam_client_req *clientReq = (sam_client_req*)qCurrCR;

      if( serviceHndl == clientReq->serviceHndl &&
          instanceID == clientReq->extInstanceID )
      {
        return clientReq;
      }
    }
  }

  return NULL;
}

/**
*  Allocate state data memory and preallocate input and output memory
*  for an algorithm Instance.
*
*  @param[i] algoInst  Algorithm instance for which memory should be allocated
*/
SNS_SAM_UIMAGE_CODE STATIC void
sns_sam_alloc_io( sns_sam_algo_inst *algoInst )
{
  uint8_t i;

  algoInst->algoPreallocBuf.preallocInput = NULL;
  algoInst->algoPreallocBuf.preallocOutput = NULL;

  for( ;; )
  {
    if( SNS_SAM_ALGOINST_UIMAGE == algoInst->imageMode )
    {
      uint32_t intputSize = 0,
               outputSize = 0;
      intptr_t inputBufStart, outputBufStart;
      void *ioBuf;

      algoInst->algoStateData.buf = (intptr_t)SNS_OS_U_MALLOC(SNS_SAM_DBG_MOD,
          algoInst->bufferSizes.algoStateSize);

      intputSize += SNS_SAM_ALGO_MAX_IO *
        ( sizeof(sns_sam_algo_input) + algoInst->bufferSizes.inputDataSize );
      outputSize += SNS_SAM_ALGO_MAX_IO *
        ( sizeof(sns_sam_algo_output) + algoInst->bufferSizes.outputDataSize );

      ioBuf = SNS_OS_U_MALLOC(SNS_SAM_DBG_MOD, intputSize + outputSize);

      if( NULL == (void*)algoInst->algoStateData.buf || NULL == ioBuf )
      {
        algoInst->imageMode = SNS_SAM_ALGOINST_BIGIMAGE;
        SNS_OS_U_FREE( (void*)algoInst->algoStateData.buf );
        SNS_OS_U_FREE( ioBuf );
        continue;
      }

      algoInst->algoPreallocBuf.preallocInput = ioBuf;
      algoInst->algoPreallocBuf.preallocOutput =
        (sns_sam_algo_output*)((uint8_t*)ioBuf + intputSize);

      inputBufStart = (intptr_t)
        ( (uint8_t*)algoInst->algoPreallocBuf.preallocInput +
        ( SNS_SAM_ALGO_MAX_IO * sizeof(sns_sam_algo_input) ) );
      outputBufStart = (intptr_t)
        ( (uint8_t*)algoInst->algoPreallocBuf.preallocOutput +
        ( SNS_SAM_ALGO_MAX_IO * sizeof(sns_sam_algo_output) ) );

      for( i = 0; i < SNS_SAM_ALGO_MAX_IO; i++ )
      {
        algoInst->algoPreallocBuf.preallocInput[ i ].data =
          inputBufStart + i * algoInst->bufferSizes.inputDataSize;

        algoInst->algoPreallocBuf.preallocOutput[ i ].data =
          outputBufStart + i * algoInst->bufferSizes.outputDataSize;

        sns_q_link( &algoInst->algoPreallocBuf.preallocInput[ i ],
          &algoInst->algoPreallocBuf.preallocInput[ i ].qLink );
        sns_q_link( &algoInst->algoPreallocBuf.preallocOutput[ i ],
          &algoInst->algoPreallocBuf.preallocOutput[ i ].qLink );
      }
    }
    else
    {
      SNS_PRINTF_STRING_LOW_0( samModule, "Algorithm instance state allocated in DDR" );
      algoInst->algoStateData.buf = (intptr_t)SNS_OS_MALLOC(SNS_SAM_DBG_MOD,
          algoInst->bufferSizes.algoStateSize);
    }
    break;
  }

  algoInst->algoStateData.bufSize = algoInst->bufferSizes.algoStateSize;
  SNS_ASSERT(NULL != (void*)algoInst->algoStateData.buf);
  SNS_OS_MEMZERO((void *)algoInst->algoStateData.buf,
      algoInst->algoStateData.bufSize);
}

/**
 * Determine whether the given buffer falls within the pre-allocated range
 * for the specified algorithm instance.  Will return false by definition, if
 * there is no pre-allocated buffer.
 *
 * @param[i] algoInst Algorithm instance to which the buffer belongs
 * @params[i] buf Pointer to the Input or Output Data object
 *
 * @return True if buf is located within the preallocated range; false otherwise
 */
SNS_SAM_UIMAGE_CODE STATIC bool
sns_sam_in_prealloc_io( sns_sam_algo_inst const *algoInst, void const *buf )
{
  return
    NULL != algoInst->algoPreallocBuf.preallocInput &&
    (uint8_t*)buf >= (uint8_t*)algoInst->algoPreallocBuf.preallocInput &&
     (uint8_t*)buf < (uint8_t*)algoInst->algoPreallocBuf.preallocInput +
      SNS_SAM_ALGO_MAX_IO *
      ( sizeof(sns_sam_algo_output) + algoInst->bufferSizes.outputDataSize +
        sizeof(sns_sam_algo_input) + algoInst->bufferSizes.inputDataSize );
}

/**
 * Determine if the given algorithm instance should be run within uImage, and
 * save that decision within the given object.
 *
 * @param[io] algoInst The algorithm instance in question
 */
STATIC void
sns_sam_determine_uimage( sns_sam_algo_inst *algoInst )
{
  uint32_t i;

  algoInst->imageMode = SNS_SAM_ALGOINST_BIGIMAGE;
  for( i = 0; i < samUImageAlgoSUIDsSize; i++ )
  {
    if( samUImageAlgoSUIDs[ i ] == algoInst->algorithm->sensor.sensorUID )
    {
      algoInst->imageMode = SNS_SAM_ALGOINST_UIMAGE;
    }
  }
}

/**
 * Create and start a new algorithm instance.
 *
 * @param[i] algo The associated algorithm
 * @param[i] configData Configuration data already generated by the algorithm
 * @param[o] algoInst The created algorithm instance object
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate algo inst object
 *         SAM_ESTATE Algorithm cannot be created with configData parameters
 *         SAM_EFAILED Unable to initialize algorithm instance or dependencies
 */
STATIC sns_sam_err
sns_sam_create_algo_inst( sns_sam_sensor_algo *algo,
    sns_sam_algo_config const *configData, sns_sam_algo_inst **algoInstOut )
{
  sns_sam_err rv = SAM_ENONE;
  sns_sam_err errAlgo;
  uint32_t i;
  sns_sam_algo_inst *algoInst;
  sns_err_code_e errSNS1;

  SNS_PRINTF_STRING_LOW_1( samModule, "Create new algo inst for %x",
    algo->sensor.sensorUID );

  algoInst = SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD, sizeof(sns_sam_algo_inst) );
  SNS_ASSERT(NULL != algoInst);
  *algoInstOut = algoInst;

  algoInst->bufferSizes.structSize = sizeof(algoInst->bufferSizes);
  errAlgo = algo->algoAPI->sns_sam_algo_mem_req(
    configData, &algoInst->bufferSizes );
  if( SAM_EFAILED == errAlgo )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
          "Unable to determine buffer sizes for %x",
          algo->sensor.sensorUID );
    rv = SAM_ESTATE;
  }
  else
  {
    sns_sam_init_cb( algoInst, &algoInst->cbFuncs );

    sns_q_link( algoInst, (sns_q_link_s*)algoInst );
    sns_q_init( &algoInst->clientRequests );
    sns_q_init( (sns_q_s *)&algoInst->algoInputQ );
    sns_q_init( &algoInst->algoOutputQ );
    algoInst->algorithm = algo;
    algoInst->dcTimer = NULL;

    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      algoInst->sensorRequests[ i ] = NULL;
    }

    sns_sam_determine_uimage( algoInst );
    sns_sam_alloc_io( algoInst );
    SNS_ASSERT(NULL != (void*)algoInst->algoStateData.buf);

    errSNS1 = sns_em_create_timer_obj( (sns_sam_timer_cb)sns_sam_timer_cb_dc,
      algoInst, SNS_EM_TIMER_TYPE_ONESHOT, &algoInst->dcTimer );
    if( SNS_SUCCESS != errSNS1 )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
        "Error creating DC timer %i", errSNS1 );
      SNS_ASSERT(SNS_SUCCESS == errSNS1);
    }

    errAlgo = algo->algoAPI->sns_sam_algo_reset( configData, &algoInst->cbFuncs,
        &((sns_sam_sensor_algo*)algo)->persistData, &algoInst->algoStateData );
    if( SAM_ENONE != errAlgo )
    {
      SNS_PRINTF_STRING_ERROR_2( samModule,
          "Unable to start algorithm instance %x, error %i",
          algo->sensor.sensorUID, errAlgo );
      rv = SAM_ESTATE;
    }
    else
    {
      rv = sns_sam_start_dependencies( algo, algoInst );
    }
  }

  // If there were errors, clean up the algo instance
  if( SAM_ENONE != rv )
  {
    if( NULL != algoInst )
    {
      if( NULL != algoInst->dcTimer )
      {
        sns_em_delete_timer_obj( algoInst->dcTimer );
      }
      SNS_SAM_MEM_FREE( (void*)algoInst->algoStateData.buf );
      SNS_SAM_MEM_FREE( (void*)algoInst );
    }

    *algoInstOut = NULL;
  }

  return rv;
}

/**
 * Removes an algorithm instance, and does all additional necessary cleanup.
 * Should only be called after no client requests remain.
 *
 * @param[i] algoInst Algorithm instance to be removed
 */
STATIC void
sns_sam_remove_algo_inst( sns_sam_algo_inst *algoInst )
{
  uint32_t i;
  sns_q_link_s *qCurr;

  SNS_PRINTF_STRING_LOW_1( samModule, "Deleting algo inst %x",
    algoInst->algorithm->sensor.sensorUID );

  sns_q_delete( (sns_q_link_s*)algoInst );
  sns_em_delete_timer_obj( algoInst->dcTimer );

  for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
  {
    if( NULL != algoInst->sensorRequests[ i ] )
    {
      sns_sam_sensor_req *sensorRequest = algoInst->sensorRequests[ i ];
      algoInst->sensorRequests[ i ] = NULL;
      sns_sam_remove_sensor_req( sensorRequest );
    }
  }

  while( NULL != ( qCurr = sns_q_get( (sns_q_s *)&algoInst->algoInputQ ) ) )
  {
    if( !sns_sam_in_prealloc_io( algoInst, qCurr ) )
    {
      SNS_SAM_MEM_FREE( qCurr );
    }
  }

  while( NULL != ( qCurr = sns_q_get( &algoInst->algoOutputQ ) ) )
  {
    if( !sns_sam_in_prealloc_io( algoInst, qCurr ) )
    {
      SNS_SAM_MEM_FREE( qCurr );
    }
  }

  SNS_SAM_MEM_FREE( (void*)algoInst->algoStateData.buf );
  SNS_SAM_MEM_FREE( (void*)algoInst->algoPreallocBuf.preallocInput);
  SNS_SAM_MEM_FREE( (void*)algoInst );
}

/**
 * Generates and returns an unique instance ID.  Must be unique
 * per client-algo connection.  (We'll approximate by making it unique
 * per algorithm).
 *
 * @note Due to inconsistencies between algorithm response messages, we must
 *       not use '0' as a valid external instance ID.
 *
 * @param[i] algo Associated algorithm
 *
 * @return Available instance ID
 */
static uint8_t
sns_sam_create_inst_id( sns_sam_sensor_algo const *algo )
{
  sns_q_link_s *qCurrAI, *qCurrCR;
  static uint8_t proposedID = 0;

  if( 0 == proposedID )
  {
    proposedID++;
  }

  qCurrAI = sns_q_check( &algoInstQ );
  while( NULL != qCurrAI )
  {
    sns_sam_algo_inst *algoInst = (sns_sam_algo_inst*)qCurrAI;

    if( algo == algoInst->algorithm )
    {
      for( qCurrCR = sns_q_check( &algoInst->clientRequests ); NULL != qCurrCR;
           qCurrCR = sns_q_next( &algoInst->clientRequests, qCurrCR ) )
      {
        sam_client_req *clientReq = (sam_client_req*)qCurrCR;

        if( proposedID == clientReq->extInstanceID )
        {
          ++proposedID;
          qCurrAI = sns_q_check( &algoInstQ );
          break;
        }
      }

      if( NULL == qCurrCR )
      {
        qCurrAI = sns_q_next( &algoInstQ, qCurrAI );
      }
    }
    else
    {
      qCurrAI = sns_q_next( &algoInstQ, qCurrAI );
    }
  }

  return proposedID++;
}


/**
 * Create a client request object from the SAM enable request message.
 * Inialize all applicable fields, but do not add to global queue.
 *
 * @param[i] reqMsg The SAM enable request message
 * @param[i] algo Corresponding algorithm from reqMsg
 * @param[o] clientReqOut Generated client request object
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate client request object
 */
STATIC sns_sam_err
sns_sam_create_client_req( sns_sam_req_msg const *reqMsg,
    sns_sam_sensor_algo const *algo, sam_client_req **clientReqOut )
{
  sam_client_req *clientReq;
  sns_err_code_e errSNS1;

  clientReq = SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD, sizeof(sam_client_req) );
  SNS_ASSERT(NULL != clientReq);
  *clientReqOut = clientReq;

  clientReq->extInstanceID = sns_sam_create_inst_id( algo );
  clientReq->serviceHndl = reqMsg->serviceHndl;
  clientReq->clientBusy = false;
  clientReq->algorithm = algo;
  clientReq->nextBatchTS = 0;
  clientReq->batchPeriod = 0;
  clientReq->batchPeriodActive = 0;
  clientReq->wuffEnabled = false;
  sns_q_init( &clientReq->outputDataQ );
  SNS_PRINTF_STRING_LOW_2( samModule, "Setting report ID for request for %x to %i",
   *reqMsg->msg.sensorUID, clientReq->extInstanceID );

  SNS_PRINTF_STRING_LOW_2( samModule, "Create client Req %i (%i)",
    clientReq->extInstanceID, reqMsg->serviceHndl );

  errSNS1 = sns_em_create_timer_obj( (sns_sam_timer_cb)sns_sam_client_timer_cb,
    clientReq, SNS_EM_TIMER_TYPE_PERIODIC, &clientReq->clientTimer );
  if( SNS_SUCCESS != errSNS1 )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Error creating client timer %i", errSNS1 );
  }

  sns_q_link( clientReq, (sns_q_link_s*)clientReq );

  return SAM_ENONE;
}

/**
 * Form the response message.
 *
 * @param[i] instanceID Resulting instance ID
 * @param[i] error Sensor1 error, or SUCCESS
 * @param[io] respMsg
 */
STATIC void
sns_sam_create_enable_resp( uint8_t instanceID, sensor1_error_e error,
    struct sns_sam_msg *respMsg )
{
  sns_sam_ped_enable_resp_msg_v01 *enableResp;
  enableResp = (sns_sam_ped_enable_resp_msg_v01*)respMsg->msg.buf;

  enableResp->resp.sns_result_t = ( SENSOR1_SUCCESS == error )
    ? SNS_RESULT_SUCCESS_V01 : SNS_RESULT_FAILURE_V01;
  enableResp->resp.sns_err_t = error;

  enableResp->instance_id_valid = instanceID;
  enableResp->instance_id = instanceID;
}

/**
 * Try and find a matching algorithm instance for a new client request.
 *
 * @param[i] configData Configuration of the new request
 * @param[i] algo Algorithm that the client is requesting
 *
 * @return Acceptable algorithm instance to use, or NULL
 */
STATIC sns_sam_algo_inst*
sns_sam_match_algo_inst( sns_sam_algo_config const *configData,
    sns_sam_sensor_algo const *algo )
{
  sns_q_link_s *qCurr;
  sns_sam_algo_inst *algoInst = NULL;

  for( qCurr = sns_q_check( &algoInstQ ); NULL != qCurr;
        qCurr = sns_q_next( &algoInstQ, qCurr ) )
  {
    algoInst = (sns_sam_algo_inst*)qCurr;

    if( algoInst->algorithm == algo &&
        algo->algoAPI->sns_sam_algo_check_config(
          &algoInst->algoStateData, configData ))
    {
      return algoInst;
    }
  }

  return NULL;
}

/**
 * Process an enable request from some client.  Generates client request
 * and algorithm instances as necessary, and adds them to the global queues.
 *
 * @param[i] reqMsg Incoming request message
 * @param[o] Response message to be generated
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate necessary objects
 *         SAM_ESTATE Algorithm will not initialize
 *         SAM_EFAILED Error creating new algorithm instance
 */
STATIC sns_sam_err
sns_sam_handle_enable_req( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg *respMsg )
{
  sns_sam_err rv = SAM_EFAILED, err;
  sns_sam_sensor_algo *algo = NULL;
  sns_sam_algo_inst *algoInst = NULL;
  sns_sam_algo_config configData;
  sam_client_req *clientReq;

  algo = sns_sam_lookup_algo( reqMsg->msg.sensorUID );

  if( NULL != algo )
  {
    sns_profiling_log_qdss( SNS_SAM_ALGO_ENABLE_ENTER, 1, algo->sensor.sensorUID );
    err = sns_sam_create_client_req( reqMsg, algo, &clientReq );
    if( SAM_ENOMEM != err )
    {
      configData.buf = (intptr_t)SNS_OS_MALLOC( SNS_SAM_DBG_MOD,
      algo->bufferSizes.configDataSize );
      SNS_ASSERT(NULL != (void*)configData.buf);

      err = algo->algoMsgAPI->sns_sam_algo_process_enable_req(
          (sns_sam_enable_req*)&reqMsg->msg, &algo->persistData,
          &clientReq->clientAttr, &configData );
      if( SAM_ENONE != err )
      {
        SNS_PRINTF_STRING_FATAL_2( samModule,
          "Unable to process enable req for %x, error %i",
          reqMsg->msg.sensorUID, err );
        rv = SAM_ESTATE;
      }
      else
      {
        // PEND: Check sample/report rate to be in valid range (per attributes)
        algoInst = sns_sam_match_algo_inst( &configData, algo );

        if( NULL == algoInst )
        {
          err = sns_sam_create_algo_inst( algo, &configData, &algoInst );
          if( SAM_ENONE != err )
          {
            SNS_PRINTF_STRING_FATAL_2( samModule,
              "Error creating algo inst %x, error %i",
              reqMsg->msg.sensorUID, err );
            rv = err;
          }
          else
          {
            void *head = sns_q_check( &algoInstQ );
            SNS_PRINTF_STRING_LOW_1( samModule,
              "Add new algo instance to queue %x",
              algoInst->algorithm->sensor.sensorUID );

            if( SNS_SAM_ALGOINST_UIMAGE == algoInst->imageMode || NULL == head )
            {
              sns_q_put( &algoInstQ, &algoInst->qLink );
            }
            else
            {
              sns_q_insert( &algoInst->qLink, head );
            }
          }
        }

        if( NULL != algoInst )
        {
          clientReq->algoInstance = algoInst;
          sns_q_put( &algoInst->clientRequests, (sns_q_link_s*)clientReq );

          sns_sam_register_dc_timer( algoInst );
          sns_sam_register_client_timer( clientReq );
          rv = SAM_ENONE;

          sns_profiling_log_qdss( SNS_SAM_ALGO_ENABLE_EXIT, 1, algo->sensor.sensorUID );
          if( algoInst->clientRequests.cnt > 1 &&
              SNS_SAM_REPORT_MODE_PERIODIC != clientReq->clientAttr.reportMode )
          {
            sns_sam_handle_report( clientReq );
          }
        }
      }
      SNS_SAM_MEM_FREE( (void*)configData.buf );
    }
    else
    {
      SNS_PRINTF_STRING_FATAL_1( samModule,
        "Unable to create client request %x", reqMsg->msg.sensorUID );
      rv = SAM_ENOMEM;
    }
  }
  else
  {
    SNS_PRINTF_STRING_FATAL_1( samModule,
      "Unable to find SAM algorithm %x", reqMsg->msg.sensorUID );
  }

  if( SAM_ENONE != rv )
  {
    sensor1_error_e errCode =
      ( SAM_ENOMEM == rv ) ? SENSOR1_ENOMEM :
      ( SAM_ESTATE == rv ) ? SENSOR1_EBAD_PARAM :
      ( SAM_EFAILED == rv ) ? SENSOR1_ENOTALLOWED : SENSOR1_EFAILED;

    sns_sam_create_enable_resp( 0, errCode, respMsg );
  }
  else
  {
    sns_sam_create_enable_resp( clientReq->extInstanceID,
      SENSOR1_SUCCESS, respMsg );
  }

  return rv;
}

/**
 * Process a disable request from some client.
 *
 * @param[i] reqMsg Incoming request message
 * @param[o] respMsg Response message to be generated
 *
 * @return SAM_ENONE
 */
STATIC sns_sam_err
sns_sam_handle_disable_req( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg *respMsg )
{
  sam_client_req *clientReq;
  sns_sam_ped_disable_resp_msg_v01 *disableResp =
    (sns_sam_ped_disable_resp_msg_v01*)respMsg->msg.buf;
  sns_sam_ped_disable_req_msg_v01 const *disableReq =
    (sns_sam_ped_disable_req_msg_v01*)reqMsg->msg.msg.buf;

  SNS_PRINTF_STRING_LOW_1( samModule,
    "Disable client request %i", disableReq->instance_id );

  clientReq = sns_sam_client_req_lookup(
    reqMsg->serviceHndl, disableReq->instance_id );

  if( NULL != clientReq )
  {
    sns_sam_remove_client_req( clientReq, NULL );
  }

  disableResp->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
  disableResp->instance_id_valid = disableReq->instance_id;
  disableResp->instance_id = disableReq->instance_id;

  return SAM_ENONE;
}

/**
 * Process a get report request from some client.
 *
 * @param[i] reqMsg Incoming request message
 * @param[o] Response message to be generated
 *
 * @return SAM_ENONE
 *         SAM_ETYPE Cannot find requested client request
 *         SAM_ENOMEM Not enough memory to generate report
 *         SAM_ESTATE Report generation is not available
 *         SAM_EFAILED Unable to generate the report
 */
STATIC sns_sam_err
sns_sam_handle_get_report_req( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg *respMsg )
{
  sns_sam_err err;
  sns_sam_err rv = SAM_ENONE;
  sam_client_req *clientReq = NULL;
  sns_sam_ped_get_report_req_msg_v01 const *reportReq =
    (sns_sam_ped_get_report_req_msg_v01*)reqMsg->msg.msg.buf;
  sns_sam_ped_get_report_resp_msg_v01 *reportResp =
    (sns_sam_ped_get_report_resp_msg_v01*)respMsg->msg.buf;

  clientReq = sns_sam_client_req_lookup(
      reqMsg->serviceHndl, reportReq->instance_id );
  if( NULL != clientReq )
  {
    sns_sam_algo_inst *algoInst = clientReq->algoInstance;
    sns_sam_algo_output *outputData;

    err = sns_sam_generate_output( algoInst, &outputData );
    if( SAM_ENONE == err )
    {
      err = algoInst->algorithm->algoMsgAPI->sns_sam_algo_gen_report(
        outputData, respMsg );

      reportResp->instance_id_valid = reportReq->instance_id;
      reportResp->instance_id = reportReq->instance_id;
      if( SAM_ENONE != err )
      {
        SNS_PRINTF_STRING_FATAL_1( samModule,
          "Error generating report %x", reqMsg->msg.sensorUID );
        rv = SAM_EFAILED;
      }

      sns_sam_free_io( algoInst, (intptr_t) outputData );
    }
    else
    {
      SNS_PRINTF_STRING_FATAL_1( samModule,
        "Error generating output %x", reqMsg->msg.sensorUID );
      rv = SAM_ESTATE;
    }
  }
  else
  {
    rv = SAM_ETYPE;
  }

  return rv;
}

/**
 * Process a get attribute request from some client.
 *
 * @param[i] reqMsg Incoming request message
 * @param[o] Response message to be generated
 *
 * @return SAM_ENONE
 *         SAM_ETYPE Cannot find requested algorithm
 */
STATIC sns_sam_err
sns_sam_handle_attr_req( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg *respMsg )
{
  sns_sam_sensor_algo *algo = NULL;
  sns_sam_get_algo_attrib_resp_msg_v01 *reportResp =
    (sns_sam_get_algo_attrib_resp_msg_v01*)respMsg->msg.buf;

  algo = sns_sam_lookup_algo( reqMsg->msg.sensorUID );

  if( NULL != algo )
  {
    reportResp->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;

    reportResp->proc_type = localProcessor;
    reportResp->max_batch_size =
      algo->sensor.attributes[ SAM_ALGO_ATTR_MAX_BATCH ].attrValue;
    reportResp->supported_reporting_modes =
      algo->sensor.attributes[ SAM_ALGO_ATTR_REPORT_MODE ].attrValue;
    reportResp->algorithm_revision =
      algo->sensor.attributes[ SAM_ALGO_ATTR_REVISION ].attrValue;
    reportResp->min_report_rate =
      algo->sensor.attributes[ SAM_ALGO_ATTR_MIN_REPORT ].supported ?
      algo->sensor.attributes[ SAM_ALGO_ATTR_MIN_REPORT ].attrValue : 0;
    reportResp->max_report_rate =
      algo->sensor.attributes[ SAM_ALGO_ATTR_MAX_REPORT ].supported ?
      algo->sensor.attributes[ SAM_ALGO_ATTR_MAX_REPORT ].attrValue : 0;
    reportResp->min_sample_rate =
      algo->sensor.attributes[ SAM_ALGO_ATTR_MIN_SAMPLE ].supported ?
      algo->sensor.attributes[ SAM_ALGO_ATTR_MIN_SAMPLE ].attrValue : 0;
    reportResp->max_sample_rate =
      algo->sensor.attributes[ SAM_ALGO_ATTR_MAX_SAMPLE ].supported ?
      algo->sensor.attributes[ SAM_ALGO_ATTR_MAX_SAMPLE ].attrValue : 0;
    reportResp->power =
      algo->sensor.attributes[ SAM_ALGO_ATTR_POWER ].attrValue;

    reportResp->sensorUID_valid = true;
    reportResp->sensorUID = algo->sensor.sensorUID;
  }
  else
  {
    return SAM_ETYPE;
  }

  return SAM_ENONE;
}

/**
 * Process a batch request from some client.
 *
 * @param[i] reqMsg Incoming request message
 * @param[o] Response message to be generated
 *
 * @return SAM_ENONE
 *         SAM_ETYPE Cannot find associated client request
 */
STATIC sns_sam_err
sns_sam_handle_batch_req( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg *respMsg )
{
  sam_client_req *clientReq = NULL;
  sns_sam_ped_batch_req_msg_v01 const *batchReq =
    (sns_sam_ped_batch_req_msg_v01*)reqMsg->msg.msg.buf;
  sns_sam_ped_batch_resp_msg_v01 *batchResp =
    (sns_sam_ped_batch_resp_msg_v01*)respMsg->msg.buf;
  sns_sam_sensor *sensor;

  sensor = sns_sam_lookup_sensor_from_suid( reqMsg->msg.sensorUID );
  SNS_ASSERT(NULL != sensor);

  batchResp->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
  batchResp->max_batch_size_valid = true;
  batchResp->max_batch_size = sensor->attributes[ SAM_ALGO_ATTR_MAX_BATCH ].attrValue;
  batchResp->timestamp_valid = true;
  batchResp->timestamp = sns_em_get_timestamp();

  // Return the Instance Id associated with enable request when Req Type is
  // SNS_BATCH_GET_MAX_FIFO_SIZE_V01
  batchResp->instance_id_valid = true;
  batchResp->instance_id = batchReq->instance_id;

  if( !batchReq->req_type_valid ||
      SNS_BATCH_GET_MAX_FIFO_SIZE_V01 != batchReq->req_type )
  {
    clientReq = sns_sam_client_req_lookup(
        reqMsg->serviceHndl, batchReq->instance_id );

    if( NULL != clientReq )
    {
      sns_sam_handle_batch( clientReq );
      clientReq->batchPeriod =
        ( SNS_SAM_REPORT_MODE_PERIODIC == clientReq->clientAttr.reportMode )
        ? clientReq->clientAttr.reportPeriod *
          ( (q16_t)batchReq->batch_period /
            clientReq->clientAttr.reportPeriod )
        : (q16_t)batchReq->batch_period;

      if( 0 == clientReq->batchPeriod && 0 != batchReq->batch_period )
      {
        clientReq->batchPeriod = clientReq->clientAttr.reportPeriod;
      }
      clientReq->batchPeriodActive = clientReq->batchPeriod;

      SNS_PRINTF_STRING_LOW_1( samModule,
        "Setting batch period to %i seconds",
        FX_FIXTOFLT_Q16(clientReq->batchPeriod) );

      clientReq->wuffEnabled = ( batchReq->req_type_valid &&
        SNS_BATCH_WAKE_UPON_FIFO_FULL_V01 == batchReq->req_type )
        ? true : false;

      batchResp->instance_id = clientReq->extInstanceID;

      sns_sam_register_client_timer( clientReq );

      sns_sam_inform_batch( clientReq->algoInstance );
    }
    else
    {
      return SAM_ETYPE;
    }
  }

  return SAM_ENONE;
}

/**
 * Process an update batch request from some client.
 *
 * @param[i] reqMsg Incoming request message
 * @param[o] Response message to be generated
 *
 * @return SAM_ENONE
 *         SAM_ETYPE Cannot find associated client request
 */
STATIC sns_sam_err
sns_sam_handle_update_batch_req( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg *respMsg )
{
  sam_client_req *clientReq = NULL;
  sns_sam_ped_update_batch_period_req_msg_v01 const *batchReq =
    (sns_sam_ped_update_batch_period_req_msg_v01*)reqMsg->msg.msg.buf;
  sns_sam_ped_update_batch_period_resp_msg_v01 *batchResp =
    (sns_sam_ped_update_batch_period_resp_msg_v01*)respMsg->msg.buf;

  clientReq = sns_sam_client_req_lookup(
      reqMsg->serviceHndl, batchReq->instance_id );

  if( NULL != clientReq )
  {
    sns_sam_handle_batch( clientReq );

    if( 0 == clientReq->batchPeriod )
    {
      SNS_PRINTF_STRING_LOW_0( samModule,
        "Received batch update request for non-batching client" );
    }
    else if( 0 == batchReq->active_batch_period )
    {
      clientReq->batchPeriodActive = clientReq->batchPeriod;
    }
    else
    {
      clientReq->batchPeriodActive =
        ( SNS_SAM_REPORT_MODE_PERIODIC == clientReq->clientAttr.reportMode )
        ? clientReq->clientAttr.reportPeriod *
          ( (q16_t)batchReq->active_batch_period /
            clientReq->clientAttr.reportPeriod )
        : (q16_t)batchReq->active_batch_period;

      if( 0 == clientReq->batchPeriodActive
          && 0 != batchReq->active_batch_period )
      {
        clientReq->batchPeriodActive = clientReq->clientAttr.reportPeriod;
      }

      SNS_PRINTF_STRING_LOW_1( samModule,
        "Setting active batch period to %i seconds",
        FX_FIXTOFLT_Q16(clientReq->batchPeriodActive) );

      sns_sam_register_client_timer( clientReq );
    }

    batchResp->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
    batchResp->instance_id_valid = true;
    batchResp->instance_id = clientReq->extInstanceID;
    batchResp->timestamp_valid = true;
    batchResp->timestamp = sns_em_get_timestamp();
  }
  else
  {
    return SAM_ETYPE;
  }

  return SAM_ENONE;
}

/**
 * Process an update request from some client.
 *
 * @param[i] reqMsg Incoming request message
 * @param[o] Response message to be generated
 *
 * @return SAM_ENONE
 *         SAM_EFAILED No response message is available
 *         SAM_ETYPE Cannot find associated client request
 */
STATIC sns_sam_err
sns_sam_handle_update_req( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg *respMsg )
{
  sam_client_req *clientReq = NULL;
  sns_sam_err err;
  sns_sam_ped_reset_req_msg_v01 const *updateReq =
    (sns_sam_ped_reset_req_msg_v01*)reqMsg->msg.msg.buf;
  sns_sam_ped_update_batch_period_resp_msg_v01 *updateResp =
    (sns_sam_ped_update_batch_period_resp_msg_v01*)respMsg->msg.buf;

  clientReq = sns_sam_client_req_lookup(
      reqMsg->serviceHndl, updateReq->instance_id );

  if( NULL != clientReq )
  {
    err = clientReq->algorithm->algoMsgAPI->sns_sam_algo_process_update_req(
      &reqMsg->msg, &clientReq->algorithm->persistData,
      &clientReq->algoInstance->algoStateData, respMsg );

    updateResp->instance_id_valid = true;
    updateResp->instance_id = clientReq->extInstanceID;

    if( SAM_ENONE != err )
    {
      SNS_PRINTF_STRING_ERROR_2( samModule,
        "Error processing update request for %x, error: %i",
        clientReq->algorithm->sensor.sensorUID, err );
      err = SAM_EFAILED;
    }
  }
  else
  {
    return SAM_ETYPE;
  }

  return SAM_ENONE;
}

/**
 * Process a cancel request, and generate an appropriate response message.
 *
 * @param[i] reqMsg Incoming request message
 * @param[o] Response message to be generated
 *
 * @return SAM_ENONE
 */
STATIC sns_sam_err
sns_sam_handle_cancel_req( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg *respMsg )
{
  sns_common_cancel_resp_msg_v01 *cancelResp =
    (sns_common_cancel_resp_msg_v01*)respMsg->msg.buf;

  SNS_PRINTF_STRING_LOW_1( samModule,
    "Removing clients from %i", reqMsg->serviceHndl );

  sns_sam_remove_all_client_req( reqMsg->serviceHndl );

  cancelResp->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;

  return SAM_ENONE;
}

/**
 * Generate a version response message.
 *
 * @param[i] reqMsg Incoming request message
 * @param[o] Response message to be generated
 *
 * @return SAM_ENONE
 *         SAM_ETYPE Cannot find associated algorithm
 */
STATIC sns_sam_err
sns_sam_handle_version_req( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg *respMsg )
{
  UNREFERENCED_PARAMETER(reqMsg);
  sns_sam_sensor *sensor;
  sns_common_version_resp_msg_v01 *versionResp =
    (sns_common_version_resp_msg_v01*)respMsg->msg.buf;

  sensor = sns_sam_lookup_sensor_from_suid( reqMsg->msg.sensorUID );

  if( NULL != sensor )
  {
    versionResp->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
    qmi_idl_get_idl_minor_version( sensor->serviceObj,
        &versionResp->interface_version_number );
    versionResp->max_message_id = SNS_SAM_MAX_MSG_ID;
  }
  else
  {
    return SAM_ETYPE;
  }

  return SAM_ENONE;
}

/*============================================================================
  Public Function Definitions
  ===========================================================================*/

void
sns_sam_remove_client_req( sam_client_req *clientReq, void *unused )
{
  UNREFERENCED_PARAMETER(unused);
  sns_q_link_s *qCurr;

  sns_q_delete( &clientReq->qLink );
  SNS_PRINTF_STRING_LOW_1( samModule, "Removing client request %i",
    clientReq->extInstanceID );

  for( qCurr = sns_q_check( &samReportTimersQ ); NULL != qCurr;
      qCurr = sns_q_next( &samReportTimersQ, qCurr ) )
  {
    if( ((sns_sam_algo_report_timers *)qCurr)->clientReq  == clientReq)
    {
      sns_q_delete( qCurr );
      SNS_OS_ANY_FREE(qCurr);
    }
  }

  sns_em_delete_timer_obj( clientReq->clientTimer );

  while( NULL != ( qCurr = sns_q_get( &clientReq->outputDataQ ) ) )
  {
    sns_sam_algo_output *algoOutput = (sns_sam_algo_output*)qCurr;
    // Data buffer will be freed as part of the algo instance
    SNS_SAM_MEM_FREE( (void*)algoOutput );
  }

  if( 0 == sns_q_cnt( &clientReq->algoInstance->clientRequests ) )
  {
    sns_sam_remove_algo_inst( clientReq->algoInstance );
  }
  else
  {
    sns_sam_inform_batch( clientReq->algoInstance );
  }

  SNS_SAM_MEM_FREE( (void*)clientReq );
}

void
sns_sam_remove_all_client_req( smr_qmi_client_handle serviceHndl )
{
  sns_q_link_s *qCurrAI, *qCurrCR;

  for( qCurrAI = sns_q_check( &algoInstQ ); NULL != qCurrAI; )
  {
    sns_sam_algo_inst *algoInst = (sns_sam_algo_inst*)qCurrAI;
    qCurrAI = sns_q_next( &algoInstQ, qCurrAI );

    for( qCurrCR = sns_q_check( &algoInst->clientRequests ); NULL != qCurrCR; )
    {
      sam_client_req *clientReq = (sam_client_req*)qCurrCR;
      qCurrCR = sns_q_next( &algoInst->clientRequests, qCurrCR );

      if( clientReq->serviceHndl == serviceHndl )
      {
        sns_sam_remove_client_req( clientReq, NULL );
      }
    }
  }
}

void
sns_sam_handle_req( sns_sam_req_msg const *reqMsg )
{
  sns_sam_err err = SAM_ENOT_SUPPORTED;

  SNS_PRINTF_STRING_LOW_1( samModule, "sns_sam_handle_req msgID %i",
    reqMsg->msg.msgID );

  struct sns_sam_msg *respMsg = (struct sns_sam_msg *)&reqMsg->respMsg;

  if( SNS_SAM_ALGO_CANCEL_REQ == reqMsg->msg.msgID )
  {
    err = sns_sam_handle_cancel_req( reqMsg, respMsg );
  }
  else if( SNS_SAM_ALGO_VERSION_REQ == reqMsg->msg.msgID )
  {
    err = sns_sam_handle_version_req( reqMsg, respMsg );
  }
  else if( SNS_SAM_ALGO_ENABLE_REQ == reqMsg->msg.msgID )
  {
    err = sns_sam_handle_enable_req( reqMsg, respMsg );
  }
  else if( SNS_SAM_ALGO_DISABLE_REQ == reqMsg->msg.msgID )
  {
    err = sns_sam_handle_disable_req( reqMsg, respMsg );
  }
  else if( SNS_SAM_ALGO_GET_REPORT_REQ == reqMsg->msg.msgID )
  {
    err = sns_sam_handle_get_report_req( reqMsg, respMsg );
  }
  else if( SNS_SAM_ALGO_UPDATE_REQ == reqMsg->msg.msgID )
  {
    err = sns_sam_handle_update_req( reqMsg, respMsg );
  }
  else if( SNS_SAM_ALGO_BATCH_REQ == reqMsg->msg.msgID )
  {
    err = sns_sam_handle_batch_req( reqMsg, respMsg );
  }
  else if( SNS_SAM_ALGO_UPDATE_BATCH_PERIOD_REQ == reqMsg->msg.msgID )
  {
    err = sns_sam_handle_update_batch_req( reqMsg, respMsg);
  }
  else if( SNS_SAM_ALGO_GET_ATTRIB_REQ == reqMsg->msg.msgID )
  {
    err = sns_sam_handle_attr_req( reqMsg, respMsg );
  }
  else
  {
    SNS_PRINTF_STRING_FATAL_2( samModule,
        "Received unknown message ID %i for %x",
        reqMsg->msg.msgID, reqMsg->msg.sensorUID );
  }

  if( SAM_ENONE != err )
  {
    // PEND: Convert SAM error into Sensor1 error
    // Respond with a generic error
    sns_common_version_resp_msg_v01 *versionResp
      = (sns_common_version_resp_msg_v01*)respMsg->msg.buf;
    versionResp->resp.sns_result_t = SNS_RESULT_FAILURE_V01;
    versionResp->resp.sns_err_t = SENSOR1_EFAILED;

    SNS_PRINTF_STRING_ERROR_3( samModule,
        "Error processing message %i for %x: %i",
        reqMsg->msg.msgID, reqMsg->msg.sensorUID, err );
  }

  err = sns_sam_service_send_resp( reqMsg, respMsg );
  if( SAM_ENONE != err )
  {
    SNS_PRINTF_STRING_ERROR_2( samModule,
        "Error sending response message",
        respMsg->sensorUID, err );
  }
}

void
sns_sam_mark_client_avail( smr_service_hndl serviceHndl )
{
  sns_q_link_s *qCurr;

  for( qCurr = sns_q_check( &algoInstQ ); NULL != qCurr;
       qCurr = sns_q_next( &algoInstQ, qCurr ) )
  {
    sns_sam_algo_inst *algoInst = (sns_sam_algo_inst*)qCurr;

    sns_q_linear_search( &algoInst->clientRequests,
      &sns_sam_mark_client, serviceHndl );
  }
}

void
sns_sam_register_dc_timer( sns_sam_algo_inst *algoInst )
{
  sns_err_code_e err;
  uint32_t deltaTickTime = 0;
  sns_q_link_s *qCurr;
  uint8_t maxDCPercentOn = 0;
  q16_t minReportPeriod = 0;

  for( qCurr = sns_q_check( &algoInst->clientRequests ); NULL != qCurr;
       qCurr = sns_q_next( &algoInst->clientRequests, qCurr ) )
  {
    sam_client_req *clientReq = (sam_client_req*)qCurr;

    maxDCPercentOn = clientReq->clientAttr.dcPercentOn > maxDCPercentOn
      ? clientReq->clientAttr.dcPercentOn : maxDCPercentOn;
    minReportPeriod = clientReq->clientAttr.reportPeriod < minReportPeriod
      ? clientReq->clientAttr.reportPeriod : minReportPeriod;
  }

  if( SNS_SAM_DC_ON_PERCENT_MAX != maxDCPercentOn && 0 != maxDCPercentOn )
  {
    double dcPercent = (SNS_SAM_DC_ON_PERCENT_MAX - maxDCPercentOn) /
      SNS_SAM_DC_ON_PERCENT_MAX;

    deltaTickTime = minReportPeriod *
      ( algoInst->dcState ? dcPercent : 1 - dcPercent );

    err = sns_em_register_timer( algoInst->dcTimer, deltaTickTime );
    if( SNS_SUCCESS != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
            "Error registering duty cycle timer %i", err );
    }
  }
  else
  {
    sns_em_cancel_timer( algoInst->dcTimer );
  }
}

void
sns_sam_register_client_timer( sam_client_req *clientReq )
{
  sns_err_code_e err;
  uint32_t deltaTickTime = 0;
  sns_sam_algo_report_timers *timerInst;

  if( SNS_SAM_REPORT_MODE_PERIODIC == clientReq->clientAttr.reportMode )
  {
    deltaTickTime =
      sns_em_convert_sec_in_q16_to_localtick( clientReq->clientAttr.reportPeriod );
    err = sns_em_register_timer( clientReq->clientTimer, deltaTickTime );
    if( SNS_SUCCESS != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
            "Error registering client timer %i", err );
    }
    timerInst =(sns_sam_algo_report_timers *) SNS_OS_ANY_MALLOC(SNS_SAM_DBG_MOD, sizeof(sns_sam_algo_report_timers));
    SNS_ASSERT(NULL != timerInst);
    timerInst->clientReq = clientReq;
    sns_q_link(timerInst, (sns_q_link_s *)timerInst);
    sns_q_put(&samReportTimersQ, (sns_q_link_s *)timerInst);
  }
  else if( 0 != clientReq->batchPeriod )
  {
    deltaTickTime = sns_sam_pm_ap_suspend()
      ? sns_em_convert_sec_in_q16_to_localtick( clientReq->batchPeriod )
      : sns_em_convert_sec_in_q16_to_localtick( clientReq->batchPeriodActive );
    err = sns_em_register_timer( clientReq->clientTimer, deltaTickTime );
    if( SNS_SUCCESS != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
            "Error registering client timer %i", err );
    }
    timerInst = (sns_sam_algo_report_timers *)
      SNS_OS_ANY_MALLOC(SNS_SAM_DBG_MOD, sizeof(sns_sam_algo_report_timers));
    SNS_ASSERT(NULL != timerInst);

    timerInst->clientReq = clientReq;
    sns_q_link(timerInst, (sns_q_link_s *)timerInst);
    sns_q_put(&samReportTimersQ, (sns_q_link_s *)timerInst);
  }
  else
  {
    sns_q_link_s *qCurr;

    for( qCurr = sns_q_check( &samReportTimersQ ); NULL != qCurr;
        qCurr = sns_q_next( &samReportTimersQ, qCurr ) )
    {
      if( ((sns_sam_algo_report_timers *)qCurr)->clientReq  == clientReq)
      {
        sns_q_delete(qCurr);
        SNS_OS_ANY_FREE(qCurr);
      }
    }

    sns_em_cancel_timer( clientReq->clientTimer );
  }
  sns_sam_inform_batch( clientReq->algoInstance );
}

SNS_SAM_UIMAGE_CODE sns_sam_algo_input*
sns_sam_alloc_input( sns_sam_algo_inst *algoInst )
{
  uint8_t i;
  sns_sam_algo_input *inputData = NULL;

  if( SNS_SAM_ALGOINST_UIMAGE == algoInst->imageMode ||
      SNS_SAM_ALGOINST_NOMEM_UIMAGE == algoInst->imageMode )
  {
    for( i = 0; i < SNS_SAM_ALGO_MAX_IO; i++ )
    {
      if( !Q_ALREADY_QUEUED(&algoInst->algoPreallocBuf.preallocInput[ i ].qLink) )
      {
        inputData = &algoInst->algoPreallocBuf.preallocInput[ i ];
        break;
      }
    }

    if( NULL == inputData )
    {
      inputData = SNS_OS_U_MALLOC(SNS_SAM_DBG_MOD,
        sizeof(sns_sam_algo_input) + algoInst->bufferSizes.inputDataSize);
      if( NULL == inputData )
      {
        sns_sam_uimage_exit(SNS_SAM_UIMAGE_BLOCK_ALGO);
        algoInst->imageMode = SNS_SAM_ALGOINST_NOMEM_UIMAGE;
      }
    }
  }

  if( NULL == inputData )
  {
    inputData = SNS_OS_MALLOC(SNS_SAM_DBG_MOD,
      sizeof(sns_sam_algo_input) + algoInst->bufferSizes.inputDataSize);
  }

  if( NULL != inputData )
  {
    if( !sns_sam_in_prealloc_io( algoInst, inputData ) )
    {
      inputData->data =
        (intptr_t)( (uint8_t*)inputData + sizeof(sns_sam_algo_input) );
      sns_q_link( inputData, &inputData->qLink );
    }
  }
  else
  {
    SNS_PRINTF_STRING_HIGH_0( samModule, "Unable to allocate input data" );
  }

  return inputData;
}

SNS_SAM_UIMAGE_CODE sns_sam_algo_output*
sns_sam_alloc_output( sns_sam_algo_inst *algoInst )
{
  uint32_t i;
  sns_sam_algo_output *outputData = NULL;

  if( SNS_SAM_ALGOINST_UIMAGE == algoInst->imageMode ||
      SNS_SAM_ALGOINST_NOMEM_UIMAGE == algoInst->imageMode )
  {
    for( i = 0; i < SNS_SAM_ALGO_MAX_IO; i++ )
    {
      if( !Q_ALREADY_QUEUED(&algoInst->algoPreallocBuf.preallocOutput[ i ].qLink) )
      {
        outputData = &algoInst->algoPreallocBuf.preallocOutput[ i ];
        break;
      }
    }

    if( NULL == outputData )
    {
      outputData = SNS_OS_U_MALLOC(SNS_SAM_DBG_MOD,
        sizeof(sns_sam_algo_output) + algoInst->bufferSizes.outputDataSize);
      if( NULL == outputData )
      {
        sns_sam_uimage_exit(SNS_SAM_UIMAGE_BLOCK_ALGO);
        algoInst->imageMode = SNS_SAM_ALGOINST_NOMEM_UIMAGE;
      }
    }
  }

  if( NULL == outputData )
  {
    outputData = SNS_OS_MALLOC(SNS_SAM_DBG_MOD,
      sizeof(sns_sam_algo_output) + algoInst->bufferSizes.outputDataSize);
  }

  if( NULL != outputData )
  {
    if( !sns_sam_in_prealloc_io( algoInst, outputData ) )
    {
      outputData->data =
        (intptr_t)( (uint8_t*)outputData + sizeof(sns_sam_algo_output) );
      sns_q_link( outputData, &outputData->qLink );
    }
  }
  else
  {
    SNS_PRINTF_STRING_HIGH_0( samModule, "Unable to allocate output data" );
  }

  return outputData;
}

SNS_SAM_UIMAGE_CODE void
sns_sam_free_io( sns_sam_algo_inst const *algoInst,
    intptr_t ioData )
{
  if( Q_ALREADY_QUEUED((sns_q_link_s *)ioData) )
  {
    sns_q_delete((sns_q_link_s *)ioData );
  }

  if( !sns_sam_in_prealloc_io( algoInst, (void const *)ioData ) )
  {
    SNS_SAM_MEM_FREE( ioData );
  }
}

SNS_SAM_UIMAGE_CODE sns_sam_err
sns_sam_generate_output( sns_sam_algo_inst *algoInst, sns_sam_algo_output **outputDataIn )
{
  sns_sam_err err;
  sns_sam_algo_output *outputData = NULL;
  *outputDataIn = NULL;

  if( !SNS_SAM_UIMAGE_ALGOINST_ISSUPPORTED(algoInst) )
  {
    sns_sam_uimage_exit(SNS_SAM_UIMAGE_BLOCK_ALGO);
  }

  outputData = sns_sam_alloc_output( algoInst );

  if( NULL == algoInst->algorithm->algoAPI->sns_sam_algo_generate )
  {
    err = SAM_EFAILED;
  }
  else if( NULL == outputData )
  {
    err = SAM_ENOMEM;
  }
  else
  {
    err = algoInst->algorithm->algoAPI->sns_sam_algo_generate(
        &algoInst->algoStateData, &algoInst->cbFuncs, outputData );
    if( SAM_ENONE != err )
    {
      sns_sam_free_io( algoInst, (intptr_t) outputData );
    }
    else
    {
      *outputDataIn = outputData;
    }
  }

  if( !SNS_SAM_UIMAGE_ALGOINST_ISSUPPORTED(algoInst) )
  {
    sns_sam_uimage_vote_enter(SNS_SAM_UIMAGE_BLOCK_ALGO);
  }

  return err;
}