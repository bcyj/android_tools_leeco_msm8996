/*============================================================================
  FILE:         SlimProviderCommon.c

  OVERVIEW:     This file defines the implementation of common methods shared
                between sensor providers

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "log_util.h"
#include "msg_q.h"
#include "SlimProviderCommon.h"
#include "slim_core.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

#undef LOG_TAG
#define LOG_TAG "slim_provider_common"

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/
static void* g_p_message_queue = NULL;
/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/
/**
@brief Map to correct SLIM daemon message type before adding to message task

Used for mapping SLIM indication message id to appropriate SLIM daemon message type
before adding to message task

@param[in] slimMessageId  Indication message id
*/
static SlimDaemonMessageType MapSlimDaemonMessageType
(
    slimMessageIdEnumT slimMessageId
)
{
    ENTRY_LOG();
    SlimDaemonMessageType slimApMessageType = SLIM_DAEMON_INVALID;
    switch(slimMessageId)
    {
    case eSLIM_MESSAGE_ID_SENSOR_DATA_IND:
        slimApMessageType = SLIM_DAEMON_SENSOR_DATA_INJECT;
        break;
    case eSLIM_MESSAGE_ID_MOTION_DATA_IND:
        slimApMessageType = SLIM_DAEMON_MOTION_DATA_INJECT;
        break;
    case eSLIM_MESSAGE_ID_PEDOMETER_IND:
        slimApMessageType = SLIM_DAEMON_PEDOMETER_INJECT;
        break;
    default:
        break;
    }

    return slimApMessageType;
}

/**
@brief Adds messages to daemon message queue

Function adds messages to daemon message queue

@param  msg_q_data: message queue data.
@param  msg_obj: message.
@param  msg_sz: message size.
*/
static bool sp_msg_q_snd(void* msg_q_data, void* msg_obj, uint32_t msg_sz)
{
   void (*dealloc)(void*) = free;
   ENTRY_LOG();
   void* msg_obj_cpy = malloc(msg_sz);
   if( msg_obj_cpy == NULL )
   {
      LOC_LOGE("%s: Memory allocation failure", __FUNCTION__);
      return false;
   }

   memcpy(msg_obj_cpy, msg_obj, msg_sz);

   msq_q_err_type rv = msg_q_snd(msg_q_data, msg_obj_cpy, dealloc);
   if ( rv != eMSG_Q_SUCCESS )
   {
      LOC_LOGE("%s: Invalid Msg Queue send!", __FUNCTION__);
      dealloc(msg_obj_cpy);
      return false;
   }

   return true;
}

/*----------------------------------------------------------------------------
 * Externalized Function Definitions
 * -------------------------------------------------------------------------*/

/**
@brief Initialize message queue

Initialize message queue

@param p_msg_q: Pointer to message queue
*/
bool slim_message_queue_init(void* p_msg_q)
{
   LOC_LOGI("%s: Initializing message queue", __FUNCTION__);

   g_p_message_queue = p_msg_q;

   return true;
}

/**
@brief Route function for data indications

Provider modules should use this function to route data indications through
SLIM-AP daemon task.

@param[in] e_Provider         SLIM provider.
@param[in] e_Service          SLIM service.
@param[in] e_Error            Indication error code.
@param[in] e_MessageId        Indication message id.
@param     pSlimDaemonMessage Pointer to message.
*/
void slim_ProviderRouteIndication
(
  slim_ProviderEnumType e_Provider,
  slimServiceEnumT e_Service,
  slimErrorEnumT e_Error,
  slimMessageIdEnumT e_MessageId,
  SlimDaemonMessage* pSlimDaemonMessage
)
{
    //add to data task
    ENTRY_LOG();
    pSlimDaemonMessage->msgHeader.msgSize = sizeof(*pSlimDaemonMessage);
    pSlimDaemonMessage->msgHeader.msgType = MapSlimDaemonMessageType(e_MessageId);

    pSlimDaemonMessage->msgData.ind.eProvider = e_Provider;
    pSlimDaemonMessage->msgData.ind.eService = e_Service;
    pSlimDaemonMessage->msgData.ind.eError = e_Error;
    pSlimDaemonMessage->msgData.ind.eMessageId = e_MessageId;

    sp_msg_q_snd(g_p_message_queue, pSlimDaemonMessage, sizeof(*pSlimDaemonMessage));
}

/**
@brief Dispatch function for data indications

Sensor providers should use this function to dispatch data indications
to SLIM core.

@param     pz_ProviderStatus  Pointer to the provider status.
@param[in] pSlimDaemonMessage Message structure.
*/
void slim_ProviderDispatchIndication
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  const SlimDaemonMessage* pSlimDaemonMessage
)
{
    ENTRY_LOG();

    uint32 q_DataSize = 0;
    void *p_Data = NULL;

    switch (pSlimDaemonMessage->msgHeader.msgType)
    {
    case SLIM_DAEMON_SENSOR_DATA_INJECT:
        q_DataSize = sizeof(pSlimDaemonMessage->msgData.ind.data.sensorData);
        p_Data = (void*)&pSlimDaemonMessage->msgData.ind.data.sensorData;
        break;
    case SLIM_DAEMON_PEDOMETER_INJECT:
        q_DataSize = sizeof(pSlimDaemonMessage->msgData.ind.data.pedometerData);
        p_Data = (void*)&pSlimDaemonMessage->msgData.ind.data.pedometerData;
        break;
    case SLIM_DAEMON_MOTION_DATA_INJECT:
        q_DataSize = sizeof(pSlimDaemonMessage->msgData.ind.data.motionData);
        p_Data = (void*)&pSlimDaemonMessage->msgData.ind.data.motionData;
        break;
    default:
        LOC_LOGE("%s:%d]: Unknown indication received %d.\n",
                 __func__, __LINE__, pSlimDaemonMessage->msgHeader.msgType);
        break;
    }

    slim_ServiceStatusDispatchDataToRegistered(
      pz_ProviderStatus,
      pSlimDaemonMessage->msgData.ind.eProvider,
      pSlimDaemonMessage->msgData.ind.eService,
      pSlimDaemonMessage->msgData.ind.eError,
      pSlimDaemonMessage->msgData.ind.eMessageId,
      q_DataSize,
      p_Data);
}

/**
@brief Route function for time responses

Provider modules should use this function to route time responses through
SLIM-AP daemon task.

@param[in] l_TxnId           SLIM transaction id.
@param[in] e_Provider        SLIM client handle.
@param[in] e_Error           Error code.
@param[in] t_RemoteTimeMsecs Remote time of provider.
*/
void slim_ProviderRouteTimeResponse
(
  int32 l_TxnId,
  slim_ProviderEnumType e_Provider,
  slimErrorEnumT e_Error,
  uint64 t_RemoteTimeMsecs
)
{
    ENTRY_LOG();
    SlimDaemonMessage z_Message;
    memset(&z_Message, 0, sizeof(z_Message));
    z_Message.msgHeader.msgSize = sizeof(z_Message);
    z_Message.msgHeader.msgType = SLIM_DAEMON_TIME_SYNC_DATA_INJECT;
    z_Message.msgData.response.serviceTxnId = l_TxnId;
    z_Message.msgData.response.eProvider = e_Provider;
    z_Message.msgData.response.eError = e_Error;
    z_Message.msgData.response.data.remoteProviderTime = t_RemoteTimeMsecs;

    sp_msg_q_snd(g_p_message_queue, &z_Message, sizeof(z_Message));
}

/**
@brief Dispatch function for time response.

AP provider modules should use this function to dispatch time responses
to SLIM core.

@param[in] pSlimDaemonMessage Message structure.
*/
void slim_ProviderDispatchTimeResponse
(
  const SlimDaemonMessage* pSlimDaemonMessage
)
{
  const SlimDaemonMsgResponseData *pz_Data = NULL;

  ENTRY_LOG();

  if (NULL != pSlimDaemonMessage)
  {
    switch (pSlimDaemonMessage->msgHeader.msgType)
    {
    case SLIM_DAEMON_TIME_SYNC_DATA_INJECT:
      pz_Data = &pSlimDaemonMessage->msgData.response;
      break;
    default:
      LOC_LOGE("%s:%d]: Unknown time response received %d.\n",
               __func__, __LINE__, pSlimDaemonMessage->msgHeader.msgType);
      break;
    }

    if (NULL != pz_Data)
    {
      slim_CoreNotifyTimeResponse(
          pz_Data->serviceTxnId,
          pz_Data->eProvider,
          pz_Data->eError,
          pz_Data->data.remoteProviderTime,
          pz_Data->data.remoteProviderTime);
    }
  }
}

/**
@brief Route function for generic responses

Provider modules should use this function to route generic service responses
through SLIM-AP daemon task.

@param[in] l_TxnId    SLIM transaction id.
@param[in] e_Provider SLIM client handle.
@param[in] e_Error    Error code.
*/
void slim_ProviderRouteGenericResponse
(
  int32 l_TxnId,
  slim_ProviderEnumType e_Provider,
  slimErrorEnumT e_Error
)
{
    ENTRY_LOG();
    SlimDaemonMessage z_Message;
    memset(&z_Message, 0, sizeof(z_Message));
    z_Message.msgHeader.msgSize = sizeof(z_Message);
    z_Message.msgHeader.msgType = SLIM_DAEMON_GENERIC_SERVICE_RESPONSE;
    z_Message.msgData.response.serviceTxnId = l_TxnId;
    z_Message.msgData.response.eProvider = e_Provider;
    z_Message.msgData.response.eError = e_Error;

    sp_msg_q_snd(g_p_message_queue, &z_Message, sizeof(z_Message));
}

/**
@brief Dispatch function for generic responses

Provider modules should use this function to dispatch generic service responses
to SLIM core.

@param[in] pSlimDaemonMessage Message structure.
*/
void slim_ProviderDispatchGenericResponse
(
    const SlimDaemonMessage *pSlimDaemonMessage
)
{
  ENTRY_LOG();

  if (NULL != pSlimDaemonMessage)
  {
    const SlimDaemonMsgResponseData *pz_Data =
            &pSlimDaemonMessage->msgData.response;

    slim_CoreNotifyResponseNoPayload(
        pz_Data->serviceTxnId,
        pz_Data->eProvider,
        pz_Data->eError);
  }
}

