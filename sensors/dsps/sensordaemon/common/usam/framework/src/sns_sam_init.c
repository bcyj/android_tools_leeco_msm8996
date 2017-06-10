/*============================================================================
  @file sns_sam_init.c

  @brief
  Initialization code for the SAM Framework.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include "sns_sam_common_v01.h"
#include "sns_smgr_api_v01.h"
#include "sns_smgr_internal_api_v02.h"
#include "sns_reg_api_v02.h"
#include "sns_init.h"
#include "sns_em.h"
#include "sns_debug_str.h"
#include "sns_sam.h"
#include "sns_sam_algo_api.h"
#include "sns_sam_init.h"
#include "sns_sam_client.h"
#include "sns_sam_service.h"
#include "sns_sam_req.h"
#include "sns_sam_reg.h"
#include "sns_sam_ind.h"
#include "sns_sam_pm.h"
#include "sns_sam_memmgr.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ===========================================================================*/

/* How long to wait for all registry responses before marking them as
 * unattainable.  In microseconds. */
#define SNS_SAM_INIT_REG_TIMEOUT_US 100000

/* How long SAM is prepared to batch algorithm output for in seconds */
#define SNS_SAM_MAX_BATCHING_TIME 10

/*============================================================================
  External Data
  ===========================================================================*/

/* Static list of available algorithms */
extern const sns_sam_algo_register samAlgoRegisterFuncs[];
/* Number of algorithms listed in samAlgoRegisterFuncs */
extern const uint32_t samAlgoRegisterFuncsSize;

/* Static list of available uImage algorithms */
extern const sns_sam_sensor_uid samUImageAlgoSUIDs[];
/* Number of algorithms listed in samUImageAlgoSUIDs */
extern const uint32_t samUImageAlgoSUIDsSize;

/*============================================================================
  Static Data
  ===========================================================================*/

/* Timeout timer for registry requests during SAM initialization */
static sns_em_timer_obj_t registryTimerObj;

/* List of services that are available, but no client connection has been made
 * with SMR for them yet */
static sns_q_s samClientInitQ;
static OS_EVENT *samClientInitQMutex;

/*============================================================================
  Static Function Definitions
  ===========================================================================*/

/**
 * Allocate memory for a new sensor algorithm object.
 *
 * @param[o] algo Algorithm pointer
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate sensor_algo object
 */
STATIC sns_sam_err
sns_sam_init_algo( sns_sam_sensor_algo **algo )
{
  sns_sam_sensor_algo *algoOut;
  uint32_t i;

  algoOut = SNS_OS_U_MALLOC( SNS_SAM_DBG_MOD, sizeof(sns_sam_sensor_algo) );
  if(NULL == algoOut)
  {
    return SAM_ENOMEM;
  }

  algoOut->algoAPI = NULL;
  algoOut->algoMsgAPI = NULL;
  for( i = 0; i < SNS_SAM_MAX_REG_GRP; i++ )
  {
    algoOut->registryGroups[ i ]= -1;
  }
  for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
  {
    algoOut->dependencies[ i ] = NULL;
  }
  SNS_OS_MEMZERO( &algoOut->bufferSizes, sizeof(sns_sam_algo_const_buf_size) );

  algoOut->persistData.bufSize = 0;
  algoOut->persistData.buf = (intptr_t)NULL;
  algoOut->serviceProvider = NULL;

  algoOut->sensor.sensorUID = 0;
  algoOut->sensor.sensorLocation = localProcessor;
  for( i = 0; i < SAM_ALGO_ATTR_CNT; i++ )
  {
    algoOut->sensor.attributes[ i ].attribute = i;
    algoOut->sensor.attributes[ i ].supported = false;
  }

  algoOut->sensor.serviceObj = NULL;
  algoOut->sensor.sensorReq.clientHndl = NULL;
  algoOut->sensor.isAvailable = false;
  algoOut->sensor.isLocal = true;

  sns_q_link( algoOut, (sns_q_link_s*)algoOut );

  *algo = algoOut;

  return SAM_ENONE;
}

/**
 * Create sns_sam_sensor_algo objects for all local SAM algorithm, and adds to
 * global database.
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate sensor_algo objects
 */
STATIC sns_sam_err
sns_sam_record_local_algos( void )
{
  sns_sam_err err;
  uint32_t i;

  SNS_PRINTF_STRING_LOW_0( samModule, "Record local algorithms" );

  for( i = 0; i < samAlgoRegisterFuncsSize; i++ )
  {
    sns_sam_sensor_algo *algo = NULL;
    err = sns_sam_init_algo( &algo );

    if( SAM_ENOMEM == err )
    {
      return SAM_ENOMEM;
    }

    samAlgoRegisterFuncs[ i ](
        &algo->algoAPI,
        &algo->algoMsgAPI,
        &algo->sensor.serviceObj,
        &algo->sensor.sensorUID );

    SNS_PRINTF_STRING_LOW_1( samModule, "Record local algorithm: %x",
      algo->sensor.sensorUID );

    sns_q_put( &sensorQ, (sns_q_link_s*)algo );
  }

  return SAM_ENONE;
}

/**
 * Populate the fields of a local SAM algorithm.  Will involve multiple calls
 * to the algorithm API.  Must be called after all local algorithms are recorded
 * to algorithm list.
 *
 * @param[io] algo Algorithm to populate
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate all necessary objects
 *         SAM_ESTATE Unable to initialize; algorithm will not be available
 */
STATIC sns_sam_err
sns_sam_init_local_algo( sns_sam_sensor_algo *algo )
{
  sns_sam_err errAlgo;
  sns_sam_err errFW;
  uint32_t count;
  sns_sam_reg_group registryGroups[SNS_SAM_MAX_REG_GRP];
  struct sns_sam_algo_dep_sensor depSensors[SNS_SAM_MAX_DEPENDENT_SENSOR];
  uint32_t i;
  bool hasDependency = false;
  bool isUImageAlgo = false;
  int32_t errQMI;
  SNS_PRINTF_STRING_LOW_1( samModule, "sns_sam_init_local_algo: %x",
    algo->sensor.sensorUID );

  count = SNS_SAM_MAX_REG_GRP;
  errAlgo = algo->algoMsgAPI->sns_sam_algo_reg_grps( &count, registryGroups );

  if( SAM_EMAX == errAlgo )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Unable to init algo reg groups %x", algo->sensor.sensorUID );
    return SAM_ESTATE;
  }
  hasDependency |= ( 0 != count );

  for( i = 0; i < count; i++ )
  {
    algo->registryGroups[ i ] = registryGroups[ i ];
  }

  count = SNS_SAM_MAX_DEPENDENT_SENSOR;
  errAlgo = algo->algoMsgAPI->sns_sam_algo_dep_sensors( &count, depSensors );
  if( SAM_EMAX == errAlgo )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Unable to init algo dep sensors %x", algo->sensor.sensorUID );
    return SAM_ESTATE;
  }
  hasDependency |= ( 0 != count );

  for( i = 0; i < count; i++ )
  {
    sns_sam_sensor *sensor =
      sns_sam_lookup_sensor_from_suid( &depSensors[ i ].sensorUID );
    if( NULL == sensor )
    {
      errFW = sns_sam_init_sensor( &sensor );
      sensor->sensorUID = depSensors[ i ].sensorUID;
      sensor->serviceObj = depSensors[ i ].serviceObj;

      if( SAM_ENOMEM == errFW )
      {
        SNS_PRINTF_STRING_ERROR_1( samModule,
          "Not enough memory to init sensor %x", algo->sensor.sensorUID );
        return SAM_ENOMEM;
      }
    }
    algo->dependencies[ i ] = sensor;
  }

  algo->bufferSizes.structSize = sizeof(algo->bufferSizes);
  algo->algoMsgAPI->sns_sam_algo_mem_const_req( &algo->bufferSizes );

  isUImageAlgo = false;

  for (i = 0; i < samUImageAlgoSUIDsSize; i++)
  {
    if(algo->sensor.sensorUID == samUImageAlgoSUIDs[i])
    {
      isUImageAlgo = true;
    }
  }

  if(isUImageAlgo)
  {
    algo->persistData.buf = (intptr_t)SNS_OS_U_MALLOC( SNS_SAM_DBG_MOD,
      algo->bufferSizes.persistDataSize );
  }
  else
  {
    algo->persistData.buf = (intptr_t)SNS_OS_MALLOC( SNS_SAM_DBG_MOD,
      algo->bufferSizes.persistDataSize );
  }
  SNS_ASSERT( (intptr_t)NULL != (intptr_t)algo->persistData.buf );

  SNS_OS_MEMZERO( (void*)algo->persistData.buf, algo->bufferSizes.persistDataSize );

  errQMI = qmi_idl_get_message_c_struct_len( algo->sensor.serviceObj, QMI_IDL_INDICATION,
    SNS_SAM_ALGO_REPORT_IND, &algo->qmiIndSize );

  if( QMI_IDL_LIB_NO_ERR != errQMI )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "QMI length lookup error: %i", errQMI );
    return SAM_ESTATE;
  }


  if( !hasDependency )
  {
    sns_sam_init_dep_met( algo );
  }

  return SAM_ENONE;
}

/**
 * Send the events query request message to SMGR.  Uses the SMGR internal
 * service API.
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate request message
 *         SAM_EFAILED Unable to send request message
 */
STATIC sns_sam_err
sns_sam_smgri_event_query( void )
{
  sns_sam_err err;
  struct sns_sam_msg reqMsg;
  int32_t errQMI;

  SNS_PRINTF_STRING_LOW_0( samModule, "sns_sam_smgri_event_query" );

  reqMsg.msgID = SNS_SMGR_SENSOR_EVENTS_QUERY_REQ_V02;

  errQMI = qmi_idl_get_message_c_struct_len(
    SNS_SMGR_INTERNAL_SVC_get_service_object_v02(),
    QMI_IDL_REQUEST, SNS_SMGR_SENSOR_EVENTS_QUERY_REQ_V02, &reqMsg.msg.bufSize );
  if( QMI_IDL_LIB_NO_ERR == errQMI )
  {
    reqMsg.msg.buf = (intptr_t)SNS_OS_MALLOC( SNS_SAM_DBG_MOD, reqMsg.msg.bufSize );
    SNS_ASSERT(NULL != (void*)reqMsg.msg.buf);
    SNS_OS_MEMZERO( (void*)reqMsg.msg.buf, reqMsg.msg.bufSize );
    reqMsg.sensorUID = NULL;

    err = sns_sam_client_send( &sensorReqSMGRI, &reqMsg );
    if( SAM_ENONE != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
        "Unable to send SMGRI event query %i", err );
      return SAM_EFAILED;
    }
    SNS_SAM_MEM_FREE( (void*)reqMsg.msg.buf );
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Unable to to lookupSMGR all sensor req size %i", errQMI );
    return SAM_EFAILED;
  }

  return SAM_ENONE;
}

/**
 * Checks if an algorithm is presently registered with SMR
 *
 * @param[i] algorithm Local SAM algorithm to check
 *
 * @return true if registered, false otherwise
 */
STATIC bool
sns_sam_algo_is_reg( sns_sam_sensor_algo const *algorithm )
{
  return algorithm->sensor.isAvailable;
}

/**
 * Send the all sensor info request message to SMGR.  Uses the public SMGR
 * service API.
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate request message
 *         SAM_EFAILED Unable to send request message
 */
STATIC sns_sam_err
sns_sam_smgr_req_all( void )
{
  sns_sam_err err = SAM_EFAILED;
  struct sns_sam_msg reqMsg;
  int32_t errQMI;

  SNS_PRINTF_STRING_LOW_0( samModule, "sns_sam_smgr_req_all" );

  errQMI = qmi_idl_get_message_c_struct_len(
    SNS_SMGR_SVC_get_service_object_v01(),
    QMI_IDL_REQUEST, SNS_SMGR_ALL_SENSOR_INFO_REQ_V01, &reqMsg.msg.bufSize );
  if( QMI_IDL_LIB_NO_ERR == errQMI )
  {
    reqMsg.msg.buf = (intptr_t)SNS_OS_MALLOC( SNS_SAM_DBG_MOD, reqMsg.msg.bufSize );
    SNS_ASSERT(NULL != (void*)reqMsg.msg.buf);
    SNS_OS_MEMZERO( (void*)reqMsg.msg.buf, reqMsg.msg.bufSize );
    reqMsg.sensorUID = NULL;
    reqMsg.msgID = SNS_SMGR_ALL_SENSOR_INFO_REQ_V01;

    err = sns_sam_client_send( &sensorReqSMGR, &reqMsg );
    if( SAM_ENONE != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
        "Unable to send SMGR all sensor req %i", err );
      err = SAM_EFAILED;
    }
    SNS_SAM_MEM_FREE( (void*)reqMsg.msg.buf );
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Unable to to lookupSMGR all sensor req size %i", errQMI );
    err = SAM_EFAILED;
  }

  return err;
}

/**
 * Timer callback function for registry request timeout.
 *
 * @param[i] unused
 */
STATIC void sns_sam_init_timer_cb( void *unused )
{
  UNREFERENCED_PARAMETER(unused);
  uint8_t errOS;
  sns_os_sigs_post( sns_sam_sig_event, SNS_SAM_REG_INIT_TIMER_SIG,
      OS_FLAG_SET, &errOS );
  SNS_ASSERT( OS_ERR_NONE == errOS );
}

/**
 * Initializes the registry service connection, and sends the registry group
 * requests for all known local algorithms.
 *
 * @return SAM_ENONE
 *         SAM_EFAILED Unable to send registry read request(s)
 *         SAM_ENOT_AVAILABLE Unable to set registry timer; but request(s) sent
 */
STATIC sns_sam_err
sns_sam_init_reg_client( void )
{
  sns_q_link_s *qCurr;
  sns_sam_err errSAM = SAM_EFAILED;
  sns_err_code_e errEM;
  uint32_t tickOffset;

  for( qCurr = sns_q_check( &sensorQ ); NULL != qCurr;
       qCurr = sns_q_next( &sensorQ, qCurr ) )
  {
    sns_sam_sensor_algo *algo = (sns_sam_sensor_algo*)qCurr;
    uint32_t i;

    if( !algo->sensor.isLocal )
    {
      break;
    }

    for( i = 0; i < SNS_SAM_MAX_REG_GRP && -1 != algo->registryGroups[ i ]; i++ )
    {
      sns_sam_reg_data regData = {
        .groupID = algo->registryGroups[ i ], .groupDataLen = 0 };
      errSAM = sns_sam_reg_req( &regData );

      if( SAM_ENONE != errSAM )
      {
        SNS_PRINTF_STRING_ERROR_1( samModule,
          "Error sending regiatry request %i", errSAM );
        errSAM = SAM_EFAILED;
      }
    }
  }

  if( SAM_ENONE == errSAM )
  {
    errEM = sns_em_create_timer_obj( &sns_sam_init_timer_cb,
        NULL, SNS_EM_TIMER_TYPE_ONESHOT, &registryTimerObj );
    if( SNS_SUCCESS == errEM )
    {
      tickOffset = sns_em_convert_usec_to_localtick(
        SNS_SAM_INIT_REG_TIMEOUT_US );
      errEM = sns_em_register_timer( registryTimerObj, tickOffset );

      if( SNS_SUCCESS != errEM )
      {
        SNS_PRINTF_STRING_ERROR_1( samModule,
          "Error registering timer %i", errEM );
        errSAM = SAM_ENOT_AVAILABLE;
      }
      else
      {
        errSAM = SAM_ENONE;
      }
    }
    else
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
        "Error creating timer %i", errEM );
      errSAM = SAM_ENOT_AVAILABLE;
    }
  }

  return errSAM;
}

/**
 * Sends an attribute request to a SAM service.
 *
 * @param[i] sensor The destination service
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate request message
 *         SAM_EFAILED Unable to send request message
 */
STATIC sns_sam_err
sns_sam_init_send_attr_req( sns_sam_sensor const *sensor  )
{
  sns_sam_err err;
  struct sns_sam_msg reqMsg;
  int32_t errQMI;

  SNS_PRINTF_STRING_LOW_0( samModule, "sns_sam_init_send_attr_req" );

  errQMI = qmi_idl_get_message_c_struct_len( sensor->serviceObj,
      QMI_IDL_REQUEST, SNS_SAM_ALGO_GET_ATTRIB_REQ, &reqMsg.msg.bufSize );

  if( QMI_IDL_LIB_NO_ERR == errQMI )
  {
    reqMsg.msg.buf = (intptr_t)SNS_OS_MALLOC( SNS_SAM_DBG_MOD, reqMsg.msg.bufSize );
    SNS_ASSERT(NULL != (void*)reqMsg.msg.buf);
    SNS_OS_MEMZERO( (void*)reqMsg.msg.buf, reqMsg.msg.bufSize );
    reqMsg.msgID = SNS_SAM_ALGO_GET_ATTRIB_REQ;
    reqMsg.sensorUID = &sensor->sensorUID;

    err = sns_sam_client_send( &sensor->sensorReq, &reqMsg );

    if( SAM_ENONE != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
        "Error sending attr request %i", err );
      err =  SAM_EFAILED;
    }
    SNS_SAM_MEM_FREE( (void*)reqMsg.msg.buf );
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Unable to to lookupSMGR all sensor req size %i", errQMI );
    err =  SAM_EFAILED;
  }

  return err;
}

/**
 * Initializes the client connection to a service SAM wishes to use.
 *
 * @param[i] initData Information about the service that has become available
 */
STATIC void
sns_sam_init_client( struct sns_sam_client_init_data *initData )
{
  sns_sam_err err;
  int32_t errQMI = QMI_IDL_LIB_NO_ERR;
  uint32_t serviceID, serviceIDREG, serviceIDSMGR, serviceIDSMGRI;

  errQMI |= qmi_idl_get_service_id( initData->serviceObj, &serviceID );
  errQMI |= qmi_idl_get_service_id(
    SNS_REG2_SVC_get_service_object_v02(), &serviceIDREG );
  errQMI |= qmi_idl_get_service_id(
    SNS_SMGR_SVC_get_service_object_v01(), &serviceIDSMGR );
  errQMI |= qmi_idl_get_service_id(
    SNS_SMGR_INTERNAL_SVC_get_service_object_v02(), &serviceIDSMGRI );

  if( QMI_IDL_LIB_NO_ERR != errQMI )
  {
    SNS_PRINTF_STRING_ERROR_0( samModule, "Error getting Service ID" );
    return ;
  }

  if( serviceIDREG == serviceID )
  {
    if( initData->timeoutExpired )
    {
      sns_sam_init_timer_cb( NULL );
    }
    else
    {
      err = sns_sam_client_init( &sensorReqREG );
      if( SAM_ENONE != err )
      {
        SNS_PRINTF_STRING_ERROR_1( samModule,
          "Error initializing REG client connection %i", err );
      }
      else
      {
        sns_sam_init_reg_client();
      }
    }
  }
  else if( initData->timeoutExpired )
  {
    SNS_PRINTF_STRING_HIGH_1( samModule, "Timer expired for svc", serviceID );
  }
  else if( serviceIDSMGR == serviceID )
  {
    err = sns_sam_client_init( &sensorReqSMGR );
    if( SAM_ENONE == err )
    {
      err = sns_sam_smgr_req_all();
      if( SAM_ENONE != err )
      {
        SNS_PRINTF_STRING_ERROR_1( samModule,
          "Error sending SMGR all info request %i", err );
      }
    }
    else if( SAM_ENOT_AVAILABLE != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
        "Error initializing SMGR client connection %i", err );
    }
  }
  else if( serviceIDSMGRI == serviceID )
  {
    err = sns_sam_client_init( &sensorReqSMGRI );
    if( SAM_ENONE == err )
    {
      err = sns_sam_smgri_event_query();
      if( SAM_ENONE != err )
      {
        SNS_PRINTF_STRING_ERROR_1( samModule,
          "Error sending SMGRI event query %i", err );
      }
    }
    else if( SAM_ENOT_AVAILABLE != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
        "Error initializing SMGRI client connection %i", err );
    }
  }
  else
  {
    sns_q_link_s *qCurr;
    uint32_t tempID;

    for( qCurr = sns_q_check( &sensorQ ); NULL != qCurr;
         qCurr = sns_q_next( &sensorQ, qCurr ) )
    {
      sns_sam_sensor *sensor = (sns_sam_sensor*)qCurr;

      if( NULL != sensor->sensorReq.clientHndl )
      {
        continue;
      }

      errQMI = qmi_idl_get_service_id( sensor->serviceObj, &tempID );
      if( QMI_IDL_LIB_NO_ERR == errQMI && serviceID == tempID )
      {
        if( SAM_ENONE != sns_sam_client_init( &sensor->sensorReq ) )
        {
          SNS_PRINTF_STRING_ERROR_1( samModule,
            "Error initializing client connection to %i", tempID );
        }
        else
        {
          sns_sam_init_send_attr_req( sensor );
        }
      }
    }
  }
}

/*============================================================================
  Function Definitions
  ===========================================================================*/

void
sns_sam_handle_reg_timeout( void )
{
  sns_q_link_s *qCurr;
  uint32_t i;
  sns_sam_reg_data temp;

  temp.groupDataLen = 0;
  temp.groupData = NULL;

  SNS_PRINTF_STRING_LOW_0( samModule, "sns_sam_handle_reg_timeout" );

  for( qCurr = sns_q_check( &sensorQ ); NULL != qCurr;
       qCurr = sns_q_next( &sensorQ, qCurr ) )
  {
    sns_sam_sensor_algo *algo = (sns_sam_sensor_algo*)qCurr;

    if( !algo->sensor.isLocal )
    {
      break;
    }
    else if( sns_sam_algo_is_reg( algo ) )
    {
      continue;
    }

    for( i = 0;
         i < SNS_SAM_MAX_REG_GRP && -1 != algo->registryGroups[ i ]; i++ )
    {
      temp.groupID = algo->registryGroups[ i ];
      SNS_PRINTF_STRING_LOW_1( samModule, "Alerting algorithm %x",
        algo->sensor.sensorUID );
      algo->algoMsgAPI->sns_sam_algo_dep_registry_met( algo,
          sns_sam_init_dep_met, &algo->persistData, &temp );
    }
  }

  if( NULL != registryTimerObj )
  {
    sns_em_delete_timer_obj( registryTimerObj );
    registryTimerObj = NULL;
  }
}

void
sns_sam_init_dep_met( sns_sam_sensor_algo const *algo )
{
  bool registered;
  sns_sam_err err;

  SNS_PRINTF_STRING_MEDIUM_1( samModule,
    "sns_sam_init_dep_met %x", algo->sensor.sensorUID );

  registered = sns_sam_algo_is_reg( algo );
  if( !registered )
  {
    algo->algoMsgAPI->sns_sam_algo_get_attr( &algo->persistData,
      ((sns_sam_sensor_algo*)algo)->sensor.attributes );

  if( !algo->sensor.attributes [SAM_ALGO_ATTR_MAX_BATCH].supported &&
      algo->sensor.attributes [SAM_ALGO_ATTR_MAX_REPORTS_PER_IND].supported &&
      1 < algo->sensor.attributes [SAM_ALGO_ATTR_MAX_REPORTS_PER_IND].attrValue &&
       NULL != algo->algoMsgAPI->sns_sam_algo_gen_batch_ind )
  {
    ((sns_sam_sensor_algo*)algo)->sensor.attributes [SAM_ALGO_ATTR_MAX_BATCH].supported = true;

    ((sns_sam_sensor_algo*)algo)->sensor.attributes [SAM_ALGO_ATTR_MAX_BATCH].attrValue =
       SNS_SAM_MAX_BATCHING_TIME * FX_CONV((int32_t)algo->sensor.attributes[SAM_ALGO_ATTR_MAX_REPORT].attrValue, FX_QFACTOR, 0);
  }

    err = sns_sam_service_reg( (sns_sam_sensor_algo*)algo );
    if( SAM_ENONE != err )
    {
      SNS_PRINTF_STRING_ERROR_2( samModule,
        "Error registering service %x, error %i",
        algo->sensor.sensorUID, err );
    }
  }
}

sns_sam_err
sns_sam_init_algos( void )
{
  sns_sam_err err = SAM_ENONE;
  sns_q_link_s *qCurr;

  err = sns_sam_record_local_algos();
  if( SAM_ENONE != err )
  {
    SNS_PRINTF_STRING_ERROR_0( samModule, "Not enough memory to init algos" );
    return SAM_EFAILED;
  }

  err = sns_sam_client_check( SNS_SMGR_SVC_get_service_object_v01(),
      SMR_CLIENT_INSTANCE_ANY, SNS_SAM_INIT_CLIENT_TIMEOUT_US );
  if( SAM_ENONE != err )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule, "smr check failed %i", err );
    return SAM_EFAILED;
  }

  err = sns_sam_client_check( SNS_SMGR_INTERNAL_SVC_get_service_object_v02(),
      SMR_CLIENT_INSTANCE_ANY, SNS_SAM_INIT_CLIENT_TIMEOUT_US );
  if( SAM_ENONE != err )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule, "smr check failed %i", err );
    return SAM_EFAILED;
  }

  for( qCurr = sns_q_check( &sensorQ ); NULL != qCurr;
       qCurr = sns_q_next( &sensorQ, qCurr ) )
  {
    sns_sam_sensor_algo *algo = (sns_sam_sensor_algo*)qCurr;
    uint32_t i;

    if( !algo->sensor.isLocal )
    {
      continue;
    }

    err = sns_sam_init_local_algo( algo );
    if( SAM_ENONE != err )
    {
      SNS_PRINTF_STRING_ERROR_2( samModule, "Unable to init algo %x (%i)",
        algo->sensor.sensorUID, err );
    }
    else
    {
      for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
      {
        if( NULL != algo->dependencies[ i ] &&
            NULL != algo->dependencies[ i ]->serviceObj )
        {
          err = sns_sam_client_check( algo->dependencies[ i ]->serviceObj,
              SMR_CLIENT_INSTANCE_ANY, SNS_SAM_INIT_CLIENT_TIMEOUT_US );

          if( SAM_ENONE != err )
          {
            SNS_PRINTF_STRING_ERROR_1( samModule,
              "Error in client_check %i", err );
          }
        }
      }
    }
  }

  err = sns_sam_client_check( SNS_REG2_SVC_get_service_object_v02(),
      SMR_CLIENT_INSTANCE_ANY, SNS_SAM_INIT_CLIENT_TIMEOUT_US );
  if( SAM_ENONE != err )
  {
    return SAM_EFAILED;
  }

  return SAM_ENONE;
}

void
sns_sam_handle_client_init_cb( sns_sam_client_init_data *initData )
{
  uint8_t errOS;

  sns_os_mutex_pend( samClientInitQMutex, 0, &errOS );
  sns_q_put( &samClientInitQ, (sns_q_link_s*)initData );
  sns_os_mutex_post( samClientInitQMutex );

  sns_os_sigs_post( sns_sam_sig_event,
    SNS_SAM_CLIENT_INIT_SIG, OS_FLAG_SET, &errOS );
}

void
sns_sam_init_clients( void )
{
  struct sns_sam_client_init_data *initData = NULL;
  uint8_t errOS;

  SNS_PRINTF_STRING_LOW_0( samModule, "sns_sam_init_clients" );

  do
  {
    sns_os_mutex_pend( samClientInitQMutex, 0, &errOS );
    initData = sns_q_get( &samClientInitQ );
    sns_os_mutex_post( samClientInitQMutex );

    if( NULL != initData )
    {
      sns_sam_init_client( initData );
      SNS_SAM_MEM_FREE( initData );
    }
  } while( NULL != initData );
}

sns_sam_err
sns_sam_init_fw( void )
{
  sns_sam_err err;
  const uint8_t priority0 = 0;
  uint8_t errOS = 0;

  SNS_PRINTF_STRING_LOW_0( samModule, "Initializaing SAM Framework" );

  samClientInitQMutex = sns_os_mutex_create( priority0, &errOS );
  if( 0 != errOS )
  {
    SNS_PRINTF_STRING_FATAL_1( samModule, "Cannot create mutex %i", errOS );
    return SAM_EFAILED;
  }

  sns_q_init( &samClientInitQ );
  sns_q_init( &sensorQ );
  sns_q_init( &algoInstQ );

  err = sns_sam_ind_init_fw();
  if( SAM_ENONE != err )
  {
    SNS_PRINTF_STRING_FATAL_1( samModule, "Error initializing FW %i", err );
    return SAM_EFAILED;
  }

  err = sns_sam_client_init_fw();
  if( SAM_ENONE != err )
  {
    SNS_PRINTF_STRING_FATAL_1( samModule, "Error initializing FW %i", err );
    return SAM_EFAILED;
  }

  err = sns_sam_service_init_fw();
  if( SAM_ENONE != err )
  {
    SNS_PRINTF_STRING_FATAL_1( samModule, "Error initializing FW %i", err );
    return SAM_EFAILED;
  }

  return SAM_ENONE;
}
