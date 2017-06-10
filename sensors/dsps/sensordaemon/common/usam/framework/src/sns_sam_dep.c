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
#include "sns_smgr_common_v01.h"
#include "sns_smgr_internal_api_v02.h"
#include "sns_em.h"
#include "sns_init.h"
#include "sns_debug_str.h"
#include "sns_sam.h"
#include "sns_sam_algo_api.h"
#include "sns_sam_client.h"
#include "sns_sam_service.h"
#include "sns_sam_ind.h"
#include "sns_sam_pm.h"
#include "sns_sam_dep.h"
#include "sns_sam_resp.h"
#include "sns_sam_memmgr.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ===========================================================================*/
/* Maximum number of available Report IDs */
#define SNS_SAM_MAX_NUM_REPORT_IDS 256

/*============================================================================
  Static Data
  ===========================================================================*/

/*============================================================================
  Static Function Definitions
  ===========================================================================*/

STATIC bool
sns_sam_is_smgr_sensor( sns_sam_sensor *sensor )
{
  int32_t errQMI = 0;
  uint32_t serviceID, smgrID, smgriID;

  errQMI |= qmi_idl_get_service_id( sensor->serviceObj, &serviceID );
  errQMI |= qmi_idl_get_service_id(
    SNS_SMGR_SVC_get_service_object_v01(), &smgrID );
  errQMI |= qmi_idl_get_service_id(
    SNS_SMGR_INTERNAL_SVC_get_service_object_v02(), &smgriID );
  if( QMI_IDL_LIB_NO_ERR != errQMI )
  {
    SNS_PRINTF_STRING_ERROR_0( samModule, "Error getting Service ID" );
    return false;
  }

  return serviceID == smgrID || serviceID == smgriID;
}

/**
 * Find an existing sensor request that would be acceptable to this algorithm
 * instance.
 *
 * @param[i] sensorUID Sensor UID we are checking for
 * @param[i] algoInst Algorithm instance who will use the dependent sensor
 *
 * @return Existing sufficient sensor request; NULL if none found
 */
STATIC sns_sam_sensor_req*
sns_sam_find_existing_sensor_req( sns_sam_sensor_uid const *sensorUID,
  sns_sam_algo_inst const *algoInst )
{
  sns_q_link_s *qCurr;
  uint32_t i;

  SNS_PRINTF_STRING_LOW_1( samModule,
    "sns_sam_find_existing_sensor_req %x", *sensorUID );

  /* Check if the sensorReq is a SAM algorithm request. If so,
     return NULL. In this way, SAM will create a new request
     to the SAM algorithm, the algorithm itself will determine
     if there's an existing algoInst which could be shared. Here,
     only share the sensorReq which is made to SMGR */
  for( qCurr = sns_q_check( &sensorQ); NULL != qCurr;
       qCurr = sns_q_next( &sensorQ, qCurr ) )
  {
    sns_sam_sensor *sensor = (sns_sam_sensor*)qCurr;
    int32_t serviceID;

    if( *sensorUID == sensor->sensorUID && !sns_sam_is_smgr_sensor( sensor ) )
    {
      return NULL;
    }
  }

  for( qCurr = sns_q_check( &algoInstQ ); NULL != qCurr;
        qCurr = sns_q_next( &algoInstQ, qCurr ) )
  {
    sns_sam_algo_inst *instTemp = (sns_sam_algo_inst*)qCurr;

    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      if( NULL != instTemp->sensorRequests[ i ] &&
          0 == instTemp->sensorRequests[ i ]->batchPeriod &&
          sensorUID == &instTemp->sensorRequests[ i ]->sensor->sensorUID &&
          algoInst->algorithm->algoMsgAPI->sns_sam_algo_check_sensor(
            &algoInst->algoStateData,
            instTemp->sensorRequests[ i ]->enableReq ) )
      {
        SNS_PRINTF_STRING_LOW_0( samModule, "Request Found" );
        return instTemp->sensorRequests[ i ];
      }
    }
  }

  SNS_PRINTF_STRING_LOW_0( samModule, "No Request Found" );
  return NULL;
}

/**
 * Inspect all existing dependent sensor streams, and check whether this new
 * sensor request may replace any of them.
 *
 * @param[i] sensorReq New sensor request required by an algorithm
 */
STATIC void
sns_sam_update_existing_sensor_req( sns_sam_sensor_req *sensorReq )
{
  sns_q_link_s *qCurr;
  uint32_t i;

  if( 0 != sensorReq->batchPeriod )
  {
    return ;
  }
  else if( !sns_sam_is_smgr_sensor( sensorReq->sensor ) )
  {
    /* Only sensor requests made to SMGR are shared within SAM */
    return;
  }

  for( qCurr = sns_q_check( &algoInstQ ); NULL != qCurr;
       qCurr = sns_q_next( &algoInstQ, qCurr ) )
  {
    sns_sam_algo_inst *algoInst = (sns_sam_algo_inst*)qCurr;

    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      sns_sam_sensor_req *tempReq = algoInst->sensorRequests[ i ];
      if( NULL != tempReq &&
          tempReq != sensorReq &&
          0 == tempReq->batchPeriod &&
          &sensorReq->sensor->sensorUID == &tempReq->sensor->sensorUID &&
          algoInst->algorithm->algoMsgAPI->sns_sam_algo_check_sensor(
            &algoInst->algoStateData, sensorReq->enableReq ) )
      {
        SNS_PRINTF_STRING_LOW_1( samModule,
          "Updating request for %x", algoInst->algorithm->sensor.sensorUID );
        algoInst->sensorRequests[ i ] = sensorReq;
        sns_sam_remove_sensor_req( tempReq );
      }
    }
  }
}

/**
 * Lookup an available entry in the algoInst::sensorRequests array to place
 * a new sensor request for the given sensor.  Remove an existing entry for the
 * same SUID if necessary.
 *
 * @param[i] sensorUID SensorUID of the sensor request
 * @param[io] algoInst Algorithm instance who will use the dependent sensor
 *
 * @return Available index into the algoInst::sensorRequests array; -1 if none
 */
STATIC int
sns_sam_find_free_sensor_req_index( sns_sam_sensor_uid const *sensorUID,
  sns_sam_algo_inst *algoInst )
{
  int i, freeIndex = -1;

  for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
  {
    if( NULL == algoInst->sensorRequests[ i ] )
    {
      freeIndex = i;
    }
    else if( *sensorUID == algoInst->sensorRequests[ i ]->sensor->sensorUID )
    {
      sns_sam_sensor_req *sensorReq = algoInst->sensorRequests[ i ];
      algoInst->sensorRequests[ i ] = NULL;
      sns_sam_remove_sensor_req( sensorReq );
      freeIndex = i;
      break;
    }
  }

  return freeIndex;
}

/**
 * Shrinks the memory buffer size of the enable request message.  All messages
 * are pre-allocated with a larger-than-necessary buffer.
 *
 * @param enableReqMsg The message intended to enable some dependent sensor
 */
STATIC void
sns_sam_shrink_enable_req( sns_sam_enable_req *enableReqMsg )
{
  void *newBuffer = SNS_OS_MALLOC( SNS_SAM_DBG_MOD, enableReqMsg->msg.bufSize );
  SNS_ASSERT(NULL != newBuffer);

  SNS_OS_MEMSCPY( newBuffer, enableReqMsg->msg.bufSize, (void*)enableReqMsg->msg.buf,
    enableReqMsg->msg.bufSize );
  SNS_SAM_MEM_FREE( (void*)enableReqMsg->msg.buf );
  enableReqMsg->msg.buf = (intptr_t)newBuffer;
}

/**
 * Allocate and initialize a new sensor request object.
 *
 * @param sensor Sensor object corresponding to the new stream
 * @param enableReqMsg The message intended to enable some dependent sensor
 *
 * @return New sensor request object, or NULL if the allocation failed
 */
STATIC sns_sam_sensor_req*
sns_sam_alloc_sensor_req( sns_sam_sensor *sensor,
  sns_sam_enable_req *enableReqMsg )
{
  sns_sam_sensor_req *sensorReq = NULL;

  sensorReq = SNS_SAM_MEM_ALLOC( SNS_SAM_DBG_MOD, sizeof(sns_sam_sensor_req) );

  SNS_PRINTF_STRING_LOW_2( samModule,
    "Creating new sensor request %x for sensor %x",
    sensorReq, sensor->sensorUID );

  if( NULL != sensorReq )
  {
    sensorReq->sensor = sensor;
    sensorReq->enableReq = enableReqMsg;
    sensorReq->batchPeriod = 0;
    sensorReq->clientHndl = NULL;

    if( SAM_ENONE != sns_sam_client_init( sensorReq ) )
    {
      SNS_SAM_MEM_FREE( sensorReq );
      SNS_PRINTF_STRING_ERROR_0( samModule,
        "Error initializing client connection" );
      sensorReq = NULL;
    }
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_0( samModule,
      "Error allocating new sensor request: Out of Mem" );
  }

  return sensorReq;
}

/**
 * Create and start a new sensor request.
 *
 * @param enableReqMsg The message intended to enable some dependent sensor
 *
 * @return New sensor request object, or NULL upon failure
 */
STATIC sns_sam_sensor_req*
sns_sam_create_new_sensor_req( sns_sam_enable_req *enableReqMsg )
{
  sns_sam_sensor *sensor;
  int32_t errQMI;
  sns_sam_sensor_req *sensorReq = NULL;
  sns_sam_err err;

  SNS_PRINTF_STRING_LOW_1( samModule,
    "sns_sam_create_new_sensor_req %x", *enableReqMsg->sensorUID );
  sensor = sns_sam_lookup_sensor_from_suid( enableReqMsg->sensorUID );
  SNS_ASSERT(NULL != sensor);

  errQMI = qmi_idl_get_message_c_struct_len( sensor->serviceObj,
    QMI_IDL_REQUEST, enableReqMsg->msgID, &enableReqMsg->msg.bufSize );
  if( QMI_NO_ERR != errQMI )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule, "QMI error: %d", errQMI );
  }
  else
  {
    sns_sam_shrink_enable_req( enableReqMsg );

    sensorReq = sns_sam_alloc_sensor_req( sensor, enableReqMsg );
    if( NULL != sensorReq )
    {
      err = sns_sam_client_send( sensorReq, enableReqMsg );
      if( SAM_ENONE != err )
      {
        SNS_PRINTF_STRING_ERROR_2( samModule,
          "Error sending enable request: %x, error %i",
          *enableReqMsg->sensorUID, err );

        sns_sam_client_release( sensorReq );
        sensorReq = NULL;
      }
    }
  }

  return sensorReq;
}

/**
 * Allocate and associate a new dependent sensor request with an algorithm
 * instance. Only one request per dependent sensor is allowed, so this
 * request may replace an existing one (will send a new request, and cancel
 * the existing one).
 *
 * @note The enableReqMsg pointer will be stored for later use.
 *
 * @param[i] algoInst Algorithm instance who will use the dependent sensor
 * @param[i] enableReqMsg The request message used to enable the sensor
 *
 * @return SAM_ENONE
 *         SAM_EFAILED Error creating or sending enable request message
 *         SAM_EMAX Too many sensor requests for this algorithm instance
 */
STATIC sns_sam_err
sns_sam_create_sensor_req( sns_sam_algo_inst *algoInst,
    sns_sam_enable_req *enableReqMsg )
{
  sns_sam_err rv = SAM_ENONE;
  int32_t availableIndex = -1;
  sns_sam_sensor_req *sensorReq;

  availableIndex = sns_sam_find_free_sensor_req_index(
    enableReqMsg->sensorUID, algoInst );
  if( -1 == availableIndex )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
        "Too many sensor requests for algorithm instance: %x",
        *enableReqMsg->sensorUID );
    rv = SAM_EMAX;
  }
  else
  {
    sensorReq = sns_sam_find_existing_sensor_req(
      enableReqMsg->sensorUID, algoInst );
    if( NULL != sensorReq )
    {
      SNS_PRINTF_STRING_LOW_1( samModule, "Using existing request for %x",
        sensorReq->sensor->sensorUID );
      algoInst->sensorRequests[ availableIndex ] = sensorReq;
    }
    else
    {
      sensorReq = sns_sam_create_new_sensor_req( enableReqMsg );

      if( NULL == sensorReq )
      {
        SNS_PRINTF_STRING_ERROR_1( samModule,
          "Failure generating new sensor request: %x",
          *enableReqMsg->sensorUID );
        rv =  SAM_EFAILED;
      }
      else
      {
        algoInst->sensorRequests[ availableIndex ] = sensorReq;
        sns_sam_update_existing_sensor_req( sensorReq );
      }
    }
  }

  return rv;
}

/**
 * Allocate an enable request message object and its associated message buffer
 * for a specific sensor UID.
 *
 * @param sensorUID The Sensor to which we intend to send a request message
 *
 * @return Sensor enable request buffer, or NULL upon failure
 */
STATIC sns_sam_enable_req*
sns_sam_alloc_enable_msg( sns_sam_sensor_uid const *sensorUID )
{
  sns_sam_enable_req *enableReqMsg = NULL;

  enableReqMsg = SNS_OS_MALLOC( SNS_SAM_DBG_MOD, sizeof(sns_sam_enable_req) );
  if( NULL != enableReqMsg )
  {
    enableReqMsg->msg.bufSize = sns_sam_enable_req_size( sensorUID );

    enableReqMsg->msg.buf = (intptr_t)SNS_OS_MALLOC(
      SNS_SAM_DBG_MOD,enableReqMsg->msg.bufSize );
    if( (intptr_t)NULL != enableReqMsg->msg.buf )
    {
      SNS_OS_MEMZERO( (void*)enableReqMsg->msg.buf, enableReqMsg->msg.bufSize );
    }
    else
    {
      SNS_SAM_MEM_FREE( enableReqMsg );
      enableReqMsg = NULL;
    }
  }

  return enableReqMsg;
}

/**
 * Send a batch request for the supplied sensor request.
 *
 * @param sensorRequest An active sensor request
 *
 * @return SAM_ENONE
 *         SAM_EFAILED There was a error when sending the request
 */
STATIC sns_sam_err
sns_sam_send_batch_req( sns_sam_sensor_req *sensorRequest )
{
  sns_sam_err err;
  struct sns_sam_msg reqMsg;
  sns_sam_ped_batch_req_msg_v01 *batchReq;

  reqMsg.msg.bufSize = sizeof(sns_sam_ped_batch_req_msg_v01);
  reqMsg.msg.buf = (intptr_t)SNS_OS_MALLOC( SNS_SAM_DBG_MOD,
    reqMsg.msg.bufSize );
  SNS_ASSERT(NULL != (void*)reqMsg.msg.buf);
  reqMsg.msgID = SNS_SAM_ALGO_BATCH_REQ;
  reqMsg.sensorUID = &sensorRequest->sensor->sensorUID;

  SNS_OS_MEMZERO( (void*)reqMsg.msg.buf, reqMsg.msg.bufSize );
  batchReq = (sns_sam_ped_batch_req_msg_v01*)reqMsg.msg.buf;
  batchReq->instance_id = sensorRequest->instanceID;
  batchReq->batch_period = sensorRequest->batchPeriod;
  batchReq->req_type_valid = false;

  err = sns_sam_client_send( sensorRequest, &reqMsg );
  if( SAM_ENONE != err )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Error sending batch request %i", err );
    return SAM_EFAILED;
  }

  return SAM_ENONE;
}

/*============================================================================
  Public Function Definitions
  ===========================================================================*/

sns_sam_err sns_sam_handle_sensor_change( sns_sam_algo_inst *algoInst,
  sns_sam_sensor_uid const *sensorUID )
{
  sns_sam_err err, rv = SAM_EFAILED;
  sns_sam_enable_req *enableReqMsg;

  SNS_PRINTF_STRING_HIGH_2( samModule,
    "Algo %x Generating dependent req %x", algoInst->algorithm->sensor.sensorUID, *sensorUID );

  sns_sam_uimage_exit(SNS_SAM_UIMAGE_BLOCK_ALGO);

  enableReqMsg = sns_sam_alloc_enable_msg( sensorUID );
  SNS_ASSERT(NULL != (void*)enableReqMsg);

  err = algoInst->algorithm->algoMsgAPI->sns_sam_algo_gen_req(
      &algoInst->algoStateData, sensorUID, enableReqMsg );

  if( SAM_ESTATE == err )
  {
    // Remove any existing dependent requests for this SUID
    sns_sam_find_free_sensor_req_index( sensorUID, algoInst );

    SNS_SAM_MEM_FREE( (void*)enableReqMsg->msg.buf );
    SNS_SAM_MEM_FREE( (void*)enableReqMsg );
    rv = SAM_ENONE;
  }
  else if( SAM_EFAILED == err )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Algo error generating dep req %x",
      algoInst->algorithm->sensor.sensorUID );
  }
  else if( SAM_ENONE == err )
  {
    // If the dependent sensor is NOT available, then stop and return an error.
    sns_sam_sensor *sensor;
    sensor = sns_sam_lookup_sensor_from_suid( sensorUID );
    if( NULL != sensor /*&& sensor->isAvailable*/ )
    {
      enableReqMsg->sensorUID = sensorUID;
      rv = sns_sam_create_sensor_req( algoInst, enableReqMsg );
    }
    else
    {
      // Return ETYPE to signify an unavailable dependent sensor.
      rv = SAM_ETYPE;
      SNS_PRINTF_STRING_ERROR_0( samModule, "Dependent sensor unavailable" );
    }

    if( SAM_ENONE != rv )
    {
      SNS_SAM_MEM_FREE( (void*)enableReqMsg->msg.buf );
      SNS_SAM_MEM_FREE( (void*)enableReqMsg );
    }
  }

  sns_sam_uimage_vote_enter(SNS_SAM_UIMAGE_BLOCK_ALGO);
  return rv;
}

sns_sam_err
sns_sam_start_dependencies( sns_sam_sensor_algo const *algo,
  sns_sam_algo_inst *algoInst )
{
  sns_sam_err rv = SAM_ENONE;
  uint32_t i;

  for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR && SAM_ENONE == rv; i++ )
  {
    if( NULL != algo->dependencies[ i ] )
    {
      rv = sns_sam_handle_sensor_change(
        algoInst, &algo->dependencies[ i ]->sensorUID );

      if( SAM_ENONE != rv )
      {
        rv = SAM_EFAILED;
        break;
      }
    }
  }

  if( SAM_EFAILED == rv )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Error starting dependencies for %x", algo->sensor.sensorUID );
    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      if( NULL != algoInst->sensorRequests[ i ] )
      {
        sns_sam_sensor_req *sensorRequest = algoInst->sensorRequests[ i ];
        algoInst->sensorRequests[ i ] = NULL;
        sns_sam_remove_sensor_req( sensorRequest );
      }
    }
  }

  return rv;
}

sns_sam_err
sns_sam_remove_sensor_req( sns_sam_sensor_req *sensorReq )
{
  sns_q_link_s *qCurr = NULL;
  uint32_t i;
  sns_sam_err err = SAM_ENONE;
  /*sns_sam_sensor_req tempRequest =
    { .sensor = &sensorREG, .enableReq = NULL, .streamID = -1 };*/

  bool streamActive = false;

  // identify if the sensor stream is in active use by any algorithm instance
  for( qCurr = sns_q_check( &algoInstQ ); (NULL != qCurr) && (!streamActive);
       qCurr = sns_q_next( &algoInstQ, qCurr ) )
  {
    sns_sam_algo_inst *tempInst = (sns_sam_algo_inst*)qCurr;

    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      if( tempInst->sensorRequests[ i ] == sensorReq )
      {
        streamActive = true;
        break;
      }
    }
  }

  if( !streamActive )
  {
    SNS_PRINTF_STRING_LOW_2( samModule, "Removing sensor %x request %i",
      sensorReq->sensor->sensorUID, sensorReq->instanceID );

    sns_sam_client_release( sensorReq );

/*for( qCurr = sns_q_check( &algoInstQ ); NULL != qCurr;
       qCurr = sns_q_next( &algoInstQ, qCurr ) )
  {
    sns_sam_algo_inst *tempInst = (sns_sam_algo_inst*)qCurr;

    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      if( tempInst->sensorRequests[ i ] == sensorRequest )
      {
        tempInst->sensorRequests[ i ] = &tempRequest;
      }
    }
  }

  for( qCurr = sns_q_check( &algoInstQ ); NULL != qCurr;
       qCurr = sns_q_next( &algoInstQ, qCurr ) )
  {
    sns_sam_algo_inst *tempInst = (sns_sam_algo_inst*)qCurr;

    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      if( tempInst->sensorRequests[ i ] == &tempRequest )
      {
        tempInst->sensorRequests[ i ] = NULL;
        sns_sam_handle_sensor_change( tempInst,
          &sensorRequest->sensor->sensorUID );
      }
    }
  }
*/
  }

  return err;
}

sns_sam_err
sns_sam_set_batch_period( sns_sam_algo_inst *algoInst,
  sns_sam_sensor_uid const *sensorUID, uint32_t batchPeriod )
{
  sns_q_link_s *qCurr = NULL;
  uint32_t i, sensorReqIndex;
  sns_sam_sensor_req *sensorRequest = NULL;
  bool shared = false;

  if( sns_sam_is_smgr_sensor( sns_sam_lookup_sensor_from_suid( sensorUID ) ) )
  {
    SNS_PRINTF_STRING_HIGH_0( samModule,
      "Ignoring batch request for SMGR sensor" );
    return SAM_ETYPE;
  }

  SNS_PRINTF_STRING_LOW_2( samModule, "sns_sam_set_batch_period for %x to %i",
    *sensorUID, batchPeriod );
  for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
  {
    if( NULL != algoInst->sensorRequests[ i ] &&
        *sensorUID == algoInst->sensorRequests[ i ]->sensor->sensorUID )
    {
      sensorRequest = algoInst->sensorRequests[ i ];
      algoInst->sensorRequests[ i ] = NULL;
      sensorReqIndex = i;
      break;
    }
  }

  if( NULL == sensorRequest )
  {
    return SAM_ETYPE;
  }

  for( qCurr = sns_q_check( &algoInstQ ); NULL != qCurr && !shared;
       qCurr = sns_q_next( &algoInstQ, qCurr ) )
  {
    sns_sam_algo_inst *tempInst = (sns_sam_algo_inst*)qCurr;

    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      if( tempInst->sensorRequests[ i ] == sensorRequest )
      {
        shared = true;
        break;
      }
    }
  }

  if( shared )
  {
    sns_sam_err err;
    sns_sam_enable_req *enableReqMsg;

    SNS_PRINTF_STRING_LOW_0( samModule,
      "Request is shared, create new request" );
    enableReqMsg = sns_sam_alloc_enable_msg( sensorUID );
    SNS_ASSERT(NULL != (void*)enableReqMsg);

    err = algoInst->algorithm->algoMsgAPI->sns_sam_algo_gen_req(
        &algoInst->algoStateData, sensorUID, enableReqMsg );
    if( SAM_ENONE == err )
    {
      sensorRequest = sns_sam_create_new_sensor_req( enableReqMsg );
      if( NULL == sensorRequest )
      {
        return SAM_EFAILED;
      }
    }
    else
    {
      return SAM_EFAILED;
    }
  }

  if( sensorRequest != NULL )
  {
    algoInst->sensorRequests[ sensorReqIndex ] = sensorRequest;
    sensorRequest->batchPeriod = batchPeriod;
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_0( samModule,
      "Sensor request not found" );
    return SAM_EFAILED;
  }

  return sns_sam_send_batch_req( sensorRequest );
}

void
sns_sam_inform_batch( sns_sam_algo_inst const *algoInst )
{
  UNREFERENCED_PARAMETER(algoInst);
  q16_t batchPeriod = INT32_MAX;
  bool flushFull = false;
  sns_q_link_s *qCurr = NULL;

  if( NULL == algoInst->algorithm->algoMsgAPI->sns_sam_algo_handle_batch )
  {
    return ;
  }

  for( qCurr = sns_q_check( &algoInst->clientRequests ); NULL != qCurr;
        qCurr = sns_q_next( &algoInst->clientRequests, qCurr ) )
  {
    sam_client_req const *clientReq = (sam_client_req*)qCurr;
    if( clientReq->batchPeriod < batchPeriod )
    {
      batchPeriod = clientReq->batchPeriod;
    }
    flushFull |= clientReq->wuffEnabled;
  }

  SNS_PRINTF_STRING_LOW_0( samModule, "Inform algorithm of batching client" );

  algoInst->algorithm->algoMsgAPI->sns_sam_algo_handle_batch(
    &algoInst->cbFuncs, &algoInst->algorithm->persistData,
    &algoInst->algoStateData, batchPeriod, flushFull );
}
