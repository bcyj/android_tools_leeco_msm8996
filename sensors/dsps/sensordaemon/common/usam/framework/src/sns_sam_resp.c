/*============================================================================
  @file sns_sam_resp.c

  @brief
  All response processing (incoming only) for the SAM Framework.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include "sns_smgr_api_v01.h"
#include "sns_sam_ped_v01.h"
#include "sns_smgr_internal_api_v02.h"
#include "sns_smgr_common_v01.h"
#include "sns_sam_common_v01.h"
#include "sns_reg_api_v02.h"
#include "sns_init.h"
#include "sns_debug_str.h"
#include "sns_sam.h"
#include "sns_sam_algo_api.h"
#include "sns_sam_req.h"
#include "sns_sam_init.h"
#include "sns_sam_client.h"
#include "sns_sam_service.h"
#include "sns_sam_resp.h"
#include "sns_sam_reg.h"
#include "sns_sam_pm.h"
#include "sns_sam_memmgr.h"
#include "sns_sam_dep.h"
#include "sns_sam_algo_utils.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

/*============================================================================
  Static Function Definitions
  ============================================================================*/

/**
 * Form and send a SAM error indication message to a particular client.
 *
 * @param[i] sensor1Err The Sensor1 API error code
 * @param[i] clientReq SAM client which deserves the error message
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate indication
 *         SAM_EFAILED Unable to send message to client
 */
static sns_sam_err
sns_sam_send_error_ind( sensor1_error_e sensor1Err, sam_client_req const *clientReq )
{
  sns_sam_err err;
  sns_sam_ind_msg indMsg;
  int32_t errQMI;

  errQMI = qmi_idl_get_message_c_struct_len(
    clientReq->algorithm->sensor.serviceObj, QMI_IDL_INDICATION,
    SNS_SAM_ALGO_ERROR_IND, &indMsg.msg.bufSize  );
  if( QMI_IDL_LIB_NO_ERR == errQMI )
  {
    indMsg.msg.buf = (intptr_t)SNS_OS_MALLOC( SNS_SAM_DBG_MOD, indMsg.msg.bufSize );
    SNS_ASSERT(NULL != (void*)indMsg.msg.buf);
    SNS_OS_MEMZERO( (void*)indMsg.msg.buf, indMsg.msg.bufSize );
    indMsg.msgID = SNS_SAM_ALGO_ERROR_IND;
    indMsg.sensorUID = &clientReq->algorithm->sensor.sensorUID;

    sns_sam_ped_error_ind_msg_v01 *errorInd
      = (sns_sam_ped_error_ind_msg_v01*)indMsg.msg.buf;

    errorInd->instance_id = clientReq->extInstanceID;
    errorInd->error = sensor1Err;

    err = sns_sam_service_send_ind( &indMsg, clientReq );

    if( SAM_ENONE != err )
    {
      SNS_PRINTF_STRING_ERROR_2( samModule,
        "Error sending error ind for %x, error: %i",
        clientReq->algorithm->sensor.sensorUID, err );
      return SAM_EFAILED;
    }
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_2( samModule,
      "Error getting struct size for %x, error: %i",
      clientReq->algorithm->sensor.sensorUID, errQMI );
    return SAM_EFAILED;
  }

  return SAM_ENONE;
}

/**
 * Process an enable response for a particular algorithm instance.  Multiple
 * instances may be associated with a particular dependent stream.
 *
 * If the enable request failed, we wish to cancel the associated algorithm
 * instances, which in turn means removing their client requests.
 *
 * @param[i] sensorReq Sensor request which failed
 * @param[i] errCode Error code to place in the error indication
 */
STATIC void
sns_sam_handle_enable_resp_err( sns_sam_sensor_req const *sensorReq,
  uint8_t errCode )
{
  sns_sam_err err = SAM_ENONE;
  sns_q_link_s *qCurrAI, *qCurrCR;
  sam_client_req *clientReq;
  uint32_t i;

  for( qCurrAI = sns_q_check( &algoInstQ ); NULL != qCurrAI; )
  {
    sns_sam_algo_inst *algoInst = (sns_sam_algo_inst*)qCurrAI;
    qCurrAI = sns_q_next( &algoInstQ, qCurrAI );

    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      if( NULL != algoInst->sensorRequests[ i ] &&
          sensorReq == algoInst->sensorRequests[ i ] )
      {
        qCurrCR = sns_q_check( &algoInst->clientRequests );
        while( NULL != qCurrCR )
        {
          clientReq = (sam_client_req*)qCurrCR;
          qCurrCR = sns_q_next( &algoInst->clientRequests, qCurrCR );

          err = sns_sam_send_error_ind( errCode, clientReq );
          if( SAM_ENONE != err )
          {
            SNS_PRINTF_STRING_ERROR_2( samModule,
              "Unable to send error ind for %x, error: %i",
              algoInst->algorithm->sensor.sensorUID, err );
          }

          sns_sam_remove_client_req( clientReq, NULL );
        }
      }
    }
  }
}

/**
 * Handle an enable response message received from some SAM algorithm.
 *
 * @param[i] respMsg Response message to process
 */
void
sns_sam_handle_sam_enable_resp( sns_sam_resp const *respMsg )
{
  sns_sam_ped_enable_resp_msg_v01 *resp =
    (sns_sam_ped_enable_resp_msg_v01*)respMsg->msg.msg.buf;

  if( SNS_RESULT_SUCCESS_V01 != resp->resp.sns_result_t )
  {
    SNS_PRINTF_STRING_ERROR_3( samModule,
        "Received error response: %i, suid: %x msg: %i",
        resp->resp.sns_err_t, respMsg->msg.sensorUID, respMsg->msg.msgID );
    sns_sam_handle_enable_resp_err( respMsg->sensorReq, resp->resp.sns_err_t );
  }
  else
  {
    sns_sam_sensor_req *sensorReq = (sns_sam_sensor_req*)respMsg->sensorReq;
    // AMD response message is smaller than all others
    sensorReq->instanceID =
      ( sizeof(sns_sam_ped_enable_resp_msg_v01) > respMsg->msg.msg.bufSize )
        ? resp->instance_id_valid : resp->instance_id;

    SNS_PRINTF_STRING_LOW_2( samModule,
        "Update instance ID for: %i, to: %x",
        sensorReq->instanceID,
        sensorReq->sensor->sensorUID );
  }
}

/**
 * Handle a disable response message received from another SAM algorithm.
 *
 * @param[i] respMsg Response message to process
 */
void
sns_sam_handle_sam_disable_resp( sns_sam_resp const *respMsg )
{
  sns_sam_ped_disable_resp_msg_v01 *resp =
    (sns_sam_ped_disable_resp_msg_v01*)respMsg->msg.msg.buf;

  if( SNS_RESULT_SUCCESS_V01 != resp->resp.sns_result_t )
  {
    SNS_PRINTF_STRING_ERROR_3( samModule,
        "Received disable error response: %i, suid: %x msg: %i",
        resp->resp.sns_err_t, respMsg->msg.sensorUID, respMsg->msg.msgID );
  }
}

/**
 * Handle an attribute response message received from another SAM service
 *
 * @param[i] respMsg Response message to process
 */
void
sns_sam_handle_sam_batch_resp( sns_sam_resp const *respMsg )
{
  sns_sam_ped_batch_resp_msg_v01 *resp =
    (sns_sam_ped_batch_resp_msg_v01*)respMsg->msg.msg.buf;

  if( SNS_RESULT_SUCCESS_V01 != resp->resp.sns_result_t )
  {
    SNS_PRINTF_STRING_ERROR_3( samModule,
        "Received batch error response: %i, suid: %x msg: %i",
        resp->resp.sns_err_t, respMsg->msg.sensorUID, respMsg->msg.msgID );
  }
}

/**
 * Handle an attribute response message received from another SAM service
 *
 * @param[i] respMsg Response message to process
 */
void
sns_sam_handle_sam_update_batch_resp( sns_sam_resp const *respMsg )
{
  sns_sam_ped_update_batch_period_resp_msg_v01 *resp =
    (sns_sam_ped_update_batch_period_resp_msg_v01*)respMsg->msg.msg.buf;

  if( SNS_RESULT_SUCCESS_V01 != resp->resp.sns_result_t )
  {
    SNS_PRINTF_STRING_ERROR_3( samModule,
        "Received batch update error response: %i, suid: %x msg: %i",
        resp->resp.sns_err_t, respMsg->msg.sensorUID, respMsg->msg.msgID );
  }
}

/**
 * Update all applicable algorithms that a new dependent sensor has become
 * available.
 *
 * @param[i] sensor The new sensor that is available for use
 */
static void
sns_sam_dep_available( sns_sam_sensor const *sensor )
{
  sns_q_link_s *qCurr;
  uint32_t i;

  SNS_PRINTF_STRING_LOW_1( samModule, "New sensor available %x",
    sensor->sensorUID );

  for( qCurr = sns_q_check( &sensorQ ); NULL != qCurr;
       qCurr = sns_q_next( &sensorQ, qCurr ) )
  {
    sns_sam_sensor_algo *algo = (sns_sam_sensor_algo*)qCurr;
    if( !algo->sensor.isLocal )
    {
      break;
    }
    for( i = 0; i < SNS_SAM_MAX_DEPENDENT_SENSOR; i++ )
    {
      if( NULL != algo->dependencies[ i ] &&
          sensor->sensorUID == algo->dependencies[ i ]->sensorUID )
      {
        SNS_PRINTF_STRING_MEDIUM_2( samModule, "Alerting algo %x of dependency %x",
          algo->sensor.sensorUID, sensor->sensorUID );
        algo->algoMsgAPI->sns_sam_algo_dep_sensors_met( algo,
            sns_sam_init_dep_met, &algo->persistData,
            &sensor->sensorUID, &sensor->attributes );
      }
    }
  }
}

/**
 * Generates gated sensors that are a combination of a streaming sensor and an
 * event that an algorithm has previously requested. An algorithm can request
 * for such a sensor by providing SAM with the bitwise XOR of the event sensor's
 * SUID and the streaming sensor's SUID. The  resulting sensor will inherit the
 * attributes of the streaming sensor, and the SMR client handle, QMI service object,
 * location and processor details from the event sensors. Messages sent to this sensor
 * will be directed to the parent event sensor.
 *
 * @param[i] parentSensor  An event sensor or a streaming sensor that is needed for the
 *                         gated sensor.
 *
*/
static void
sns_sam_gen_gated_sensors( sns_sam_sensor const *parentSensor)
{
  sns_q_link_s *qCurr;

  SNS_PRINTF_STRING_HIGH_1(samModule, "Check for gated Sensor: %x", parentSensor->sensorUID);

  for( qCurr = sns_q_check( &sensorQ ); NULL != qCurr;
       qCurr = sns_q_next( &sensorQ, qCurr ) )
  {
    sns_sam_sensor *gatingSensor = (sns_sam_sensor*)qCurr;

    if( (SNS_SAM_SENSOR_STREAM == parentSensor->gatingType) && (SNS_SAM_SENSOR_EVENT == gatingSensor->gatingType))
    {
      sns_sam_sensor_uid suid = ( parentSensor->sensorUID ^ gatingSensor->sensorUID);
      sns_sam_sensor *gatedSensor = sns_sam_lookup_sensor_from_suid( &suid );
      if(gatedSensor != NULL)
      {
        uint8_t i;
        SNS_PRINTF_STRING_HIGH_3(samModule, "An algorithm wants gated Sensor: %x with parents: %x and %x",
          gatedSensor->sensorUID, parentSensor->sensorUID, gatingSensor->sensorUID);

        for(i = 0; i < SAM_ALGO_ATTR_CNT ; i++)
        {
           gatedSensor->attributes[i].attribute = parentSensor->attributes[i].attribute;
           gatedSensor->attributes[i].attrValue = parentSensor->attributes[i].attrValue;
           gatedSensor->attributes[i].supported = parentSensor->attributes[i].supported;
        }

        gatedSensor->sensorLocation = gatingSensor->sensorLocation;
        gatedSensor->sensorReq.clientHndl = gatingSensor->sensorReq.clientHndl;
        gatedSensor->serviceObj = gatingSensor->serviceObj;
        gatedSensor->gatingType = SNS_SAM_SENSOR_GATED_STREAM;
        gatedSensor->sensorUID = suid;
        gatedSensor->isAvailable = true;
        sns_sam_dep_available( gatedSensor );
      }
    }
    else if( (SNS_SAM_SENSOR_EVENT == parentSensor->gatingType) && (SNS_SAM_SENSOR_STREAM == gatingSensor->gatingType))
    {
      sns_sam_sensor_uid suid = ( parentSensor->sensorUID ^ gatingSensor->sensorUID);
      sns_sam_sensor *gatedSensor = sns_sam_lookup_sensor_from_suid( &suid );
      if(gatedSensor != NULL)
      {
        uint8_t i;
        SNS_PRINTF_STRING_HIGH_3(samModule, "An algorithm wants gated Sensor: %x with parents: %x and %x",
          gatedSensor->sensorUID, parentSensor->sensorUID, gatingSensor->sensorUID);

        for(i = 0; i < SAM_ALGO_ATTR_CNT ; i++)
        {
           gatedSensor->attributes[i].attribute = gatingSensor->attributes[i].attribute;
           gatedSensor->attributes[i].attrValue = gatingSensor->attributes[i].attrValue;
           gatedSensor->attributes[i].supported = gatingSensor->attributes[i].supported;
        }

        gatedSensor->sensorLocation = parentSensor->sensorLocation;
        gatedSensor->sensorReq.clientHndl = parentSensor->sensorReq.clientHndl;
        gatedSensor->serviceObj = parentSensor->serviceObj;
        gatedSensor->gatingType = SNS_SAM_SENSOR_GATED_STREAM;
        gatedSensor->sensorUID = suid;
        gatedSensor->isAvailable = true;
        sns_sam_dep_available( gatedSensor );
      }
    }
  }
}

static sns_sam_sensor *
sns_sam_lookup_gating_event_sensor(sns_sam_sensor_uid const * sensorUID)
{
  sns_q_link_s *qCurr;
  sns_sam_sensor * gatedSensor = sns_sam_lookup_sensor_from_suid(sensorUID);
  if(gatedSensor == NULL ||
     SNS_SAM_SENSOR_GATED_STREAM != gatedSensor->gatingType )
  {
    return NULL;
  }

  for( qCurr = sns_q_check( &sensorQ ); NULL != qCurr;
       qCurr = sns_q_next( &sensorQ, qCurr ) )
  {
    sns_sam_sensor *eventSensor = (sns_sam_sensor*)qCurr;
    sns_sam_sensor_uid suid = eventSensor->sensorUID ^ *sensorUID;
    if( SNS_SAM_SENSOR_EVENT == eventSensor->gatingType &&
        NULL != sns_sam_lookup_sensor_from_suid( &suid ))
    {
      return eventSensor;
    }
  }
  return NULL;
}

/**
 * Handle an attribute response message received from another SAM service
 *
 * @param[i] respMsg Response message to process
 */
static void
sns_sam_handle_sam_attr_resp( sns_sam_resp const *respMsg )
{
  sns_sam_get_algo_attrib_resp_msg_v01 *resp =
    (sns_sam_get_algo_attrib_resp_msg_v01*)respMsg->msg.msg.buf;

  if( SNS_RESULT_SUCCESS_V01 == resp->resp.sns_result_t )
  {
    sns_sam_sensor_uid sensorUID = resp->sensorUID_valid
      ? resp->sensorUID : *respMsg->msg.sensorUID;
    sns_sam_sensor *sensor;

    sensor = sns_sam_lookup_sensor_from_suid( &sensorUID );
    if( NULL == sensor )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule, "Sensor not found %x",
        sensorUID );
    }
    else
    {
      sensor->isAvailable = true;
      sensor->attributes[ SAM_ALGO_ATTR_POWER ].attrValue =
        resp->power;
      sensor->attributes[ SAM_ALGO_ATTR_POWER ].supported = true;

      sensor->attributes[ SAM_ALGO_ATTR_MIN_REPORT ].supported = true;
      sensor->attributes[ SAM_ALGO_ATTR_MIN_REPORT ].attrValue =
        resp->min_report_rate;

      sensor->attributes[ SAM_ALGO_ATTR_MAX_REPORT ].attrValue =
        resp->max_report_rate;
      sensor->attributes[ SAM_ALGO_ATTR_MAX_REPORT ].supported = true;

      sensor->attributes[ SAM_ALGO_ATTR_MIN_SAMPLE ].supported = true;
      sensor->attributes[ SAM_ALGO_ATTR_MIN_SAMPLE ].attrValue =
        resp->min_sample_rate;

      sensor->attributes[ SAM_ALGO_ATTR_MAX_SAMPLE ].attrValue =
        resp->max_sample_rate;
      sensor->attributes[ SAM_ALGO_ATTR_MAX_SAMPLE ].supported = true;

      sensor->attributes[ SAM_ALGO_ATTR_REVISION ].attrValue =
        resp->algorithm_revision;
      sensor->attributes[ SAM_ALGO_ATTR_REVISION ].supported = true;

      sensor->attributes[ SAM_ALGO_ATTR_MAX_BATCH ].attrValue =
        resp->max_batch_size;
      sensor->attributes[ SAM_ALGO_ATTR_MAX_BATCH ].supported = true;

      sensor->sensorLocation = resp->proc_type;

      sns_sam_dep_available( sensor );
    }
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_2( samModule,
        "Error in SAM attribute response %i %i",
        resp->resp.sns_result_t, resp->resp.sns_err_t );
  }
}

/**
 * Handle an response message from another SAM service.
 *
 * @param[i] respMsg Response message to process
 *
 * @note Message buffer will be freed by caller
 *
 * @return SAM_ENONE
 *         SAM_ENOT_SUPPORTED Unknown message ID
 */
sns_sam_err
sns_sam_handle_sam_resp( sns_sam_resp const *respMsg )
{
  SNS_PRINTF_STRING_LOW_1( samModule,
    "sns_sam_handle_sam_resp %i", respMsg->msg.msgID );

  if( SNS_SAM_ALGO_ENABLE_RESP == respMsg->msg.msgID )
  {
    sns_sam_handle_sam_enable_resp( respMsg );
  }
  else if( SNS_SAM_ALGO_DISABLE_RESP == respMsg->msg.msgID )
  {
    sns_sam_handle_sam_disable_resp( respMsg );
  }
  else if( SNS_SAM_ALGO_BATCH_RESP == respMsg->msg.msgID )
  {
    sns_sam_handle_sam_batch_resp( respMsg );
  }
  else if( SNS_SAM_ALGO_UPDATE_BATCH_PERIOD_RESP == respMsg->msg.msgID )
  {
    sns_sam_handle_sam_update_batch_resp( respMsg );
  }
  else if( SNS_SAM_ALGO_GET_ATTRIB_RESP == respMsg->msg.msgID )
  {
    sns_sam_handle_sam_attr_resp( respMsg );
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_2( samModule,
        "Received unknown SAM message type; sensor: %x, msg: %i",
        respMsg->msg.sensorUID, respMsg->msg.msgID );
    return SAM_ENOT_SUPPORTED;
  }

  return SAM_ENONE;
}

/**
 * Process a single sensor info response message from SMGR.
 *
 * @param[i] respMsg Message to be processed
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM No memory to allocate Sensor object
 *         SAM_ESTATE Received error response message
 */
static sns_sam_err
sns_sam_smgr_handle_single_resp( sns_sam_resp const *respMsg )
{
  sns_q_link_s *qCurr;
  sns_smgr_single_sensor_info_resp_msg_v01 *resp;
  sns_sam_sensor *sensor = NULL;
  uint32_t i;
  sns_sam_err err = SAM_ENONE;

  resp = (sns_smgr_single_sensor_info_resp_msg_v01*)respMsg->msg.msg.buf;

  if( SNS_RESULT_SUCCESS_V01 == resp->Resp.sns_result_t )
  {
    for( i = 0; i < resp->SensorInfo.data_type_info_len; i++ )
    {
      for( qCurr = sns_q_check( &sensorQ ); NULL != qCurr;
          qCurr = sns_q_next( &sensorQ, qCurr ) )
      {
        sensor = (sns_sam_sensor*)qCurr;
        if( sensor->sensorUID == resp->suid[ i ] )
        {
          break;
        }
      }

      if( NULL == qCurr )
      {
        err = sns_sam_init_sensor( &sensor );
        if( SAM_ENONE != err )
        {
          SNS_PRINTF_STRING_ERROR_2( samModule,
            "Unable to initialize new sensor for %x, msg: %i",
            respMsg->msg.sensorUID, respMsg->msg.msgID );
          continue;
        }
      }

      SNS_PRINTF_STRING_LOW_3( samModule,
          "Created new sensor; SMGR ID %i, type %i, UID %x",
          resp->SensorInfo.data_type_info[ i ].SensorID, i, resp->suid[ i ] );

      sensor->sensorUID = resp->suid_valid
        ? resp->suid[ i ]
        : sns_sam_get_sensor_uid( resp->SensorInfo.data_type_info[ i ].SensorID );
      sensor->serviceObj = SNS_SMGR_SVC_get_service_object_v01();
      sensor->sensorReq = *respMsg->sensorReq;
      sensor->isAvailable = true;
      sensor->attributes[ SAM_ALGO_ATTR_POWER ].attrValue
        = resp->SensorInfo.data_type_info[ i ].MaxPower;
      sensor->attributes[ SAM_ALGO_ATTR_POWER ].supported = true;

      sensor->attributes[ SAM_ALGO_ATTR_MIN_REPORT ].supported = false;

      sensor->attributes[ SAM_ALGO_ATTR_MAX_REPORT ].attrValue =
        resp->SensorInfo.data_type_info[ i ].MaxSampleRate;
      sensor->attributes[ SAM_ALGO_ATTR_MAX_REPORT ].supported = true;

      sensor->attributes[ SAM_ALGO_ATTR_MIN_SAMPLE ].supported = false;

      sensor->attributes[ SAM_ALGO_ATTR_MAX_SAMPLE ].attrValue =
        FX_FLTTOFIX_Q16(resp->SensorInfo.data_type_info[ i ].MaxSampleRate);
      sensor->attributes[ SAM_ALGO_ATTR_MAX_SAMPLE ].supported = true;

      sensor->attributes[ SAM_ALGO_ATTR_REVISION ].attrValue =
        resp->SensorInfo.data_type_info[ i ].Version;
      sensor->attributes[ SAM_ALGO_ATTR_REVISION ].supported = true;

      if( resp->num_buffered_reports_valid &&
          i < resp->num_buffered_reports_len )
      {
        sensor->attributes[ SAM_ALGO_ATTR_MAX_BATCH ].attrValue =
          resp->num_buffered_reports[ i ];
        sensor->attributes[ SAM_ALGO_ATTR_MAX_BATCH ].supported = true;
      }
      sensor->gatingType = SNS_SAM_SENSOR_STREAM;
      sns_sam_dep_available( sensor );

      sns_sam_gen_gated_sensors( sensor );
    }

  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
        "Error in SMGR single info resp: %i",
        resp->Resp.sns_err_t );
    err = SAM_ESTATE;
  }

  return err;
}

/**
 * Send a single sensor info request to SMGR.
 *
 * @param[i] sensorID Sensor found in the all sensor info response message
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM No memory to send single sensor info requests
 *         SAM_EFAILED Unable to send request message
 */
static sns_sam_err
sns_sam_send_smgr_single_req( uint8_t sensorID )
{
  sns_sam_err err;
  struct sns_sam_msg reqMsg;
  int32_t errQMI;

  errQMI = qmi_idl_get_message_c_struct_len(
    SNS_SMGR_SVC_get_service_object_v01(),
    QMI_IDL_REQUEST, SNS_SMGR_SINGLE_SENSOR_INFO_REQ_V01,
    &reqMsg.msg.bufSize );
  if( QMI_IDL_LIB_NO_ERR != errQMI )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "QMI length lookup error: %i", errQMI );
    err = SAM_EFAILED;
  }
  else
  {
    reqMsg.msg.buf = (intptr_t)SNS_OS_MALLOC( SNS_SAM_DBG_MOD, reqMsg.msg.bufSize );
    SNS_ASSERT(NULL != (void*)reqMsg.msg.buf);
    SNS_OS_MEMZERO( (void*)reqMsg.msg.buf, reqMsg.msg.bufSize );
    reqMsg.sensorUID = NULL;
    reqMsg.msgID = SNS_SMGR_SINGLE_SENSOR_INFO_REQ_V01;

    sns_smgr_single_sensor_info_req_msg_v01 *regReqMsg =
      (sns_smgr_single_sensor_info_req_msg_v01*)reqMsg.msg.buf;
    regReqMsg->SensorID = sensorID;

    err = sns_sam_client_send( &sensorReqSMGR, &reqMsg );
    if( SAM_ENONE != err )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
        "Error sending request message %i", err );
      err = SAM_EFAILED;
    }
    SNS_SAM_MEM_FREE( (void*)reqMsg.msg.buf );
  }

  return err;
}

/**
 * Process the all sensor info response message from SMGR.
 *
 * @param[i] respMsg Message to be processed
 *
 * @return SAM_ENONE
 *         SAM_EFAILED Unable to send request message
 *         SAM_ENOMEM No memory to send single sensor info requests
 *         SAM_ESTATE Received error response message
 */
static sns_sam_err
sns_sam_smgr_handle_all_resp( sns_sam_resp const *respMsg )
{
  sns_sam_err err = SAM_ENONE;
  uint32_t i;
  sns_smgr_all_sensor_info_resp_msg_v01 *resp =
    (sns_smgr_all_sensor_info_resp_msg_v01*)respMsg->msg.msg.buf;

  if( SNS_RESULT_SUCCESS_V01 == resp->Resp.sns_result_t )
  {
    for( i = 0; i < resp->SensorInfo_len; i++ )
    {
      err = sns_sam_send_smgr_single_req( resp->SensorInfo[ i ].SensorID );

      if( SAM_ENONE != err )
      {
        SNS_PRINTF_STRING_ERROR_2( samModule,
          "Unable to send SMGR single sensor info req for sensor %i (%i)",
          resp->SensorInfo[ i ].SensorID, err );
        err = SAM_ENOMEM == err ? SAM_ENOMEM : SAM_EFAILED;
      }
    }
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
        "Error in SMGR all sensor info response: %i",
        resp->Resp.sns_err_t );
    err = SAM_ESTATE;
  }

  return err;
}

/**
 * Handle a buffered response message from SMGR.
 *
 * @param[i] respMsg Message to be processed
 *
 * @return SAM_ENONE
 *         SAM_ESTATE Received error response message
 */
sns_sam_err
sns_sam_smgr_handle_enable_resp( sns_sam_resp const *respMsg )
{
  sns_sam_err rv = SAM_ENONE;
  sns_smgr_buffering_resp_msg_v01 *resp =
    (sns_smgr_buffering_resp_msg_v01*)respMsg->msg.msg.buf;

  if( SNS_RESULT_SUCCESS_V01 != resp->Resp.sns_result_t )
  {
    SNS_PRINTF_STRING_ERROR_3( samModule,
        "Received SMGR error response: %i, suid: %x msg: %i",
        resp->Resp.sns_err_t, *respMsg->msg.sensorUID, respMsg->msg.msgID );
    SNS_PRINTF_STRING_ERROR_2( samModule,
        "AckNak %i, reason: %i", resp->AckNak, resp->ReasonPair[0].Reason );

    sns_sam_handle_enable_resp_err( respMsg->sensorReq, resp->Resp.sns_err_t );
    rv = SAM_ENONE;
  }
  else
  {
    SNS_PRINTF_STRING_LOW_2( samModule,
        "Received SMGR success response; suid: %x msg: %i",
        *respMsg->msg.sensorUID, respMsg->msg.msgID );
  }

  return rv;
}

/**
 * Handle an response message from SMGR.
 *
 * @param[i] respMsg Response message to process
 *
 * @note Message buffer will be freed by caller
 */
void
sns_sam_handle_smgr_resp( sns_sam_resp const *respMsg )
{
  sns_sam_err err = SAM_EFAILED;

  SNS_PRINTF_STRING_LOW_1( samModule,
    "sns_sam_handle_smgr_resp %i", respMsg->msg.msgID );

  if( SNS_SMGR_SINGLE_SENSOR_INFO_RESP_V01 == respMsg->msg.msgID )
  {
    err = sns_sam_smgr_handle_single_resp( respMsg );
  }
  else if( SNS_SMGR_ALL_SENSOR_INFO_RESP_V01 == respMsg->msg.msgID )
  {
    err = sns_sam_smgr_handle_all_resp( respMsg );
  }
  else if( SNS_SMGR_BUFFERING_RESP_V01 == respMsg->msg.msgID ||
           SNS_SMGR_REPORT_RESP_V01 == respMsg->msg.msgID )
  {
    err = sns_sam_smgr_handle_enable_resp( respMsg );
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "Unknown SMGR message ID: %i", respMsg->msg.msgID );
  }

  if( SAM_ENONE != err )
  {
    SNS_PRINTF_STRING_ERROR_2( samModule,
      "Error processing resp msg: %i, error %i",
      respMsg->msg.msgID, err );
  }
}

/**
 * Process the events query response message from SMGR.
 *
 * @param[i] respMsg Message to be processed
 *
 * @return SAM_ENONE
 *         SAM_ESTATE Received error response message
 */
static sns_sam_err
sns_sam_smgri_handle_query_resp( sns_sam_resp const *respMsg )
{
  sns_sam_err err = SAM_ENONE;
  uint32_t i;
  sns_q_link_s *qCurr;
  sns_sam_sensor *sensor = NULL;
  sns_smgr_sensor_events_query_resp_msg_v02 *resp =
    (sns_smgr_sensor_events_query_resp_msg_v02*)respMsg->msg.msg.buf;

  if( SNS_RESULT_SUCCESS_V01 == resp->resp.sns_result_t &&
      resp->sensor_events_valid )
  {
    for( i = 0; i < resp->sensor_events_len; i++ )
    {
      for( qCurr = sns_q_check( &sensorQ ); NULL != qCurr;
          qCurr = sns_q_next( &sensorQ, qCurr ) )
      {
        sensor = (sns_sam_sensor*)qCurr;
        if( sensor->sensorUID == resp->sensor_events[ i ] )
        {
          break;
        }
      }

      if( NULL == qCurr )
      {
        err = sns_sam_init_sensor( &sensor );
        if( SAM_ENONE != err )
        {
          SNS_PRINTF_STRING_ERROR_2( samModule,
            "Unable to initialize new sensor for %x, error: %i",
            resp->sensor_events[ i ], err );
          continue;
        }
      }

      SNS_PRINTF_STRING_LOW_1( samModule,
        "Created new sensor; UID %x", resp->sensor_events[ i ] );

      sensor->sensorUID = resp->sensor_events[ i ];
      sensor->serviceObj = SNS_SMGR_INTERNAL_SVC_get_service_object_v02();
      sensor->sensorReq = *respMsg->sensorReq;
      sensor->isAvailable = true;
      sensor->gatingType = SNS_SAM_SENSOR_EVENT;

      sns_sam_dep_available( sensor );
      sns_sam_gen_gated_sensors( sensor );
    }
  }
  else
  {
    err = SAM_ESTATE;
    SNS_PRINTF_STRING_ERROR_1( samModule,
        "Error in SMGR all sensor info response: %i",
        resp->resp.sns_err_t );
  }

  return err;
}

/**
 * Handle a sensor event response message.
 *
 * @param[i] respMsg Message to be processed
 *
 * @return SAM_ENONE
 *         SAM_ESTATE Received error response message
 */
sns_sam_err
sns_sam_smgri_handle_event_response( sns_sam_resp const *respMsg )
{
  sns_sam_err rv = SAM_ENONE;
  sns_smgr_sensor_event_resp_msg_v02 *resp =
    (sns_smgr_sensor_event_resp_msg_v02*)respMsg->msg.msg.buf;

  if( SNS_RESULT_SUCCESS_V01 != resp->resp.sns_result_t )
  {
    SNS_PRINTF_STRING_ERROR_3( samModule,
        "Received SMGRI error response: %i, suid: %x msg: %i",
        resp->resp.sns_err_t, *respMsg->msg.sensorUID, respMsg->msg.msgID );

    sns_sam_handle_enable_resp_err( respMsg->sensorReq, resp->resp.sns_err_t );
    rv = SAM_ESTATE;
  }
  else
  {
    SNS_PRINTF_STRING_LOW_2( samModule,
        "Received SMGRI success response; suid: %x msg: %i",
        *respMsg->msg.sensorUID, respMsg->msg.msgID );
  }

  return rv;
}

/**
 * Handle an response message from the SMGR Internal service.
 *
 * @param[i] respMsg Response message to process
 *
 * @note Message buffer will be freed by caller
 */
void
sns_sam_handle_smgri_resp( sns_sam_resp const *respMsg )
{
  sns_sam_err err = SAM_EFAILED;

  SNS_PRINTF_STRING_LOW_1( samModule,
    "sns_sam_handle_smgri_resp %i", respMsg->msg.msgID );

  if( SNS_SMGR_SENSOR_EVENTS_QUERY_RESP_V02 == respMsg->msg.msgID )
  {
    err = sns_sam_smgri_handle_query_resp( respMsg );
  }
  else if( SNS_SMGR_SENSOR_EVENT_RESP_V02 == respMsg->msg.msgID )
  {
    err = sns_sam_smgri_handle_event_response( respMsg );
  }
  else if( SNS_SMGR_EVENT_GATED_BUFFERING_RESP_V02 == respMsg->msg.msgID )
  {
    err = sns_sam_smgr_handle_enable_resp( respMsg );
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
        "Unknown SMGR message ID: %i", respMsg->msg.msgID );
  }

  if( SAM_ENONE != err )
  {
    SNS_PRINTF_STRING_ERROR_2( samModule,
      "Error processing resp msg: %i, error %i",
      respMsg->msg.msgID, err );
  }
}

/*============================================================================
  Public Function Definitions
  ===========================================================================*/

void
sns_sam_handle_resp( sns_sam_resp const *respMsg )
{
  SNS_PRINTF_STRING_LOW_1( samModule,
    "sns_sam_handle_resp from SUID %x", *respMsg->msg.sensorUID );

  if( SNS_REG2_SVC_get_service_object_v02() ==
      respMsg->sensorReq->sensor->serviceObj )
  {
    sns_sam_reg_handle_resp( respMsg );
  }
  else if( SNS_SMGR_SVC_get_service_object_v01() ==
           respMsg->sensorReq->sensor->serviceObj )
  {
    sns_sam_handle_smgr_resp( respMsg );
  }
  else if( SNS_SMGR_INTERNAL_SVC_get_service_object_v02() ==
           respMsg->sensorReq->sensor->serviceObj )
  {
    sns_sam_handle_smgri_resp( respMsg );
  }
  else
  {
    sns_sam_handle_sam_resp( respMsg );
  }
}