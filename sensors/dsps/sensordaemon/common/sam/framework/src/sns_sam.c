/*============================================================================
  FILE: sns_sam.c

  This file contains the Sensors Algorithm Manager implementation

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
 ============================================================================*/

#define __SNS_MODULE__ SNS_SAM

/*---------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "sns_sam_priv.h"

#include "sns_osa.h"
#include "sns_init.h"
#include "sns_memmgr.h"
#include "sns_debug_str.h"
#include "fixed_point.h"
#include "sns_reg_common.h"

#include "sns_smgr_api_v01.h"
#include "sns_smgr_internal_api_v01.h"
#include "sns_reg_api_v02.h"
#include "sns_sam_qmd_v01.h"

#ifdef SNS_DSPS_BUILD
#include "sns_profiling.h"
#endif

/*---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

//SAM registry read timeout periods (low - 100ms; high - 1s)
#define SNS_SAM_REG_READ_TIMER_LOW_PERIOD_USEC   100000
#define SNS_SAM_REG_READ_TIMER_HIGH_PERIOD_USEC 1000000

/*---------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/
//SAM task stack
static OS_STK sns_sam_task_stk[SNS_SAM_MODULE_STK_SIZE];

//SAM event signal
static OS_FLAG_GRP *sns_sam_sig_event;

//algorithm database
static sns_sam_algo_s* sns_sam_algo_dbase[SNS_SAM_MAX_ALGOS];

//algorithm instance database
static sns_sam_algo_inst_s* sns_sam_algo_inst_dbase[SNS_SAM_MAX_ALGO_INSTS];
//algorithm instance count
static uint8_t sns_sam_algo_inst_count;

//client request database
static sns_sam_client_req_s* sns_sam_client_req_dbase[SNS_SAM_MAX_CLIENT_REQS];
//client request count
static uint8_t sns_sam_client_req_count;

//sensor data request database
static sns_sam_data_req_s* sns_sam_data_req_dbase[SNS_SAM_MAX_DATA_REQS];
//data request count
static uint8_t sns_sam_data_req_count;

//SMGR Buffering Support flag
static bool sns_sam_smgr_buffering_flag = false;
static bool sns_sam_sensor_report_rate_available = false;

static sns_sam_time_state_s sns_sam_time_state;

#ifdef SNS_PLAYBACK_SKIP_SMGR
//time for next periodic report; initialized to maximum
static uint32_t nextReportTime[SNS_SAM_MAX_CLIENT_REQS] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
static uint8_t nextReportTimeEnable[SNS_SAM_MAX_CLIENT_REQS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif

OS_FLAGS sam_sig_mask;

static sns_sam_sensor_uuid_s sns_sam_sensor_uuids[SNS_SAM_NUM_SENSORS];
static sns_sam_sensor_info_s sns_sam_sensor_info_dbase[SNS_SAM_NUM_SENSORS];

// Service id of last registered algorithm that needs item(s) from registry
static uint8_t sns_sam_last_algo_with_registry_dep;

/*---------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------*/
#ifdef SNS_SAM_UNIT_TEST
extern sns_err_code_e sns_sam_test_init(void);
#endif

/*=========================================================================
  FUNCTION:  sns_sam_get_time_state
  =========================================================================*/
sns_sam_time_state_s sns_sam_get_time_state()
{
  return sns_sam_time_state;
}

/*=========================================================================
  FUNCTION:  sns_sam_set_time_state
  =========================================================================*/
void sns_sam_set_time_state(sns_sam_time_state_s *sam_time_state)
{
  SNS_OS_MEMCOPY( &sns_sam_time_state, sam_time_state, sizeof(sns_sam_time_state_s) );
}

/*=========================================================================
  FUNCTION:  sns_sam_data_req_dbase_acc
  =========================================================================*/
/*!
  @brief Accessor function for the sns_sam_data_req_dbase array.

  @param[i] index: Index into the request database.

  @return Database entry, or NULL, if index is out of bounds.
*/
/*=======================================================================*/
sns_sam_data_req_s* sns_sam_data_req_dbase_acc(int32_t index)
{
  return ( index >= SNS_SAM_MAX_DATA_REQS || index < 0 ) ? NULL : sns_sam_data_req_dbase[ index ];
}

/*=========================================================================
  FUNCTION:  sns_sam_algo_dbase_acc
  =========================================================================*/
/*!
  @brief Accessor function for the sns_sam_algo_dbase array.

  @param[i] index: Index into the request database.

  @return Database entry, or NULL, if index is out of bounds.
*/
/*=======================================================================*/
sns_sam_algo_s* sns_sam_algo_dbase_acc(int32_t index)
{
  return ( index >= SNS_SAM_MAX_ALGOS || index < 0 ) ? NULL : sns_sam_algo_dbase[ index ];
}

/*=========================================================================
  FUNCTION:  sns_sam_sig_event_acc
  =========================================================================*/
/*!
  @brief Accessor function for the sns_sam_sig_event object.

  @return sns_sam_sig_event
*/
/*=======================================================================*/
OS_FLAG_GRP* sns_sam_sig_event_acc(void) { return sns_sam_sig_event; }

/*=========================================================================
  FUNCTION:  sns_sam_timer_cb
  =========================================================================*/
/*!
  @brief Callback registered for timer expiry

  @param[i] argPtr: pointer to argument for callback function

  @return None
*/
/*=======================================================================*/
void sns_sam_timer_cb(void *argPtr)
{
   uint8_t err;
   uint32_t clientReqId = (uint32_t)(uintptr_t)argPtr;

   if (clientReqId < SNS_SAM_MAX_CLIENT_REQS &&
       sns_sam_client_req_dbase[clientReqId] != NULL)
   {
      sns_sam_client_req_dbase[clientReqId]->timeout = true;

      sns_os_sigs_post(sns_sam_sig_event,
                       SNS_SAM_REPORT_TIMER_SIG,
                       OS_FLAG_SET,
                       &err);
      if (err != OS_ERR_NONE)
      {
         SNS_SAM_DEBUG1(ERROR, DBG_SAM_TIMER_CB_SIGNALERR, err);
      }
   }
}

/*=========================================================================
  FUNCTION:  sns_sam_reg_timer
  =========================================================================*/
/*!
  @brief Register timer for client reports

  @param[i] clientReqId: client request id
  @param[i] period: timer period

  @return Sensors error code
*/
/*=======================================================================*/
sns_err_code_e sns_sam_reg_timer(
   uint32_t clientReqId,
   uint32_t period)
{
   sns_err_code_e err = SNS_SUCCESS;
   sns_sam_client_req_s* clientReqPtr = sns_sam_client_req_dbase[clientReqId];

   sns_em_timer_type_e timerType;
   if (clientReqPtr->reportType == SNS_SAM_PERIODIC_REPORT)
   {
      timerType = SNS_EM_TIMER_TYPE_PERIODIC;
   }
   else if (clientReqPtr->reportType == SNS_SAM_ONE_SHOT_REPORT)
   {
      timerType = SNS_EM_TIMER_TYPE_ONESHOT;
   }
   else
   {
      return SNS_ERR_FAILED;
   }

   if (clientReqPtr->timer == NULL)
   {
      err = sns_em_create_timer_obj(sns_sam_timer_cb,
                                    (void*)(intptr_t)clientReqId,
                                    timerType,
                                    &(clientReqPtr->timer));
   }

#ifndef SNS_PLAYBACK_SKIP_SMGR
   if (err == SNS_SUCCESS)
   {
      err = sns_em_register_timer(clientReqPtr->timer,
                                  period);
      if (err == SNS_SUCCESS)
      {

        SNS_SAM_DEBUG2(LOW, DBG_SAM_REG_TIMER_STARTED,
                       sns_sam_mr_get_client_id(clientReqPtr->mrClientId), clientReqPtr->period);
      }
      else
      {
        SNS_SAM_DEBUG1(ERROR, DBG_SAM_REG_TIMER_FAILED, err);
      }
   }
#endif

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_dereg_timer
  =========================================================================*/
/*!
  @brief Deregister timer

  @param[i] clientReqId: client request id

  @return Sensors error code
*/
/*=======================================================================*/
sns_err_code_e sns_sam_dereg_timer(
   uint8_t clientReqId)
{
   sns_err_code_e err;

   if (sns_sam_client_req_dbase[clientReqId]->timer == NULL)
   {
      return SNS_SUCCESS;
   }

   err = sns_em_delete_timer_obj(sns_sam_client_req_dbase[clientReqId]->timer);
   if (err == SNS_SUCCESS)
   {
      sns_sam_client_req_dbase[clientReqId]->timer = NULL;

      SNS_SAM_DEBUG3(MEDIUM, DBG_SAM_DEREG_TIMER_DELETED,
                     sns_sam_mr_get_client_id(sns_sam_client_req_dbase[clientReqId]->mrClientId),
                     sns_sam_client_req_dbase[clientReqId]->reportId,
                     sns_em_get_timestamp());
   }
   else
   {
     SNS_SAM_DEBUG1(ERROR, DBG_SAM_DEREG_TIMER_FAILED, err);
   }

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_get_algo_report
  =========================================================================*/
/*!
  @brief get the current algorithm report

  @param[i] algoInstId: index to the algorithm instance in the database

  @return Algorithm report for the specified algorithm instance
*/
/*=======================================================================*/
sns_sam_algo_rpt_s *sns_sam_get_algo_report(
   uint8_t algoInstId)
{
   sns_sam_algo_rpt_s *algoRepPtr = NULL;

   if (algoInstId < SNS_SAM_MAX_ALGO_INSTS &&
       sns_sam_algo_inst_dbase[algoInstId] != NULL)
   {
      algoRepPtr = &(sns_sam_algo_inst_dbase[algoInstId]->outputData);
   }

   return algoRepPtr;
}

/*=========================================================================
  FUNCTION:  sns_sam_get_algo_config
  =========================================================================*/
/*!
  @brief get the current algorithm configuration

  @param[i] algoInstId: index to the algorithm instance in the database

  @return Algorithm configuration for the specified algorithm instance
*/
/*=======================================================================*/
sns_sam_algo_mem_s *sns_sam_get_algo_config(
   uint8_t algoInstId)
{
   sns_sam_algo_mem_s *algoConfigPtr = NULL;

   if (algoInstId < SNS_SAM_MAX_ALGO_INSTS &&
       sns_sam_algo_inst_dbase[algoInstId] != NULL)
   {
      algoConfigPtr = &(sns_sam_algo_inst_dbase[algoInstId]->configData);
   }

   return algoConfigPtr;
}

/*=========================================================================
  FUNCTION:  sns_sam_get_algo_state
  =========================================================================*/
/*!
  @brief get the current algorithm state

  @param[i] algoInstId: index to the algorithm instance in the database

  @return Algorithm state for the specified algorithm instance
*/
/*=======================================================================*/
sns_sam_algo_mem_s *sns_sam_get_algo_state(
   uint8_t algoInstId)
{
   sns_sam_algo_mem_s *algoStatePtr = NULL;

   if (algoInstId < SNS_SAM_MAX_ALGO_INSTS &&
       sns_sam_algo_inst_dbase[algoInstId] != NULL)
   {
      algoStatePtr = &(sns_sam_algo_inst_dbase[algoInstId]->stateData);
   }

   return algoStatePtr;
}

/*=========================================================================
  FUNCTION:  sns_sam_get_algo_inst_handle
  =========================================================================*/
/*!
  @brief Get the handle to the specified algorithm instance

  @param[i] algoInstId: index to the algorithm instance in the database

  @return handle to the specified algorithm instance if found,
          NULL otherwise
*/
/*=======================================================================*/
sns_sam_algo_inst_s* sns_sam_get_algo_inst_handle(
   uint8_t algoInstId)
{
    return ( (algoInstId < SNS_SAM_MAX_ALGO_INSTS) ?
               sns_sam_algo_inst_dbase[algoInstId] : NULL );
}

/*=========================================================================
  FUNCTION:  sns_sam_get_client_req_handle
  =========================================================================*/
/*!
  @brief Get the handle to the specified client request

  @param[i] clientReqId: index to the client request in the database

  @return handle to the specified client request if found,
          NULL otherwise
*/
/*=======================================================================*/
sns_sam_client_req_s* sns_sam_get_client_req_handle(
   uint8_t clientReqId)
{
    return ( (clientReqId < SNS_SAM_MAX_CLIENT_REQS) ?
               sns_sam_client_req_dbase[clientReqId] : NULL );
}

/*=========================================================================
  FUNCTION:  sns_sam_get_smgr_msg_req_type
  =========================================================================*/
/*!
  @brief  Determines the type of message request used to communicate with
          Sensor Manager.

  @param[i] algoSvcId: algorithm service ID

  @return  message request ID
*/
/*=======================================================================*/
uint8_t sns_sam_get_smgr_msg_req_type(
   uint8_t algoSvcId)
{
   uint8_t retVal = SNS_SMGR_REPORT_REQ_V01;
   if( sns_sam_smgr_buffering_flag )
   {
      if( algoSvcId == SNS_SAM_GYROINT_SVC_ID_V01 )
      {
         // Gyroint uses the buffering query API (report rate = 0)
         retVal = SNS_SMGR_BUFFERING_REQ_V01;
      }
      else
      {
         uint8_t algoIndex = sns_sam_get_algo_index(algoSvcId);
         if( algoIndex < SNS_SAM_MAX_ALGOS &&
             sns_sam_algo_dbase[algoIndex] != NULL &&
             sns_sam_algo_dbase[algoIndex]->defSensorReportRate != 0 )
         {
            // Other algorithms that want to support buffering have set nonzero
            // default sensor report rates in registry
            retVal = SNS_SMGR_BUFFERING_REQ_V01;
         }
      }
   }

   return retVal;
}

/*=========================================================================
  FUNCTION:  sns_sam_prep_resp_msg
  =========================================================================*/
/*!
  @brief Prepares a response message for the specified request

  @param[i] reqMsgPtr: Pointer to request message for which
            response needs to be sent
  @param[i] respMsgPtr: Pointer to response message body,
            to be sent with header
  @param[i] respMsgBodyLen: Response message body length
            (excluding the header part)

  @return None
*/
/*=======================================================================*/
void sns_sam_prep_resp_msg(
   const uint8_t *reqMsgPtr,
   void *respMsgPtr,
   uint16_t respMsgBodyLen)
{
   sns_smr_header_s reqMsgHdr, respMsgHdr;

   sns_smr_get_hdr(&reqMsgHdr, reqMsgPtr);
   if (reqMsgHdr.msg_type == SNS_SMR_MSG_TYPE_REQ)
   {
      respMsgHdr.dst_module = reqMsgHdr.src_module;
      respMsgHdr.src_module = reqMsgHdr.dst_module;
      respMsgHdr.svc_num = reqMsgHdr.svc_num;
      respMsgHdr.msg_type = SNS_SMR_MSG_TYPE_RESP;
      respMsgHdr.priority = SNS_SMR_MSG_PRI_HIGH;
      respMsgHdr.body_len = respMsgBodyLen;

      respMsgHdr.txn_id = reqMsgHdr.txn_id;
      respMsgHdr.ext_clnt_id = reqMsgHdr.ext_clnt_id;

      //request and response messages are assumed to have same message ids
      respMsgHdr.msg_id = reqMsgHdr.msg_id;

      sns_smr_set_hdr(&respMsgHdr, respMsgPtr);

      SNS_SAM_DEBUG3(LOW, DBG_SAM_SEND_RSP_MSG,
                    reqMsgHdr.msg_id, reqMsgHdr.src_module, reqMsgHdr.svc_num);
   }
}

/*=========================================================================
  FUNCTION:  sns_sam_report_client_error
  =========================================================================*/
/*!
  @brief Report error message to client with error code if client request
  procesing fails.

  @param[i] clientReqMsgPtr: Pointer to client request message
            errCode: Error code
  @param[o] clientRespMsgPtr: Pointer to the response message created.

  @return None
*/
/*=======================================================================*/
void sns_sam_report_client_error(
   const void* clientReqMsgPtr,
   void** clientRespMsgPtr,
   sns_err_code_e errCode)
{
   sns_smr_header_s msgHdr;
   uint8_t algoSvcId;

   sns_smr_get_hdr(&msgHdr, clientReqMsgPtr);
   if(msgHdr.msg_type != SNS_SMR_MSG_TYPE_REQ)
   {
      return;
   }

   algoSvcId = msgHdr.svc_num;

   switch (msgHdr.msg_id)
   {
   case SNS_SAM_ALGO_ENABLE_REQ:
      sns_sam_prep_algo_enable_err(clientReqMsgPtr, clientRespMsgPtr, algoSvcId, errCode);
      break;

   case SNS_SAM_ALGO_DISABLE_REQ:
      sns_sam_prep_algo_disable_err(clientReqMsgPtr, clientRespMsgPtr, algoSvcId, errCode);
      break;

   case SNS_SAM_ALGO_GET_REPORT_REQ:
      sns_sam_prep_algo_report_err(clientReqMsgPtr, clientRespMsgPtr, algoSvcId, errCode);
      break;

   case SNS_SAM_ALGO_UPDATE_REQ:
      sns_sam_prep_algo_update_err(clientReqMsgPtr, clientRespMsgPtr, algoSvcId, errCode);
      break;

   case SNS_SAM_ALGO_BATCH_REQ:
      sns_sam_prep_algo_batch_err(clientReqMsgPtr, clientRespMsgPtr, algoSvcId, errCode);
      break;

   case SNS_SAM_ALGO_UPDATE_BATCH_PERIOD_REQ:
      sns_sam_prep_algo_update_batch_period_err(clientReqMsgPtr, clientRespMsgPtr, algoSvcId, errCode);
      break;

   case SNS_SAM_ALGO_GET_ATTRIB_REQ:
      {
         sns_sam_get_algo_attrib_resp_msg_v01 *attribRespMsgPtr =
            sns_smr_msg_alloc(SNS_SAM_DBG_MOD, sizeof(sns_sam_get_algo_attrib_resp_msg_v01));
         SNS_ASSERT(attribRespMsgPtr != NULL);

         attribRespMsgPtr->resp.sns_result_t = SNS_RESULT_FAILURE_V01;
         attribRespMsgPtr->resp.sns_err_t = errCode;

         sns_sam_prep_resp_msg(clientReqMsgPtr, attribRespMsgPtr,
             sizeof(sns_sam_get_algo_attrib_resp_msg_v01));
         *clientRespMsgPtr = attribRespMsgPtr;
      }
      break;

   case SNS_SAM_ALGO_CANCEL_REQ:
      {
         sns_common_cancel_resp_msg_v01 *cancelRespMsgPtr =
            sns_smr_msg_alloc(SNS_SAM_DBG_MOD, sizeof(sns_common_cancel_resp_msg_v01));
         SNS_ASSERT(cancelRespMsgPtr != NULL);

         cancelRespMsgPtr->resp.sns_result_t = SNS_RESULT_FAILURE_V01;
         cancelRespMsgPtr->resp.sns_err_t = errCode;
         sns_sam_prep_resp_msg(clientReqMsgPtr, cancelRespMsgPtr,
             sizeof(sns_common_cancel_resp_msg_v01));
         *clientRespMsgPtr = cancelRespMsgPtr;
      }
      break;

   case SNS_SAM_ALGO_VERSION_REQ:
      {
         sns_common_version_resp_msg_v01 *versionRespMsgPtr =
            sns_smr_msg_alloc(SNS_SAM_DBG_MOD, sizeof(sns_common_version_resp_msg_v01));
         SNS_ASSERT(versionRespMsgPtr != NULL);

         versionRespMsgPtr->resp.sns_result_t = SNS_RESULT_FAILURE_V01;
         versionRespMsgPtr->resp.sns_err_t = errCode;

         sns_sam_get_algo_version_resp(algoSvcId, versionRespMsgPtr);

         sns_sam_prep_resp_msg(clientReqMsgPtr, versionRespMsgPtr,
             sizeof(sns_common_version_resp_msg_v01));
         *clientRespMsgPtr = versionRespMsgPtr;
      }
      break;
   }
}

/*=========================================================================
  FUNCTION:  sns_sam_process_algo_report
  =========================================================================*/
/*!
  @brief Process report to specified client with specified algorithm output

  @param[i] clientReqId: index to client request in the database
  @param[i] algoRptPtr: pointer to algorithm report
  @param[i] mrClntId: MR Client ID

  @return Sensors error code
*/
/*=======================================================================*/
static void sns_sam_process_algo_report(
   uint8_t                  clientReqId,
   const sns_sam_algo_rpt_s *algoRptPtr,
   sns_sam_mr_conn_hndl     mrClntId)
{
   sns_smr_header_s msgHdr;
   uint8_t serviceId;
   sns_sam_client_req_s* clientReqPtr = sns_sam_client_req_dbase[clientReqId];
   void* clientIndMsgPtr = NULL;
   bool send_indications_on_change_only = true;

   serviceId = sns_sam_algo_inst_dbase[clientReqPtr->algoInstId]->serviceId;

   //Do not report to client before receiving initial sensor data
   //the probability for timestamp to roll over (and happen to be 0 again)
   //is very low, and missing one report indication isn't fatal.
   if (algoRptPtr->timestamp == 0)
   {
      return;
   }

   //get on change reporting requirement for algorithm service
   send_indications_on_change_only = sns_sam_on_change_report_requirement(serviceId);

   //skip report if output has not been updated since last report for non QMD and non PED algos
   if (send_indications_on_change_only)
   {
      if (algoRptPtr->timestamp == clientReqPtr->timestamp)
      {
         return;
      }
   }

   //generate indication message to specified client
   msgHdr.dst_module = clientReqPtr->reqModule;
   msgHdr.src_module = SNS_SAM_MODULE;

   msgHdr.svc_num = serviceId;
   msgHdr.msg_type = SNS_SMR_MSG_TYPE_IND;

   if (clientReqPtr->reportType == SNS_SAM_ASYNC_REPORT ||
       clientReqPtr->reportType == SNS_SAM_ONE_SHOT_REPORT)
   {
      msgHdr.priority = SNS_SMR_MSG_PRI_HIGH;
   }
   else
   {
      msgHdr.priority = SNS_SMR_MSG_PRI_LOW;
   }

   clientReqPtr->reportId++;
   msgHdr.txn_id = 0;
   msgHdr.ext_clnt_id = sns_sam_mr_get_client_id(clientReqPtr->mrClientId);

   msgHdr.msg_id = SNS_SAM_ALGO_REPORT_IND;

   sns_sam_prep_algo_report_ind(clientReqPtr, &clientIndMsgPtr, algoRptPtr, &msgHdr);
   if( clientIndMsgPtr != NULL )
   {
      if( clientReqPtr->directReportReq == true )
      {
         // Client is internal. No need to send a message. Process report synchronously
         SNS_SAM_PRINTF2( LOW, "SAM: Sending internal report to client id %d from svc id %d",
            clientReqPtr->mrClientId, serviceId );
         sns_sam_process_sam_ind( clientIndMsgPtr );
         sns_smr_msg_free( clientIndMsgPtr );
      }
      else
      {
         sns_sam_mr_send_ind(clientIndMsgPtr,
                             mrClntId,
                             clientReqPtr->notify_suspend.send_ind_during_suspend);
      }
   }

   //update client report timestamp
   clientReqPtr->timestamp = algoRptPtr->timestamp;

   SNS_SAM_DEBUG3(LOW, DBG_SAM_RPT_IND_MSG,
                  sns_sam_mr_get_client_id(clientReqPtr->mrClientId),
                  serviceId, clientReqPtr->reportId-1);
}

/*=========================================================================
  FUNCTION:  sns_sam_send_error_ind
  =========================================================================*/
/*!
  @brief Send indication to the specified client with asynchronous error

  @param[i] clientReqId: index to client request in the database
  @param[i] error: error status to be notified to client
  @param[i] mrClntId: MR Client ID

  @return Sensors error code
*/
/*=======================================================================*/
static void sns_sam_send_error_ind(
   uint8_t              clientReqId,
   uint8_t              error,
   sns_sam_mr_conn_hndl mrClntId)
{
   sns_smr_header_s msgHdr;
   uint8_t serviceId;
   void *msgIndPtr = NULL;
   sns_sam_client_req_s* clientReqPtr = sns_sam_client_req_dbase[clientReqId];

   serviceId = sns_sam_algo_inst_dbase[clientReqPtr->algoInstId]->serviceId;

   //generate indication message to specified client
   msgHdr.dst_module = clientReqPtr->reqModule;
   msgHdr.src_module = SNS_SAM_MODULE;

   msgHdr.svc_num = serviceId;
   msgHdr.msg_type = SNS_SMR_MSG_TYPE_IND;
   msgHdr.priority = SNS_SMR_MSG_PRI_HIGH;

   clientReqPtr->reportId++;
   msgHdr.txn_id = 0;
   msgHdr.ext_clnt_id = sns_sam_mr_get_client_id(clientReqPtr->mrClientId);
   msgHdr.msg_id = SNS_SAM_ALGO_ERROR_IND;

   sns_sam_prep_algo_error_ind(clientReqPtr->algoInstId, error, &msgHdr, &msgIndPtr);
   sns_sam_mr_send_ind(msgIndPtr, mrClntId, true);

   SNS_SAM_DEBUG3(HIGH, DBG_SAM_RPT_ERRIND_MSG,
                  sns_sam_mr_get_client_id(clientReqPtr->mrClientId),
                  serviceId, clientReqPtr->reportId-1);
}

/*=========================================================================
  FUNCTION:  sns_sam_handle_duty_cycle_timeout
  =========================================================================*/
/*!
  @brief Sets duty cycle state associated with algorithm instance

  @param[i] clientReqId: index to client request in the database
  @param[i] algoInstId: index to algorithm instance in the database

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_handle_duty_cycle_timeout(
   uint32_t clientReqId,
   uint8_t algoInstId)
{
   sns_sam_algo_inst_s *algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];
   bool dutycycleStateChange = false;

   if (algoInstPtr == NULL)
   {
      return SNS_ERR_FAILED;
   }

   /* if dutycycleTimerSource is SNS_SAM_DUTY_CYCLE_TIMER_SOURCE_NONE,
   indicates that duty cycling is disabled and will not enter if condition.*/
   if (algoInstPtr->dutycycleTimerSource == (int32_t)clientReqId)
   {
      uint8_t dutycycleResetCount = (uint8_t)(100/algoInstPtr->dutycycleOnPercent);

      algoInstPtr->dutycycleTimeoutCount++;

      if ((algoInstPtr->dutycycleTimeoutCount % dutycycleResetCount)==1)
      {
         algoInstPtr->dutycycleStateOn = false;
         dutycycleStateChange = true;
      }
      else if ((algoInstPtr->dutycycleTimeoutCount % dutycycleResetCount)==0)
      {
         algoInstPtr->dutycycleStateOn = true;
         algoInstPtr->dutycycleTimeoutCount = 0;
         dutycycleStateChange = true;
      }
   }

   if (dutycycleStateChange)
   {
      uint8_t algoIndex = sns_sam_get_algo_index(algoInstPtr->serviceId);
      sns_sam_algo_s *algoPtr;

      if (algoIndex >= SNS_SAM_MAX_ALGOS)
      {
         return SNS_ERR_FAILED;
      }

      algoPtr = sns_sam_algo_dbase[algoIndex];

      // Inform algorithm about duty cycle state change
      if( algoPtr && algoPtr->algoApi.sns_sam_algo_handle_duty_cycle_state_change )
      {
         void * statePtr = algoInstPtr->stateData.memPtr;
         void * outputPtr = algoInstPtr->outputData.memPtr;
         uint32_t timestamp = sns_em_get_timestamp();
         algoPtr->algoApi.sns_sam_algo_handle_duty_cycle_state_change(
                                                 algoInstPtr->dutycycleStateOn,
                                                 statePtr, outputPtr, timestamp);
      }

      sns_sam_handle_duty_cycle_state_change(
         algoInstId,
         algoInstPtr);
   }
   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_handle_report_timeout
  =========================================================================*/
/*!
  @brief Handle the timeout indicating periodic report to client is due

  @return None
*/
/*=======================================================================*/
static void sns_sam_handle_report_timeout(void)
{
   uint8_t clientReqId, clientReqCnt;
   sns_sam_algo_rpt_s *algoRptPtr = NULL;

   //go through the client request database for periodic requests
   //check which responses are due
   for (clientReqId = 0, clientReqCnt = 0;
        clientReqCnt < sns_sam_client_req_count &&
        clientReqId < SNS_SAM_MAX_CLIENT_REQS;
        clientReqId++)
   {
      sns_sam_client_req_s* clientReqPtr =
         sns_sam_client_req_dbase[clientReqId];

      if (clientReqPtr != NULL)
      {
         if (clientReqPtr->reportType == SNS_SAM_PERIODIC_REPORT ||
             clientReqPtr->reportType == SNS_SAM_ONE_SHOT_REPORT)
         {
            if (clientReqPtr->timeout == true)
            {
               uint8_t algoInstId = clientReqPtr->algoInstId;
               sns_sam_algo_inst_s *algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];
               clientReqPtr->timeout = false;

               algoRptPtr = sns_sam_get_algo_report(algoInstId);

               if( algoRptPtr && algoInstPtr )
               {
                  //process algorithm report indication to client
                  sns_sam_update_algo_report_ts(algoInstPtr,
                                                sns_sam_mr_get_client_id(clientReqPtr->mrClientId));
                  sns_sam_process_algo_report(clientReqId, algoRptPtr,
                                              clientReqPtr->mrClientId);
               }

               SNS_SAM_PRINTF3(LOW,
                   "Handled report timeout for client %d, period %d at time %d",
                   sns_sam_mr_get_client_id( clientReqPtr->mrClientId ),
                   clientReqPtr->period, sns_em_get_timestamp() );

               //if periodic reporting
               if (clientReqPtr->reportType == SNS_SAM_PERIODIC_REPORT)
               {
                  sns_sam_handle_duty_cycle_timeout(clientReqId,
                                                    algoInstId);
               }
            }
         }

         clientReqCnt++;
      }
   }
}

/*=========================================================================
  FUNCTION:  sns_sam_find_client_req
=========================================================================*/
/*!
  @brief Find the client request

  @param[i] msgHdrPtr: Pointer to client request message header
  @param[i] mrClntId: MR Client ID

  @return client request id if found, SNS_SAM_INVALID_ID otherwise
*/
/*=======================================================================*/
uint8_t sns_sam_find_client_req(
   const sns_smr_header_s *msgHdrPtr,
   sns_sam_mr_conn_hndl   mrClntId)
{
   uint8_t clientReqId, clientReqCnt;

   for (clientReqId = 0, clientReqCnt = 0;
        clientReqCnt < sns_sam_client_req_count &&
        clientReqId < SNS_SAM_MAX_CLIENT_REQS;
        clientReqId++)
   {
      sns_sam_client_req_s* clientReqPtr =
         sns_sam_client_req_dbase[clientReqId];
      if (clientReqPtr != NULL)
      {
         if (sns_sam_mr_confirm_client_req(clientReqPtr, msgHdrPtr, mrClntId))
         {
            if (clientReqPtr->algoInstId < SNS_SAM_MAX_ALGO_INSTS &&
                sns_sam_algo_inst_dbase[clientReqPtr->algoInstId] != NULL &&
                sns_sam_algo_inst_dbase[clientReqPtr->algoInstId]->serviceId ==
                msgHdrPtr->svc_num)
            {
               return clientReqId;
            }
         }

         clientReqCnt++;
      }
   }

   return SNS_SAM_INVALID_ID;
}

/*=========================================================================
  FUNCTION:  sns_sam_add_client_req
  =========================================================================*/
/*!
  @brief Adds the client request to the client request database

  @param[i] clientReqMsgPtr: pointer to the client request message
  @param[i] algoInstId: index to the algorithm instance in the database
  @param[i] mrClntId: MR Client ID
  @param[i] directReportReq: indicates if reporting using messages can be
                             bypassed using a direct function call instead

  @return client request id if successful, SNS_SAM_INVALID_ID otherwise
*/
/*=======================================================================*/
static uint8_t sns_sam_add_client_req(
   const void           *clientReqMsgPtr,
   uint8_t              algoInstId,
   sns_sam_mr_conn_hndl mrClntId,
   bool                 directReportReq)
{
   uint8_t clientReqId;
   uint32_t reportPeriod;
   sns_sam_report_e reportType;
   uint32_t proc_type;
   bool notify_suspend;

   sns_smr_header_s msgHdr;

   if (sns_sam_client_req_count >= SNS_SAM_MAX_CLIENT_REQS)
   {
      SNS_SAM_DEBUG0(ERROR, DBG_SAM_ADD_CLIENT_MAX_ERR);
      return SNS_SAM_INVALID_ID;
   }

   reportPeriod = sns_sam_get_algo_report_period(
      sns_sam_algo_inst_dbase[algoInstId]->serviceId,
      clientReqMsgPtr,
      sns_sam_algo_inst_dbase[algoInstId]->configData.memPtr);

   if (reportPeriod == SNS_SAM_INVALID_PERIOD)
   {
      return SNS_SAM_INVALID_ID;
   }

   if( directReportReq )
   {
      // Always report synchronously to internal clients
      reportType = SNS_SAM_SYNC_REPORT;
   }
   else if (reportPeriod == 0)
   {
      /* reportPeriod = 0 indicates synchronous report at sampling rate for sensor fusion algorithms
         Add EIS algorithm (Gyroint): treat reportPeriod of 0 as sync reporting
         reportPeriod = 0 indicates asynchronous report for all other algorithms
      */
      switch (sns_sam_algo_inst_dbase[algoInstId]->serviceId)
      {
         case SNS_SAM_MAG_CAL_SVC_ID_V01:
         case SNS_SAM_FILTERED_MAG_SVC_ID_V01:
         case SNS_SAM_ROTATION_VECTOR_SVC_ID_V01:
         case SNS_SAM_GAME_ROTATION_VECTOR_SVC_ID_V01:
         case SNS_SAM_QUATERNION_SVC_ID_V01:
         case SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01:
         case SNS_SAM_ORIENTATION_SVC_ID_V01:
         case SNS_SAM_INTEG_ANGLE_SVC_ID_V01:
         case SNS_SAM_GYROINT_SVC_ID_V01:
         case SNS_SAM_PAM_SVC_ID_V01:
            reportType = SNS_SAM_SYNC_REPORT;
            break;
         case SNS_SAM_DISTANCE_BOUND_SVC_ID_V01:
            reportType = SNS_SAM_ONE_SHOT_REPORT;
            break;
         default:
            reportType = SNS_SAM_ASYNC_REPORT;
            break;
      }
   }
   else
   {
      reportType = SNS_SAM_PERIODIC_REPORT;
   }

   sns_smr_get_hdr(&msgHdr, clientReqMsgPtr);

   /* get the information about notifying during suspend from the client
      request message */
   sns_sam_get_request_suspend_notif_info(
      sns_sam_algo_inst_dbase[algoInstId]->serviceId,
      clientReqMsgPtr,
      &proc_type,
      &notify_suspend);

   for (clientReqId = 0; clientReqId < SNS_SAM_MAX_CLIENT_REQS; clientReqId++)
   {
      if (sns_sam_client_req_dbase[clientReqId] == NULL)
      {
         sns_sam_client_req_s* clientReqPtr;

         sns_sam_client_req_dbase[clientReqId] =
            SNS_OS_MALLOC(SNS_SAM_DBG_MOD, sizeof(sns_sam_client_req_s));
         SNS_ASSERT(sns_sam_client_req_dbase[clientReqId] != NULL);
         clientReqPtr = sns_sam_client_req_dbase[clientReqId];

         //add client req
         clientReqPtr->algoInstId = algoInstId;
         clientReqPtr->reqModule = msgHdr.src_module;
         clientReqPtr->reportId = 0;

         clientReqPtr->period = reportPeriod;
         clientReqPtr->timeout = false;
         clientReqPtr->timer = NULL;
         clientReqPtr->timestamp = 0;

         clientReqPtr->mrClientId = mrClntId;
         clientReqPtr->reportType = reportType;

         SNS_SAM_PRINTF2(LOW, "Client request id %d enabled with report type %d",
                         clientReqId, reportType);

         // update client request data base
         clientReqPtr->dutycycleOnPercent =
            sns_sam_algo_inst_dbase[algoInstId]->dutycycleOnPercent;

         // duty-cycle is not currently required for async algorithms
         if (clientReqPtr->reportType == SNS_SAM_PERIODIC_REPORT)
         {
            sns_sam_reg_timer(clientReqId, reportPeriod);
         }

         if ((clientReqPtr->reportType != SNS_SAM_ASYNC_REPORT) &&
             (clientReqPtr->reportType != SNS_SAM_ONE_SHOT_REPORT))
         {
            sns_sam_update_power_vote(clientReqPtr, true);
         }

         // save the proc type and notify during suspend information in client db
         clientReqPtr->notify_suspend.proc_type = proc_type;
         clientReqPtr->notify_suspend.send_ind_during_suspend = notify_suspend;

         clientReqPtr->directReportReq = directReportReq;

         sns_sam_client_req_count++;

#ifdef SNS_PLAYBACK_SKIP_SMGR
         // reportPeriod is in millisec, convert to DSPS clock ticks
         if (reportPeriod != 0)
         {
            nextReportTime[clientReqId] =
               (uint32_t)(reportPeriod*DSPS_SLEEP_CLK/1000);
         }
#endif

         SNS_SAM_DEBUG3(LOW, DBG_SAM_ADD_CLIENT_INFO,
                        clientReqId, clientReqPtr->reqModule,
                        clientReqPtr->algoInstId);

         return clientReqId;
      }
   }

   return SNS_SAM_INVALID_ID;
}

/*=========================================================================
  FUNCTION:  sns_sam_delete_client_req
  =========================================================================*/
/*!
  @brief Deletes the client request from the client request database

  @param[i] clientReqId: index to the client request in the database

  @return None
*/
/*=======================================================================*/
static void sns_sam_delete_client_req(
   uint8_t clientReqId)
{
   sns_sam_client_req_s* clientReqPtr = sns_sam_client_req_dbase[clientReqId];

   if (clientReqPtr->reportType == SNS_SAM_PERIODIC_REPORT ||
       clientReqPtr->reportType == SNS_SAM_ONE_SHOT_REPORT)
   {
      sns_sam_dereg_timer(clientReqId);
   }

   if ((clientReqPtr->reportType != SNS_SAM_ASYNC_REPORT) &&
       (clientReqPtr->reportType != SNS_SAM_ONE_SHOT_REPORT))
   {
      sns_sam_update_power_vote(clientReqPtr, false);
   }

   SNS_SAM_DEBUG3(MEDIUM, DBG_SAM_DELETE_CLIENT_INFO,
                  clientReqId, clientReqPtr->reqModule,
                  clientReqPtr->algoInstId);

   SNS_OS_FREE(sns_sam_client_req_dbase[clientReqId]);
   sns_sam_client_req_dbase[clientReqId] = NULL;

   sns_sam_client_req_count--;
}

/*=========================================================================
  FUNCTION:  sns_sam_find_data_req
  =========================================================================*/
/*!
  @brief Searches the active sensor data request database for an instance
  with the same sensor id and report rate

  @param[i] algoInstId: algorithm instance id
  @param[i] reportRate: report generation rate
  @param[i] sensorReqCnt: number of sensors in data request
  @param[i] sensorReq: sensors for which data is requested

  @return  Index to data request matching the specified configuration
  SNS_SAM_INVALID_ID if match is not found
*/
/*=======================================================================*/
static uint8_t sns_sam_find_data_req(
   uint8_t algoInstId,
   uint32_t reportRate,
   uint8_t sensorReqCnt,
   sns_sam_sensor_req_s sensorReq[])
{
   uint8_t dataReqId, dataReqCnt;

   for (dataReqId = 0, dataReqCnt = 0;
        dataReqCnt < sns_sam_data_req_count &&
        dataReqId < SNS_SAM_MAX_DATA_REQS;
        dataReqId++)
   {
      if (sns_sam_data_req_dbase[dataReqId] != NULL)
      {
         sns_sam_data_req_s *dataReq = sns_sam_data_req_dbase[dataReqId];

         if (dataReq->sensorCount == sensorReqCnt &&
             dataReq->reportRate == reportRate)
         {
            uint8_t i, j;
            for (j = 0; j < sensorReqCnt; j++)
            {
               if (dataReq->sensorDbase[j].sensorId != sensorReq[j].sensorId ||
                   dataReq->sensorDbase[j].dataType != sensorReq[j].dataType ||
                   dataReq->sensorDbase[j].sampleRate != sensorReq[j].sampleRate ||
                   dataReq->sensorDbase[j].sampleQual != sensorReq[j].sampleQual)
               {
                  break;
               }
            }

            //Found matching request
            if (j >= sensorReqCnt)
            {
               //Avoid duplicate
               for (i = 0; i < dataReq->algoInstCount; i++)
               {
                  if(dataReq->algoInstDbase[i] == algoInstId)
                  {
                     return dataReqId;
                  }
               }

               //Add request
               if(i < SNS_SAM_MAX_ALGO_INSTS_PER_DATA_REQ)
               {
                  dataReq->algoInstDbase[i] = algoInstId;
                  dataReq->algoInstCount++;
                  return dataReqId;
               }
            }
         }

         dataReqCnt++;
      }
   }

   return SNS_SAM_INVALID_ID;
}

/*=========================================================================
  FUNCTION:  sns_sam_find_data_req_for_query
  =========================================================================*/
/*!
  @brief Searches the active sensor data request database for an instance
  with the same algorithm instance ID associated with a query request.

  @param[i] algoInstId: algorithm instance ID

  @return  Index to data request matching the specified configuration
           SNS_SAM_INVALID_ID if match is not found
*/
/*=======================================================================*/
uint8_t sns_sam_find_data_req_for_query(
   uint8_t algoInstId)
{
   uint8_t dataReqId, dataReqCnt, i;
   sns_sam_data_req_s* dataReq;

   for (dataReqId = 0, dataReqCnt = 0;
        dataReqCnt < sns_sam_data_req_count &&
        dataReqId < SNS_SAM_MAX_DATA_REQS;
        dataReqId++)
   {
      if (sns_sam_data_req_dbase[dataReqId] != NULL)
      {
         dataReq = sns_sam_data_req_dbase[dataReqId];
         for (i = 0; i < dataReq->algoInstCount; i++)
         {
            if(dataReq->algoInstDbase[i] == algoInstId &&
               dataReq->reportRate == 0)
            {
               return dataReqId;
            }
         }
         dataReqCnt++;
      }
   }

   return SNS_SAM_INVALID_ID;
}

/*=========================================================================
  FUNCTION:  sns_sam_send_smgr_start_req
  =========================================================================*/
/*!
  @brief Send a request to sensors manager for sensor data

  @param[i] dataReqId: Index of data request in database
  @param[i] algoSvcId: algorithm service ID

  @return Sensors error code
*/
/*=======================================================================*/
sns_err_code_e sns_sam_send_smgr_start_req(
   uint8_t dataReqId,
   uint8_t algoSvcId)
{
   sns_smr_header_s msgHdr;
   uint_fast16_t msgSize = 0;
   sns_err_code_e err;
   uint8_t i;
   void *msgPtr = NULL;

   msgHdr.src_module = SNS_SAM_MODULE;
   msgHdr.svc_num = SNS_SMGR_SVC_ID_V01;
   msgHdr.msg_type = SNS_SMR_MSG_TYPE_REQ;
   msgHdr.priority = SNS_SMR_MSG_PRI_LOW;
   msgHdr.txn_id = 0;  //SAM uses external client id 0 for requests to SMGR
   msgHdr.ext_clnt_id = 0;

   if(sns_sam_get_smgr_msg_req_type(algoSvcId) == SNS_SMGR_BUFFERING_REQ_V01)
   {
      msgSize = sizeof(sns_smgr_buffering_req_msg_v01);
      sns_smgr_buffering_req_msg_v01* reqMsgPtr =
         (sns_smgr_buffering_req_msg_v01 *) sns_smr_msg_alloc(SNS_SAM_DBG_MOD, msgSize);
      SNS_ASSERT(reqMsgPtr != NULL);

      msgHdr.msg_id = SNS_SMGR_BUFFERING_REQ_V01;
      msgHdr.body_len = msgSize;

      reqMsgPtr->ReportId = dataReqId;
      reqMsgPtr->ReportRate = sns_sam_data_req_dbase[dataReqId]->reportRate;
      reqMsgPtr->Action = SNS_SMGR_BUFFERING_ACTION_ADD_V01;
      reqMsgPtr->Item_len = sns_sam_data_req_dbase[dataReqId]->sensorCount;
      for(i = 0; i < reqMsgPtr->Item_len; i++)
      {
         reqMsgPtr->Item[i].SensorId =
            sns_sam_data_req_dbase[dataReqId]->sensorDbase[i].sensorId;
         reqMsgPtr->Item[i].DataType =
            sns_sam_data_req_dbase[dataReqId]->sensorDbase[i].dataType;
         reqMsgPtr->Item[i].SamplingRate =
            sns_sam_data_req_dbase[dataReqId]->sensorDbase[i].sampleRate;
         reqMsgPtr->Item[i].SampleQuality =
            sns_sam_data_req_dbase[dataReqId]->sensorDbase[i].sampleQual;
         reqMsgPtr->Item[i].Decimation = SNS_SMGR_DECIMATION_FILTER_V01;
         if (reqMsgPtr->Item[i].SensorId == SNS_SMGR_ID_ACCEL_V01)
         {
            reqMsgPtr->Item[i].Calibration = SNS_SMGR_CAL_SEL_FACTORY_CAL_V01;
         }
         else
         {
            reqMsgPtr->Item[i].Calibration = SNS_SMGR_CAL_SEL_FULL_CAL_V01;
         }
      }

      if (SNS_SAM_MODULE == SNS_MODULE_DSPS_SAM)
      {
        reqMsgPtr->notify_suspend_valid = true;
        reqMsgPtr->notify_suspend.proc_type = SNS_PROC_SSC_V01;
        reqMsgPtr->notify_suspend.send_indications_during_suspend = true;
      }
      else if(SNS_SAM_MODULE == SNS_MODULE_APPS_SAM)
      {
        reqMsgPtr->notify_suspend_valid = true;
        reqMsgPtr->notify_suspend.proc_type = SNS_PROC_APPS_V01;
        reqMsgPtr->notify_suspend.send_indications_during_suspend = false;
      }
      else
      {
        reqMsgPtr->notify_suspend_valid = false;
      }

      reqMsgPtr->SrcModule_valid = true;
      reqMsgPtr->SrcModule = SNS_SAM_MODULE;

      msgPtr = (void*) reqMsgPtr;
   }
   else
   {
      msgSize = sizeof(sns_smgr_periodic_report_req_msg_v01);
      sns_smgr_periodic_report_req_msg_v01* reqMsgPtr =
         (sns_smgr_periodic_report_req_msg_v01 *) sns_smr_msg_alloc(SNS_SAM_DBG_MOD, msgSize);
      SNS_ASSERT(reqMsgPtr != NULL);

      msgHdr.msg_id = SNS_SMGR_REPORT_REQ_V01;
      msgHdr.body_len = msgSize;

      reqMsgPtr->BufferFactor = 1;
      reqMsgPtr->ReportId = dataReqId;
      reqMsgPtr->Action = SNS_SMGR_REPORT_ACTION_ADD_V01;
      reqMsgPtr->ReportRate = sns_sam_data_req_dbase[dataReqId]->reportRate;
      reqMsgPtr->cal_sel_valid = true;
      reqMsgPtr->cal_sel_len = sns_sam_data_req_dbase[dataReqId]->sensorCount;
      reqMsgPtr->SrcModule_valid = true;
      reqMsgPtr->SrcModule = SNS_SAM_MODULE;
      reqMsgPtr->Item_len = sns_sam_data_req_dbase[dataReqId]->sensorCount;
      for (i = 0; i < sns_sam_data_req_dbase[dataReqId]->sensorCount; i++)
      {
         reqMsgPtr->Item[i].SensorId =
            sns_sam_data_req_dbase[dataReqId]->sensorDbase[i].sensorId;
         reqMsgPtr->Item[i].DataType =
            sns_sam_data_req_dbase[dataReqId]->sensorDbase[i].dataType;

         reqMsgPtr->Item[i].Decimation = SNS_SMGR_DECIMATION_FILTER_V01;

         // Use factory calibrated accel, till accel autocal stabilizes
         // Since factory calibrated accelerometer is acceptable for
         // existing features, this will allow accel-autocal to be tested
         // in isolation with minimal impact on existing features
         if (reqMsgPtr->Item[i].SensorId == SNS_SMGR_ID_ACCEL_V01)
         {
            reqMsgPtr->cal_sel[i] = SNS_SMGR_CAL_SEL_FACTORY_CAL_V01;
         }
         else
         {
            reqMsgPtr->cal_sel[i] = SNS_SMGR_CAL_SEL_FULL_CAL_V01;
         }
      }

      if (SNS_SAM_MODULE == SNS_MODULE_DSPS_SAM)
      {
        reqMsgPtr->notify_suspend_valid = true;
        reqMsgPtr->notify_suspend.proc_type = SNS_PROC_SSC_V01;
        reqMsgPtr->notify_suspend.send_indications_during_suspend = true;
      }
      else if(SNS_SAM_MODULE == SNS_MODULE_APPS_SAM)
      {
        reqMsgPtr->notify_suspend_valid = true;
        reqMsgPtr->notify_suspend.proc_type = SNS_PROC_APPS_V01;
        reqMsgPtr->notify_suspend.send_indications_during_suspend = false;
      }
      else
      {
        reqMsgPtr->notify_suspend_valid = false;
      }

      msgPtr = (void*) reqMsgPtr;
   }

   sns_smr_set_hdr(&msgHdr, msgPtr);
   err = sns_sam_mr_send(msgPtr, NULL);

   SNS_SAM_DEBUG3(MEDIUM, DBG_SAM_REQ_SNSR_DATA_MSG, dataReqId,
                  sns_sam_data_req_dbase[dataReqId]->sensorDbase[0].sensorId,
                  sns_sam_data_req_dbase[dataReqId]->reportRate);

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_start_sensor_data
  =========================================================================*/
/*!
  @brief Request sensor data according to the algorithm needs

  @param[i] algoInstId: algorithm instance id
  @param[i] reportRate: report generation rate
  @param[i] sensorReqCnt: number of sensors in data request
  @param[i] sensorReq: sensors for which data is requested

  @return  Index to data request matching the specified configuration
  SNS_SAM_INVALID_ID if match is not found
*/
/*=======================================================================*/
uint8_t sns_sam_start_sensor_data(
   uint8_t algoInstId,
   uint32_t reportRate,
   uint8_t sensorReqCnt,
   sns_sam_sensor_req_s sensorReq[])
{
   uint8_t i, dataReqId = SNS_SAM_INVALID_ID;

   //check if there is an identical existing request
   dataReqId = sns_sam_find_data_req(algoInstId,
                                     reportRate,
                                     sensorReqCnt,
                                     sensorReq);

   //add request if none exists
   if (dataReqId >= SNS_SAM_MAX_DATA_REQS &&
       sns_sam_data_req_count < SNS_SAM_MAX_DATA_REQS)
   {
      for (dataReqId = 0; dataReqId < SNS_SAM_MAX_DATA_REQS; dataReqId++)
      {
         if (sns_sam_data_req_dbase[dataReqId] == NULL)
         {
            sns_sam_data_req_dbase[dataReqId] =
               SNS_OS_MALLOC(SNS_SAM_DBG_MOD, sizeof(sns_sam_data_req_s));
            SNS_ASSERT(sns_sam_data_req_dbase[dataReqId] != NULL);

            for (i = 0; i < SNS_SAM_MAX_ALGO_INSTS_PER_DATA_REQ; i++)
            {
               sns_sam_data_req_dbase[dataReqId]->algoInstDbase[i] =
                  SNS_SAM_INVALID_ID;
            }
            sns_sam_data_req_dbase[dataReqId]->reportRate = reportRate;
            for (i = 0; i < sensorReqCnt; i++)
            {
              sns_sam_data_req_dbase[dataReqId]->sensorDbase[i] = sensorReq[i];
            }
            sns_sam_data_req_dbase[dataReqId]->sensorCount = sensorReqCnt;
            sns_sam_data_req_dbase[dataReqId]->algoInstDbase[0] = algoInstId;
            sns_sam_data_req_dbase[dataReqId]->algoInstCount = 1;

            //send message to sensors manager requesting required sensor data
            sns_sam_send_smgr_start_req(dataReqId,
                                        sns_sam_algo_inst_dbase[algoInstId]->serviceId);

            sns_sam_data_req_count++;

            break;
         }
      }
   }

   return dataReqId;
}

/*=========================================================================
  FUNCTION:  sns_sam_send_smgr_stop_req
  =========================================================================*/
/*!
  @brief Send a request to sensors manager to stop sensor data received by
  specified algorithm

  @param[i] dataReqId: Index of data request in database
  @param[i] algoSvcId: algorithm service ID

  @return None
*/
/*=======================================================================*/
static void sns_sam_send_smgr_stop_req(
   uint8_t dataReqId,
   uint8_t algoSvcId)
{
   sns_smr_header_s msgHdr;
   void *msgPtr = NULL;

   msgHdr.src_module = SNS_SAM_MODULE;
   msgHdr.svc_num = SNS_SMGR_SVC_ID_V01;
   msgHdr.msg_type = SNS_SMR_MSG_TYPE_REQ;
   msgHdr.priority = SNS_SMR_MSG_PRI_LOW;
   msgHdr.txn_id = 0; //SAM uses external client id 0 for requests to SMGR
   msgHdr.ext_clnt_id = 0;

   if(sns_sam_get_smgr_msg_req_type(algoSvcId) == SNS_SMGR_BUFFERING_REQ_V01)
   {
      sns_smgr_buffering_req_msg_v01* reqMsgPtr =
         (sns_smgr_buffering_req_msg_v01 *) sns_smr_msg_alloc(SNS_SAM_DBG_MOD,
                 sizeof(sns_smgr_buffering_req_msg_v01));
      SNS_ASSERT(reqMsgPtr != NULL);

      msgHdr.msg_id = SNS_SMGR_BUFFERING_REQ_V01;
      msgHdr.body_len = sizeof(sns_smgr_buffering_req_msg_v01);

      reqMsgPtr->ReportId = dataReqId;
      reqMsgPtr->Action = SNS_SMGR_BUFFERING_ACTION_DELETE_V01;
      reqMsgPtr->Item_len = 0;
      reqMsgPtr->notify_suspend_valid = false;
      reqMsgPtr->SrcModule_valid = true;
      reqMsgPtr->SrcModule = SNS_SAM_MODULE;

      msgPtr = (void*) reqMsgPtr;
   }
   else
   {
      //send request to stop sensor data by sensor manager
      sns_smgr_periodic_report_req_msg_v01* reqMsgPtr =
         (sns_smgr_periodic_report_req_msg_v01 *) sns_smr_msg_alloc(SNS_SAM_DBG_MOD,
                 sizeof(sns_smgr_periodic_report_req_msg_v01));
      SNS_ASSERT(reqMsgPtr != NULL);

      msgHdr.msg_id = SNS_SMGR_REPORT_REQ_V01;
      msgHdr.body_len = sizeof(sns_smgr_periodic_report_req_msg_v01);

      reqMsgPtr->Item_len = 0;
      reqMsgPtr->BufferFactor = 1;
      reqMsgPtr->ReportId = dataReqId;
      reqMsgPtr->Action = SNS_SMGR_REPORT_ACTION_DELETE_V01;
      reqMsgPtr->SrcModule_valid = true;
      reqMsgPtr->SrcModule = SNS_SAM_MODULE;

      msgPtr = (void*) reqMsgPtr;
   }
   sns_smr_set_hdr(&msgHdr, msgPtr);
   sns_sam_mr_send(msgPtr, NULL);

   sns_sam_sensor_data_stop_ind(dataReqId);

   SNS_SAM_DEBUG3(MEDIUM, DBG_SAM_STOP_SNSR_DATA_MSG, dataReqId,
                  sns_sam_data_req_dbase[dataReqId]->sensorDbase[0].sensorId,
                  sns_sam_data_req_dbase[dataReqId]->reportRate);
}

/*=========================================================================
  FUNCTION:  sns_sam_stop_sensor_data
  =========================================================================*/
/*!
  @brief Stop sensor data received for specified algorithm instance

  @param[i] algoInstId: Index of algorithm instance in database

  @return None
*/
/*=======================================================================*/
void sns_sam_stop_sensor_data(
   uint8_t algoInstId)
{
   uint8_t dataReqId, dataReqCnt, i;

   for (dataReqId = 0, dataReqCnt = 0;
        dataReqCnt < sns_sam_data_req_count &&
        dataReqId < SNS_SAM_MAX_DATA_REQS;
        dataReqId++)
   {
      if (sns_sam_data_req_dbase[dataReqId] != NULL)
      {
         sns_sam_data_req_s *dataReq = sns_sam_data_req_dbase[dataReqId];
         for (i = 0; i < dataReq->algoInstCount; i++)
         {
            if(dataReq->algoInstDbase[i] == algoInstId)
            {
               dataReq->algoInstCount--;
               dataReq->algoInstDbase[i] =
                  dataReq->algoInstDbase[dataReq->algoInstCount];
               dataReq->algoInstDbase[dataReq->algoInstCount] =
                  SNS_SAM_INVALID_ID;

               if(dataReq->algoInstCount == 0)
               {
                  sns_sam_send_smgr_stop_req(dataReqId,
                                             sns_sam_algo_inst_dbase[algoInstId]->serviceId);
                  SNS_OS_FREE(sns_sam_data_req_dbase[dataReqId]);
                  sns_sam_data_req_dbase[dataReqId] = NULL;

                  sns_sam_data_req_count--;
                  break;
               }
            }
         }
         dataReqCnt++;
      }
   }
}

/*=========================================================================
  FUNCTION:  sns_sam_get_algo_index
  =========================================================================*/
/*!
  @brief Get the index into the algorithm database for the specified algorithm

  @param[i] algoSvcId: algorithm service id

  @return algorithm index for the specified algorithm if found,
          SNS_SAM_INVALID_ID otherwise
*/
/*=======================================================================*/
uint8_t sns_sam_get_algo_index(
   uint8_t algoSvcId)
{
   uint8_t i;

   for (i = 0; i < SNS_SAM_MAX_ALGOS && sns_sam_algo_dbase[i] != NULL; i++)
   {
      if (sns_sam_algo_dbase[i]->serviceId == algoSvcId)
      {
         return i;
      }
   }

   SNS_SAM_DEBUG1(ERROR, DBG_SAM_GET_ALGO_INDX_ERR, algoSvcId);

   return SNS_SAM_INVALID_ID;
}

/*=========================================================================
  FUNCTION:  sns_sam_get_algo_handle
  =========================================================================*/
/*!
  @brief Get the handle to the specified algorithm in the algorithm database

  @param[i] algoSvcId: algorithm service id

  @return handle to the specified algorithm if found,
          NULL otherwise
*/
/*=======================================================================*/
sns_sam_algo_s* sns_sam_get_algo_handle(
   uint8_t algoSvcId)
{
   uint8_t i;

   for (i = 0; i < SNS_SAM_MAX_ALGOS && sns_sam_algo_dbase[i] != NULL; i++)
   {
      if (sns_sam_algo_dbase[i]->serviceId == algoSvcId)
      {
         return sns_sam_algo_dbase[i];
      }
   }
   SNS_SAM_DEBUG1(ERROR, DBG_SAM_GET_ALGO_INDX_ERR, algoSvcId);

   return NULL;
}

/*=========================================================================
  FUNCTION:  sns_sam_find_algo_inst
  =========================================================================*/
/*!
  @brief Searches the active algorithm database for an instance
  of an algorithm with specified configuration

  @param[i] algoSvcId: algorithm service id
  @param[i] algoCfgPtr: pointer to configuration of specified algorithm

  @return  Index to algorithm instance matching the specified configuration
  if successful, SNS_SAM_INVALID_ID otherwise
*/
/*=======================================================================*/
static uint8_t sns_sam_find_algo_inst(
   uint8_t algoSvcId,
   void *algoCfgPtr)
{
   uint8_t algoInstId, algoInstCnt;

   for (algoInstId = 0, algoInstCnt = 0;
        algoInstCnt < sns_sam_algo_inst_count &&
        algoInstId < SNS_SAM_MAX_ALGO_INSTS;
        algoInstId++)
   {
      sns_sam_algo_inst_s *algo_instance = sns_sam_algo_inst_dbase[algoInstId];
      if (algo_instance != NULL)
      {
         if ((algo_instance->serviceId == algoSvcId) &&
            (algoCfgPtr == NULL ||
             !SNS_OS_MEMCMP(algoCfgPtr,
                            algo_instance->configData.memPtr,
                            algo_instance->configData.memSize)) &&
            (algo_instance->clientReqCount < SNS_SAM_MAX_CLIENT_REQS_PER_ALGO_INST))
         {
            return algoInstId;
         }
         algoInstCnt++;
      }
   }
   return SNS_SAM_INVALID_ID;
}

/*=========================================================================
  FUNCTION:  sns_sam_delete_algo_inst
  =========================================================================*/
/*!
  @brief Deletes the specified algorithm instance

  @param[i] algoInstId: index to algorithm instance in database

  @return None
*/
/*=======================================================================*/
static void sns_sam_delete_algo_inst(
   uint8_t algoInstId)
{
   sns_sam_algo_inst_s* algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];

   sns_sam_mr_delete_algo_inst( algoInstPtr->mrAlgoConnHndl );
   algoInstPtr->mrAlgoConnHndl = NULL;

   //free memory reserved for this algorithm instance
   SNS_OS_FREE(algoInstPtr->configData.memPtr);
   algoInstPtr->configData.memPtr = NULL;
   algoInstPtr->configData.memSize = 0;

   SNS_OS_FREE(algoInstPtr->stateData.memPtr);
   algoInstPtr->stateData.memPtr = NULL;
   algoInstPtr->stateData.memSize = 0;

   SNS_OS_FREE(algoInstPtr->inputData.memPtr);
   algoInstPtr->inputData.memPtr = NULL;
   algoInstPtr->inputData.memSize = 0;

   SNS_OS_FREE(algoInstPtr->outputData.memPtr);
   algoInstPtr->outputData.memPtr = NULL;
   algoInstPtr->outputData.memSize = 0;
   algoInstPtr->outputData.timestamp = 0;

   SNS_OS_FREE(sns_sam_algo_inst_dbase[algoInstId]);
   sns_sam_algo_inst_dbase[algoInstId] = NULL;

   sns_sam_algo_inst_count--;
}

/*=========================================================================
  FUNCTION:  sns_sam_get_algo_inst
  =========================================================================*/
/*!
  @brief
  If an instance of the specified algorithm doesnt exist,
  creates an instance and initializes the reference count.
  If an instance with identical configuration exists,
  updates the reference count.

  @param[i] algoIndex: index to algorithm in the algorithm database
  @param[i] clientReqMsgPtr: pointer to client request message

  @return index of the algorithm instance in database if successful,
          SNS_SAM_INVALID_ID otherwise
*/
/*=======================================================================*/
static uint8_t sns_sam_get_algo_inst(
   uint8_t algoIndex,
   const void *clientReqMsgPtr)
{
   uint8_t algoInstId = SNS_SAM_INVALID_ID, instId;
   uint8_t algoSvcId = sns_sam_algo_dbase[algoIndex]->serviceId;
   void *algoDefCfgPtr = sns_sam_algo_dbase[algoIndex]->defConfigData.memPtr;
   void *algoCfgPtr = NULL;
   sns_sam_algo_inst_s* algoInstPtr = NULL;

   uint32_t memSize = sns_sam_algo_dbase[algoIndex]->defConfigData.memSize;
   if (memSize > 0)
   {
      algoCfgPtr = SNS_OS_MALLOC(SNS_SAM_DBG_MOD, memSize);
      SNS_ASSERT(algoCfgPtr != NULL);
      SNS_OS_MEMCOPY(algoCfgPtr, algoDefCfgPtr, memSize);

      //update algorithm configuration if specified by client
      if (NULL != clientReqMsgPtr)
      {
         sns_sam_update_algo_config(algoSvcId, clientReqMsgPtr, algoCfgPtr);
      }
   }

   /*Mag calibration algorithm by vendors support single instance
    so we need to support all clients with single algorithm instance*/
   if (algoSvcId == SNS_SAM_MAG_CAL_SVC_ID_V01)
   {
      algoInstId = sns_sam_find_algo_inst(algoSvcId, NULL);

      if ((algoInstId < SNS_SAM_MAX_ALGO_INSTS) && (algoCfgPtr != NULL))
      {
         uint32_t* sampleRatePtr = (uint32_t*)
            (sns_sam_algo_inst_dbase[algoInstId]->configData.memPtr);
         uint32_t* newSampleRatePtr = (uint32_t*)algoCfgPtr;
         uint8_t dataReqId, dataReqCnt, i;
         algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];

         if (*sampleRatePtr > 0 && *newSampleRatePtr > *sampleRatePtr)
         {
            for (i = 0; i<algoInstPtr->clientReqCount; i++)
            {
               uint8_t clientReqId = algoInstPtr->clientReqDbase[i];
               if (clientReqId < SNS_SAM_MAX_CLIENT_REQS)
               {
                  sns_sam_client_req_s* clientReqPtr =
                     sns_sam_client_req_dbase[clientReqId];
                  if (clientReqPtr != NULL &&
                      clientReqPtr->reportType == SNS_SAM_SYNC_REPORT)
                  {
                     clientReqPtr->reportType = SNS_SAM_PERIODIC_REPORT;
                     clientReqPtr->period =
                        sns_em_convert_sec_in_q16_to_localtick(
                           (uint32_t)FX_DIV_Q16(FX_ONE_Q16,*sampleRatePtr));

                     sns_sam_reg_timer(clientReqId, clientReqPtr->period);
                  }
               }
            }

            *sampleRatePtr = *newSampleRatePtr;

            for (dataReqId = 0, dataReqCnt = 0;
                 dataReqCnt < sns_sam_data_req_count &&
                 dataReqId < SNS_SAM_MAX_DATA_REQS;
                 dataReqId++)
            {
               if (sns_sam_data_req_dbase[dataReqId] != NULL)
               {
                  sns_sam_data_req_s *dataReq = sns_sam_data_req_dbase[dataReqId];

                  for (i = 0; i < dataReq->algoInstCount; i++)
                  {
                     if(dataReq->algoInstDbase[i] == algoInstId)
                     {
                        sns_sam_send_smgr_stop_req(dataReqId,
                                                   sns_sam_algo_inst_dbase[algoInstId]->serviceId);

                        dataReq->reportRate = (*sampleRatePtr >> 16);

                        sns_sam_send_smgr_start_req(dataReqId,
                                                    sns_sam_algo_inst_dbase[algoInstId]->serviceId);

                        /*updated algo instance - exit both for loops*/
                        dataReqId = SNS_SAM_MAX_DATA_REQS;
                        break;
                     }
                  }
                  dataReqCnt++;
               }
            }
         }
      }
   }
   else
   {
      //find algorithm instance with same configuration
      algoInstId = sns_sam_find_algo_inst(algoSvcId, algoCfgPtr);
   }

   //check if new algorithm instance is needed and can be created
   if (algoInstId < SNS_SAM_MAX_ALGO_INSTS ||
       sns_sam_algo_inst_count >= SNS_SAM_MAX_ALGO_INSTS)
   {
      if (algoCfgPtr != NULL)
      {
         SNS_OS_FREE(algoCfgPtr);
      }
      return algoInstId;
   }

   //create algorithm instance
   for (instId = 0; instId < SNS_SAM_MAX_ALGO_INSTS; instId++)
   {
      if (sns_sam_algo_inst_dbase[instId] == NULL)
      {
         uint8_t i;
         void *statePtr = NULL;
         memSize = sizeof(sns_sam_algo_inst_s);

         //create algorithm instance
         sns_sam_algo_inst_dbase[instId] = SNS_OS_MALLOC(SNS_SAM_DBG_MOD, memSize);
         SNS_ASSERT(sns_sam_algo_inst_dbase[instId] != NULL);
         algoInstPtr = sns_sam_algo_inst_dbase[instId];

         //initialize the algorithm instance
         algoInstPtr->clientReqCount = 0;
         // Initialize motion state to true to indicate that sensor data will be
         // requested when the algorithm is enabled
         algoInstPtr->motion_state = SNS_SAM_MOTION_MOVE_V01;
         for (i = 0; i < SNS_SAM_MAX_CLIENT_REQS_PER_ALGO_INST; i++)
         {
            algoInstPtr->clientReqDbase[i] = SNS_SAM_INVALID_ID;
         }

         algoInstPtr->stateData.memPtr = NULL;
         algoInstPtr->inputData.memPtr = NULL;
         algoInstPtr->outputData.memPtr = NULL;

         algoInstPtr->configData.memPtr = algoCfgPtr;
         algoInstPtr->configData.memSize =
            sns_sam_algo_dbase[algoIndex]->defConfigData.memSize;

         memSize =
            sns_sam_algo_dbase[algoIndex]->algoApi.
            sns_sam_algo_mem_req(algoCfgPtr);
         if (memSize > 0)
         {
            algoInstPtr->stateData.memPtr = SNS_OS_MALLOC(SNS_SAM_DBG_MOD, memSize);
            SNS_ASSERT(algoInstPtr->stateData.memPtr != NULL);
            SNS_OS_MEMZERO(algoInstPtr->stateData.memPtr, memSize);
         }
         algoInstPtr->stateData.memSize = memSize;

         memSize = sns_sam_algo_dbase[algoIndex]->defInputDataSize;
         if (memSize > 0)
         {
            algoInstPtr->inputData.memPtr = SNS_OS_MALLOC(SNS_SAM_DBG_MOD, memSize);
            SNS_ASSERT(algoInstPtr->inputData.memPtr != NULL);
            SNS_OS_MEMZERO(algoInstPtr->inputData.memPtr, memSize);
         }
         algoInstPtr->inputData.memSize = memSize;

         memSize = sns_sam_algo_dbase[algoIndex]->defOutputDataSize;
         if (memSize > 0)
         {
            algoInstPtr->outputData.memPtr = SNS_OS_MALLOC(SNS_SAM_DBG_MOD, memSize);
            SNS_ASSERT(algoInstPtr->outputData.memPtr != NULL);
            SNS_OS_MEMZERO(algoInstPtr->outputData.memPtr, memSize);
         }
         algoInstPtr->outputData.memSize = memSize;
         algoInstPtr->outputData.timestamp = 0;

         // use default duty cycle config in sns_sam_algo_dbase
         algoInstPtr->dutycycleOnPercent =
            sns_sam_process_client_duty_cycle_req(algoSvcId, clientReqMsgPtr);
         algoInstPtr->dutycycleOnDuration =
            sns_sam_get_algo_report_period(algoSvcId,clientReqMsgPtr,algoCfgPtr);
         algoInstPtr->dutycycleStateOn = true;
         algoInstPtr->dutycycleTimeoutCount = 0;
         algoInstPtr->dutycycleTimerSource = (int16_t)SNS_SAM_DUTY_CYCLE_TIMER_SOURCE_NONE;

         algoInstPtr->cumulativeDataReq = false;

         statePtr = sns_sam_algo_dbase[algoIndex]->algoApi.
            sns_sam_algo_reset(algoInstPtr->configData.memPtr,
                               algoInstPtr->stateData.memPtr);
         if (statePtr == NULL)
         {
            SNS_SAM_DEBUG0(ERROR, DBG_SAM_ENABLE_ALGO_STATE_NULL);
         }

         algoInstPtr->serviceId = algoSvcId;

         algoInstId = instId;

         sns_sam_algo_inst_count++;

         if( SNS_SUCCESS != sns_sam_mr_init_algo_inst( &algoInstPtr->mrAlgoConnHndl, algoSvcId ) )
         {
            SNS_ASSERT(algoInstPtr->mrAlgoConnHndl != NULL);
         }

         //Log algorithm configuration
         sns_sam_log_algo_config(algoInstId, algoInstPtr,algoIndex);

         SNS_SAM_DEBUG3(LOW, DBG_SAM_ALGO_MEM_INFO,
                        algoInstPtr->serviceId,
                        sizeof(sns_sam_algo_inst_s),
                        algoInstPtr->configData.memSize);
         SNS_SAM_DEBUG3(LOW, DBG_SAM_ALGO_STATE_MEM_INFO,
                        algoInstPtr->stateData.memSize,
                        algoInstPtr->inputData.memSize,
                        algoInstPtr->outputData.memSize);

         break;
      }
   }

   return algoInstId;
}

/*=========================================================================
  FUNCTION:  sns_sam_update_duty_cycle_config_client_removal
  =========================================================================*/
/*!
  @brief
  updates the duty cycle config setting when a client is removed

  @param[i] deletedClientReqId: index of removed client request
  @param[i] algoInstId: index of algorithm instance
  @param[i] algoInstPtr: pointer to algo instance

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_update_duty_cycle_config_client_removal(
   uint8_t deletedClientReqId,
   uint8_t algoInstId,
   sns_sam_algo_inst_s *algoInstPtr)
{
   UNREFERENCED_PARAMETER(algoInstId);
   if ( (algoInstPtr->dutycycleTimerSource != SNS_SAM_DUTY_CYCLE_TIMER_SOURCE_NONE) &&
       (algoInstPtr->dutycycleTimerSource == deletedClientReqId) )
   {
      // algorithm instance uses dutycyling and the deleted client request
      // is the currently employed duty cycle timer source
      uint8_t j=0,k=0,clientReqId;

      uint8_t dutycycleOnDurationMin, dutycycleOnPercentMax;
      uint8_t clientIdList[SNS_SAM_MAX_CLIENT_REQS_PER_ALGO_INST];

      // find max requested duty cycle among all clients for the algo instance
      dutycycleOnPercentMax =
         sns_sam_client_req_dbase[algoInstPtr->clientReqDbase[0]]->dutycycleOnPercent;
      for (j=0; j<algoInstPtr->clientReqCount; j++)
      {
         clientReqId = algoInstPtr->clientReqDbase[j];
         if (sns_sam_client_req_dbase[clientReqId]->dutycycleOnPercent >
             dutycycleOnPercentMax)
         {
            dutycycleOnPercentMax =
               sns_sam_client_req_dbase[clientReqId]->dutycycleOnPercent;
         }
      }

      // find out if there are multiple clients requesting the same max duty cycle
      // on percentage
      k = 0;
      for (j=0; j<algoInstPtr->clientReqCount; j++)
      {
         clientReqId = algoInstPtr->clientReqDbase[j];
         if (sns_sam_client_req_dbase[clientReqId]->dutycycleOnPercent ==
             dutycycleOnPercentMax)
         {
            clientIdList[k]=clientReqId;
            k = k+1;
         }
      }

      // find the minimum requested On Duration among all the clients requsting the
      // max on percentage
      dutycycleOnDurationMin = sns_sam_client_req_dbase[clientIdList[0]]->period;
      clientReqId = clientIdList[0];
      for (j=1; j<k; j++)
      {
         if (sns_sam_client_req_dbase[clientIdList[j]]->period < dutycycleOnDurationMin)
         {
            dutycycleOnDurationMin = sns_sam_client_req_dbase[clientIdList[j]]->period;
            clientReqId = clientIdList[j];
         }
      }

      // reset duty cycle configuration for algo instance
      algoInstPtr->dutycycleOnDuration = dutycycleOnDurationMin;
      algoInstPtr->dutycycleOnPercent = dutycycleOnPercentMax;
      algoInstPtr->dutycycleTimerSource = clientReqId;
   }
   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_update_duty_cycle_config_client_add
  =========================================================================*/
/*!
  @brief
  updates the duty cycle config setting when a client is added

  @param[i] algoInstId: index of algo instance data base
  @param[i] algoIndex: index of algorithm in database
  @param[i] clientReqId: index to client request database

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_update_duty_cycle_config_client_add(
   uint8_t algoInstId,
   uint8_t algoIndex,
   uint8_t clientReqId)
{
   bool dutycycleUpdateConfig = false;
   uint8_t dutycycleOnPercent =
      sns_sam_client_req_dbase[clientReqId]->dutycycleOnPercent;
   uint32_t dutycycleOnDuration =
      sns_sam_client_req_dbase[clientReqId]->period;
   sns_sam_algo_inst_s *algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];

   UNREFERENCED_PARAMETER(algoIndex);

   if(algoInstPtr->clientReqCount == 1)
   {
      // first client request for algo instance
      dutycycleUpdateConfig = true;
   }
   else if (dutycycleOnPercent > algoInstPtr->dutycycleOnPercent)
   {
      dutycycleUpdateConfig = true;
   }
   else if (dutycycleOnPercent == algoInstPtr->dutycycleOnPercent)
   {
      if (dutycycleOnDuration < algoInstPtr->dutycycleOnDuration)
      {
         dutycycleUpdateConfig = true;
      }
   }

   // update algo instance database
   if (dutycycleUpdateConfig)
   {
      algoInstPtr->dutycycleOnPercent = dutycycleOnPercent;
      algoInstPtr->dutycycleOnDuration = dutycycleOnDuration;
      algoInstPtr->dutycycleStateOn = true;
      algoInstPtr->dutycycleTimeoutCount = 0;
      algoInstPtr->dutycycleTimerSource = clientReqId;
   }

   if((algoInstPtr->dutycycleOnPercent >= 100) ||
      (algoInstPtr->dutycycleOnDuration <= 0))
   {
      // disable duty cycling
      algoInstPtr->dutycycleTimerSource =
         (int16_t)SNS_SAM_DUTY_CYCLE_TIMER_SOURCE_NONE;
   }
   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_is_sensor_report_rate_available
  =========================================================================*/
/*!
  @brief    Detects sensor report rate availability in Registry

  @return   'true' if sensor report rate is expected to be available;
            'false' otherwise.
*/
/*=======================================================================*/
bool sns_sam_is_sensor_report_rate_available()
{
   return sns_sam_sensor_report_rate_available;
}

/*=========================================================================
  FUNCTION:  sns_sam_enable_algo
  =========================================================================*/
/*!
  @brief
  enables specified algorithm with the specified configuration

  @param[i] algoIndex: index to algorithm in the algorithm database
  @param[i] clientReqMsgPtr: pointer to client request message
  @param[i] mrClntId: MR Client ID

  @return index of the algorithm instance in database if successful,
          SNS_SAM_INVALID_ID otherwise
*/
/*=======================================================================*/
static uint8_t sns_sam_enable_algo(
   uint8_t              algoIndex,
   const void           *clientReqMsgPtr,
   sns_sam_mr_conn_hndl mrClntId)
{
   uint8_t algoInstId, clientReqId;
   uint8_t algoSvcId = sns_sam_algo_dbase[algoIndex]->serviceId;
   sns_sam_algo_inst_s * algoInstPtr = NULL;

   algoInstId = sns_sam_get_algo_inst(algoIndex, clientReqMsgPtr);
   if( algoInstId >= SNS_SAM_MAX_ALGO_INSTS )
   {
      return SNS_SAM_INVALID_ID;
   }

   algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];
   if( algoInstPtr == NULL )
   {
      return SNS_SAM_INVALID_ID;
   }

   //add client request
   clientReqId = sns_sam_add_client_req(clientReqMsgPtr,
                                        algoInstId,
                                        mrClntId,
                                        false);
   if (clientReqId >= SNS_SAM_MAX_CLIENT_REQS)
   {
      return SNS_SAM_INVALID_ID;
   }

   //Register client with algorithm
   algoInstPtr->clientReqDbase[algoInstPtr->clientReqCount] =
      clientReqId;
   algoInstPtr->clientReqCount++;

   //send latest report to new client
   if (algoInstPtr->clientReqCount > 1)
   {
      sns_sam_algo_rpt_s *algoRptPtr = sns_sam_get_algo_report(algoInstId);
      if (NULL != algoRptPtr)
      {
         //process algorithm report indication to client
         sns_sam_process_algo_report(clientReqId, algoRptPtr, mrClntId);
      }
   }

   sns_sam_update_duty_cycle_config_client_add(algoInstId,
                                               algoIndex,
                                               clientReqId);

   // Register client with algorithm instance
   if( sns_sam_algo_dbase[algoIndex]->algoApi.sns_sam_algo_register_client )
   {
      void * statePtr = algoInstPtr->stateData.memPtr;
      void * outputPtr = algoInstPtr->outputData.memPtr;
      uint8_t clientId = sns_sam_mr_get_client_id( sns_sam_client_req_dbase[clientReqId]->mrClientId );
      uint32_t timestamp = sns_em_get_timestamp();
      sns_sam_algo_dbase[algoIndex]->algoApi.
         sns_sam_algo_register_client(clientId, true, statePtr, outputPtr, timestamp);
   }

   if( !sns_sam_algo_dbase[algoIndex]->dataSyncRequired )
   {
      // If algorithm has no synchronous dependencies on other algorithms,
      // request for sensor data now. For algorithms that have dependencies,
      // sensor data will be requested after aggregating requests from all
      // dependent algorithms
      int sensorCnt = 0;
      uint32_t reportRate = 0;
      sns_sam_sensor_req_s sensorReq[SNS_SAM_MAX_SENSORS_PER_DATA_REQ];

      sensorCnt = sns_sam_get_sensor_requirements(algoInstId,
                                                  algoSvcId,
                                                  algoInstPtr->configData.memPtr,
                                                  &reportRate,
                                                  sensorReq);
      //send request to sensor manager
      if( sensorCnt > 0 )
      {
         sns_sam_start_sensor_data(algoInstId, reportRate, sensorCnt, sensorReq);
      }
   }

   SNS_SAM_DEBUG3(MEDIUM, DBG_SAM_ENABLE_ALGO,
                  sns_sam_algo_dbase[algoIndex]->serviceId,
                  clientReqId,
                  algoInstId);

   return algoInstId;
}

/*=========================================================================
  FUNCTION:  sns_sam_disable_algo
  =========================================================================*/
/*!
  @brief Decrements the reference count of the algorithm instance.
         If count is zero, deletes an instance and frees up its resources

  @param[i] clientReqId: index to the client request in database

  @return Sensors error code
*/
/*=======================================================================*/
sns_err_code_e sns_sam_disable_algo(
   uint8_t clientReqId)
{
   uint8_t algoInstId, algoIndex, i;
   sns_sam_algo_inst_s *algoInstPtr;
   void * statePtr;
   void * outputPtr;

   algoInstId = sns_sam_client_req_dbase[clientReqId]->algoInstId;

   if (algoInstId >= SNS_SAM_MAX_ALGO_INSTS)
   {
      SNS_SAM_DEBUG1(ERROR, DBG_SAM_DISABLE_ALGO_INSTANCE_ERR, algoInstId);
      return SNS_ERR_FAILED;
   }

   algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];
   if (algoInstPtr == NULL || algoInstPtr->clientReqCount == 0)
   {
      SNS_SAM_DEBUG1(ERROR, DBG_SAM_DISABLE_ALGO_INSTANCE_ERR, algoInstId);
      return SNS_ERR_FAILED;
   }

   for (i = 0; i<algoInstPtr->clientReqCount; i++)
   {
      if (algoInstPtr->clientReqDbase[i] == clientReqId)
      {
         algoInstPtr->clientReqCount--;
         algoInstPtr->clientReqDbase[i] =
            algoInstPtr->clientReqDbase[algoInstPtr->clientReqCount];
         algoInstPtr->clientReqDbase[algoInstPtr->clientReqCount] =
            SNS_SAM_INVALID_ID;

         // Deregister client from algorithm instance
         algoIndex = sns_sam_get_algo_index(algoInstPtr->serviceId);
         if( algoIndex < SNS_SAM_MAX_ALGOS &&
             sns_sam_algo_dbase[algoIndex] != NULL &&
             sns_sam_algo_dbase[algoIndex]->algoApi.sns_sam_algo_register_client )
         {
            statePtr = algoInstPtr->stateData.memPtr;
            outputPtr = algoInstPtr->outputData.memPtr;
            sns_sam_algo_dbase[algoIndex]->algoApi.
               sns_sam_algo_register_client(
                   sns_sam_mr_get_client_id(sns_sam_client_req_dbase[clientReqId]->mrClientId),
                   false, statePtr, outputPtr, 0);
         }

         if (algoInstPtr->clientReqCount > 0)
         {
            sns_sam_delete_client_req(clientReqId);

            sns_sam_update_duty_cycle_config_client_removal(clientReqId,
                                                            algoInstId,
                                                            algoInstPtr);

            return SNS_SUCCESS;
         }

         break;
      }
   }

   algoIndex = sns_sam_get_algo_index(algoInstPtr->serviceId);
   if (algoIndex >= SNS_SAM_MAX_ALGOS || sns_sam_algo_dbase[algoIndex] == NULL)
   {
      return SNS_ERR_FAILED;
   }

   sns_sam_algo_dbase[algoIndex]->algoApi.sns_sam_algo_reset(
      algoInstPtr->configData.memPtr,
      algoInstPtr->stateData.memPtr);

   //stop sensor data
   sns_sam_stop_sensor_data(algoInstId);

   SNS_SAM_DEBUG3(MEDIUM, DBG_SAM_DISABLE_ALGO,
                  sns_sam_algo_dbase[algoIndex]->serviceId,
                  clientReqId,
                  algoInstId);

   sns_sam_delete_client_req(clientReqId);

   //delete the algo instance
   sns_sam_delete_algo_inst(algoInstId);

   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_send_algo_enable_msg
  =========================================================================*/
/*!
  @brief Send the algorithm enable request message

  @param[i] algoInstId: dependor algorithm instance id
  @param[i] algoServiceId: dependee algorithm service id
  @param[i] clientReqMsgPtr: pointer to client request message
  @param[i] txnId: transaction id
  @param[i] mrClntId: Client ID

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_send_algo_enable_msg(
   uint8_t                       algoInstId,
   uint8_t                       algoServiceId,
   const void                    *clientReqMsgPtr,
   uint8_t                       algoDepId,
   sns_sam_mr_algo_conn_hndl     *mrAlgoConnHndl)
{
   sns_err_code_e err = SNS_ERR_FAILED;
   void* msgPtr = NULL;
   sns_sam_algo_inst_s *algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];

   sns_smr_header_s msgHdr;

   msgHdr.src_module = SNS_SAM_MODULE;
   msgHdr.msg_type = SNS_SMR_MSG_TYPE_REQ;
   msgHdr.priority = SNS_SMR_MSG_PRI_HIGH;
   msgHdr.msg_id = SNS_SAM_ALGO_ENABLE_REQ;

   msgHdr.svc_num = algoServiceId;
   /* use external client id to detect dependent algorithm instance */
   msgHdr.ext_clnt_id = algoInstId;
   msgHdr.txn_id = algoDepId;

   msgPtr = sns_sam_gen_algo_enable_msg(algoInstPtr,
                                        clientReqMsgPtr,
                                        &msgHdr);

   err = sns_sam_mr_send(msgPtr, mrAlgoConnHndl);

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_send_algo_disable_msg
  =========================================================================*/
/*!
  @brief Send the algorithm enable request message

  @param[i] algoSvcId: algorithm service id
  @param[i] algoInstId: algorithm instance id
  @param[i] extClientId: external client id
  @param[i] mrClntId: MR Client ID

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_send_algo_disable_msg(
   uint8_t                       algoSvcId,
   uint8_t                       algoInstId,
   uint8_t                       extClientId,
   sns_sam_mr_algo_conn_hndl     *mrAlgoConnHndl)
{
   sns_err_code_e err = SNS_ERR_FAILED;
   void* msgPtr = NULL;

   sns_smr_header_s msgHdr;

   msgHdr.src_module = SNS_SAM_MODULE;
   msgHdr.msg_type = SNS_SMR_MSG_TYPE_REQ;
   msgHdr.priority = SNS_SMR_MSG_PRI_HIGH;
   msgHdr.msg_id = SNS_SAM_ALGO_DISABLE_REQ;

   msgHdr.svc_num = algoSvcId;

   /* use external client id to detect dependent algorithm instance */
   msgHdr.ext_clnt_id = extClientId;
   msgHdr.txn_id = 0;

   msgPtr = sns_sam_gen_algo_disable_msg(algoInstId, &msgHdr);

   err = sns_sam_mr_send(msgPtr, mrAlgoConnHndl);

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_enable_sync_dependent_algos
  =========================================================================*/
/*!
  @brief    Recursively enables all dependent algorithms with the
            specified configuration

  @param[i] currAlgoInstId: algorithm instance ID
  @param[i] algo: pointer to algorithm structure
  @param[i] msgPtr: pointer to client request message
  @param[i/o] totalSensorReq: pointer to aggregated sensor request
  @param[i/o] totalSensorCount: pointer to num elements in sensor request

  @return   SNS_SUCCESS if algo is instantiated properly,
            SNS_ERR_FAILED otherwise

*/
/*=======================================================================*/
static sns_err_code_e sns_sam_enable_sync_dependent_algos(
   uint8_t currAlgoInstId,
   const sns_sam_algo_s* algo,
   const void* msgPtr,
   sns_sam_sensor_req_s totalSensorReq[],
   uint32_t* totalSensorCount)
{
   uint8_t depAlgoIndex, sensorReqIndex, sensorIndex, algoInstIndex, algoIndex, algoInstId, clientReqId;
   uint32_t reportRate;
   sns_sam_sensor_req_s sensorReq[SNS_SAM_MAX_SENSORS_PER_DATA_REQ];
   int sensorReqCnt = 0;

   if( !sns_sam_algo_inst_dbase[currAlgoInstId] )
   {
      SNS_SAM_PRINTF1( ERROR, "parent algo inst id %d unknown", currAlgoInstId );
      return SNS_ERR_FAILED;
   }

   if (sns_sam_algo_inst_dbase[currAlgoInstId]->clientReqCount >= 1)
   {
      for(depAlgoIndex = 0; depAlgoIndex < algo->algoDepCount; depAlgoIndex++)
      {
         void* enableMsgPtr;
         sns_smr_header_s enableMsgHdr;
         sns_err_code_e err;
         bool directReportReq = true;

         enableMsgHdr.src_module = SNS_SAM_MODULE;
         enableMsgHdr.msg_type = SNS_SMR_MSG_TYPE_REQ;
         enableMsgHdr.priority = SNS_SMR_MSG_PRI_HIGH;
         enableMsgHdr.msg_id = SNS_SAM_ALGO_ENABLE_REQ;

         enableMsgHdr.svc_num = algo->algoDepDbase[depAlgoIndex];
         enableMsgHdr.ext_clnt_id = currAlgoInstId;
         enableMsgHdr.txn_id = depAlgoIndex;
         enableMsgPtr = sns_sam_gen_algo_enable_msg(
                              sns_sam_algo_inst_dbase[currAlgoInstId],
                              msgPtr, &enableMsgHdr);

         SNS_SAM_PRINTF2( LOW, "SAM enabling sync dep algo svc %d for parent svc %d",
            algo->algoDepDbase[depAlgoIndex], sns_sam_algo_inst_dbase[currAlgoInstId]->serviceId );

         if(enableMsgPtr == NULL)
         {
            continue;
         }

         algoIndex = sns_sam_get_algo_index(algo->algoDepDbase[depAlgoIndex]);
         if (algoIndex >= SNS_SAM_MAX_ALGOS)
         {
            if(enableMsgPtr != NULL)
            {
               sns_smr_msg_free(enableMsgPtr);
            }
            return SNS_ERR_FAILED;
         }

         algoInstId = sns_sam_get_algo_inst(algoIndex, enableMsgPtr);
         if (algoInstId >= SNS_SAM_MAX_ALGO_INSTS ||
             sns_sam_algo_inst_dbase[algoInstId] == NULL)
         {
            SNS_SAM_PRINTF1( ERROR, "SAM unknown algo inst id %d", algoInstId );
            if(enableMsgPtr != NULL)
            {
               sns_smr_msg_free(enableMsgPtr);
            }
            return SNS_ERR_FAILED;
         }
         if (sns_sam_algo_inst_dbase[algoInstId]->clientReqCount >=
            SNS_SAM_MAX_CLIENT_REQS_PER_ALGO_INST)
         {
            SNS_SAM_PRINTF1( ERROR, "SAM - no space for more client reqs in algo inst %d", algoInstId );
            if(enableMsgPtr != NULL)
            {
               sns_smr_msg_free(enableMsgPtr);
            }
            return SNS_ERR_FAILED;
         }

         clientReqId = sns_sam_add_client_req(enableMsgPtr, algoInstId, currAlgoInstId, directReportReq);
         if (clientReqId >= SNS_SAM_MAX_CLIENT_REQS)
         {
            SNS_SAM_PRINTF1( ERROR, "SAM - add client req failed for algo inst %d", algoInstId );
            if (enableMsgPtr != NULL)
            {
               sns_smr_msg_free(enableMsgPtr);
            }
            return SNS_ERR_FAILED;
         }

         if (sns_sam_algo_inst_dbase[currAlgoInstId]->clientReqCount == 1)
         {
            sns_sam_algo_inst_dbase[algoInstId]->
            clientReqDbase[sns_sam_algo_inst_dbase[algoInstId]->clientReqCount] =
              clientReqId;
            sns_sam_algo_inst_dbase[algoInstId]->clientReqCount++;
         }

         sensorReqCnt = sns_sam_get_sensor_requirements(algoInstId,
                                                  sns_sam_algo_inst_dbase[algoInstId]->serviceId,
                                                  sns_sam_algo_inst_dbase[algoInstId]->configData.memPtr,
                                                  &reportRate,
                                                  sensorReq);

         // Adds new sensors to data request
         if (sensorReqCnt > 0)
         {
            for(sensorReqIndex = 0; sensorReqIndex < sensorReqCnt; sensorReqIndex++)
            {
               // Checks if the sensor already exists
               for(sensorIndex = 0; sensorIndex < *totalSensorCount; sensorIndex++)
               {
                  if( (sensorReq[sensorReqIndex].sensorId == totalSensorReq[sensorIndex].sensorId) &&
                      (sensorReq[sensorReqIndex].dataType == totalSensorReq[sensorIndex].dataType) &&
                      (sensorReq[sensorReqIndex].sampleRate == totalSensorReq[sensorIndex].sampleRate) &&
                      (sensorReq[sensorReqIndex].sampleQual == totalSensorReq[sensorIndex].sampleQual))
                  {
                     break;
                  }
               }

               if(sensorIndex >= *totalSensorCount)
               {
                  if( *totalSensorCount >= SNS_SAM_MAX_SENSORS_PER_DATA_REQ)
                  {
                     SNS_SAM_PRINTF0( ERROR, "SAM cannot add more sensor reqs to cumulative data req" );
                     if(enableMsgPtr != NULL)
                     {
                        sns_smr_msg_free(enableMsgPtr);
                     }
                     return SNS_ERR_FAILED;
                  }

                  totalSensorReq[*totalSensorCount].sensorId = sensorReq[sensorReqIndex].sensorId;
                  totalSensorReq[*totalSensorCount].dataType = sensorReq[sensorReqIndex].dataType;
                  totalSensorReq[*totalSensorCount].sampleRate = sensorReq[sensorReqIndex].sampleRate;
                  totalSensorReq[*totalSensorCount].sampleQual = sensorReq[sensorReqIndex].sampleQual;
                  totalSensorReq[*totalSensorCount].algoInstIds[0] = algoInstId;
                  (*totalSensorCount)++;
               }
               else
               {
                  // Sensor already exists - check if algo instance does too
                  for( algoInstIndex = 0; algoInstIndex < SNS_SAM_MAX_ALGO_INSTS_PER_SENSOR; ++algoInstIndex )
                  {
                     if( totalSensorReq[sensorIndex].algoInstIds[algoInstIndex] == algoInstId )
                     {
                        break;
                     }
                     if( totalSensorReq[sensorIndex].algoInstIds[algoInstIndex] == SNS_SAM_INVALID_ID )
                     {
                        totalSensorReq[sensorIndex].algoInstIds[algoInstIndex] = algoInstId;
                        break;
                     }
                  }
                  if( algoInstIndex == SNS_SAM_MAX_ALGO_INSTS_PER_SENSOR )
                  {
                     // Cannot support another algorithm instance
                     SNS_SAM_PRINTF0( ERROR, "SAM cannot support more algo insts in sensor req" );
                     if(enableMsgPtr != NULL)
                     {
                        sns_smr_msg_free(enableMsgPtr);
                     }
                     return SNS_ERR_FAILED;
                  }
               }
            }
         }

         // Enables other algos by recursion
         err = sns_sam_enable_sync_dependent_algos(algoInstId,
                                                   sns_sam_algo_dbase[algoIndex],
                                                   enableMsgPtr,
                                                   totalSensorReq,
                                                   totalSensorCount);
         if(enableMsgPtr != NULL)
         {
            sns_smr_msg_free(enableMsgPtr);
         }

         if( err != SNS_SUCCESS )
         {
            return err;
         }

         if (enableMsgHdr.src_module == SNS_SAM_MODULE &&
             sns_sam_algo_inst_dbase[currAlgoInstId]->clientReqCount == 1)
         {
            // store algo instance id in dependent algorithm request database
            if (enableMsgHdr.ext_clnt_id < SNS_SAM_MAX_ALGO_INSTS &&
               enableMsgHdr.txn_id < SNS_SAM_MAX_ALGO_DEPS &&
               sns_sam_algo_inst_dbase[enableMsgHdr.ext_clnt_id] != NULL)
            {
               sns_sam_algo_inst_dbase[enableMsgHdr.ext_clnt_id]->algoReqDbase[enableMsgHdr.txn_id]
                  = algoInstId;
            }
            else
            {
               return SNS_ERR_FAILED;
            }
         }
      } /* for all dep algos */
   }

   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_enable_dependent_algos
  =========================================================================*/
/*!
  @brief Enables algorithms that are dependent on a given algorithm

  @param[i] algoInstId: Algorithm Instance ID
  @param[i] algoIndex: Index of algorithm in the algorithm database
  @param[i] clientReqMsgPtr: Pointer to client request message
  @param[i] mrClntId: external client ID set by SAM MR

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_enable_dependent_algos(
   uint8_t               algoInstId,
   uint8_t               algoIndex,
   const void           *clientReqMsgPtr,
   sns_sam_mr_conn_hndl  mrClntId
   )
{
   sns_err_code_e err = SNS_SUCCESS;

   if( algoIndex >= SNS_SAM_MAX_ALGOS ||
       sns_sam_algo_dbase[algoIndex] == NULL ||
       algoInstId  >= SNS_SAM_MAX_ALGO_INSTS ||
       sns_sam_algo_inst_dbase[algoInstId] == NULL ||
       clientReqMsgPtr == NULL )
   {
      return SNS_ERR_BAD_PARM;
   }

   if( sns_sam_algo_dbase[algoIndex]->dataSyncRequired )
   {
      // Algorithm requires synchronous sensor and dependent algo outputs
      // Enable dependent algorithms recursively for synchronization
      sns_sam_algo_inst_s * algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];
      sns_sam_sensor_req_s sensorReq[SNS_SAM_MAX_SENSORS_PER_DATA_REQ];
      uint32_t i, j, sensorReqCnt = 0;
      uint32_t reportRate = 0;

      for( i = 0; i < SNS_SAM_MAX_SENSORS_PER_DATA_REQ; ++i )
      {
         for( j = 0; j < SNS_SAM_MAX_ALGO_INSTS_PER_SENSOR; ++j )
         {
            sensorReq[i].algoInstIds[j] = SNS_SAM_INVALID_ID;
         }
      }

      sensorReqCnt = sns_sam_get_sensor_requirements(algoInstId,
                                                     algoInstPtr->serviceId,
                                                     algoInstPtr->configData.memPtr,
                                                     &reportRate,
                                                     sensorReq);

      for( i = 0; i < sensorReqCnt; ++i )
      {
         sensorReq[i].algoInstIds[0] = algoInstId;
      }

      // Enable dependent algorithms and aggregate data request
      err = sns_sam_enable_sync_dependent_algos( algoInstId, sns_sam_algo_dbase[algoIndex],
          clientReqMsgPtr, sensorReq, &sensorReqCnt );

      SNS_SAM_PRINTF2( LOW, "SAM: cumulative sensor requirements - count %d, err %d",
          sensorReqCnt, err );
      for( i = 0; i < sensorReqCnt; ++i )
      {
         SNS_SAM_PRINTF3( LOW, "SAM: Item %d Sensor id %d Sample Rate %d", i,
             sensorReq[i].sensorId, sensorReq[i].sampleRate );
         SNS_SAM_PRINTF2( LOW, "SAM: Algo inst 1 %d Algo inst 2 %d",
             sensorReq[i].algoInstIds[0], sensorReq[i].algoInstIds[1] );
      }

      if( err == SNS_SUCCESS && sensorReqCnt > 0 )
      {
         // Request for sensor data
         uint8_t dataReqId;
         sns_smr_header_s msgHdr;
         uint8_t clientReqId;

         msgHdr.svc_num = algoInstPtr->serviceId;
         clientReqId = sns_sam_find_client_req( &msgHdr, mrClntId );

         if( clientReqId < SNS_SAM_MAX_CLIENT_REQS &&
             sns_sam_client_req_dbase[clientReqId] != NULL &&
             sns_sam_client_req_dbase[clientReqId]->reportType == SNS_SAM_PERIODIC_REPORT )
         {
            // Periodic reporting - use client report rate as sensor report rate
            float report_rate = (float)(SNS_SAM_USEC_PER_SEC / sns_em_convert_localtick_to_usec(sns_sam_client_req_dbase[clientReqId]->period));
            reportRate = FX_FLTTOFIX_Q16(report_rate);
         }

         SNS_SAM_PRINTF1( LOW, "SAM: cumulative data request at report rate %d", reportRate );
         algoInstPtr->cumulativeDataReq = true;
         dataReqId = sns_sam_start_sensor_data(algoInstId, reportRate,
             sensorReqCnt, sensorReq);
         if( dataReqId == SNS_SAM_INVALID_ID )
         {
            return SNS_ERR_FAILED;
         }
      }
   }
   else
   {
      // Send messages to enable dependent algorithms
      int32_t i;
      for (i = 0; i < sns_sam_algo_dbase[algoIndex]->algoDepCount; i++)
      {
         err = sns_sam_send_algo_enable_msg(algoInstId,
                                            sns_sam_algo_dbase[algoIndex]->algoDepDbase[i],
                                            clientReqMsgPtr,
                                            i,
                                            sns_sam_algo_inst_dbase[algoInstId]->mrAlgoConnHndl);

         if (err != SNS_SUCCESS)
         {
            return err;
         }
      }
   }
   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_disable_sync_dependent_algos
  =========================================================================*/
/*!
  @brief    Recursively disables all dependent algorithms from the bottom up

  @param[i] algoInstId: algorithm instance ID
  @param[i] algoIndex: index of algorithm in algo dbase
  @param[i] parentAlgoInstId: parent algorithm's instance ID

  @return   SNS_SUCCESS if algo is disabled properly,
            SNS_ERR_FAILED otherwise

*/
/*=======================================================================*/
static sns_err_code_e sns_sam_disable_sync_dependent_algos(
   uint8_t algoInstId,
   uint8_t algoIndex,
   uint8_t parentAlgoInstId)
{
   sns_err_code_e err= SNS_SUCCESS;
   uint8_t i, childAlgoInstId, childAlgoIndex, clientReqId;
   sns_sam_algo_inst_s *algoInstPtr;
   void * statePtr;
   void * outputPtr;
   sns_smr_header_s msgHdr;

   SNS_SAM_PRINTF1( LOW, "SAM disabling sync dep children of algo inst id %d", algoInstId );

   if( algoIndex >= SNS_SAM_MAX_ALGOS ||
       sns_sam_algo_dbase[algoIndex] == NULL ||
       algoInstId  >= SNS_SAM_MAX_ALGO_INSTS ||
       sns_sam_algo_inst_dbase[algoInstId] == NULL )
   {
      return SNS_ERR_BAD_PARM;
   }
   if (algoInstId >= SNS_SAM_MAX_ALGO_INSTS)
   {
      SNS_SAM_DEBUG1(ERROR, DBG_SAM_DISABLE_ALGO_INSTANCE_ERR, algoInstId);
      return SNS_ERR_FAILED;
   }

   algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];
   if (algoInstPtr == NULL || algoInstPtr->clientReqCount == 0)
   {
      SNS_SAM_DEBUG1(ERROR, DBG_SAM_DISABLE_ALGO_INSTANCE_ERR, algoInstId);
      return SNS_ERR_FAILED;
   }

   if( algoInstPtr->clientReqCount == 1 )
   {
      for (i = 0; i < sns_sam_algo_dbase[algoIndex]->algoDepCount; i++)
   {
      childAlgoInstId = algoInstPtr->algoReqDbase[i];
      childAlgoIndex = sns_sam_get_algo_index(sns_sam_algo_dbase[algoIndex]->algoDepDbase[i]);
      err = sns_sam_disable_sync_dependent_algos( childAlgoInstId, childAlgoIndex, algoInstId);
   }
   }

   msgHdr.svc_num = algoInstPtr->serviceId;
   clientReqId = sns_sam_find_client_req( &msgHdr, parentAlgoInstId );
   if( clientReqId == SNS_SAM_INVALID_ID )
   {
      SNS_SAM_PRINTF2( ERROR, "Client req id not found: algo inst %d, parent algo inst %d",
          algoInstId, parentAlgoInstId );
      return SNS_ERR_FAILED;
   }

   for (i = 0; i<algoInstPtr->clientReqCount; i++)
   {
      if (algoInstPtr->clientReqDbase[i] == clientReqId)
      {
         algoInstPtr->clientReqCount--;
         algoInstPtr->clientReqDbase[i] =
            algoInstPtr->clientReqDbase[algoInstPtr->clientReqCount];
         algoInstPtr->clientReqDbase[algoInstPtr->clientReqCount] =
            SNS_SAM_INVALID_ID;

         // Deregister client from algorithm instance
         if( sns_sam_algo_dbase[algoIndex] != NULL &&
             sns_sam_algo_dbase[algoIndex]->algoApi.sns_sam_algo_register_client )
         {
            statePtr = algoInstPtr->stateData.memPtr;
            outputPtr = algoInstPtr->outputData.memPtr;
            sns_sam_algo_dbase[algoIndex]->algoApi.
               sns_sam_algo_register_client(
                   sns_sam_mr_get_client_id(sns_sam_client_req_dbase[clientReqId]->mrClientId),
                   false, statePtr, outputPtr, 0);
         }

         if (algoInstPtr->clientReqCount > 0)
         {
            sns_sam_delete_client_req(clientReqId);

            sns_sam_update_duty_cycle_config_client_removal(clientReqId,
                                                            algoInstId,
                                                            algoInstPtr);

            // othe client requests are pending - do not delete algo instance
            return SNS_SUCCESS;
         }

         break;
      }
   }

   sns_sam_algo_dbase[algoIndex]->algoApi.sns_sam_algo_reset(
      algoInstPtr->configData.memPtr,
      algoInstPtr->stateData.memPtr);

   //stop sensor data
   sns_sam_stop_sensor_data(algoInstId);

   sns_sam_delete_client_req(clientReqId);

   //delete the algo instance
   sns_sam_delete_algo_inst(algoInstId);

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_disable_dependent_algos
  =========================================================================*/
/*!
  @brief Enables algorithms that are dependent on a given algorithm

  @param[i] algoInstId: Algorithm Instance ID
  @param[i] algoIndex: Index of algorithm in the algorithm database

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_disable_dependent_algos(
   uint8_t               algoInstId,
   uint8_t               algoIndex
   )
{
   sns_err_code_e err = SNS_SUCCESS;

   if( algoIndex >= SNS_SAM_MAX_ALGOS ||
       sns_sam_algo_dbase[algoIndex] == NULL ||
       algoInstId  >= SNS_SAM_MAX_ALGO_INSTS ||
       sns_sam_algo_inst_dbase[algoInstId] == NULL )
   {
      return SNS_ERR_BAD_PARM;
   }

   if( sns_sam_algo_dbase[algoIndex]->dataSyncRequired )
   {
      // Disable all dependent algorithms synchronously
      int32_t i;
      for (i = 0; i < sns_sam_algo_dbase[algoIndex]->algoDepCount; i++)
      {
         uint8_t childAlgoInstId = sns_sam_algo_inst_dbase[algoInstId]->algoReqDbase[i];
         uint8_t childAlgoIndex = sns_sam_get_algo_index(sns_sam_algo_dbase[algoIndex]->algoDepDbase[i]);
         err = sns_sam_disable_sync_dependent_algos( childAlgoInstId, childAlgoIndex, algoInstId );

         if (err != SNS_SUCCESS)
         {
            return err;
         }
      }
   }
   else
   {
      // Send messages to disable dependent algorithms
      int32_t i;
      for (i = 0; i < sns_sam_algo_dbase[algoIndex]->algoDepCount; i++)
      {
         err = sns_sam_send_algo_disable_msg(
                     sns_sam_algo_dbase[algoIndex]->algoDepDbase[i],
                     sns_sam_algo_inst_dbase[algoInstId]->algoReqDbase[i],
                     algoInstId,
                     sns_sam_algo_inst_dbase[algoInstId]->mrAlgoConnHndl);

         if (err != SNS_SUCCESS)
         {
            return err;
         }
      }
   }

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_client_req
  =========================================================================*/
/*!
  @brief Processes the client request for specified algorithm

  @param[i] clientReqMsgPtr: Pointer to client request message
  @param[i] mrClntId: Client ID
  @param[o] clientRespMsgPtr: Pointer to the response message created.

  @return Sensors error code
*/
/*=======================================================================*/
sns_err_code_e sns_sam_process_client_req(
   const void           *clientReqMsgPtr,
   sns_sam_mr_conn_hndl mrClntId,
   void                 **clientRespMsgPtr)
{
   uint8_t algoSvcId, algoIndex;
   uint8_t algoInstId = SNS_SAM_INVALID_ID;
   uint8_t clientReqId = SNS_SAM_INVALID_ID;
   sns_smr_header_s msgHdr;
   sns_err_code_e err;
   *clientRespMsgPtr = NULL;

   sns_smr_get_hdr(&msgHdr, clientReqMsgPtr);

   clientReqId = sns_sam_find_client_req(&msgHdr, mrClntId);
   if (clientReqId < SNS_SAM_MAX_CLIENT_REQS)
   {
      if (msgHdr.msg_id == SNS_SAM_ALGO_ENABLE_REQ)
      {
         return SNS_ERR_NOTALLOWED;
      }

      algoInstId = sns_sam_client_req_dbase[clientReqId]->algoInstId;
   }
   else
   {
      if ((msgHdr.msg_id == SNS_SAM_ALGO_DISABLE_REQ) ||
          (msgHdr.msg_id == SNS_SAM_ALGO_GET_REPORT_REQ) ||
          (msgHdr.msg_id == SNS_SAM_ALGO_CANCEL_REQ))
      {
         return SNS_ERR_NOTALLOWED;
      }
   }

   algoSvcId = msgHdr.svc_num;
   algoIndex = sns_sam_get_algo_index(algoSvcId);

   if (algoIndex >= SNS_SAM_MAX_ALGOS || sns_sam_algo_dbase[algoIndex] == NULL)
   {
      return SNS_ERR_BAD_PARM;
   }

   switch (msgHdr.msg_id)
   {
   case SNS_SAM_ALGO_ENABLE_REQ:
   {
      //check request parameters
      err = sns_sam_validate_client_req_parameter(msgHdr.msg_id,clientReqMsgPtr,algoSvcId);

      if ( (err != SNS_SUCCESS) && (err != SNS_ERR_NOTSUPPORTED) )
      {
         return err;
      }

      //enable algorithm
      algoInstId = sns_sam_enable_algo(algoIndex, clientReqMsgPtr, mrClntId);

      if (algoInstId  >= SNS_SAM_MAX_ALGO_INSTS ||
          sns_sam_algo_inst_dbase[algoInstId] == NULL)
      {
         return SNS_ERR_FAILED;
      }

      //enable algorithms on which this algorithm depends
      if (sns_sam_algo_inst_dbase[algoInstId]->clientReqCount == 1)
      {
         if( sns_sam_algo_dbase[algoIndex]->algoDepCount )
         {
            err = sns_sam_enable_dependent_algos(algoInstId,
                                                 algoIndex,
                                                 clientReqMsgPtr,
                                                 mrClntId);

            if (err != SNS_SUCCESS)
            {
               return err;
            }
         }
      }

      if (msgHdr.src_module == SNS_SAM_MODULE)
      {
         // store algo instance id in dependent algorithm request database
         if (msgHdr.ext_clnt_id < SNS_SAM_MAX_ALGO_INSTS &&
             msgHdr.txn_id < SNS_SAM_MAX_ALGO_DEPS &&
             sns_sam_algo_inst_dbase[msgHdr.ext_clnt_id] != NULL)
         {
            sns_sam_algo_inst_dbase[msgHdr.ext_clnt_id]->algoReqDbase[msgHdr.txn_id]
               = algoInstId;
         }
         else
         {
            SNS_SAM_DEBUG2(ERROR, DBG_SAM_INS_ID_ERR, msgHdr.ext_clnt_id, msgHdr.txn_id);
            return SNS_ERR_BAD_PARM;
         }
      }

      return (sns_sam_prep_algo_enable_resp(algoInstId,
         sns_sam_algo_inst_dbase[algoInstId]->serviceId,
         clientReqMsgPtr,
         clientRespMsgPtr));
   }

   case SNS_SAM_ALGO_DISABLE_REQ:
   {
      if (sns_sam_algo_inst_dbase[algoInstId]->clientReqCount == 1)
      {
         //disable algorithms on which this algorithm depends
         if( sns_sam_algo_dbase[algoIndex]->algoDepCount )
         {
            err = sns_sam_disable_dependent_algos(algoInstId,
                                                  algoIndex);

            if (err != SNS_SUCCESS)
            {
               return err;
            }
         }
      }

      err = sns_sam_disable_algo(clientReqId);

      if (err == SNS_SUCCESS)
      {
         err = sns_sam_prep_algo_disable_resp(algoInstId,
                                              algoSvcId,
                                              clientReqMsgPtr,
                                              clientRespMsgPtr);
      }
      return err;
   }

   case SNS_SAM_ALGO_GET_REPORT_REQ:
   {
      if (algoInstId >= SNS_SAM_MAX_ALGO_INSTS ||
          sns_sam_algo_inst_dbase[algoInstId] == NULL)
      {
         return SNS_ERR_FAILED;
      }

      err = sns_sam_send_algo_report_req(algoInstId,
                                         sns_sam_algo_inst_dbase[algoInstId],
                                         clientReqMsgPtr);

      if (err == SNS_SUCCESS)
      {
         err = sns_sam_prep_algo_report_resp(sns_sam_client_req_dbase[clientReqId],
                                             clientReqMsgPtr,
                                             clientRespMsgPtr,
                                             algoSvcId);
      }
      return err;
   }

   case SNS_SAM_ALGO_CANCEL_REQ:
   {
      sns_common_cancel_resp_msg_v01 *cancelRespMsgPtr;

      if (sns_sam_algo_inst_dbase[algoInstId]->clientReqCount == 1)
      {
         //disable algorithms on which this algorithm depends
         if( sns_sam_algo_dbase[algoIndex]->algoDepCount )
         {
            err = sns_sam_disable_dependent_algos(algoInstId,
                                                  algoIndex);

            if (err != SNS_SUCCESS)
            {
               return err;
            }
         }
      }

      err = sns_sam_disable_algo(clientReqId);

      if (err == SNS_SUCCESS)
      {
         cancelRespMsgPtr =
         sns_smr_msg_alloc(SNS_SAM_DBG_MOD,sizeof(sns_common_cancel_resp_msg_v01));
         SNS_ASSERT(cancelRespMsgPtr != NULL);
         cancelRespMsgPtr->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
         cancelRespMsgPtr->resp.sns_err_t = SNS_SUCCESS;

         sns_sam_prep_resp_msg(clientReqMsgPtr, cancelRespMsgPtr,
             sizeof(sns_common_cancel_resp_msg_v01));
         *clientRespMsgPtr = cancelRespMsgPtr;
      }
      break;
   }

   case SNS_SAM_ALGO_VERSION_REQ:
   {
      sns_common_version_resp_msg_v01 *versionRespMsgPtr =
         sns_smr_msg_alloc(SNS_SAM_DBG_MOD,sizeof(sns_common_version_resp_msg_v01));
      SNS_ASSERT(versionRespMsgPtr != NULL);

      versionRespMsgPtr->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
      versionRespMsgPtr->resp.sns_err_t = SNS_SUCCESS;

      sns_sam_get_algo_version_resp(algoSvcId, versionRespMsgPtr);

      sns_sam_prep_resp_msg(clientReqMsgPtr, versionRespMsgPtr,
          sizeof(sns_common_version_resp_msg_v01));
      *clientRespMsgPtr = versionRespMsgPtr;
      break;
   }

   case SNS_SAM_ALGO_UPDATE_REQ:
   {
      if( algoInstId >= SNS_SAM_MAX_ALGO_INSTS ||
          sns_sam_algo_inst_dbase[algoInstId] == NULL )
      {
         return SNS_ERR_BAD_PARM;
      }

      err = sns_sam_process_algo_update_req( algoInstId,
          sns_sam_mr_get_client_id(sns_sam_client_req_dbase[clientReqId]->mrClientId), clientReqMsgPtr );

      if( err != SNS_SUCCESS )
      {
         return err;
      }

      // send response to client
      return sns_sam_prep_algo_update_resp( clientReqMsgPtr, clientRespMsgPtr, algoSvcId );
   }

   case SNS_SAM_ALGO_BATCH_REQ:
   case SNS_SAM_ALGO_UPDATE_BATCH_PERIOD_REQ:
   {
      SNS_SAM_DEBUG1(ERROR, DBG_SAM_PROCESS_REQ_INVALID_REQ, msgHdr.msg_id);
      return SNS_ERR_NOTSUPPORTED;
   }

   case SNS_SAM_ALGO_GET_ATTRIB_REQ:
   {
      sns_sam_get_algo_attrib_resp_msg_v01 *attribRespMsgPtr =
         sns_smr_msg_alloc(SNS_SAM_DBG_MOD,sizeof(sns_sam_get_algo_attrib_resp_msg_v01));
      SNS_ASSERT(attribRespMsgPtr != NULL);

      attribRespMsgPtr->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
      attribRespMsgPtr->resp.sns_err_t = SNS_SUCCESS;

      sns_sam_get_algo_attrib_resp(sns_sam_algo_dbase[algoIndex], attribRespMsgPtr);

      sns_sam_prep_resp_msg(clientReqMsgPtr, attribRespMsgPtr,
          sizeof(sns_sam_get_algo_attrib_resp_msg_v01));
      *clientRespMsgPtr = attribRespMsgPtr;
      break;
   }

   default:
      SNS_SAM_DEBUG1(ERROR, DBG_SAM_PROCESS_REQ_INVALID_REQ, msgHdr.msg_id);
      return SNS_ERR_FAILED;
   }

   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_smgr_item
  =========================================================================*/
/*!
  @brief    Processes an item in SMGR indication message

  @param[i] indMsgType: indication message type
  @param[i] smgrIndPtr: pointer to SMGR indication message
  @param[i] timestamp:  timestamp of the first sensor sample
  @param[i] dataReqId:  index of data request entry in dbase
  @param[i] algoInstIds:  list of algo instances that need this item
  @param[i] algoInstCount:  number of algo instances in algoInstIds[]

  @return   Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_process_smgr_item(
   uint8_t indMsgType,
   const void* smgrIndPtr,
   uint32_t timestamp,
   uint8_t dataReqId,
   uint8_t algoInstIds[],
   uint8_t algoInstCount)
{
   uint8_t i;
   sns_err_code_e err;

   // Executes algorithms waiting for the sensor sample.
   for (i = 0; i < algoInstCount; i++)
   {
      void *algoRptPtr;
      sns_sam_algo_inst_s *algoInstPtr;
      uint8_t algoIndex, j;
      uint8_t algoInstId = algoInstIds[i];

      algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];
      if (NULL == algoInstPtr)
      {
         SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, algoInstId);
         continue;
      }

      algoIndex = sns_sam_get_algo_index(algoInstPtr->serviceId);
      if (algoIndex >= SNS_SAM_MAX_ALGOS || sns_sam_algo_dbase[algoIndex] == NULL)
      {
         SNS_SAM_DEBUG2(ERROR, DBG_SAM_SMGR_IND_INVALID, algoIndex, algoInstPtr->serviceId);
         continue;
      }

#ifdef SNS_PLAYBACK_SKIP_SMGR
      {
         //before first report, update nextReportTime with timestamp from smgr
         uint8_t count_pb_clientReqId = 0;
         while (sns_sam_client_req_dbase[count_pb_clientReqId]!=NULL)
         {
            if (nextReportTimeEnable[count_pb_clientReqId] == 0)
            {
               nextReportTime[count_pb_clientReqId] += timestamp;
               nextReportTimeEnable[count_pb_clientReqId]=1; // 1: Enabled 0: Not Assigned
            }
            else
            {
               //check if it is time to report
               if (timestamp > nextReportTime[count_pb_clientReqId])
               {
                  // report to client
                  sns_sam_client_req_s* clientReqPtr =
                  sns_sam_client_req_dbase[count_pb_clientReqId];

                  sns_sam_timer_cb((void *)count_pb_clientReqId);

                  // update next report time
                  // reportPeriod is in millisec, convert to DSPS clock ticks
                  nextReportTime[count_pb_clientReqId] +=
                     (uint32_t)(clientReqPtr->period*DSPS_SLEEP_CLK/1000);
               }
            }
            count_pb_clientReqId++;
         }
      }
#endif

      // Updates inputs of algorithm
      err = sns_sam_update_algo_input(algoInstPtr->serviceId,
                                      indMsgType, smgrIndPtr,
                                      algoInstPtr->inputData.memPtr,
                                      algoInstPtr->inputData.memSize,
                                      timestamp);
      if (err != SNS_SUCCESS)
      {
         for (j = 0; j < algoInstPtr->clientReqCount; j++)
         {
            uint8_t clientReqId = algoInstPtr->clientReqDbase[j];
            sns_sam_client_req_s* clientReqPtr =
               sns_sam_client_req_dbase[clientReqId];

            if( clientReqPtr != NULL &&
                clientReqPtr->algoInstId == algoInstId )
            {
               // notify corresponding clients of internal error
               sns_sam_send_error_ind(clientReqId,
                                      SNS_ERR_FAILED,
                                      clientReqPtr->mrClientId);
            }
         }
         return err;
      }

      //backup the algo report
      algoRptPtr = SNS_OS_MALLOC(SNS_SAM_DBG_MOD,algoInstPtr->outputData.memSize);
      SNS_ASSERT(algoRptPtr != NULL);
      SNS_OS_MEMCOPY(algoRptPtr,
                     algoInstPtr->outputData.memPtr,
                     algoInstPtr->outputData.memSize);

#ifdef SNS_DSPS_BUILD
      sns_profiling_log_timestamp((uint64_t) (((uint64_t)(((uint64_t)SNS_SAM_ALGO_EXEC_PROFILE_START)<<32)) |
                                                          ((uint64_t)(algoInstPtr->serviceId))));
#endif

      //execute the algorithm
      sns_sam_algo_dbase[algoIndex]->algoApi.sns_sam_algo_update(
         algoInstPtr->stateData.memPtr,
         algoInstPtr->inputData.memPtr,
         algoInstPtr->outputData.memPtr);

#ifdef SNS_DSPS_BUILD
      sns_profiling_log_timestamp((uint64_t) (((uint64_t)(((uint64_t)SNS_SAM_ALGO_EXEC_PROFILE_END)<<32)) |
                                                          ((uint64_t)(algoInstPtr->serviceId))));
#endif

      //update algorithm report timestamp
      algoInstPtr->outputData.timestamp = timestamp;

      for (j = 0; j < SNS_SAM_MAX_CLIENT_REQS_PER_ALGO_INST; j++)
      {
         if (algoInstPtr->clientReqDbase[j] < SNS_SAM_MAX_CLIENT_REQS)
         {
            uint8_t clientReqId = algoInstPtr->clientReqDbase[j];
            sns_sam_client_req_s* clientReqPtr =
               sns_sam_client_req_dbase[clientReqId];

            //send synchronous and asynchronous indications
            if (clientReqPtr != NULL)
            {
               sns_sam_update_algo_report_ts(algoInstPtr, sns_sam_mr_get_client_id(clientReqPtr->mrClientId));
               if ((clientReqPtr->reportType == SNS_SAM_SYNC_REPORT) ||
                   ((clientReqPtr->reportType == SNS_SAM_ASYNC_REPORT) &&
                    (SNS_OS_MEMCMP(algoRptPtr,
                                   algoInstPtr->outputData.memPtr,
                                   algoInstPtr->outputData.memSize))))
               {
                  //process algorithm report indication to client
                  sns_sam_process_algo_report(clientReqId,
                                              &(algoInstPtr->outputData),
                                              clientReqPtr->mrClientId);

                  SNS_SAM_DEBUG1(HIGH, DBG_SAM_SMGR_IND_DELIVERY_SUCC,
                                 sns_sam_mr_get_client_id(clientReqPtr->mrClientId));
               }
            }
         }
      }
      if(sns_sam_get_smgr_msg_req_type(algoInstPtr->serviceId) ==
         SNS_SMGR_BUFFERING_REQ_V01)
      {
         sns_sam_update_input_type(algoInstPtr->serviceId, dataReqId, algoRptPtr,
                                  algoInstPtr->outputData.memPtr);
      }
      sns_sam_log_algo_result(algoInstId, algoInstPtr, SNS_SAM_INVALID_ID);

      SNS_OS_FREE(algoRptPtr);
   }

   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_smgr_report_resp
  =========================================================================*/
/*!
  @brief Processes the response received from sensors manager

  @param[i] smgrRespPtr: Pointer to sensors manager response message

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_process_smgr_report_resp(
   const void *smgrRespPtr)
{
   uint8_t dataReqId = SNS_SAM_INVALID_ID;
   sns_sam_client_req_s* clientReqPtr;

   sns_smgr_periodic_report_resp_msg_v01* respPtr =
      (sns_smgr_periodic_report_resp_msg_v01*)smgrRespPtr;

   dataReqId = respPtr->ReportId;

   if (respPtr->AckNak == SNS_SMGR_RESPONSE_ACK_SUCCESS_V01 ||
       respPtr->AckNak == SNS_SMGR_RESPONSE_ACK_MODIFIED_V01)
   {
      SNS_SAM_DEBUG1(HIGH, DBG_SAM_SMGR_RESP_SUCCESS, dataReqId);
      return SNS_SUCCESS;
   }

   if (dataReqId < SNS_SAM_MAX_DATA_REQS &&
       sns_sam_data_req_dbase[dataReqId] != NULL)
   {
      uint8_t i, j;
      for (i = 0; i < sns_sam_data_req_dbase[dataReqId]->algoInstCount; i++)
      {
         uint8_t algoInstId =
            sns_sam_data_req_dbase[dataReqId]->algoInstDbase[i];

         for (j = 0;
              j < sns_sam_algo_inst_dbase[algoInstId]->clientReqCount;
              j++)
         {
            uint8_t clientReqId =
               sns_sam_algo_inst_dbase[algoInstId]->clientReqDbase[j];

            if (sns_sam_client_req_dbase[clientReqId] != NULL &&
                sns_sam_client_req_dbase[clientReqId]->algoInstId ==
                algoInstId)
            {
               clientReqPtr = sns_sam_client_req_dbase[clientReqId];
               // notify corresponding clients of internal error
               sns_sam_send_error_ind(clientReqId,
                                      SNS_ERR_FAILED,
                                      clientReqPtr->mrClientId);
            }
         }
      }
   }
   else
   {
      SNS_SAM_DEBUG1(HIGH, DBG_SAM_SMGR_RESP_DROPPED, dataReqId);
   }

   SNS_SAM_DEBUG2(MEDIUM, DBG_SAM_SMGR_RESP_ACK_VAL,
                  respPtr->AckNak, respPtr->ReasonPair_len);

   if (respPtr->ReasonPair_len < SNS_SMGR_MAX_NUM_REASONS_V01)
   {
      uint32_t i;
      for (i = 0; i < respPtr->ReasonPair_len; i++)
      {
         SNS_SAM_DEBUG2(MEDIUM, DBG_SAM_SMGR_RESP_RESPINFO,
                        respPtr->ReasonPair[i].ItemNum,
                        respPtr->ReasonPair[i].Reason);
      }
   }

   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_smgr_report_ind
  =========================================================================*/
/*!
  @brief Processes the indication received from sensors manager

  @param[i] smgrIndPtr: Pointer to sensors manager indication message

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_process_smgr_report_ind(
   const void *smgrIndPtr)
{
   sns_smgr_periodic_report_ind_msg_v01* indPtr;
   uint8_t dataReqId = SNS_SAM_INVALID_ID;
   uint32_t i;
   sns_err_code_e err;
   uint8_t algoInstIds[SNS_SAM_MAX_ALGO_INSTS_PER_DATA_REQ];
   uint8_t algoInstCount = 0;

   indPtr = (sns_smgr_periodic_report_ind_msg_v01*)smgrIndPtr;
   dataReqId = indPtr->ReportId;
   if (dataReqId >= SNS_SAM_MAX_DATA_REQS ||
       sns_sam_data_req_dbase[dataReqId] == NULL)
   {
      SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, dataReqId);
      return SNS_ERR_FAILED;
   }

   if (indPtr->status != SNS_SMGR_REPORT_OK_V01)
   {
      SNS_SAM_DEBUG2(MEDIUM, DBG_SAM_SMGR_IND_STATUS,
                     dataReqId, indPtr->status);
      return SNS_ERR_FAILED;
   }

   if (indPtr->Item_len != sns_sam_data_req_dbase[dataReqId]->sensorCount)
   {
      SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, dataReqId);
      return SNS_ERR_FAILED;
   }
   if (indPtr->CurrentRate != sns_sam_data_req_dbase[dataReqId]->reportRate)
   {
      SNS_SAM_DEBUG2(MEDIUM, DBG_SAM_SMGR_IND_STATUS,
                     dataReqId, indPtr->CurrentRate);
   }

   //Initialize instance ids for algorithms using this indication
   for( i = 0; i < sns_sam_data_req_dbase[dataReqId]->algoInstCount; i++ )
   {
      algoInstIds[algoInstCount++] = sns_sam_data_req_dbase[dataReqId]->algoInstDbase[i];
   }

   //Check for indication matching request
   for (i = 0; i < indPtr->Item_len; i++)
   {
      if (indPtr->Item[i].SensorId !=
          sns_sam_data_req_dbase[dataReqId]->sensorDbase[i].sensorId ||
          indPtr->Item[i].DataType !=
          sns_sam_data_req_dbase[dataReqId]->sensorDbase[i].dataType)
      {
         SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, dataReqId);
         return SNS_ERR_FAILED;
      }
   }

   //Validate sensor data and process valid data only
   for (i = 0; i < indPtr->Item_len; i++)
   {
      if (indPtr->Item[i].ItemFlags == SNS_SMGR_ITEM_FLAG_INVALID_V01 ||
          indPtr->Item[i].ItemQuality ==
          SNS_SMGR_ITEM_QUALITY_INVALID_FAILED_SENSOR_V01 ||
          indPtr->Item[i].ItemQuality ==
          SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY_V01 ||
          indPtr->Item[i].ItemQuality ==
          SNS_SMGR_ITEM_QUALITY_INVALID_SUSPENDED_V01)
      {
         SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, dataReqId);
      }
      else
      {
         //execute algorithms waiting for this sensor data
         err = sns_sam_process_smgr_item(SNS_SMGR_REPORT_IND_V01,
                                         &(indPtr->Item[i]),
                                         indPtr->Item[i].TimeStamp, dataReqId,
                                         algoInstIds, algoInstCount);
      }
   }

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_find_max_sample_rate
  =========================================================================*/
/*!
  @brief Finds max sample rate in a data request

  @param[i] dataReqId: Index of data request in data request database

  @return sample rate in Hz (Q16 format)
*/
/*=======================================================================*/
int32_t sns_sam_find_max_sample_rate(
   const uint8_t dataReqId)
{
   int32_t maxSampleRate = 0;

   if( dataReqId < SNS_SAM_MAX_DATA_REQS &&
       sns_sam_data_req_dbase[dataReqId] != NULL )
   {
      int32_t i;
      for( i = 0; i < sns_sam_data_req_dbase[dataReqId]->sensorCount; i++ )
      {
         if( sns_sam_data_req_dbase[dataReqId]->sensorDbase[i].sampleRate >
             maxSampleRate )
         {
            maxSampleRate =
               sns_sam_data_req_dbase[dataReqId]->sensorDbase[i].sampleRate;
         }
      }
   }
   return FX_CONV_Q16(maxSampleRate,0);
}

/*=========================================================================
  FUNCTION:  sns_sam_switch_smgr_data_req
  =========================================================================*/
/*!
  @brief Switch from buffering to periodic data requests to SMGR

  @param[i] dataReqId: Index of data request in data request database

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_switch_smgr_data_req(
   const uint8_t dataReqId)
{
   sns_err_code_e err = SNS_ERR_FAILED;
   if( dataReqId < SNS_SAM_MAX_DATA_REQS &&
       sns_sam_data_req_dbase[dataReqId] != NULL &&
       sns_sam_data_req_dbase[dataReqId]->algoInstCount > 0 )
   {
      uint8_t algoInstId, svcId = 0;

      sns_sam_data_req_dbase[dataReqId]->reportRate =
          sns_sam_find_max_sample_rate(dataReqId);

      // Find one service associated with this data request
      algoInstId = sns_sam_data_req_dbase[dataReqId]->algoInstDbase[0];
      if( algoInstId < SNS_SAM_MAX_ALGO_INSTS &&
          sns_sam_algo_inst_dbase[algoInstId] != NULL )
      {
         svcId = sns_sam_algo_inst_dbase[algoInstId]->serviceId;

         // Send request to switch to new message type
         err = sns_sam_send_smgr_start_req( dataReqId, svcId );
      }
   }
   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_smgr_buffering_resp
  =========================================================================*/
/*!
  @brief Processes the buffering response received from sensors manager

  @param[i] smgrRespPtr: Pointer to sensors manager response message

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_process_smgr_buffering_resp(
   const void *smgrRespPtr)
{
   uint8_t dataReqId = SNS_SAM_INVALID_ID;
   sns_sam_client_req_s* clientReqPtr;

   sns_smgr_buffering_resp_msg_v01* respPtr =
      (sns_smgr_buffering_resp_msg_v01*) smgrRespPtr;

   if (respPtr->ReportId_valid)
   {
      dataReqId = respPtr->ReportId;
   }

   if( respPtr->Resp.sns_result_t != SNS_RESULT_SUCCESS_V01 &&
       respPtr->Resp.sns_err_t != SENSOR1_SUCCESS )
   {
     if( respPtr->Resp.sns_err_t == SENSOR1_EBAD_MSG_ID )
     {
        sns_sam_smgr_buffering_flag = false;
        if( sns_sam_switch_smgr_data_req( dataReqId ) == SNS_SUCCESS )
        {
           // Retrying with periodic data request
           return SNS_ERR_WOULDBLOCK;
        }
     }
     else if( respPtr->AckNak_valid &&
              respPtr->AckNak == SNS_SMGR_RESPONSE_NAK_REPORT_RATE_V01 )
     {
        if( sns_sam_switch_smgr_data_req( dataReqId ) == SNS_SUCCESS )
        {
           // Retrying with periodic data request
           return SNS_ERR_WOULDBLOCK;
        }
     }
   }

   if (respPtr->AckNak_valid &&
       (respPtr->AckNak == SNS_SMGR_RESPONSE_ACK_SUCCESS_V01 ||
        respPtr->AckNak == SNS_SMGR_RESPONSE_ACK_MODIFIED_V01))
   {
      SNS_SAM_DEBUG1(HIGH, DBG_SAM_SMGR_RESP_SUCCESS, dataReqId);
      return SNS_SUCCESS;
   }

   if (dataReqId < SNS_SAM_MAX_DATA_REQS &&
       sns_sam_data_req_dbase[dataReqId] != NULL)
   {
      uint8_t i, j;
      for (i = 0; i < sns_sam_data_req_dbase[dataReqId]->algoInstCount; i++)
      {
         uint8_t algoInstId =
            sns_sam_data_req_dbase[dataReqId]->algoInstDbase[i];

         for (j = 0;
              j < sns_sam_algo_inst_dbase[algoInstId]->clientReqCount;
              j++)
         {
            uint8_t clientReqId =
               sns_sam_algo_inst_dbase[algoInstId]->clientReqDbase[j];

            if (sns_sam_client_req_dbase[clientReqId] != NULL &&
                sns_sam_client_req_dbase[clientReqId]->algoInstId ==
                algoInstId)
            {
               clientReqPtr = sns_sam_client_req_dbase[clientReqId];
               // notify corresponding clients of internal error
               sns_sam_send_error_ind(clientReqId,
                                      SNS_ERR_FAILED,
                                      clientReqPtr->mrClientId);
            }
         }
      }
   }
   else
   {
      SNS_SAM_DEBUG1(HIGH, DBG_SAM_SMGR_RESP_DROPPED, dataReqId);
   }

   SNS_SAM_DEBUG2(MEDIUM, DBG_SAM_SMGR_RESP_ACK_VAL,
                  respPtr->AckNak, respPtr->ReasonPair_len);

   if (respPtr->ReasonPair_valid &&
       (respPtr->ReasonPair_len < SNS_SMGR_MAX_NUM_REASONS_V01))
   {
      uint32_t i;
      for (i = 0; i < respPtr->ReasonPair_len; i++)
      {
         SNS_SAM_DEBUG2(MEDIUM, DBG_SAM_SMGR_RESP_RESPINFO,
                        respPtr->ReasonPair[i].ItemNum,
                        respPtr->ReasonPair[i].Reason);
      }
   }

   return SNS_ERR_FAILED;
}

/*=========================================================================
  FUNCTION:  sns_sam_get_algo_inst_in_data_req
  =========================================================================*/
/*!
  @brief    Gets the list of algorithm instances that requested for a
            particular sensor id

  @param[i] dataReqPtr: pointer to data request
  @param[i] sensorId:  sensor id
  @param[o] algoInstIds:  array of algorithm instance ids

  @return   number of algorithm instances in output array
*/
/*=======================================================================*/
static int32_t sns_sam_get_algo_inst_in_data_req(
   sns_sam_data_req_s* dataReqPtr,
   uint8_t sensorId,
   uint8_t algoInstIds[])
{
   int32_t i, j, k, algoInstCount = 0;

   if( dataReqPtr == NULL ||
       sensorId == SNS_SAM_INVALID_ID )
   {
      return 0;
   }

   for( i = 0; i < dataReqPtr->algoInstCount; i++ )
   {
      uint8_t algoInstId = dataReqPtr->algoInstDbase[i];
      if( algoInstId < SNS_SAM_MAX_ALGO_INSTS &&
          sns_sam_algo_inst_dbase[algoInstId] != NULL )
      {
         if( !sns_sam_algo_inst_dbase[algoInstId]->cumulativeDataReq )
         {
            algoInstIds[algoInstCount++] = dataReqPtr->algoInstDbase[i];
         }
         else
         {
            // only cumulative data requests specify separate algorithm instances per sensor id
            for( j = 0; j < SNS_SAM_MAX_SENSORS_PER_DATA_REQ; j++ )
            {
               if( dataReqPtr->sensorDbase[j].sensorId == sensorId )
               {
                  for( k = 0; k < SNS_SAM_MAX_ALGO_INSTS_PER_SENSOR; k++ )
                  {
                     if( dataReqPtr->sensorDbase[j].algoInstIds[k] != SNS_SAM_INVALID_ID )
                     {
                        algoInstIds[algoInstCount++] = dataReqPtr->sensorDbase[j].algoInstIds[k];
                     }
                  }
                  break;
               }
            }
         }
      }
   }

   return algoInstCount;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_smgr_buffer
  =========================================================================*/
/*!
  @brief Processes samples in the buffering indication received from SMGR

  @param[i] indPtr: Pointer to buffering indication message
  @param[i] dataReqId: Index of entry in data request database

  @return None
*/
/*=======================================================================*/
static void sns_sam_process_smgr_buffer(
   sns_smgr_buffering_ind_msg_v01 *indPtr,
   uint8_t dataReqId)
{
   uint32_t i;
   sns_smgr_buffering_sample_s_v01 * samplePtr[SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01];
   uint32_t sampleCount[SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01];
   uint32_t timestamp[SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01];
   uint32_t samplesProcessed = 0;
   uint8_t algoInstIds[SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01][SNS_SAM_MAX_ALGO_INSTS_PER_DATA_REQ];
   uint8_t algoInstCount[SNS_SMGR_BUFFERING_REQUEST_MAX_ITEMS_V01];

   SNS_OS_MEMSET( samplePtr, 0, sizeof(samplePtr) );
   SNS_OS_MEMSET( algoInstIds, 0, sizeof(algoInstIds) );
   SNS_OS_MEMSET( algoInstCount, 0, sizeof(algoInstCount) );

   for (i = 0; i < indPtr->Indices_len; i++)
   {
      uint8_t firstSampleIndex = indPtr->Indices[i].FirstSampleIdx;
      samplePtr[i] = &indPtr->Samples[firstSampleIndex];
      sampleCount[i] = indPtr->Indices[i].SampleCount;
      timestamp[i] = indPtr->Indices[i].FirstSampleTimestamp;
      // Find algo instances that need this sensor type
      algoInstCount[i] = sns_sam_get_algo_inst_in_data_req(sns_sam_data_req_dbase[dataReqId],
                                                           indPtr->Indices[i].SensorId,
                                                           algoInstIds[i]);
   }

   while( samplesProcessed < indPtr->Samples_len )
   {
      uint32_t minTimestamp = timestamp[0];
      uint32_t oldestSampleIdx = 0;
      sns_smgr_buffering_sample_s_v01 * oldestSamplePtr = NULL;

      samplesProcessed++;
      for (i = 1; i < indPtr->Indices_len; i++)
      {
         // Find sample with min timestamp
         // TODO: Account for rollover
         if( timestamp[i] < minTimestamp &&
             samplePtr[i] != NULL )
         {
            minTimestamp = timestamp[i];
            oldestSampleIdx = i;
         }
      }

      oldestSamplePtr = samplePtr[oldestSampleIdx];
      if( sampleCount[oldestSampleIdx] > 1 )
      {
         samplePtr[oldestSampleIdx]++;
         timestamp[oldestSampleIdx] += (samplePtr[oldestSampleIdx]->TimeStampOffset);
         sampleCount[oldestSampleIdx]--;
      }
      else
      {
         samplePtr[oldestSampleIdx] =  NULL;
         timestamp[oldestSampleIdx] = UINT32_MAX;
      }

      if (oldestSamplePtr->Flags == SNS_SMGR_ITEM_FLAG_INVALID_V01 ||
          oldestSamplePtr->Quality ==
          SNS_SMGR_ITEM_QUALITY_INVALID_FAILED_SENSOR_V01 ||
          oldestSamplePtr->Quality ==
          SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY_V01 ||
          oldestSamplePtr->Quality ==
          SNS_SMGR_ITEM_QUALITY_INVALID_SUSPENDED_V01)
      {
         SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, oldestSampleIdx);
         continue;
      }

      //execute algorithms waiting for this sensor data
      sns_sam_process_smgr_item( SNS_SMGR_BUFFERING_IND_V01, oldestSamplePtr,
                                 minTimestamp, dataReqId, 
                                 algoInstIds[oldestSampleIdx],
                                 algoInstCount[oldestSampleIdx]);
   }
   SNS_SAM_PRINTF1( LOW, "SAM processed %d samples from buffering indication", samplesProcessed );
}

/*=========================================================================
  FUNCTION:  sns_sam_process_smgr_buffering_ind
  =========================================================================*/
/*!
  @brief Processes the buffering indication received from sensor manager

  @param[i] smgrIndPtr: Pointer to sensors manager indication message

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_process_smgr_buffering_ind(
   const void *smgrIndPtr)
{
   sns_smgr_buffering_ind_msg_v01* indPtr;
   uint8_t dataReqId = SNS_SAM_INVALID_ID;
   sns_sam_data_req_s * dataReqPtr = NULL;
   uint32_t i, j;
   bool foundType = false;

   indPtr = (sns_smgr_buffering_ind_msg_v01*)smgrIndPtr;
   dataReqId = indPtr->ReportId;
   if (dataReqId >= SNS_SAM_MAX_DATA_REQS ||
       sns_sam_data_req_dbase[dataReqId] == NULL)
   {
      SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, dataReqId);
      return SNS_ERR_FAILED;
   }

   if( indPtr->Indices_len == 0 || indPtr->Samples_len == 0 )
   {
      SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, dataReqId);
      return SNS_ERR_FAILED;
   }

   SNS_SAM_PRINTF3( LOW, "SAM: Rxd buffering ind with %d samples and %d types for dataReqId %d",
       indPtr->Samples_len, indPtr->Indices_len, dataReqId );

   //Validate sensor data
   dataReqPtr = sns_sam_data_req_dbase[dataReqId];
   for( i = 0; !foundType && i < indPtr->Indices_len; i++ )
   {
      // Check if report has at least one of the types that was requested
      for( j = 0; !foundType && j < dataReqPtr->sensorCount; j++ )
      {
         if( dataReqPtr->sensorDbase[j].sensorId == indPtr->Indices[i].SensorId &&
             dataReqPtr->sensorDbase[j].dataType == indPtr->Indices[i].DataType )
         {
            foundType = true;
         }
      }
   }
   //TODO: If report has types that were not requested, bail
   if( !foundType )
   {
      SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, dataReqId);
      return SNS_ERR_FAILED;
   }

   //execute algorithms waiting for this sensor data
   sns_sam_process_smgr_buffer( indPtr, dataReqId);
   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_smgr_buffering_query_resp
  =========================================================================*/
/*!
  @brief Processes the response received from sensors manager

  @param[i] smgrRespPtr: Pointer to sensors manager response message

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_process_smgr_buffering_query_resp(
   const void *smgrRespPtr)
{
   sns_smgr_buffering_query_resp_msg_v01* respPtr =
      (sns_smgr_buffering_query_resp_msg_v01*) smgrRespPtr;

   /* SMGR request was successful. No further action required */
   if (respPtr->AckNak_valid && respPtr->QueryId_valid &&
      (respPtr->AckNak == SNS_SMGR_RESPONSE_ACK_SUCCESS_V01 ||
       respPtr->AckNak == SNS_SMGR_RESPONSE_ACK_MODIFIED_V01))
   {
      SNS_SAM_DEBUG1(HIGH, DBG_SAM_SMGR_RESP_SUCCESS,
                     (respPtr->QueryId & 0x00FF));
   }
   else
   {
      SNS_SAM_DEBUG1(HIGH, DBG_SAM_SMGR_RESP_DROPPED,
                     (respPtr->QueryId & 0x00FF));
   }

   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_smgr_buffering_query_ind
  =========================================================================*/
/*!
  @brief Processes the indication received from sensors manager

  @param[i] smgrIndPtr: Pointer to sensors manager indication message

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_process_smgr_buffering_query_ind(
   const void *smgrIndPtr)
{
   sns_err_code_e err;
   uint32_t i;
   uint8_t dataReqId;
   uint8_t algoInstIds[SNS_SAM_MAX_ALGO_INSTS_PER_DATA_REQ];
   uint8_t algoInstCount = 0;

   sns_smgr_buffering_query_ind_msg_v01 *indPtr =
      (sns_smgr_buffering_query_ind_msg_v01*) smgrIndPtr;

   dataReqId = (uint8_t) (indPtr->QueryId & 0x00FF);

   if (dataReqId >= SNS_SAM_MAX_DATA_REQS ||
       sns_sam_data_req_dbase[dataReqId] == NULL)
   {
      SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, dataReqId);
      return SNS_ERR_FAILED;
   }

   for (i = 0; i < indPtr->Samples_len; i++)
   {
      if (indPtr->Samples[i].Flags == SNS_SMGR_ITEM_FLAG_INVALID_V01 ||
          indPtr->Samples[i].Quality ==
          SNS_SMGR_ITEM_QUALITY_INVALID_FAILED_SENSOR_V01 ||
          indPtr->Samples[i].Quality ==
          SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY_V01 ||
          indPtr->Samples[i].Quality ==
          SNS_SMGR_ITEM_QUALITY_INVALID_SUSPENDED_V01)
      {
         SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_DROPPED, dataReqId);
         return SNS_ERR_FAILED;
      }
   }

   for( i = 0; i < sns_sam_data_req_dbase[dataReqId]->algoInstCount; i++ )
   {
      algoInstIds[algoInstCount++] = sns_sam_data_req_dbase[dataReqId]->algoInstDbase[i];
   }

   //execute algorithms waiting for this sensor data
   err = sns_sam_process_smgr_item(SNS_SMGR_BUFFERING_QUERY_IND_V01, smgrIndPtr,
                                   indPtr->FirstSampleTimestamp, dataReqId,
                                   algoInstIds, algoInstCount);

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_sam_resp
  =========================================================================*/
/*!
  @brief Processes the response received from sensors algorithm manager

  @param[i] samRespPtr: Pointer to sensors algorithm manager response message

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_process_sam_resp(
   const void *samRespPtr)
{
   uint8_t algoInstId, algoDepId;
   uint8_t depAlgoInstId;
   sns_err_code_e err = SNS_ERR_FAILED;
   sns_smr_header_s msgHdr;

   sns_smr_get_hdr(&msgHdr, samRespPtr);
   algoInstId = msgHdr.ext_clnt_id;
   algoDepId = msgHdr.txn_id;

   if (algoInstId >= SNS_SAM_MAX_ALGO_INSTS)
   {
      return SNS_ERR_FAILED;
   }

   depAlgoInstId = sns_sam_process_sam_response(samRespPtr);

   if (depAlgoInstId < SNS_SAM_MAX_ALGO_INSTS)
   {
      if (msgHdr.msg_id == SNS_SAM_ALGO_ENABLE_RESP)
      {
         // Check to discard delayed enable response for
         // algorithm instance that has been disabled per client request
         if (sns_sam_algo_inst_dbase[algoInstId] != NULL &&
             algoDepId < SNS_SAM_MAX_ALGO_DEPS)
         {
            sns_sam_algo_inst_dbase[algoInstId]->algoReqDbase[algoDepId] =
               depAlgoInstId;
         }
      }
      else if (msgHdr.msg_id == SNS_SAM_ALGO_DISABLE_RESP)
      {
         SNS_SAM_DEBUG3(MEDIUM, DBG_SAM_DISABLE_ALGO,
                        algoInstId, algoDepId, depAlgoInstId);
      }
      err = SNS_SUCCESS;
   }

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_sam_ind
  =========================================================================*/
/*!
  @brief Processes the indication received from sensors algorithm manager

  @param[i] smgrIndPtr: Pointer to sensors manager indication message

  @return Sensors error code
*/
/*=======================================================================*/
sns_err_code_e sns_sam_process_sam_ind(
   const void *samIndPtr)
{
   sns_smr_header_s msgHdr;
   uint8_t algoInstId, i;
   sns_err_code_e err = SNS_SUCCESS;
   sns_sam_algo_inst_s* algoInstPtr;

   sns_smr_get_hdr(&msgHdr, samIndPtr);
   algoInstId = msgHdr.ext_clnt_id;

   if (algoInstId >= SNS_SAM_MAX_ALGO_INSTS ||
       sns_sam_algo_inst_dbase[algoInstId] == NULL)
   {
      return SNS_ERR_FAILED;
   }

   algoInstPtr = sns_sam_algo_inst_dbase[algoInstId];

   if (SNS_SAM_ALGO_REPORT_IND == msgHdr.msg_id)
   {
      err = sns_sam_process_sam_report_ind(samIndPtr,
                                           algoInstPtr,
                                           msgHdr.svc_num,
                                           algoInstId);
      if (err == SNS_SUCCESS)
      {
         //send synchronous indication
         for (i = 0; i < SNS_SAM_MAX_CLIENT_REQS_PER_ALGO_INST; i++)
         {
            if (algoInstPtr->clientReqDbase[i] < SNS_SAM_MAX_CLIENT_REQS)
            {
               uint8_t clientReqId = algoInstPtr->clientReqDbase[i];
               sns_sam_client_req_s* clientReqPtr =
                  sns_sam_client_req_dbase[clientReqId];

               if (clientReqPtr != NULL)
               {
                  if( clientReqPtr->reportType == SNS_SAM_SYNC_REPORT )
                  {
                     //process algorithm report indication to client
                     sns_sam_process_algo_report(clientReqId,
                                                 &(algoInstPtr->outputData),
                                                 clientReqPtr->mrClientId);
                  }
                  else if (clientReqPtr->reportType == SNS_SAM_ONE_SHOT_REPORT)
                  {
                     sns_sam_process_one_shot_algo_report(clientReqId,
                                                          &(algoInstPtr->outputData));
                  }
               }
            }
         }

         sns_sam_log_dep_algo_result(algoInstId,
                                     sns_sam_algo_inst_dbase[algoInstId]);
      }
   }
   else if (SNS_SAM_ALGO_ERROR_IND == msgHdr.msg_id)
   {
      //process algo error indication, currently ignored
   }
   else
   {
      err = SNS_ERR_BAD_PARM;
   }

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_time_resp
  =========================================================================*/
/*!
  @brief Processes response from time service

  @param[i] : timeRespMsgPtr    pointer to message from Time Service

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_process_time_resp(
   const void *timeRespMsgPtr)
{
   sns_smr_header_s msgHdr;
   sns_err_code_e   err = SNS_SUCCESS;

   sns_smr_get_hdr(&msgHdr, timeRespMsgPtr);
   err = sns_sam_process_time_response(timeRespMsgPtr, &msgHdr);

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_msg
  =========================================================================*/
/*!
  @brief Process the messages from SAM's input message queue

  @return Sensors error code
*/
/*=======================================================================*/
static sns_err_code_e sns_sam_process_msg(void)
{
   void *msgPtr;
   sns_smr_header_s msgHdr;
   sns_err_code_e err;

   while ((msgPtr = sns_sam_mr_msg_rcv()) != NULL)
   {
      err = sns_smr_get_hdr(&msgHdr, msgPtr);
      if (err != SNS_SUCCESS)
      {
         SNS_SAM_DEBUG1(ERROR, DBG_SAM_PROC_MSG_HDR_ERR, err);
         sns_smr_msg_free(msgPtr);
         continue;
      }

      switch (msgHdr.src_module)
      {
         case SNS_MODULE_DSPS_SMGR:
         {
            if (msgHdr.msg_type == SNS_SMR_MSG_TYPE_RESP)
            {
               if (msgHdr.msg_id == SNS_SMGR_REPORT_RESP_V01)
               {
                  err = sns_sam_process_smgr_report_resp(msgPtr);
               }
               else if (msgHdr.msg_id == SNS_SMGR_REG_HW_MD_INT_RESP_V01)
               {
                  sns_sam_process_smgr_mot_int_resp(msgPtr);
               }
               else if (msgHdr.msg_id == SNS_SMGR_BUFFERING_RESP_V01)
               {
                  err = sns_sam_process_smgr_buffering_resp(msgPtr);
               }
               else if (msgHdr.msg_id == SNS_SMGR_BUFFERING_QUERY_RESP_V01)
               {
                  err = sns_sam_process_smgr_buffering_query_resp(msgPtr);
               }
               else
               {
                  SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_MSGID,
                                 msgHdr.msg_id);
               }
            }
            else if (msgHdr.msg_type == SNS_SMR_MSG_TYPE_IND)
            {
               if (msgHdr.msg_id == SNS_SMGR_REPORT_IND_V01)
               {
                  err = sns_sam_process_smgr_report_ind(msgPtr);
               }
               else if (msgHdr.msg_id == SNS_SMGR_BUFFERING_IND_V01)
               {
                  err = sns_sam_process_smgr_buffering_ind(msgPtr);
               }
               else if (msgHdr.msg_id == SNS_SMGR_BUFFERING_QUERY_IND_V01)
               {
                  err = sns_sam_process_smgr_buffering_query_ind(msgPtr);
               }
               else if (msgHdr.msg_id == SNS_SMGR_REG_HW_MD_INT_IND_V01)
               {
                  sns_sam_process_smgr_mot_int_ind(msgPtr);
               }
               else
               {
                  SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_SMGR_IND_MSGID,
                                 msgHdr.msg_id);
               }
            }
            break;
         }

         case SNS_MODULE_DSPS_SAM:
         case SNS_MODULE_APPS_SAM:
         {
            if (msgHdr.msg_type == SNS_SMR_MSG_TYPE_RESP)
            {
               err = sns_sam_process_sam_resp(msgPtr);
            }
            else if (msgHdr.msg_type == SNS_SMR_MSG_TYPE_IND)
            {
               err = sns_sam_process_sam_ind(msgPtr);
            }
            break;
         }

         case SNS_MODULE_APPS_TIME:
         {
            if (msgHdr.msg_type == SNS_SMR_MSG_TYPE_RESP)
            {
               err = sns_sam_process_time_resp(msgPtr);
            }
            break;
         }

         case SNS_MODULE_DSPS_PM:
         {
            SNS_SAM_DEBUG3(MEDIUM, DBG_SAM_PROCESS_MSG_RCVD_MSG,
                           msgHdr.msg_id, msgHdr.src_module, msgHdr.svc_num);
            SNS_SAM_DEBUG2(MEDIUM, DBG_SAM_PROCESS_MSG_STATUS,
                           msgHdr.msg_type, err);
            break;
         }

         default:
         {
            SNS_SAM_DEBUG3(MEDIUM, DBG_SAM_MSG_DROP,
                           msgHdr.msg_id, msgHdr.src_module, msgHdr.svc_num);
            break;
         }
      }

      sns_smr_msg_free(msgPtr);
   }

   return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_reg_algo_svc
  =========================================================================*/
/*!
  @brief Register the algorithm with SAM. This is expected to be done
         at SAM initialization for all algorithms to be handled by SAM

  @param[i] algoSvcId: Algorithm service id

  @return Sensors error code
*/
/*=======================================================================*/
sns_err_code_e sns_sam_reg_algo_svc(
   uint8_t algoSvcId)
{
   uint8_t i;
   sns_err_code_e err = SNS_ERR_FAILED;

   //check if algorithm already registered
   for (i = 0; i < SNS_SAM_MAX_ALGOS && sns_sam_algo_dbase[i] != NULL; i++)
   {
      if (sns_sam_algo_dbase[i]->serviceId == algoSvcId)
      {
         SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_REG_ALGO_SUCCESS,
                        algoSvcId);
         return SNS_SUCCESS;
      }
   }

   //register algorithm
   if (i < SNS_SAM_MAX_ALGOS)
   {
      uint8_t algoIndex = i;

      sns_sam_algo_dbase[algoIndex] = SNS_OS_MALLOC(SNS_SAM_DBG_MOD, sizeof(sns_sam_algo_s));
      SNS_ASSERT(sns_sam_algo_dbase[algoIndex] != NULL);

      sns_sam_algo_dbase[algoIndex]->serviceId = algoSvcId;

      sns_sam_algo_dbase[algoIndex]->algoDepCount = 0;
      for (i = 0; i < SNS_SAM_MAX_ALGO_DEPS; i++)
      {
         sns_sam_algo_dbase[algoIndex]->algoDepDbase[i] = SNS_SAM_INVALID_ID;
      }
      sns_sam_algo_dbase[algoIndex]->regItemType = SNS_SAM_REG_ITEM_TYPE_NONE;
      sns_sam_algo_dbase[algoIndex]->defSensorReportRate = 0;
      sns_sam_algo_dbase[algoIndex]->dataSyncRequired = false;

      err = sns_sam_reg_algo(sns_sam_algo_dbase[algoIndex], sns_sam_sensor_uuids);
      if( sns_sam_algo_dbase[algoIndex]->regItemType != SNS_SAM_REG_ITEM_TYPE_NONE )
      {
         sns_sam_last_algo_with_registry_dep = algoSvcId;
      }
#ifndef  SNS_DSPS_BUILD
      if( err != SNS_SUCCESS )
      {
         SNS_OS_FREE( sns_sam_algo_dbase[algoIndex] );
         sns_sam_algo_dbase[algoIndex] = NULL;
      }
#endif
   }

   if (err == SNS_SUCCESS)
   {
      SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_REG_ALGO_SUCCESS,
                     algoSvcId);
   }
   else
   {
      SNS_SAM_DEBUG1(ERROR, DBG_SAM_REG_ALGO_ERR, algoSvcId);
   }

   return err;
}

/*=========================================================================
  FUNCTION:  sns_sam_process_events
  =========================================================================*/
/*!
  @brief Wait for events and process signaled events.

  @return None
*/
/*=======================================================================*/
static void sns_sam_process_events(void)
{
   uint8_t err;
   OS_FLAGS sigFlags;

   //wait for event
   sigFlags = sns_os_sigs_pend(sns_sam_sig_event,
                               sam_sig_mask,
                               OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME,
                               0,
                               &err);
   SNS_ASSERT(err == 0);

   //message event check
   if (sigFlags & SNS_SAM_MSG_SIG)
   {
      sns_sam_process_msg();
      sigFlags &= (~SNS_SAM_MSG_SIG);
   }
   //client report timer event check
   if (sigFlags & SNS_SAM_REPORT_TIMER_SIG)
   {
      sns_sam_handle_report_timeout();
      sigFlags &= (~SNS_SAM_REPORT_TIMER_SIG);
   }

   sns_sam_mr_handle_event( &sigFlags );
   if (sigFlags != 0)
   {
      SNS_SAM_DEBUG1(MEDIUM, DBG_SAM_PROCESS_EVT_UNKWN_EVT, sigFlags);
   }
}

/*=========================================================================
  FUNCTION:  sns_sam_config_algos
  =========================================================================*/
/*!
  @brief Configure algorithms

  @detail
  All algorithms are executed in this task context.
  Waits on events primarily from sensors manager or client.

  @param[i] argPtr: pointer to task argument

  @return None
*/
/*=======================================================================*/
static void sns_sam_config_algos(void)
{
   uint8_t i;

   // Request for AMD group registry items
   // AMD config paras are applied to all algos implementing
   // the absolute rest detector.
   sns_err_code_e err =
      sns_sam_req_reg_data(SNS_SAM_REG_ITEM_TYPE_GROUP,
                           SNS_REG_SAM_GROUP_AMD_V02);

   if (SNS_SUCCESS == err)
   {
      sns_sam_get_registry_read_resp(SNS_SAM_REG_READ_TIMER_LOW_PERIOD_USEC);
   }
   else
   {
      SNS_SAM_DEBUG1(HIGH, DBG_SAM_REG_REQ_FAIL, err);
   }

   // Request for Gyro AMD group registry items
   // Gyro AMD config paras are applied to all algos implementing
   // the gyro based absolute rest detector.
   err = sns_sam_req_reg_data(SNS_SAM_REG_ITEM_TYPE_GROUP,
                           SNS_REG_SAM_GROUP_GYRO_AMD_V02);

   if (SNS_SUCCESS == err)
   {
      sns_sam_get_registry_read_resp(SNS_SAM_REG_READ_TIMER_LOW_PERIOD_USEC);
   }
   else
   {
      SNS_SAM_DEBUG1(HIGH, DBG_SAM_REG_REQ_FAIL, err);
   }


   for (i = 0;
        i < SNS_SAM_MAX_ALGOS && sns_sam_algo_dbase[i] != NULL; i++)
   {
      if (sns_sam_algo_dbase[i]->regItemType != SNS_SAM_REG_ITEM_TYPE_NONE)
      {
         uint32_t timeout = SNS_SAM_REG_READ_TIMER_LOW_PERIOD_USEC;
         if( sns_sam_last_algo_with_registry_dep == sns_sam_algo_dbase[i]->serviceId )
         {
            timeout = SNS_SAM_REG_READ_TIMER_HIGH_PERIOD_USEC;
         }

         err = sns_sam_req_reg_data(sns_sam_algo_dbase[i]->regItemType,
                                    sns_sam_algo_dbase[i]->regItemId);

         if (SNS_SUCCESS == err)
         {
            sns_sam_get_registry_read_resp(timeout);
         }
         else
         {
            /* Could not send request message. Use default cal. Already set for
               default at start of init process */
            SNS_SAM_DEBUG1(HIGH, DBG_SAM_REG_REQ_FAIL, err);
         }
      }
   }
}

/*=========================================================================
  FUNCTION:  sns_sam_init_time_state
  =========================================================================*/
void sns_sam_init_time_state(void)
{
  /* Resets time service parameters */
  sns_sam_time_state.ts_offset  = 0;
  sns_sam_time_state.ts_cnt     = 0;
  sns_sam_time_state.ts_offset_valid = false;

  /* Resets DSPS clock rollver parameters */
  /* Note: this is a temporary solution. MUST use the solution from Time
     Service eventually, because ts_dsps_ro_cnt is reset when SAM reset is
     called (that doesn't necessarily mean DSPS core was reset).
   */
  sns_sam_time_state.ts_dsps_prev   = 0;
  sns_sam_time_state.ts_dsps_ro_cnt = 0;
}

/*=========================================================================
  FUNCTION:  sns_sam_get_mag_cal_module_support
  =========================================================================*/
/*!
  @brief Query for the module on which mag cal is supported

  @param[i] None

  @return None
*/
/*=======================================================================*/
static void sns_sam_get_mag_cal_module_support(void)
{
   sns_err_code_e err = SNS_SUCCESS;

   // QMag Cal Algo enable registry bit is used to determine whether Mag Cal is supported
   // on ADSP or Apps.

   err = sns_sam_req_reg_data(SNS_SAM_REG_ITEM_TYPE_GROUP,
                              SNS_REG_SCM_GROUP_QMAG_CAL_ALGO_V02);

   if (SNS_SUCCESS == err)
   {
      sns_sam_get_registry_read_resp(SNS_SAM_REG_READ_TIMER_HIGH_PERIOD_USEC);
   }
   else
   {
      SNS_SAM_PRINTF1(ERROR, "failed to receive registry data &d", SNS_REG_SCM_GROUP_QMAG_CAL_ALGO_V02);
   }
}

/*=========================================================================
  FUNCTION:  sns_sam_mr_reg_algos
  =========================================================================*/
/*!
  @brief Registers all supported algorithms with QCSI

  @param[i] None

  @return None
*/
/*=======================================================================*/
static void sns_sam_mr_reg_algos(void)
{
   uint8_t i;

   // Register supported algorithms with QCSI
   for (i = 0; i < SNS_SAM_MAX_ALGOS && sns_sam_algo_dbase[i] != NULL; i++)
   {
      sns_sam_mr_qcsi_reg(sns_sam_algo_dbase[i]->serviceId);
   }
}

/*=========================================================================
  FUNCTION:  sns_sam_task
  =========================================================================*/
/*!
  @brief Sensors algorithm manager task

  @detail
  All algorithms are executed in this task context.
  Waits on events primarily from sensors manager or client.

  @param[i] argPtr: pointer to task argument

  @return None
*/
/*=======================================================================*/
static void sns_sam_task(
   void *argPtr)
{
   uint8_t i;
   //uint32_t numModemContexts;
   //uint8_t modemContexts[32];
   bool gotSensorInfo = false;

   UNREFERENCED_PARAMETER(argPtr);

   sns_sam_init_time_state();

   SNS_OS_MEMSET( sns_sam_sensor_uuids, 0, sizeof(sns_sam_sensor_uuids) );
   SNS_OS_MEMSET( sns_sam_sensor_info_dbase, 0, sizeof(sns_sam_sensor_info_dbase) );

   //initialize algorithm database
   for (i = 0; i < SNS_SAM_MAX_ALGOS; i++)
   {
      sns_sam_algo_dbase[i] = NULL;
   }

   //initialize algorithm instance database
   for (i = 0; i < SNS_SAM_MAX_ALGO_INSTS; i++)
   {
      sns_sam_algo_inst_dbase[i] = NULL;
   }
   sns_sam_algo_inst_count = 0;

   //initialize client request database
   for (i = 0; i < SNS_SAM_MAX_CLIENT_REQS; i++)
   {
      sns_sam_client_req_dbase[i] = NULL;
   }
   sns_sam_client_req_count = 0;

   //initialize sensor data request database
   for (i = 0; i < SNS_SAM_MAX_DATA_REQS; i++)
   {
      sns_sam_data_req_dbase[i] = NULL;
   }
   sns_sam_data_req_count = 0;

   sam_sig_mask = SNS_SAM_SIGNAL_MASK;

   //register SAM to SMR so as to receive messages
   sns_sam_mr_init();

   //get information on supported sensors
   gotSensorInfo = sns_sam_get_sensor_info(sns_sam_sig_event, sns_sam_sensor_info_dbase);

   if( gotSensorInfo )
   {
      // Autodetect is done. Get UUIDs of detected sensors
      sns_sam_get_all_sensor_uuids(sns_sam_sig_event, sns_sam_sensor_uuids);
   }
   else
   {
      SNS_SAM_PRINTF0(ERROR, "SMGR not ready!");
   }

   //get information on what the modem supports
   // sns_sam_get_modem_info(sns_sam_sig_event, &numModemContexts, modemContexts);

#if defined(SNS_PCSIM) || defined(SNS_QDSP_SIM)
   sns_sam_sensor_info_dbase[SNS_SAM_ACCEL].valid = true;
   sns_sam_sensor_info_dbase[SNS_SAM_GYRO].valid = true;
   sns_sam_sensor_info_dbase[SNS_SAM_MAG].valid = true;
   //numModemContexts = 0;
#endif

   sns_sam_smgr_buffering_flag = sns_sam_detect_smgr_buffering( sns_sam_sig_event );

   if( sns_sam_smgr_buffering_flag )
   {
      // If buffering is supported, check if default sensor report rates are present in registry
      // A registry version check is sufficient to get this info.
      sns_sam_sensor_report_rate_available = sns_sam_check_sensor_report_rate( sns_sam_sig_event );
   }

   // query the module on which mag cal is supported
   sns_sam_get_mag_cal_module_support();

   //register algorithms with SAM
   sns_sam_last_algo_with_registry_dep = 0;
   sns_sam_reg_algos();

   //configure algorithms
   sns_sam_config_algos();

   // kickstart persistent algorithm instances
   // sns_sam_start_persistent_instances(numModemContexts, modemContexts);

   //initialize GPIOs
   sns_sam_init_gpios();

   //register algorithm services with QCSI
   sns_sam_mr_reg_algos();

   SNS_SAM_DEBUG0(MEDIUM, DBG_SAM_SAM_TASK_SAMSTARTED);

   sns_init_done();

#ifdef SNS_SAM_UNIT_TEST
   sns_sam_test_init();
#endif

   while (1)
   {
      sns_sam_process_events(); //wait for events and process received events
   }
}

/*=========================================================================
  FUNCTION:  sns_sam_sensor_info_dbase_acc
  =========================================================================*/
/*!
  @brief Gets sensor info for specified type

  @return Sensor info structure
*/
/*=======================================================================*/
sns_sam_sensor_info_s * sns_sam_sensor_info_dbase_acc(uint8_t type)
{
   if( type < SNS_SAM_NUM_SENSORS )
   {
      return &sns_sam_sensor_info_dbase[type];
   }
   return NULL;
}


/*---------------------------------------------------------------------------
 * Externalized Function Definitions
 * -------------------------------------------------------------------------*/
/*=========================================================================
  FUNCTION:  sns_sam_init
  =========================================================================*/
/*!
  @brief Sensors algorithm manager initialization.
         Creates the SAM task and internal databases.

  @return Sensors error code
*/
/*=======================================================================*/
sns_err_code_e sns_sam_init(void)
{
   uint8_t err;

   //initialize events
   sns_sam_sig_event = sns_os_sigs_create((OS_FLAGS)SNS_SAM_MSG_SIG, &err);
   sns_os_sigs_add(sns_sam_sig_event, SNS_SAM_REPORT_TIMER_SIG);
   sns_os_sigs_add(sns_sam_sig_event, SNS_SAM_QMI_DISC_SIG);
   SNS_ASSERT(sns_sam_sig_event != NULL);

   //create the SAM task
#if defined(SNS_DSPS_BUILD) || defined(SNS_PCSIM)
   err = sns_os_task_create_ext(sns_sam_task,
                            NULL,
                            &sns_sam_task_stk[SNS_SAM_MODULE_STK_SIZE-1],
                            SNS_SAM_MODULE_PRIORITY,
                            SNS_SAM_MODULE_PRIORITY,
                            &sns_sam_task_stk[0],
                            SNS_SAM_MODULE_STK_SIZE,
                            (void *)0,
                            OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR,
                            (uint8_t*)("SNS_SAM"));
#else
   err = sns_os_task_create(sns_sam_task,
                            NULL,
                            &sns_sam_task_stk[SNS_SAM_MODULE_STK_SIZE-1],
                            SNS_SAM_MODULE_PRIORITY);
#endif
   SNS_ASSERT(err == 0);

   return SNS_SUCCESS;
}
