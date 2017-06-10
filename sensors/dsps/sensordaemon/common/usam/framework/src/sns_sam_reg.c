/*============================================================================
  @file sns_sam_reg.c

  @brief
  Implements all handling, processing, and generating messages to/from the
  Sensors Registry Service.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include "sns_reg_api_v02.h"
#include "sns_init.h"
#include "sns_debug_str.h"
#include "sns_sam.h"
#include "sns_sam_algo_api.h"
#include "sns_sam_client.h"
#include "sns_sam_init.h"
#include "sns_sam_reg.h"
#include "sns_sam_pm.h"
#include "sns_sam_memmgr.h"

/*============================================================================
  Public Function Definitions
  ============================================================================*/

sns_sam_err
sns_sam_reg_req( sns_sam_reg_data *regData )
{
  UNREFERENCED_PARAMETER(regData);
  sns_sam_err rv;
  struct sns_sam_msg reqMsg;
  int32_t errQMI;

  SNS_PRINTF_STRING_LOW_1( samModule, "sns_sam_reg_req %i", regData->groupID );

  errQMI = qmi_idl_get_message_c_struct_len(
    SNS_REG2_SVC_get_service_object_v02(),
    QMI_IDL_REQUEST,
    ( 0 == regData->groupDataLen ) ?
      SNS_REG_GROUP_READ_REQ_V02 :
      SNS_REG_GROUP_WRITE_REQ_V02,
    &reqMsg.msg.bufSize );
  if( QMI_IDL_LIB_NO_ERR != errQMI )
  {
    SNS_PRINTF_STRING_ERROR_1( samModule,
      "QMI error from c struct lookup %i", errQMI );
    rv = SAM_EFAILED;
  }
  else
  {
    if( 0 == regData->groupDataLen )
    {
      reqMsg.msg.buf = (intptr_t)SNS_OS_MALLOC( SNS_SAM_DBG_MOD,
        reqMsg.msg.bufSize );
      SNS_ASSERT(NULL != (void*)reqMsg.msg.buf);
      SNS_OS_MEMZERO( (void*)reqMsg.msg.buf, reqMsg.msg.bufSize );
      reqMsg.msgID = SNS_REG_GROUP_READ_REQ_V02;

      sns_reg_group_read_req_msg_v02 *regReqMsg =
        (sns_reg_group_read_req_msg_v02*)reqMsg.msg.buf;
      regReqMsg->group_id = regData->groupID;
    }
    else
    {
      reqMsg.msg.buf = (intptr_t)SNS_OS_MALLOC( SNS_SAM_DBG_MOD,
        reqMsg.msg.bufSize );
      SNS_ASSERT(NULL != (void*)reqMsg.msg.buf);
      SNS_OS_MEMZERO( (void*)reqMsg.msg.buf, reqMsg.msg.bufSize );
      reqMsg.msgID = SNS_REG_GROUP_WRITE_REQ_V02;

      sns_reg_group_write_req_msg_v02 *regReqMsg =
        (sns_reg_group_write_req_msg_v02*)reqMsg.msg.buf;
      regReqMsg->group_id = regData->groupID;
      regReqMsg->data_len = regData->groupDataLen;
      SNS_OS_MEMSCPY( regReqMsg->data,  regData->groupDataLen,
        regData->groupData, regData->groupDataLen );
    }

    rv = sns_sam_client_send( &sensorReqREG, &reqMsg );
    if( SAM_ENONE != rv )
    {
      SNS_PRINTF_STRING_ERROR_1( samModule,
        "Error sending registry request %i", rv );
      rv = SAM_EFAILED;
    }
    SNS_SAM_MEM_FREE( (void*)reqMsg.msg.buf );
  }

  return rv;
}

sns_sam_err
sns_sam_reg_handle_resp( sns_sam_resp const *respMsg )
{
  sns_q_link_s *qCurr;

  if( SNS_REG_GROUP_READ_RESP_V02 == respMsg->msg.msgID )
  {
    sns_reg_group_read_resp_msg_v02 *resp =
      (sns_reg_group_read_resp_msg_v02*)respMsg->msg.msg.buf;

    sns_sam_reg_data temp;
    temp.groupID = resp->group_id;
    temp.groupDataLen = resp->data_len;
    temp.groupData = resp->data;

    SNS_PRINTF_STRING_LOW_1( samModule, "Processing reg read response %i",
      resp->group_id );
    for( qCurr = sns_q_check( &sensorQ ); NULL != qCurr;
        qCurr = sns_q_next( &sensorQ, qCurr ) )
    {
      uint32_t i;
      sns_sam_sensor_algo *algo = (sns_sam_sensor_algo*)qCurr;

      if( !algo->sensor.isLocal )
      {
        break;
      }

      for( i = 0; i < SNS_SAM_MAX_REG_GRP; i++ )
      {
        if( resp->group_id == algo->registryGroups[ i ] )
        {
          SNS_PRINTF_STRING_MEDIUM_2( samModule,
            "Alerting algorithm %x for reg %i",
            algo->sensor.sensorUID, resp->group_id );
          algo->algoMsgAPI->sns_sam_algo_dep_registry_met( algo,
              sns_sam_init_dep_met, &algo->persistData, &temp );
        }
      }
    }

  }
  else if( SNS_REG_GROUP_WRITE_RESP_V02 == respMsg->msg.msgID )
  {
    sns_reg_group_write_resp_msg_v02 *resp =
      (sns_reg_group_write_resp_msg_v02*)respMsg->msg.msg.buf;

    SNS_PRINTF_STRING_LOW_2( samModule,
        "Received registry write response %i %i",
        resp->resp.sns_result_t, resp->resp.sns_err_t );
  }
  else
  {
    SNS_PRINTF_STRING_FATAL_1( samModule,
        "Received unknown message from registry service %i",
        respMsg->msg.msgID );
  }

  return SAM_ENONE;
}
