/******************************************************************************
  @file    qcril_qmi_ims_misc.c
  @brief   qcril qmi - ims misc

  DESCRIPTION
    Utility functions for ims socket.

  ---------------------------------------------------------------------------

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qcril_qmi_ims_misc.h"
#include "qcril_log.h"
#include "qcrili.h"

#define WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING "CD-04"

//===========================================================================
// qcril_qmi_ims_map_event_to_request
//===========================================================================
Ims__MsgId qcril_qmi_ims_map_event_to_request(int event)
{
  Ims__MsgId ret;

  switch ( event )
  {
    case QCRIL_EVT_IMS_SOCKET_IMS_REGISTRATION_STATE:
      ret = IMS__MSG_ID__REQUEST_IMS_REGISTRATION_STATE;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_DIAL:
      ret = IMS__MSG_ID__REQUEST_DIAL;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_ANSWER:
      ret = IMS__MSG_ID__REQUEST_ANSWER;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_HANGUP:
      ret = IMS__MSG_ID__REQUEST_HANGUP;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_LAST_CALL_FAIL_CAUSE:
      ret = IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_GET_CURRENT_CALLS:
      ret = IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_WAITING_OR_BACKGROUND:
      ret = IMS__MSG_ID__REQUEST_HANGUP_WAITING_OR_BACKGROUND;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_FOREGROUND_RESUME_BACKGROUND:
      ret = IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
      ret = IMS__MSG_ID__REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_CONFERENCE:
        ret = IMS__MSG_ID__REQUEST_CONFERENCE;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_EXIT_ECBM:
        ret = IMS__MSG_ID__REQUEST_EXIT_EMERGENCY_CALLBACK_MODE;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_DTMF:
    case QCRIL_EVT_IMS_SOCKET_REQ_STOP_CONT_DTMF:
        ret = IMS__MSG_ID__REQUEST_DTMF;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_DTMF_START:
        ret = IMS__MSG_ID__REQUEST_DTMF_START;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_DTMF_STOP:
        ret = IMS__MSG_ID__REQUEST_DTMF_STOP;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE:
        ret = IMS__MSG_ID__REQUEST_MODIFY_CALL_INITIATE;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM:
        ret = IMS__MSG_ID__REQUEST_MODIFY_CALL_CONFIRM;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CLIP:
        ret = IMS__MSG_ID__REQUEST_QUERY_CLIP;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_GET_CLIR:
        ret = IMS__MSG_ID__REQUEST_GET_CLIR;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SET_CLIR:
        ret = IMS__MSG_ID__REQUEST_SET_CLIR;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_FORWARD_STATUS:
        ret = IMS__MSG_ID__REQUEST_QUERY_CALL_FORWARD_STATUS;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_FORWARD_STATUS:
        ret = IMS__MSG_ID__REQUEST_SET_CALL_FORWARD_STATUS;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_WAITING:
        ret = IMS__MSG_ID__REQUEST_QUERY_CALL_WAITING;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_WAITING:
        ret = IMS__MSG_ID__REQUEST_SET_CALL_WAITING;
        break;

    case QCRIL_EVT_IMS_SOCKET_REQ_IMS_REG_STATE_CHANGE:
      ret = IMS__MSG_ID__REQUEST_IMS_REG_STATE_CHANGE;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SET_SUPP_SVC_NOTIFICATION:
      ret = IMS__MSG_ID__REQUEST_SET_SUPP_SVC_NOTIFICATION;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_ADD_PARTICIPANT:
      ret = IMS__MSG_ID__REQUEST_ADD_PARTICIPANT;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_QUERY_SERVICE_STATUS:
      ret = IMS__MSG_ID__REQUEST_QUERY_SERVICE_STATUS;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SET_SERVICE_STATUS:
      ret = IMS__MSG_ID__REQUEST_SET_SERVICE_STATUS;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS:
      ret = IMS__MSG_ID__REQUEST_SUPP_SVC_STATUS;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_CALL_DEFLECTION:
      ret = IMS__MSG_ID__REQUEST_DEFLECT_CALL;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_GET_COLR:
      ret = IMS__MSG_ID__REQUEST_GET_COLR;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SET_COLR:
      ret = IMS__MSG_ID__REQUEST_SET_COLR;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_QUERY_VT_CALL_QUALITY:
      ret = IMS__MSG_ID__REQUEST_QUERY_VT_CALL_QUALITY;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SET_VT_CALL_QUALITY:
      ret = IMS__MSG_ID__REQUEST_SET_VT_CALL_QUALITY;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_HOLD:
      ret = IMS__MSG_ID__REQUEST_HOLD;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_RESUME:
      ret = IMS__MSG_ID__REQUEST_RESUME;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SET_TTY_MODE:
      ret = IMS__MSG_ID__REQUEST_SEND_UI_TTY_MODE;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_STATISTICS:
      ret = IMS__MSG_ID__REQUEST_GET_RTP_STATISTICS;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_ERROR_STATISTICS:
      ret = IMS__MSG_ID__REQUEST_GET_RTP_ERROR_STATISTICS;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_GET_WIFI_CALLING_STATUS:
      ret = IMS__MSG_ID__REQUEST_GET_WIFI_CALLING_STATUS;
      break;

    case QCRIL_EVT_IMS_SOCKET_REQ_SET_WIFI_CALLING_STATUS:
      ret = IMS__MSG_ID__REQUEST_SET_WIFI_CALLING_STATUS;
      break;

    default:
      QCRIL_LOG_DEBUG("didn't find direct mapping for event %d", event);
      if ( event > QCRIL_EVT_IMS_SOCKET_REQ_BASE && event < QCRIL_EVT_IMS_SOCKET_REQ_MAX )
      {
        ret = event - QCRIL_EVT_IMS_SOCKET_REQ_BASE;
      }
      else
      {
        ret = IMS__MSG_ID__UNKNOWN_REQ;
      }
  }

  QCRIL_LOG_INFO("event %d mapped to ims_msg %d", event, ret);
  return ret;
} // qcril_qmi_ims_map_event_to_request

//===========================================================================
// qcril_qmi_ims_map_request_to_event
//===========================================================================
qcril_evt_e_type qcril_qmi_ims_map_request_to_event(Ims__MsgId msg_id)
{
  qcril_evt_e_type ret = QCRIL_EVT_NONE;

  switch ( msg_id )
  {
    case IMS__MSG_ID__REQUEST_IMS_REGISTRATION_STATE:
      ret = QCRIL_EVT_IMS_SOCKET_IMS_REGISTRATION_STATE;
      break;

    case IMS__MSG_ID__REQUEST_DIAL:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_DIAL;
      break;

    case IMS__MSG_ID__REQUEST_ANSWER:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_ANSWER;
      break;

    case IMS__MSG_ID__REQUEST_HANGUP:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_HANGUP;
      break;

    case IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_LAST_CALL_FAIL_CAUSE;
      break;

    case IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_GET_CURRENT_CALLS;
      break;

    case IMS__MSG_ID__REQUEST_HANGUP_WAITING_OR_BACKGROUND:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_WAITING_OR_BACKGROUND;
      break;

    case IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_FOREGROUND_RESUME_BACKGROUND;
      break;

    case IMS__MSG_ID__REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE;
      break;

    case IMS__MSG_ID__REQUEST_CONFERENCE:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_CONFERENCE;
        break;

    case IMS__MSG_ID__REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_EXIT_ECBM;
        break;

    case IMS__MSG_ID__REQUEST_DTMF:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_DTMF;
        break;

    case IMS__MSG_ID__REQUEST_DTMF_START:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_DTMF_START;
        break;

    case IMS__MSG_ID__REQUEST_DTMF_STOP:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_DTMF_STOP;
        break;

    case IMS__MSG_ID__REQUEST_MODIFY_CALL_INITIATE:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE;
        break;

    case IMS__MSG_ID__REQUEST_MODIFY_CALL_CONFIRM:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM;
        break;

    case IMS__MSG_ID__REQUEST_QUERY_CLIP:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CLIP;
        break;

    case IMS__MSG_ID__REQUEST_GET_CLIR:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_GET_CLIR;
        break;

    case IMS__MSG_ID__REQUEST_SET_CLIR:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_SET_CLIR;
        break;

    case IMS__MSG_ID__REQUEST_QUERY_CALL_FORWARD_STATUS:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_FORWARD_STATUS;
        break;

    case IMS__MSG_ID__REQUEST_SET_CALL_FORWARD_STATUS:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_FORWARD_STATUS;
        break;

    case IMS__MSG_ID__REQUEST_QUERY_CALL_WAITING:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_WAITING;
        break;

    case IMS__MSG_ID__REQUEST_SET_CALL_WAITING:
        ret = QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_WAITING;
        break;

    case IMS__MSG_ID__REQUEST_IMS_REG_STATE_CHANGE:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_IMS_REG_STATE_CHANGE;
      break;

    case IMS__MSG_ID__REQUEST_SET_SUPP_SVC_NOTIFICATION:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_SET_SUPP_SVC_NOTIFICATION;
      break;

    case IMS__MSG_ID__REQUEST_ADD_PARTICIPANT:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_ADD_PARTICIPANT;
      break;

    case IMS__MSG_ID__REQUEST_QUERY_SERVICE_STATUS:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_QUERY_SERVICE_STATUS;
      break;

    case IMS__MSG_ID__REQUEST_SET_SERVICE_STATUS:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_SET_SERVICE_STATUS;
      break;

    case IMS__MSG_ID__REQUEST_SUPP_SVC_STATUS:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS;
      break;

    case IMS__MSG_ID__REQUEST_DEFLECT_CALL:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_CALL_DEFLECTION;
      break;

    case IMS__MSG_ID__REQUEST_GET_COLR:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_GET_COLR;
      break;

    case IMS__MSG_ID__REQUEST_SET_COLR:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_SET_COLR;
      break;

    case IMS__MSG_ID__REQUEST_QUERY_VT_CALL_QUALITY:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_QUERY_VT_CALL_QUALITY;
      break;

    case IMS__MSG_ID__REQUEST_SET_VT_CALL_QUALITY:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_SET_VT_CALL_QUALITY;
      break;

    case IMS__MSG_ID__REQUEST_HOLD:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_HOLD;
      break;

    case IMS__MSG_ID__REQUEST_RESUME:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_RESUME;
      break;

    case IMS__MSG_ID__REQUEST_SEND_UI_TTY_MODE:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_SET_TTY_MODE;
      break;

    case IMS__MSG_ID__REQUEST_GET_RTP_STATISTICS:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_STATISTICS;
      break;

    case IMS__MSG_ID__REQUEST_GET_RTP_ERROR_STATISTICS:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_ERROR_STATISTICS;
      break;

    case IMS__MSG_ID__REQUEST_GET_WIFI_CALLING_STATUS:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_GET_WIFI_CALLING_STATUS;
      break;

    case IMS__MSG_ID__REQUEST_SET_WIFI_CALLING_STATUS:
      ret = QCRIL_EVT_IMS_SOCKET_REQ_SET_WIFI_CALLING_STATUS;
      break;

    default:
      QCRIL_LOG_DEBUG("didn't find direct mapping for msg_id %d", msg_id);
      ret = QCRIL_EVT_NONE;
      break;
  }

  QCRIL_LOG_INFO("msg_id %d mapped to qcril_event %d", msg_id, ret);
  return ret;
} // qcril_qmi_ims_map_event_to_request
//===========================================================================
// qcril_qmi_ims_map_ril_error_to_ims_error
//===========================================================================
Ims__Error qcril_qmi_ims_map_ril_error_to_ims_error(int ril_error)
{
  Ims__Error ret;

  switch ( ril_error )
  {
    case RIL_E_SUCCESS:
      ret = IMS__ERROR__E_SUCCESS;
      break;

    case RIL_E_RADIO_NOT_AVAILABLE:
      ret = IMS__ERROR__E_RADIO_NOT_AVAILABLE;
      break;

    case RIL_E_GENERIC_FAILURE:
      ret = IMS__ERROR__E_GENERIC_FAILURE;
      break;

    case RIL_E_REQUEST_NOT_SUPPORTED:
      ret = IMS__ERROR__E_REQUEST_NOT_SUPPORTED;
      break;

    case RIL_E_CANCELLED:
      ret = IMS__ERROR__E_CANCELLED;
      break;

    default:
      ret = IMS__ERROR__E_GENERIC_FAILURE;
  }

  QCRIL_LOG_INFO("ril error %d mapped to ims error %d", ril_error, ret);
  return ret;
} // qcril_qmi_ims_map_ril_error_to_ims_error

//===========================================================================
// qcril_qmi_ims_map_ril_reg_state_to_ims_reg_state
//===========================================================================
Ims__Registration__RegState qcril_qmi_ims_map_ril_reg_state_to_ims_reg_state(int state)
{
  Ims__Registration__RegState ret;

  switch ( state )
  {
    case 0:
      ret = IMS__REGISTRATION__REG_STATE__NOT_REGISTERED;
      break;

    case 1:
      ret = IMS__REGISTRATION__REG_STATE__REGISTERED;
      break;

    default:
      ret = IMS__REGISTRATION__REG_STATE__NOT_REGISTERED;
  }

  QCRIL_LOG_INFO("ril reg state %d mapped to ims reg state %d", state, ret);

  return ret;
} // qcril_qmi_ims_map_ril_reg_state_to_ims_reg_state

//===========================================================================
// qcril_qmi_ims_map_ril_call_type_to_ims_call_type
//===========================================================================
Ims__CallType qcril_qmi_ims_map_ril_call_type_to_ims_call_type(RIL_Call_Type call_type)
{
  Ims__CallType ret;

  switch ( call_type )
  {
    case RIL_CALL_TYPE_VOICE:
      ret = IMS__CALL_TYPE__CALL_TYPE_VOICE;
      break;

    case RIL_CALL_TYPE_VS_TX:
      ret = IMS__CALL_TYPE__CALL_TYPE_VT_TX;
      break;

    case RIL_CALL_TYPE_VS_RX:
      ret = IMS__CALL_TYPE__CALL_TYPE_VT_RX;
      break;

    case RIL_CALL_TYPE_VT:
      ret = IMS__CALL_TYPE__CALL_TYPE_VT;
      break;

    case RIL_CALL_TYPE_VT_NODIR:
      ret = IMS__CALL_TYPE__CALL_TYPE_VT_NODIR;
      break;

    default:
      ret = call_type; // do not do any mapping if it does not fill in any above categories.
  }

  QCRIL_LOG_INFO("ril call_type %d mapped to ims call_type %d", call_type, ret);

  return ret;
} // qcril_qmi_ims_map_ril_call_type_to_ims_call_type

//===========================================================================
// qcril_qmi_ims_map_ril_call_domain_to_ims_call_domain
//===========================================================================
Ims__CallDomain qcril_qmi_ims_map_ril_call_domain_to_ims_call_domain(RIL_Call_Domain call_domain)
{
  Ims__CallDomain ret;

  switch ( call_domain )
  {
    case RIL_CALL_DOMAIN_UNKNOWN:
      ret = IMS__CALL_DOMAIN__CALL_DOMAIN_UNKNOWN;
      break;

    case RIL_CALL_DOMAIN_CS:
      ret = IMS__CALL_DOMAIN__CALL_DOMAIN_CS;
      break;

    case RIL_CALL_DOMAIN_PS:
      ret = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
      break;

    case RIL_CALL_DOMAIN_AUTOMATIC:
      ret = IMS__CALL_DOMAIN__CALL_DOMAIN_AUTOMATIC;
      break;

    default:
      ret = IMS__CALL_DOMAIN__CALL_DOMAIN_UNKNOWN;
  }

  QCRIL_LOG_INFO("ril call_domain %d mapped to ims call_domain %d", call_domain, ret);

  return ret;
} // qcril_qmi_ims_map_ril_call_domain_to_ims_call_domain

//===========================================================================
// qcril_qmi_ims_map_ims_call_type_to_ril_call_type
//===========================================================================
RIL_Call_Type qcril_qmi_ims_map_ims_call_type_to_ril_call_type(boolean has_call_type, Ims__CallType call_type)
{
  Ims__CallType ret = IMS__CALL_TYPE__CALL_TYPE_VOICE;

  if (has_call_type)
  {
     switch ( call_type )
     {
       case IMS__CALL_TYPE__CALL_TYPE_VOICE:
         ret = RIL_CALL_TYPE_VOICE;
         break;

       case IMS__CALL_TYPE__CALL_TYPE_VT_TX:
         ret = RIL_CALL_TYPE_VS_TX;
         break;

       case IMS__CALL_TYPE__CALL_TYPE_VT_RX:
         ret = RIL_CALL_TYPE_VS_RX;
         break;

       case IMS__CALL_TYPE__CALL_TYPE_VT:
         ret = RIL_CALL_TYPE_VT;
         break;

       case IMS__CALL_TYPE__CALL_TYPE_VT_NODIR:
         ret = RIL_CALL_TYPE_VT_NODIR;
         break;

       default:
         ret = RIL_CALL_TYPE_VOICE;
     }
  }

  QCRIL_LOG_INFO("ims has_call_type %d, call_type %d mapped to ril call_type %d", has_call_type, call_type, ret);

  return ret;
} // qcril_qmi_ims_map_ims_call_type_to_ril_call_type

//===========================================================================
// qcril_qmi_ims_map_ims_call_domain_to_ril_call_domain
//===========================================================================
RIL_Call_Domain qcril_qmi_ims_map_ims_call_domain_to_ril_call_domain(boolean has_call_domain, Ims__CallDomain call_domain)
{
  RIL_Call_Domain ret = RIL_CALL_DOMAIN_UNKNOWN;

  if (has_call_domain)
  {
     switch ( call_domain )
     {
       case IMS__CALL_DOMAIN__CALL_DOMAIN_UNKNOWN:
         ret = RIL_CALL_DOMAIN_UNKNOWN;
         break;

       case IMS__CALL_DOMAIN__CALL_DOMAIN_CS:
         ret = RIL_CALL_DOMAIN_CS;
         break;

       case IMS__CALL_DOMAIN__CALL_DOMAIN_PS:
         ret = RIL_CALL_DOMAIN_PS;
         break;

       case IMS__CALL_DOMAIN__CALL_DOMAIN_AUTOMATIC:
         ret = RIL_CALL_DOMAIN_AUTOMATIC;
         break;

       default:
         ret = RIL_CALL_DOMAIN_UNKNOWN;
     }
  }

  QCRIL_LOG_INFO("ims has_call_domain %d, call_domain %d mapped to ims call_domain %d", has_call_domain, call_domain, ret);

  return ret;
} // qcril_qmi_ims_map_ims_call_domain_to_ril_call_domain

//===========================================================================
// qcril_qmi_ims_convert_ims_token_to_ril_token
//===========================================================================
RIL_Token qcril_qmi_ims_convert_ims_token_to_ril_token(uint32_t ims_token)
{
  RIL_Token ret = qcril_malloc(sizeof(uint32_t));
  if (NULL != ret)
  {
    uint32_t *tmp = (uint32_t*) ret;
    *tmp = ims_token ^ 0x80000000;
  }
  return ret;
} // qcril_qmi_ims_convert_ims_token_to_ril_token

//===========================================================================
// qcril_qmi_ims_free_and_convert_ril_token_to_ims_token
//===========================================================================
uint32_t qcril_qmi_ims_free_and_convert_ril_token_to_ims_token(RIL_Token ril_token)
{
  uint32_t ret = 0xFFFFFFFF;
  if (ril_token)
  {
      ret = (*((uint32_t *) ril_token)) ^ 0x80000000;
      QCRIL_LOG_INFO("ims token: %d", ret);
      qcril_free((void*) ril_token);
  }
  else
  {
      QCRIL_LOG_INFO("ril_token is NULL");
  }

  return ret;
} // qcril_qmi_ims_free_and_convert_ril_token_to_ims_token

//===========================================================================
// qcril_qmi_ims_translate_ims_calldetails_to_ril_calldetails
//===========================================================================
void qcril_qmi_ims_translate_ims_calldetails_to_ril_calldetails(const Ims__CallDetails *ims_data, RIL_Call_Details* ril_data)
{
  if (NULL == ril_data || NULL == ims_data)
  {
    QCRIL_LOG_ERROR("ril_data or ims_data is NULL");
    if (NULL != ril_data)
    {
       QCRIL_LOG_INFO("ril_data is not NULL, set it to default value");
       ril_data->callType = qcril_qmi_ims_map_ims_call_type_to_ril_call_type(FALSE, 0);
       ril_data->callDomain = qcril_qmi_ims_map_ims_call_domain_to_ril_call_domain(FALSE, 0);
    }
  }
  else
  {
    ril_data->callType = qcril_qmi_ims_map_ims_call_type_to_ril_call_type(ims_data->has_calltype, ims_data->calltype);
    ril_data->callDomain = qcril_qmi_ims_map_ims_call_domain_to_ril_call_domain(ims_data->has_calldomain, ims_data->calldomain);
  }
} // qcril_qmi_ims_translate_ims_calldetails_to_ril_calldetails

//===========================================================================
// qcril_qmi_ims_translate_ims_calldetails_to_ril_calldetails
//===========================================================================
void qcril_qmi_ims_translate_ril_callmodify_to_ims_callmodify(const RIL_Call_Modify* ril_data, Ims__CallModify* ims_data)
{
  if (NULL == ril_data || NULL == ims_data)
  {
    QCRIL_LOG_ERROR("ril_data or ims_data is NULL");
    if (NULL != ims_data)
    {
       QCRIL_LOG_INFO("ims_data is not NULL, set it to default value");
       ims_data->has_callindex = FALSE;
       qcril_qmi_ims_translate_ril_calldetails_to_ims_calldetails(NULL, ims_data->calldetails);
    }
  }
  else
  {
    ims_data->has_callindex = TRUE;
    ims_data->callindex = ril_data->callIndex;
    qcril_qmi_ims_translate_ril_calldetails_to_ims_calldetails(ril_data->callDetails, ims_data->calldetails);
  }
} // qcril_qmi_ims_translate_ril_callmodify_to_ims_callmodify

//===========================================================================
// qcril_qmi_ims_translate_ril_calldetails_to_ims_calldetails
//===========================================================================
void qcril_qmi_ims_translate_ril_calldetails_to_ims_calldetails(const RIL_Call_Details* ril_data, Ims__CallDetails* ims_data)
{
  if (NULL == ril_data || NULL == ims_data)
  {
    QCRIL_LOG_ERROR("ril_data or ims_data is NULL");
    if (NULL != ims_data)
    {
       QCRIL_LOG_INFO("ims_data is not NULL, set it to default value");
       ims_data->has_calltype = FALSE;
       ims_data->has_calldomain = FALSE;
    }
  }
  else
  {
    ims_data->has_calltype = TRUE;
    ims_data->calltype = qcril_qmi_ims_map_ril_call_type_to_ims_call_type(ril_data->callType);
    ims_data->has_calldomain = TRUE;
    ims_data->calldomain = qcril_qmi_ims_map_ril_call_domain_to_ims_call_domain(ril_data->callDomain);
  }
} // qcril_qmi_ims_translate_ril_calldetails_to_ims_calldetails

//===========================================================================
// qcril_qmi_ims_translate_ril_callforwdinfo_to_ims_callforwdinfo
//===========================================================================
void qcril_qmi_ims_translate_ril_callforwdinfo_to_ims_callforwdinfo
(
 const qcril_qmi_voice_callforwd_info_param_u_type* ril_data,
 int num,
 voice_time_type_v02 *call_fwd_start_time,
 voice_time_type_v02 *call_fwd_end_time,
 Ims__CallForwardInfoList* ims_data
)
{
  int i;
  boolean failed = FALSE;
  Ims__CallForwardInfoList__CallForwardInfo* cfi_array = NULL;

  if (NULL == ril_data || NULL == ims_data || num < 0)
  {
    QCRIL_LOG_ERROR("ril_data or ims_data is NULL, or num < 0");
  }
  else
  {
    do
    {
      Ims__CallForwardInfoList tmp_cfil = IMS__CALL_FORWARD_INFO_LIST__INIT;
      memcpy(ims_data, &tmp_cfil, sizeof(Ims__CallForwardInfoList));
      ims_data->n_info = num;

      if (num > 0)
      {
        ims_data->info = qcril_malloc(sizeof(Ims__CallForwardInfoList__CallForwardInfo*) * num);
        if (NULL == ims_data->info)
        {
          QCRIL_LOG_FATAL("malloc failed");
          failed = TRUE;
          break;
        }

        cfi_array = qcril_malloc(sizeof(Ims__CallForwardInfoList__CallForwardInfo) * num);
        if (NULL == cfi_array)
        {
          QCRIL_LOG_FATAL("malloc failed");
          failed = TRUE;
          break;
        }

        Ims__CallForwardInfoList__CallForwardInfo tmp_cfil_cfi = IMS__CALL_FORWARD_INFO_LIST__CALL_FORWARD_INFO__INIT;
        for (i = 0; i < num; i++)
        {
          memcpy(&(cfi_array[i]), &tmp_cfil_cfi, sizeof(Ims__CallForwardInfoList__CallForwardInfo));

          cfi_array[i].has_status = TRUE;
          cfi_array[i].status = ril_data[i].status;

          cfi_array[i].has_reason = TRUE;
          cfi_array[i].reason = ril_data[i].reason;

          cfi_array[i].has_service_class = TRUE;
          cfi_array[i].service_class = ril_data[i].service_class;

          cfi_array[i].has_toa = TRUE;
          cfi_array[i].toa = ril_data[i].toa;

          if (ril_data[i].number)
          {
            cfi_array[i].number = qmi_ril_util_str_clone(ril_data[i].number);
          }

          cfi_array[i].has_time_seconds = TRUE;
          cfi_array[i].time_seconds = ril_data[i].no_reply_timer;

          ims_data->info[i] = &(cfi_array[i]);

          if (ims_data->info[i]->reason == QCRIL_QMI_VOICE_CCFC_REASON_UNCOND)
          {
            if (call_fwd_start_time)
            {
              ims_data->info[i]->callfwdtimerstart = qcril_malloc(sizeof(Ims__CallFwdTimerInfo));
              if (NULL == ims_data->info[i]->callfwdtimerstart)
              {
                QCRIL_LOG_FATAL("malloc failed");
                failed = TRUE;
                break;
              }
              qcril_qmi_ims__call_fwd_timer_info__init(ims_data->info[i]->callfwdtimerstart);
              qcril_qmi_ims_translate_voice_time_type_to_ims_callfwdtimerinfo(call_fwd_start_time,
                      ims_data->info[i]->callfwdtimerstart);
            }
            if (call_fwd_end_time)
            {
              ims_data->info[i]->callfwdtimerend = qcril_malloc(sizeof(Ims__CallFwdTimerInfo));
              if (NULL == ims_data->info[i]->callfwdtimerend)
              {
                QCRIL_LOG_FATAL("malloc failed");
                failed = TRUE;
                break;
              }
              qcril_qmi_ims__call_fwd_timer_info__init(ims_data->info[i]->callfwdtimerend);
              qcril_qmi_ims_translate_voice_time_type_to_ims_callfwdtimerinfo(call_fwd_end_time,
                      ims_data->info[i]->callfwdtimerend);
            }
          }
        }
      }
    } while (FALSE);
  }

  if (failed)
  {
    if (ims_data && ims_data->info)
    {
      for (i = 0; i < num; i++)
      {
        if (ims_data->info[i] && ims_data->info[i]->callfwdtimerstart)
        {
          qcril_free(ims_data->info[i]->callfwdtimerstart);
        }
        if (ims_data->info[i] && ims_data->info[i]->callfwdtimerend)
        {
          qcril_free(ims_data->info[i]->callfwdtimerend);
        }
      }
      qcril_free(ims_data->info);
    }
    qcril_free(cfi_array);
    if (ims_data)
    {
      qcril_qmi_ims__call_forward_info_list__init(ims_data);
    }
  }
} // qcril_qmi_ims_translate_ril_callforwdinfo_to_ims_callforwdinfo

//===========================================================================
// qcril_qmi_ims_translate_ril_service_status_class_to_ims_callwaitinginfo
//===========================================================================
void qcril_qmi_ims_translate_ril_service_status_class_to_ims_callwaitinginfo(int service_status, int service_class, Ims__CallWaitingInfo* ims_data)
{
  int i;

  if (NULL == ims_data)
  {
    QCRIL_LOG_ERROR("ims_data is NULL");
  }
  else
  {
    Ims__CallWaitingInfo tmp_cwi = IMS__CALL_WAITING_INFO__INIT;
    memcpy(ims_data, &tmp_cwi, sizeof(Ims__CallWaitingInfo));

    ims_data->has_service_status = TRUE;
    ims_data->service_status = service_status;

    if (service_status)
    {
      ims_data->service_class = qcril_malloc(sizeof(Ims__ServiceClass));
      if (ims_data->service_class)
      {
        Ims__ServiceClass tmp_sc = IMS__SERVICE_CLASS__INIT;
        memcpy(ims_data->service_class, &tmp_sc, sizeof(Ims__ServiceClass));
        ims_data->service_class->has_service_class = TRUE;
        ims_data->service_class->service_class = service_class;
      }
      else
      {
        QCRIL_LOG_FATAL("malloc failed");
      }
    }
  }
} // qcril_qmi_ims_translate_ril_service_status_class_to_ims_callwaitinginfo

//===========================================================================
// qcril_qmi_ims_map_qmi_ims_reg_state_to_ims_reg_state
//===========================================================================
Ims__Registration__RegState qcril_qmi_ims_map_qmi_ims_reg_state_to_ims_reg_state(uint8_t ims_registered)
{
   Ims__Registration__RegState ret;

   switch(ims_registered)
   {
   case IMSA_STATUS_NOT_REGISTERED_V01:
       ret = IMS__REGISTRATION__REG_STATE__NOT_REGISTERED;
       break;
   case IMSA_STATUS_REGISTERING_V01:
       ret = IMS__REGISTRATION__REG_STATE__REGISTERING;
       break;
   case IMSA_STATUS_REGISTERED_V01:
       ret = IMS__REGISTRATION__REG_STATE__REGISTERED;
       break;
   default:
       ret = IMS__REGISTRATION__REG_STATE__NOT_REGISTERED;
       break;
   }

   QCRIL_LOG_INFO("qmi ims_reg_state %d mapped to ims ims_reg_state %d", ims_registered, ret);

   return ret;
} // qcril_qmi_ims_map_qmi_ims_reg_state_to_ims_reg_state

//===========================================================================
// qcril_qmi_ims_translate_ril_suppsvcnotification_to_ims_suppsvcnotification
//===========================================================================
void qcril_qmi_ims_translate_ril_suppsvcnotification_to_ims_suppsvcnotification(const RIL_SuppSvcNotification* ril_data, Ims__SuppSvcNotification* ims_data)
{
  if (NULL == ril_data || NULL == ims_data)
  {
    QCRIL_LOG_ERROR("ril_data or ims_data is NULL");
    if (NULL != ims_data)
    {
       QCRIL_LOG_INFO("ims_data is not NULL, set it to default value");
       ims_data->has_notificationtype = FALSE;
       ims_data->has_code = FALSE;
       ims_data->has_index = FALSE;
       ims_data->has_type = FALSE;
       ims_data->number = NULL;
    }
  }
  else
  {
    ims_data->has_notificationtype = TRUE;
    ims_data->notificationtype = ril_data->notificationType;

    ims_data->has_code = TRUE;
    ims_data->code = ril_data->code;

    ims_data->has_index = TRUE;
    ims_data->index = ril_data->index;

    ims_data->has_type = TRUE;
    ims_data->type = ril_data->type;

    ims_data->number = qmi_ril_util_str_clone(ril_data->number);
  }
}

//===========================================================================
// qcril_qmi_ims_map_call_mode_to_ims_radiotechtype
//===========================================================================
Ims__RadioTechType qcril_qmi_ims_map_call_mode_to_ims_radiotechtype(call_mode_enum_v02 call_mode)
{
  Ims__RadioTechType ims_rat;

  switch(call_mode)
  {
    case CALL_MODE_LTE_V02:
      ims_rat = IMS__RADIO_TECH_TYPE__RADIO_TECH_LTE;
      break;

    case CALL_MODE_WLAN_V02:
      ims_rat = IMS__RADIO_TECH_TYPE__RADIO_TECH_IWLAN;
      break;

    default:
      ims_rat = IMS__RADIO_TECH_TYPE__RADIO_TECH_UNKNOWN;
      break;
  }

  return ims_rat;
}

//===========================================================================
// qcril_qmi_ims_make_ims_info
//===========================================================================
Ims__Info *qcril_qmi_ims_make_ims_info
(
 Ims__CallType calltype,
 Ims__StatusType status,
 uint32_t restrictcause,
 boolean networkmode_valid,
 Ims__RadioTechType networkmode
)
{
  Ims__Info *ims_info = NULL;

  ims_info = qcril_malloc(sizeof(Ims__Info));

  if (ims_info)
  {
    qcril_qmi_ims__info__init(ims_info);

    QCRIL_LOG_INFO("calltype = %d, status = %d, restrictcause = %d, "
                   "networkmode_valid = %d, networkmode = %d\n",
                   calltype, status, restrictcause, networkmode_valid, networkmode);

    ims_info->has_isvalid = TRUE;
    ims_info->isvalid = TRUE;

    ims_info->has_calltype = TRUE;
    ims_info->calltype = calltype;

    ims_info->has_status = TRUE;
    ims_info->status = status;

    ims_info->has_restrictcause = TRUE;
    ims_info->restrictcause = restrictcause;

    if (networkmode_valid)
    {
      ims_info->acctechstatus = qcril_malloc(1 * sizeof(Ims__StatusForAccessTech *));

      if (ims_info->acctechstatus)
      {
        ims_info->acctechstatus[0] = qcril_malloc(sizeof(Ims__StatusForAccessTech));

        if (ims_info->acctechstatus[0])
        {
          qcril_qmi_ims__status_for_access_tech__init(ims_info->acctechstatus[0]);
          ims_info->n_acctechstatus = 1;

          ims_info->acctechstatus[0]->has_networkmode = TRUE;
          ims_info->acctechstatus[0]->networkmode = networkmode;
          ims_info->acctechstatus[0]->has_status = TRUE;
          ims_info->acctechstatus[0]->status = status;
          ims_info->acctechstatus[0]->has_restrictioncause = TRUE;
          ims_info->acctechstatus[0]->restrictioncause = restrictcause;
        }
      }
    }
  }
  return ims_info;
}

//===========================================================================
// qcril_qmi_ims_translate_ril_callcapabilities_to_ims_srvstatusinfo
//===========================================================================
void qcril_qmi_ims_translate_ril_callcapabilities_to_ims_srvstatusinfo
(
 const voice_ip_call_capabilities_info_type_v02* ril_data,
 Ims__SrvStatusList* ims_data,
 Ims__CallType current_call_type,
 call_mode_enum_v02 call_mode
)
{
  int n_srvstatusinfo    = 2; // 2 - one for voip capability and one for VT capability
  boolean call_type_found = FALSE;
  Ims__RadioTechType networkmode = qcril_qmi_ims_map_call_mode_to_ims_radiotechtype(call_mode);
  Ims__CallType calltype;
  Ims__StatusType status;

  if (NULL == ril_data || NULL == ims_data)
  {
    QCRIL_LOG_ERROR("ril_data or ims_data is NULL");
  }
  else
  {
    if (current_call_type != IMS__CALL_TYPE__CALL_TYPE_UNKNOWN)
    {
      n_srvstatusinfo += 1;
    }

    ims_data->srvstatusinfo = qcril_malloc( n_srvstatusinfo * sizeof(Ims__Info*) );

    if(NULL != ims_data->srvstatusinfo)
    {
      if( ( VOICE_CALL_ATTRIB_TX_V02 == ril_data->audio_attrib ) ||
          ( VOICE_CALL_ATTRIB_RX_V02 == ril_data->audio_attrib ) ||
          ( (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02) == ril_data->audio_attrib ) )
      {
        calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
        status = IMS__STATUS_TYPE__STATUS_ENABLED;
      }
      else
      {
        calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
        status = IMS__STATUS_TYPE__STATUS_DISABLED;
      }

      ims_data->srvstatusinfo[ims_data->n_srvstatusinfo] =
                qcril_qmi_ims_make_ims_info(calltype, status, ril_data->audio_cause,
                                            (current_call_type == calltype), networkmode);

      if (current_call_type == calltype)
      {
        call_type_found = TRUE;
      }

      if( NULL != ims_data->srvstatusinfo[ims_data->n_srvstatusinfo] )
      {
        QCRIL_LOG_INFO("%d - calltype: %d status: %d restrictcause: %d", ims_data->n_srvstatusinfo,
                                       ims_data->srvstatusinfo[ims_data->n_srvstatusinfo]->calltype,
                                       ims_data->srvstatusinfo[ims_data->n_srvstatusinfo]->status,
                                       ims_data->srvstatusinfo[ims_data->n_srvstatusinfo]->restrictcause);
      }
      ims_data->n_srvstatusinfo++;

      if( VOICE_CALL_ATTRIB_TX_V02 == ril_data->video_attrib )
      {
        calltype = IMS__CALL_TYPE__CALL_TYPE_VT_TX;
        status = IMS__STATUS_TYPE__STATUS_PARTIALLY_ENABLED;
      }
      else if ( VOICE_CALL_ATTRIB_RX_V02 == ril_data->video_attrib )
      {
        calltype = IMS__CALL_TYPE__CALL_TYPE_VT_RX;
        status = IMS__STATUS_TYPE__STATUS_PARTIALLY_ENABLED;
      }
      else if ( (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02) == ril_data->video_attrib )
      {
        calltype = IMS__CALL_TYPE__CALL_TYPE_VT;
        status = IMS__STATUS_TYPE__STATUS_ENABLED;
      }
      else
      {
        calltype = IMS__CALL_TYPE__CALL_TYPE_VT;
        status = IMS__STATUS_TYPE__STATUS_DISABLED;
      }

      ims_data->srvstatusinfo[ims_data->n_srvstatusinfo] =
                qcril_qmi_ims_make_ims_info(calltype, status, ril_data->video_cause,
                                            (current_call_type == calltype), networkmode);

      if (current_call_type == calltype)
      {
        call_type_found = TRUE;
      }

      if( NULL != ims_data->srvstatusinfo[ims_data->n_srvstatusinfo] )
      {
         QCRIL_LOG_INFO("%d - calltype: %d status: %d restrictcause: %d", ims_data->n_srvstatusinfo,
                                        ims_data->srvstatusinfo[ims_data->n_srvstatusinfo]->calltype,
                                        ims_data->srvstatusinfo[ims_data->n_srvstatusinfo]->status,
                                        ims_data->srvstatusinfo[ims_data->n_srvstatusinfo]->restrictcause);
      }
      ims_data->n_srvstatusinfo++;

      if (current_call_type != IMS__CALL_TYPE__CALL_TYPE_UNKNOWN &&
          !call_type_found)
      {
        ims_data->srvstatusinfo[ims_data->n_srvstatusinfo] =
                  qcril_qmi_ims_make_ims_info(current_call_type,
                                              IMS__STATUS_TYPE__STATUS_ENABLED, 0,
                                              TRUE, networkmode);
        if( NULL != ims_data->srvstatusinfo[ims_data->n_srvstatusinfo] )
        {
           QCRIL_LOG_INFO("%d - calltype: %d status: %d restrictcause: %d", ims_data->n_srvstatusinfo,
                          ims_data->srvstatusinfo[ims_data->n_srvstatusinfo]->calltype,
                          ims_data->srvstatusinfo[ims_data->n_srvstatusinfo]->status,
                          ims_data->srvstatusinfo[ims_data->n_srvstatusinfo]->restrictcause);
        }
        ims_data->n_srvstatusinfo++;
      }
    }
  }
}

//===========================================================================
// qcril_qmi_ims_create_ims_info
//===========================================================================
Ims__Info* qcril_qmi_ims_create_ims_info(
    Ims__CallType type,
    imsa_service_status_enum_v01 status,
    boolean rat_valid,
    imsa_service_rat_enum_v01 rat
)
{
    boolean failure = FALSE;
    Ims__Info *ims_info_ptr = NULL;

    do
    {
        ims_info_ptr = qcril_malloc(sizeof(*ims_info_ptr));
        if (NULL == ims_info_ptr)
        {
            failure = TRUE;
            break;
        }
        Ims__Info ims_info_copy = IMS__INFO__INIT;
        memcpy(ims_info_ptr, &ims_info_copy, sizeof(*ims_info_ptr));

        ims_info_ptr->has_isvalid = TRUE;
        ims_info_ptr->isvalid = TRUE;

        ims_info_ptr->has_calltype = TRUE;
        ims_info_ptr->calltype = type;

        ims_info_ptr->n_acctechstatus = 1; // only have one possible rat per current IMSA design
        ims_info_ptr->acctechstatus =
            qcril_malloc( sizeof(*ims_info_ptr->acctechstatus) * ims_info_ptr->n_acctechstatus );
        if (NULL == ims_info_ptr->acctechstatus)
        {
            failure = TRUE;
            break;
        }

        ims_info_ptr->acctechstatus[0] = qcril_malloc(sizeof(*ims_info_ptr->acctechstatus[0]));
        if (NULL == ims_info_ptr->acctechstatus[0])
        {
            failure = TRUE;
            break;
        }
        Ims__StatusForAccessTech ims_status_copy = IMS__STATUS_FOR_ACCESS_TECH__INIT;
        memcpy( ims_info_ptr->acctechstatus[0],
                &ims_status_copy,
                sizeof(*ims_info_ptr->acctechstatus[0]) );

        ims_info_ptr->acctechstatus[0]->registered =
            qcril_malloc(sizeof(*ims_info_ptr->acctechstatus[0]->registered));
        if (NULL == ims_info_ptr->acctechstatus[0]->registered)
        {
            failure = TRUE;
            break;
        }
        Ims__Registration ims_reg_copy = IMS__REGISTRATION__INIT;
        memcpy( ims_info_ptr->acctechstatus[0]->registered,
                &ims_reg_copy,
                sizeof(*ims_info_ptr->acctechstatus[0]->registered) );

        ims_info_ptr->acctechstatus[0]->has_status = TRUE;
        ims_info_ptr->acctechstatus[0]->registered->has_state = TRUE;
        switch (status)
        {
            case IMSA_NO_SERVICE_V01:
                ims_info_ptr->acctechstatus[0]->status = IMS__STATUS_TYPE__STATUS_DISABLED;
                ims_info_ptr->acctechstatus[0]->registered->state = IMS__REGISTRATION__REG_STATE__NOT_REGISTERED;
                break;
            case IMSA_LIMITED_SERVICE_V01:
                ims_info_ptr->acctechstatus[0]->status = IMS__STATUS_TYPE__STATUS_PARTIALLY_ENABLED;
                ims_info_ptr->acctechstatus[0]->registered->state = IMS__REGISTRATION__REG_STATE__REGISTERED;
                break;
            case IMSA_FULL_SERVICE_V01:
                ims_info_ptr->acctechstatus[0]->status = IMS__STATUS_TYPE__STATUS_ENABLED;
                ims_info_ptr->acctechstatus[0]->registered->state = IMS__REGISTRATION__REG_STATE__REGISTERED;
                break;
            default:
                QCRIL_LOG_DEBUG("no matched status");
                ims_info_ptr->acctechstatus[0]->status = IMS__STATUS_TYPE__STATUS_ENABLED;
                ims_info_ptr->acctechstatus[0]->registered->state = IMS__REGISTRATION__REG_STATE__REGISTERED;
        }

        if (rat_valid)
        {
            switch (rat)
            {
                case IMSA_WLAN_V01:
                    ims_info_ptr->acctechstatus[0]->has_networkmode = TRUE;
                    ims_info_ptr->acctechstatus[0]->networkmode = IMS__RADIO_TECH_TYPE__RADIO_TECH_WIFI;
                    break;
                case IMSA_WWAN_V01:
                    ims_info_ptr->acctechstatus[0]->has_networkmode = TRUE;
                    ims_info_ptr->acctechstatus[0]->networkmode = IMS__RADIO_TECH_TYPE__RADIO_TECH_LTE;
                    break;
                default:
                    QCRIL_LOG_DEBUG("no matched rat");
            }
        }
    } while (FALSE);

    if (failure)
    {
        qcril_qmi_ims_free_ims_info(ims_info_ptr);
        ims_info_ptr = NULL;
    }
    else
    {
        QCRIL_LOG_INFO( "calltype: %d %d, n_acctechstatus: %d, networkmode: %d, %d, "
                        "restrictioncause: %d, %d, status: %d, %d, reg state: %d, %d",
                        ims_info_ptr->has_calltype,
                        ims_info_ptr->calltype,
                        ims_info_ptr->n_acctechstatus,
                        ims_info_ptr->acctechstatus[0]->has_networkmode,
                        ims_info_ptr->acctechstatus[0]->networkmode,
                        ims_info_ptr->acctechstatus[0]->has_restrictioncause,
                        ims_info_ptr->acctechstatus[0]->restrictioncause,
                        ims_info_ptr->acctechstatus[0]->has_status,
                        ims_info_ptr->acctechstatus[0]->status,
                        ims_info_ptr->acctechstatus[0]->registered->has_state,
                        ims_info_ptr->acctechstatus[0]->registered->state );
    }
    return ims_info_ptr;
} // qcril_qmi_ims_create_ims_info

//===========================================================================
// qcril_qmi_ims_create_ims_srvstatusinfo
//===========================================================================
Ims__SrvStatusList* qcril_qmi_ims_create_ims_srvstatusinfo(const qcril_qmi_imsa_srv_status_type* qmi_data)
{
    Ims__SrvStatusList* ims_srv_status_list_ptr = NULL;
    if (NULL == qmi_data)
    {
        QCRIL_LOG_DEBUG("qmi_data is NULL");
    }
    else
    {
        boolean failure = FALSE;
        do
        {
            ims_srv_status_list_ptr = qcril_malloc(sizeof(*ims_srv_status_list_ptr));

            if (NULL == ims_srv_status_list_ptr)
            {
                failure = TRUE;
                break;
            }

            Ims__SrvStatusList srv_status_list_copy = IMS__SRV_STATUS_LIST__INIT;
            memcpy( ims_srv_status_list_ptr, &srv_status_list_copy, sizeof(*ims_srv_status_list_ptr) );

            ims_srv_status_list_ptr->n_srvstatusinfo = qmi_data->sms_service_status_valid +
                                                       qmi_data->voip_service_status_valid +
                                                       qmi_data->vt_service_status_valid * 3 + // we need to fill three types if vt status is valid
                                                       qmi_data->ut_service_status_valid;

            if (ims_srv_status_list_ptr->n_srvstatusinfo > 0)
            {
                ims_srv_status_list_ptr->srvstatusinfo = qcril_malloc( sizeof(Ims__Info*) * ims_srv_status_list_ptr->n_srvstatusinfo );
            }

            if (NULL == ims_srv_status_list_ptr->srvstatusinfo)
            {
                failure = TRUE;
                break;
            }

            int idx = 0;

            if (qmi_data->sms_service_status_valid)
            {
                ims_srv_status_list_ptr->srvstatusinfo[idx] = qcril_qmi_ims_create_ims_info(
                                                                  IMS__CALL_TYPE__CALL_TYPE_SMS,
                                                                  qmi_data->sms_service_status,
                                                                  qmi_data->sms_service_rat_valid,
                                                                  qmi_data->sms_service_rat );
                if (NULL == ims_srv_status_list_ptr->srvstatusinfo[idx])
                {
                    failure = TRUE;
                    break;
                }
                idx++;
            }

            if (qmi_data->voip_service_status_valid)
            {
                ims_srv_status_list_ptr->srvstatusinfo[idx] = qcril_qmi_ims_create_ims_info(
                                                                  IMS__CALL_TYPE__CALL_TYPE_VOICE,
                                                                  qmi_data->voip_service_status,
                                                                  qmi_data->voip_service_rat_valid,
                                                                  qmi_data->voip_service_rat );
                if (NULL == ims_srv_status_list_ptr->srvstatusinfo[idx])
                {
                    failure = TRUE;
                    break;
                }
                idx++;
            }

            if (qmi_data->vt_service_status_valid)
            {
                ims_srv_status_list_ptr->srvstatusinfo[idx] = qcril_qmi_ims_create_ims_info(
                                                                  IMS__CALL_TYPE__CALL_TYPE_VT,
                                                                  qmi_data->vt_service_status,
                                                                  qmi_data->vt_service_rat_valid,
                                                                  qmi_data->vt_service_rat );
                if (NULL == ims_srv_status_list_ptr->srvstatusinfo[idx])
                {
                    failure = TRUE;
                    break;
                }
                idx++;

                ims_srv_status_list_ptr->srvstatusinfo[idx] = qcril_qmi_ims_create_ims_info(
                                                                  IMS__CALL_TYPE__CALL_TYPE_VT_TX,
                                                                  qmi_data->vt_service_status,
                                                                  qmi_data->vt_service_rat_valid,
                                                                  qmi_data->vt_service_rat );
                if (NULL == ims_srv_status_list_ptr->srvstatusinfo[idx])
                {
                    failure = TRUE;
                    break;
                }
                idx++;

                ims_srv_status_list_ptr->srvstatusinfo[idx] = qcril_qmi_ims_create_ims_info(
                                                                  IMS__CALL_TYPE__CALL_TYPE_VT_RX,
                                                                  qmi_data->vt_service_status,
                                                                  qmi_data->vt_service_rat_valid,
                                                                  qmi_data->vt_service_rat );
                if (NULL == ims_srv_status_list_ptr->srvstatusinfo[idx])
                {
                    failure = TRUE;
                    break;
                }
                idx++;
            }

            if (qmi_data->ut_service_status_valid)
            {
                ims_srv_status_list_ptr->srvstatusinfo[idx] = qcril_qmi_ims_create_ims_info(
                                                                  IMS__CALL_TYPE__CALL_TYPE_UT,
                                                                  qmi_data->ut_service_status,
                                                                  qmi_data->ut_service_rat_valid,
                                                                  qmi_data->ut_service_rat );
                if (NULL == ims_srv_status_list_ptr->srvstatusinfo[idx])
                {
                    failure = TRUE;
                    break;
                }
                idx++;
            }
        } while (FALSE);

        if (failure)
        {
            qcril_qmi_ims_free_srvstatuslist(ims_srv_status_list_ptr);
            ims_srv_status_list_ptr = NULL;
        }
    }

    QCRIL_LOG_FUNC_RETURN();
    return ims_srv_status_list_ptr;
} // qcril_qmi_ims_create_ims_srvstatusinfo

//===========================================================================
// qcril_qmi_ims_free_srvstatuslist
//===========================================================================
void qcril_qmi_ims_free_srvstatuslist(Ims__SrvStatusList* ims_srv_status_list_ptr)
{
    if (NULL != ims_srv_status_list_ptr)
    {
        if (NULL != ims_srv_status_list_ptr->srvstatusinfo)
        {
            size_t i;
            for (i = 0; i<ims_srv_status_list_ptr->n_srvstatusinfo; i++)
            {
                if (ims_srv_status_list_ptr->srvstatusinfo[i])
                {
                    qcril_qmi_ims_free_ims_info(ims_srv_status_list_ptr->srvstatusinfo[i]);
                }
            }
            qcril_free(ims_srv_status_list_ptr->srvstatusinfo);
        }
        qcril_free(ims_srv_status_list_ptr);
    }
    else
    {
        QCRIL_LOG_DEBUG("ims_srv_status_list_ptr is NULL")   ;
    }
} // qcril_qmi_ims_free_srvstatuslist

//===========================================================================
// qcril_qmi_ims_free_ims_info
//===========================================================================
void qcril_qmi_ims_free_ims_info(Ims__Info* ims_info_ptr)
{
    if (NULL != ims_info_ptr)
    {
        if (NULL != ims_info_ptr->acctechstatus)
        {
            size_t i;
            for (i=0; i<ims_info_ptr->n_acctechstatus; i++)
            {
                if (NULL != ims_info_ptr->acctechstatus[i])
                {
                    if (NULL != ims_info_ptr->acctechstatus[i]->registered)
                    {
                        qcril_free(ims_info_ptr->acctechstatus[i]->registered);
                    }
                    qcril_free(ims_info_ptr->acctechstatus[i]);
                }
            }
            qcril_free(ims_info_ptr->acctechstatus);
        }
        qcril_free(ims_info_ptr);
    }
    else
    {
        QCRIL_LOG_DEBUG("ims_info_ptr is NULL")   ;
    }
} // qcril_qmi_ims_free_ims_info

//===========================================================================
// qcril_qmi_ims_mics_map_imsa_rat_to_ims_rat
//===========================================================================
Ims__RadioTechType qcril_qmi_ims_map_imsa_rat_to_ims_rat(imsa_service_rat_enum_v01 imsa_rat)
{
    if (IMSA_WLAN_V01 == imsa_rat)
    {
        return IMS__RADIO_TECH_TYPE__RADIO_TECH_WIFI;
    }
    else if (IMSA_IWLAN_V01 == imsa_rat)
    {
        return IMS__RADIO_TECH_TYPE__RADIO_TECH_IWLAN;
    }
    else
    {
        return IMS__RADIO_TECH_TYPE__RADIO_TECH_LTE;
    }
} // qcril_qmi_ims_mics_map_imsa_rat_to_ims_rat

//===========================================================================
// qcril_qmi_ims_create_ims_handover_from_imsa_rat_info
//===========================================================================
Ims__Handover* qcril_qmi_ims_create_ims_handover_from_imsa_rat_info(const imsa_rat_handover_status_info_v01* qmi_data)
{
    Ims__Handover* ims_handover_ptr = NULL;
    if (NULL == qmi_data)
    {
        QCRIL_LOG_DEBUG("qmi_data is NULL");
    }
    else
    {
        boolean failure = FALSE;
        do
        {
            ims_handover_ptr = qcril_malloc(sizeof(*ims_handover_ptr));

            if (NULL == ims_handover_ptr)
            {
                failure = TRUE;
                break;
            }

            qcril_qmi_ims__handover__init(ims_handover_ptr);

            ims_handover_ptr->has_type = TRUE;
            switch(qmi_data->rat_ho_status)
            {
            case IMSA_STATUS_RAT_HO_SUCCESS_V01:
                ims_handover_ptr->type = IMS__HANDOVER__MSG__TYPE__COMPLETE_SUCCESS;
                break;
            case IMSA_STATUS_RAT_HO_FAILURE_V01:
                ims_handover_ptr->type = IMS__HANDOVER__MSG__TYPE__COMPLETE_FAIL;
                break;
            case IMSA_STATUS_RAT_HO_NOT_TRIGGERED_V01:
                ims_handover_ptr->type = IMS__HANDOVER__MSG__TYPE__NOT_TRIGGERED;
                break;
            default:
                ims_handover_ptr->type = IMS__HANDOVER__MSG__TYPE__COMPLETE_FAIL;
                break;
            }

            ims_handover_ptr->has_srctech = TRUE;
            ims_handover_ptr->srctech = qcril_qmi_ims_map_imsa_rat_to_ims_rat(qmi_data->source_rat);

            ims_handover_ptr->has_targettech = TRUE;
            ims_handover_ptr->targettech = qcril_qmi_ims_map_imsa_rat_to_ims_rat(qmi_data->target_rat);

            if (strlen(qmi_data->cause_code))
            {
                /* Error is reported when the handover is NOT_TRIGGERED while the device is on active
                 * Wifi call and the wifi Rssi is nearing threshold roveout (-85dbm) and there is
                 * no qualified LTE network to handover to. Modem sends "CD-04:No Available qualified
                 * mobile network". Here it is decoded and sent as errorcode(CD-04) and errormessage
                 * to telephony.
                 */
                if( (qmi_data->rat_ho_status == IMSA_STATUS_RAT_HO_NOT_TRIGGERED_V01) &&
                    (strncmp(qmi_data->cause_code, WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING,
                           strlen(WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING)) == 0) )
                {
                    //Copy the code to errorcode string
                    ims_handover_ptr->errorcode = qcril_malloc(strlen(WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING) + 1);
                    if(ims_handover_ptr->errorcode != NULL)
                    {
                        strlcpy(ims_handover_ptr->errorcode, WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING,
                                strlen(WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING) + 1);
                        QCRIL_LOG_DEBUG("handover error code: %s", ims_handover_ptr->errorcode);
                    }

                    int errormessage_start = strlen(WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING);

                    //Check for the delimeter ":" after the errorcode
                    // and discard any extra spaces
                    while(errormessage_start < strlen(qmi_data->cause_code))
                    {
                        if(strncmp(&(qmi_data->cause_code)[errormessage_start], ":", strlen(":")) == 0)
                        {
                            errormessage_start++;
                            while((strncmp(&(qmi_data->cause_code)[errormessage_start], " ", strlen(" ")) == 0) &&
                                  errormessage_start < strlen(qmi_data->cause_code))
                            {
                                errormessage_start++;
                            }
                            break;
                        }
                        errormessage_start++;
                    }
                    //The string after the delimeter ":" is the error message
                    if(errormessage_start < strlen(qmi_data->cause_code))
                    {
                        ims_handover_ptr->errormessage = qcril_malloc((strlen(qmi_data->cause_code) + 1)- errormessage_start);
                        if(ims_handover_ptr->errormessage)
                        {
                            strlcpy(ims_handover_ptr->errormessage, &(qmi_data->cause_code)[errormessage_start],
                                    ((strlen(qmi_data->cause_code) + 1)- errormessage_start));
                            QCRIL_LOG_DEBUG("handover error message: %s", ims_handover_ptr->errormessage);
                        }
                    }
                }
                else
                {
                    ims_handover_ptr->hoextra = qcril_malloc(sizeof(*ims_handover_ptr->hoextra));
                    if (NULL == ims_handover_ptr->hoextra)
                    {
                        failure = TRUE;
                        break;
                    }

                    qcril_qmi_ims__extra__init(ims_handover_ptr->hoextra);

                    ims_handover_ptr->hoextra->has_type = TRUE;
                    ims_handover_ptr->hoextra->type = IMS__EXTRA__TYPE__LTE_TO_IWLAN_HO_FAIL;

                    ims_handover_ptr->hoextra->has_extrainfo = TRUE;
                    ims_handover_ptr->hoextra->extrainfo.len = strlen(qmi_data->cause_code);
                    ims_handover_ptr->hoextra->extrainfo.data = qcril_malloc(ims_handover_ptr->hoextra->extrainfo.len);
                    if (NULL == ims_handover_ptr->hoextra->extrainfo.data)
                    {
                        failure = TRUE;
                        break;
                    }
                    memcpy( ims_handover_ptr->hoextra->extrainfo.data,
                            qmi_data->cause_code,
                            ims_handover_ptr->hoextra->extrainfo.len );
                }
            }
        } while (FALSE);

        if (failure)
        {
            qcril_qmi_ims_free_ims_handover(ims_handover_ptr);
            ims_handover_ptr = NULL;
        }
    }

    return ims_handover_ptr;
} // qcril_qmi_ims_create_ims_handover_from_imsa_rat_info
//===========================================================================
// qcril_qmi_ims_free_ims_handover
//===========================================================================
void qcril_qmi_ims_free_ims_handover(Ims__Handover* ims_handover_ptr)
{
    if (NULL != ims_handover_ptr)
    {
        if (NULL != ims_handover_ptr->hoextra)
        {
            if (NULL != ims_handover_ptr->hoextra->extrainfo.data)
            {
                qcril_free(ims_handover_ptr->hoextra->extrainfo.data);
            }
            qcril_free(ims_handover_ptr->hoextra);
        }
        if(NULL != ims_handover_ptr->errorcode)
        {
            qcril_free(ims_handover_ptr->errorcode);
        }
        if(NULL != ims_handover_ptr->errormessage)
        {
            qcril_free(ims_handover_ptr->errormessage);
        }
        qcril_free(ims_handover_ptr);
    }
    else
    {
        QCRIL_LOG_DEBUG("ims_handover_ptr is NULL");
    }
} // qcril_qmi_ims_free_ims_handover
//===========================================================================
// qcril_qmi_ims_free_ims_registration
//===========================================================================
void qcril_qmi_ims_free_ims_registration(Ims__Registration* ims_reg_ptr)
{
    if(NULL != ims_reg_ptr)
    {
        if(NULL != ims_reg_ptr->errormessage)
        {
            qcril_free(ims_reg_ptr->errormessage);
        }
        qcril_free(ims_reg_ptr);
    }
    else
    {
        QCRIL_LOG_DEBUG("ims_reg_ptr is NULL");
    }
}// qcril_qmi_ims_free_ims_registration
//===========================================================================
// qcril_qmi_ims_map_ril_failcause_to_ims_failcause
//===========================================================================
Ims__CallFailCause qcril_qmi_ims_map_ril_failcause_to_ims_failcause(RIL_LastCallFailCause ril_failcause, int ims_extended_error_code)
{
  Ims__CallFailCause ret = ril_failcause;

  if( CALL_FAIL_ERROR_UNSPECIFIED == ril_failcause )
  {
    switch ( ims_extended_error_code )
    {
      case CALL_END_CAUSE_MULTIPLE_CHOICES_V02:
      case CALL_END_CAUSE_MOVED_PERMANENTLY_V02:
      case CALL_END_CAUSE_MOVED_TEMPORARILY_V02:
      case CALL_END_CAUSE_USE_PROXY_V02:
      case CALL_END_CAUSE_ALTERNATE_SERVICE_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_REDIRECTED;
        break;

      case CALL_END_CAUSE_BAD_REQ_WAIT_INVITE_V02:
      case CALL_END_CAUSE_BAD_REQ_WAIT_REINVITE_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_BAD_REQUEST;
        break;

      case CALL_END_CAUSE_SIP_403_FORBIDDEN_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_FORBIDDEN;
        break;

      case CALL_END_CAUSE_INVALID_REMOTE_URI_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_NOT_FOUND;
        break;

      case CALL_END_CAUSE_UNSUPPORTED_URI_SCHEME_V02:
      case CALL_END_CAUSE_REMOTE_UNSUPP_MEDIA_TYPE_V02:
      case CALL_END_CAUSE_BAD_EXTENSION_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_NOT_SUPPORTED;
        break;

      case CALL_END_CAUSE_NETWORK_NO_RESP_TIME_OUT_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_REQUEST_TIMEOUT;
        break;

      case CALL_END_CAUSE_PEER_NOT_REACHABLE_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE;
        break;

      case CALL_END_CAUSE_ADDRESS_INCOMPLETE_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_BAD_ADDRESS;
        break;

      case CALL_END_CAUSE_USER_BUSY_V02:
      case CALL_END_CAUSE_BUSY_EVERYWHERE_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_BUSY;
        break;

      case CALL_END_CAUSE_REQUEST_TERMINATED_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_REQUEST_CANCELLED;
        break;

      case CALL_END_CAUSE_NOT_ACCEPTABLE_V02:
      case CALL_END_CAUSE_NOT_ACCEPTABLE_HERE_V02:
      case CALL_END_CAUSE_SESS_DESCR_NOT_ACCEPTABLE_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_NOT_ACCEPTABLE;
        break;

      case CALL_END_CAUSE_GONE_V02:
      case CALL_END_CAUSE_DOES_NOT_EXIST_ANYWHERE_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_NOT_REACHABLE;
        break;

      case CALL_END_CAUSE_SERVER_INTERNAL_ERROR_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_SERVER_INTERNAL_ERROR;
        break;

      case CALL_END_CAUSE_NO_NETWORK_RESP_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_SERVICE_UNAVAILABLE;
        break;

      case CALL_END_CAUSE_SERVER_TIME_OUT_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_SERVER_TIMEOUT;
        break;

      case CALL_END_CAUSE_CALL_REJECTED_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_SIP_USER_REJECTED;
        break;

      case CALL_END_CAUSE_ANSWERED_ELSEWHERE_V02:
      case CALL_END_CAUSE_CALL_DEFLECTED_V02:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_ANSWERED_ELSEWHERE;
        break;

      default:
        ret = IMS__CALL_FAIL_CAUSE__CALL_FAIL_ERROR_UNSPECIFIED;
        break;
    }
  }

  QCRIL_LOG_INFO("RIL_LastCallFailCause %d with extended error code %d mapped to Ims__CallFailCause %d", ril_failcause, ims_extended_error_code, ret);
  return ret;
}

//===========================================================================
// qcril_qmi_ims_map_qmi_call_state_to_ims_conf_call_state
//===========================================================================
boolean qcril_qmi_ims_map_qmi_call_state_to_ims_conf_call_state(call_state_enum_v02 qmi_state, Ims__ConfCallState *ims_state_ptr)
{
    boolean success = TRUE;
    if (ims_state_ptr)
    {
        switch (qmi_state)
        {
        case CALL_STATE_ORIGINATING_V02:
        case CALL_STATE_CC_IN_PROGRESS_V02:
        case CALL_STATE_ALERTING_V02:
        case CALL_STATE_CONVERSATION_V02:
            *ims_state_ptr = IMS__CONF_CALL_STATE__FOREGROUND;
            break;

        case CALL_STATE_HOLD_V02:
            *ims_state_ptr = IMS__CONF_CALL_STATE__BACKGROUND;
            break;

        case CALL_STATE_INCOMING_V02:
        case CALL_STATE_WAITING_V02:
            *ims_state_ptr = IMS__CONF_CALL_STATE__RINGING;
            break;

        default:
            success = FALSE;
        }
    }
    else
    {
        success = FALSE;
    }
    return success;
}

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_map_qmi_reason_to_ims_facility

===========================================================================*/
/*!
    @brief
    Maps reason code from QMI Voice Get Call Barring response message
    to corresponding IMS Ims__SuppSvcFacilityType.

    @return
    Success: Ims__SuppSvcFacilityType.
    Error:   0
*/
/*=========================================================================*/
Ims__SuppSvcFacilityType qcril_qmi_voice_map_qmi_reason_to_ims_facility
(
  /* Reason code from QMI Voice Get Call Barring response message */
  voice_cc_sups_result_reason_enum_v02 reason
)
{
  switch (reason)
  {

    /* Bar All Outgoing Calls */
    case VOICE_CC_SUPS_RESULT_REASON_BARR_ALLOUTGOING_V02:
      return IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOC;

    /* Bar All Outgoing International Calls */
    case VOICE_CC_SUPS_RESULT_REASON_BARR_OUTGOINGINT_V02:
      return IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOIC;

    /* Bar all Outgoing International Calls except those
       directed to home PLMN country */
    case VOICE_CC_SUPS_RESULT_REASON_BARR_OUTGOINGINTEXTOHOME_V02:
      return IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOICxH;

    /* Bar All Incoming Calls */
    case VOICE_CC_SUPS_RESULT_REASON_BARR_ALLINCOMING_V02:
      return IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAIC;

    /* Bar All Incoming Calls when Roaming outside
       the home PLMN country */
    case VOICE_CC_SUPS_RESULT_REASON_BARR_INCOMINGROAMING_V02:
      return IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAICr;

    /* Bar All incoming & outgoing Calls */
    case VOICE_CC_SUPS_RESULT_REASON_BARR_ALLBARRING_V02:
      return IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_ALL;

    /* Bar All Outgoing Calls */
    case VOICE_CC_SUPS_RESULT_REASON_BARR_ALLOUTGOINGBARRING_V02:
      return IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_MO;

    /* Bar All Incoming Calls */
    case VOICE_CC_SUPS_RESULT_REASON_BARR_ALLINCOMINGBARRING_V02:
      return IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_MT;

    /* Bar Specific Incoming Calls */
    case VOICE_CC_SUPS_RESULT_REASON_BARR_INCOMING_NUMBER_V02:
      return IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BS_MT;

    /* Bar Anonymous Incoming Calls */
    case VOICE_CC_SUPS_RESULT_REASON_BARR_INCOMING_ANONYMOUS_V02:
      return IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAICa;

    default:
      return 0;
  }
} /* qcril_qmi_voice_map_qmi_reason_to_ims_facility */

//===========================================================================
// qcril_qmi_ims_map_ril_call_substate_to_ims_call_substate
//===========================================================================
Ims__CallSubstate qcril_qmi_ims_map_ril_call_substate_to_ims_call_substate
(
 RIL_Call_Sub_State  ril_call_substate
)
{
  Ims__CallSubstate ims_call_substate = IMS__CALL_SUBSTATE__CALL_SUBSTATE_NONE;
  if (ril_call_substate ==
          (RIL_CALL_SUB_STATE_AUDIO_CONNECTED_SUSPENDED |
           RIL_CALL_SUB_STATE_VIDEO_CONNECTED_SUSPENDED))
  {
    ims_call_substate = (IMS__CALL_SUBSTATE__CALL_SUBSTATE_AUDIO_CONNECTED_SUSPENDED |
                         IMS__CALL_SUBSTATE__CALL_SUBSTATE_VIDEO_CONNECTED_SUSPENDED);
  }
  else
  {
    switch (ril_call_substate)
    {
      case RIL_CALL_SUB_STATE_AUDIO_CONNECTED_SUSPENDED:
        ims_call_substate = IMS__CALL_SUBSTATE__CALL_SUBSTATE_AUDIO_CONNECTED_SUSPENDED;
        break;
      case RIL_CALL_SUB_STATE_VIDEO_CONNECTED_SUSPENDED:
        ims_call_substate = IMS__CALL_SUBSTATE__CALL_SUBSTATE_VIDEO_CONNECTED_SUSPENDED;
        break;
      case RIL_CALL_SUB_STATE_AVP_RETRY:
        ims_call_substate = IMS__CALL_SUBSTATE__CALL_SUBSTATE_AVP_RETRY;
        break;
      case RIL_CALL_SUB_STATE_MEDIA_PAUSED:
        ims_call_substate = IMS__CALL_SUBSTATE__CALL_SUBSTATE_MEDIA_PAUSED;
        break;
      case RIL_CALL_SUB_STATE_UNDEFINED:
      default:
        ims_call_substate = IMS__CALL_SUBSTATE__CALL_SUBSTATE_NONE;
        break;
    }
  }
  return ims_call_substate;
} /* qcril_qmi_ims_map_ril_call_substate_to_ims_call_substate */

//============================================================================
// FUNCTION: qcril_qmi_sms_map_qmi_mwi_msg_type_to_ims_msg_type
//
// DESCRIPTION:
// Convert qmi wms mwi message type to ims mwi message type
//============================================================================
Ims__MwiMessageType qcril_qmi_sms_map_qmi_mwi_msg_type_to_ims_msg_type
(
 transport_mwi_wms_message_type_enum_v01 mwi_wms_msg_type
)
{
  Ims__MwiMessageType ims_mwi_msg_type;
  switch(mwi_wms_msg_type)
  {
    case TRANSPORT_MWI_MESSAGE_TYPE_VOICEMAIL_V01:
      ims_mwi_msg_type = IMS__MWI_MESSAGE_TYPE__MWI_MSG_VOICE;
      break;

    case TRANSPORT_MWI_MESSAGE_TYPE_VIDEOMAIL_V01:
      ims_mwi_msg_type = IMS__MWI_MESSAGE_TYPE__MWI_MSG_VIDEO;
      break;

    case TRANSPORT_MWI_MESSAGE_TYPE_FAX_V01:
      ims_mwi_msg_type = IMS__MWI_MESSAGE_TYPE__MWI_MSG_FAX;
      break;

    case TRANSPORT_MWI_MESSAGE_TYPE_PAGER_V01:
      ims_mwi_msg_type = IMS__MWI_MESSAGE_TYPE__MWI_MSG_PAGER;
      break;

    case TRANSPORT_MWI_MESSAGE_TYPE_MULTIMEDIA_V01:
      ims_mwi_msg_type = IMS__MWI_MESSAGE_TYPE__MWI_MSG_MULTIMEDIA;
      break;

    case TRANSPORT_MWI_MESSAGE_TYPE_TEXT_V01:
      ims_mwi_msg_type = IMS__MWI_MESSAGE_TYPE__MWI_MSG_TEXT;
      break;

    default:
      ims_mwi_msg_type = IMS__MWI_MESSAGE_TYPE__MWI_MSG_NONE;
      break;
  }
  QCRIL_LOG_INFO("qmi mwi_wms_msg_type %d mapped to ims mwi_msg_type %d",
          mwi_wms_msg_type, ims_mwi_msg_type);
  return ims_mwi_msg_type;
}

//============================================================================
// FUNCTION: qcril_qmi_sms_map_qmi_mwi_priority_to_ims_priority
//
// DESCRIPTION:
// Convert qmi wms mwi priority to ims mwi priority
//============================================================================
Ims__MwiPriority qcril_qmi_sms_map_qmi_mwi_priority_to_ims_priority
(
 wms_mwi_priority_type_enum_v01 wms_mwi_priority
)
{
  Ims__MwiPriority mwi_priority;
  switch(wms_mwi_priority)
  {
    case WMS_LOW_PRIORITY_V01:
      mwi_priority = IMS__MWI_PRIORITY__MWI_MSG_PRIORITY_LOW;
      break;

    case WMS_NORMAL_PRIORITY_V01:
      mwi_priority = IMS__MWI_PRIORITY__MWI_MSG_PRIORITY_NORMAL;
      break;

    case WMS_URGENT_PRIORITY_V01:
      mwi_priority = IMS__MWI_PRIORITY__MWI_MSG_PRIORITY_URGENT;
      break;

    case WMS_UNKNOWN_PRIORITY_V01:
    default:
      mwi_priority = IMS__MWI_PRIORITY__MWI_MSG_PRIORITY_UNKNOWN;
      break;
  }
  QCRIL_LOG_INFO("qmi wms_mwi_priority %d mapped to ims mwi_priority %d",
          wms_mwi_priority, mwi_priority);
  return mwi_priority;
}

boolean qcril_qmi_ims_translate_ims_ttymodetype_to_qmi_tty_mode
(
 Ims__TtyModeType mode,
 tty_mode_enum_v02 *tty_mode
)
{
  boolean result = FALSE;
  if (tty_mode)
  {
    result = TRUE;
    switch(mode)
    {
      case IMS__TTY__MODE__TYPE__TTY_MODE_FULL:
        *tty_mode = TTY_MODE_FULL_V02;
        break;

      case IMS__TTY__MODE__TYPE__TTY_MODE_VCO:
        *tty_mode = TTY_MODE_VCO_V02;
        break;

      case IMS__TTY__MODE__TYPE__TTY_MODE_HCO:
        *tty_mode = TTY_MODE_HCO_V02;
        break;

      case IMS__TTY__MODE__TYPE__TTY_MODE_OFF:
        *tty_mode = TTY_MODE_OFF_V02;
        break;
      default:
        result = FALSE;
        break;
    }
  }
  return result;
}

//============================================================================
// FUNCTION: qcril_qmi_ims_translate_ims_callfwdtimerinfo_to_voice_call_fwd_timer_type
//
// DESCRIPTION:
// Convert ims call fwd timer info to qmi voice call fwd timer
//============================================================================
boolean qcril_qmi_ims_translate_ims_callfwdtimerinfo_to_voice_time_type
(
 const Ims__CallFwdTimerInfo *callfwdtimerinfo,
 voice_time_type_v02 *call_fwd_timer
)
{
  boolean result = FALSE;

  if (callfwdtimerinfo && call_fwd_timer)
  {
    if (callfwdtimerinfo->has_year)
    {
        call_fwd_timer->year = callfwdtimerinfo->year;
    }
    if (callfwdtimerinfo->has_month)
    {
        call_fwd_timer->month = callfwdtimerinfo->month;
    }
    if (callfwdtimerinfo->has_day)
    {
        call_fwd_timer->day = callfwdtimerinfo->day;
    }
    if (callfwdtimerinfo->has_hour)
    {
        call_fwd_timer->hour = callfwdtimerinfo->hour;
    }
    if (callfwdtimerinfo->has_minute)
    {
        call_fwd_timer->minute = callfwdtimerinfo->minute;
    }
    if (callfwdtimerinfo->has_second)
    {
        call_fwd_timer->second = callfwdtimerinfo->second;
    }
    if (callfwdtimerinfo->has_timezone)
    {
        call_fwd_timer->time_zone = callfwdtimerinfo->timezone;
    }
    result = TRUE;
  }

  return result;
}

boolean qcril_qmi_ims_translate_voice_time_type_to_ims_callfwdtimerinfo
(
 const voice_time_type_v02 *call_fwd_timer,
 Ims__CallFwdTimerInfo *callfwdtimerinfo
)
{
  boolean result = FALSE;

  if (callfwdtimerinfo && call_fwd_timer)
  {
    callfwdtimerinfo->has_year = TRUE;
    callfwdtimerinfo->year     = call_fwd_timer->year;
    callfwdtimerinfo->has_month = TRUE;
    callfwdtimerinfo->month     = call_fwd_timer->month;
    callfwdtimerinfo->has_day = TRUE;
    callfwdtimerinfo->day     = call_fwd_timer->day;
    callfwdtimerinfo->has_hour = TRUE;
    callfwdtimerinfo->hour     = call_fwd_timer->hour;
    callfwdtimerinfo->has_minute = TRUE;
    callfwdtimerinfo->minute     = call_fwd_timer->minute;
    callfwdtimerinfo->has_second = TRUE;
    callfwdtimerinfo->second     = call_fwd_timer->second;
    callfwdtimerinfo->has_timezone = TRUE;
    callfwdtimerinfo->timezone     = call_fwd_timer->time_zone;
    result = TRUE;
  }

  return result;
}

//===========================================================================
// qcril_qmi_ims_map_ims_failcause_qmi_reject_cause
//===========================================================================
int32_t qcril_qmi_ims_map_ims_failcause_qmi_reject_cause
(
  Ims__CallFailCause failcause
)
{
  int32_t res = INVALID_NEGATIVE_ONE;

  switch(failcause)
  {
    case IMS__CALL_FAIL_CAUSE__CALL_FAIL_BLACKLISTED_CALL_ID:
      res = VOICE_REJECT_CAUSE_BLACKLISTED_CALL_ID_V02;
      break;

// Modem expects user busy, when the user rejects the call
// Reason for inverted mapping.

    case IMS__CALL_FAIL_CAUSE__CALL_FAIL_USER_BUSY:
      res = VOICE_REJECT_CAUSE_USER_REJECT_V02;
      break;

    case IMS__CALL_FAIL_CAUSE__CALL_FAIL_USER_REJECT:
      res = VOICE_REJECT_CAUSE_USER_BUSY_V02;
      break;

    case IMS__CALL_FAIL_CAUSE__CALL_FAIL_LOW_BATTERY:
      res = VOICE_REJECT_CAUSE_LOW_BATTERY_V02;
      break;

    default:
      res = INVALID_NEGATIVE_ONE;
  }
  return res;
}

//===========================================================================
// qcril_qmi_ims_map_wificallingstatus_to_ims_settings_wfc_status
//===========================================================================
uint8_t qcril_qmi_ims_map_wificallingstatus_to_ims_settings_wfc_status
(
 Ims__WifiCallingStatus            status,
 ims_settings_wfc_status_enum_v01 *wifi_call
)
{
  uint8_t result = FALSE;
  if (wifi_call)
  {
    result = TRUE;
    switch (status)
    {
      case IMS__WIFI_CALLING_STATUS__WIFI_NOT_SUPPORTED:
        *wifi_call = IMS_SETTINGS_WFC_STATUS_NOT_SUPPORTED_V01;
        break;
      case IMS__WIFI_CALLING_STATUS__WIFI_STATUS_ON:
        *wifi_call = IMS_SETTINGS_WFC_STATUS_ON_V01;
        break;
      case IMS__WIFI_CALLING_STATUS__WIFI_STATUS_OFF:
        *wifi_call = IMS_SETTINGS_WFC_STATUS_OFF_V01;
        break;
      default:
        result = FALSE;
        break;
    }
  }
  return result;
}

//===========================================================================
// qcril_qmi_ims_map_ims_settings_wfc_status_to_wificallingstatus
//===========================================================================
boolean qcril_qmi_ims_map_ims_settings_wfc_status_to_wificallingstatus
(
 ims_settings_wfc_status_enum_v01 wifi_call,
 Ims__WifiCallingStatus          *status
)
{
  boolean result = FALSE;
  if (status)
  {
    result = TRUE;
    switch (wifi_call)
    {
      case IMS_SETTINGS_WFC_STATUS_NOT_SUPPORTED_V01:
        *status = IMS__WIFI_CALLING_STATUS__WIFI_NOT_SUPPORTED;
        break;
      case IMS_SETTINGS_WFC_STATUS_ON_V01:
        *status = IMS__WIFI_CALLING_STATUS__WIFI_STATUS_ON;
        break;
      case IMS_SETTINGS_WFC_STATUS_OFF_V01:
        *status = IMS__WIFI_CALLING_STATUS__WIFI_STATUS_OFF;
        break;
      default:
        result = FALSE;
        break;
    }
  }
  return result;
}

//===========================================================================
// qcril_qmi_ims_map_wificallingpreference_to_ims_settings_wfc_preference
//===========================================================================
uint8_t qcril_qmi_ims_map_wificallingpreference_to_ims_settings_wfc_preference
(
 Ims__WifiCallingPreference       preference,
 ims_settings_wfc_preference_v01 *wifi_call_preference
)
{
  uint8_t result = FALSE;
  if (wifi_call_preference)
  {
    result = TRUE;
    switch (preference)
    {
      case IMS__WIFI_CALLING_PREFERENCE__WIFI_PREF_NONE:
        *wifi_call_preference = IMS_SETTINGS_WFC_CALL_PREF_NONE_V01;
        break;
      case IMS__WIFI_CALLING_PREFERENCE__WIFI_PREFERRED:
        *wifi_call_preference = IMS_SETTINGS_WFC_WLAN_PREFERRED_V01;
        break;
      case IMS__WIFI_CALLING_PREFERENCE__WIFI_ONLY:
        *wifi_call_preference = IMS_SETTINGS_WFC_WLAN_ONLY_V01;
        break;
      case IMS__WIFI_CALLING_PREFERENCE__CELLULAR_PREFERRED:
        *wifi_call_preference = IMS_SETTINGS_WFC_CELLULAR_PREFERRED_V01;
        break;
      case IMS__WIFI_CALLING_PREFERENCE__CELLULAR_ONLY:
        *wifi_call_preference = IMS_SETTINGS_WFC_CELLULAR_ONLY_V01;
        break;
      default:
        result = FALSE;
        break;
    }
  }
  return result;
}

//===========================================================================
// qcril_qmi_ims_map_ims_settings_wfc_preference_to_wificallingpreference
//===========================================================================
boolean qcril_qmi_ims_map_ims_settings_wfc_preference_to_wificallingpreference
(
 ims_settings_wfc_preference_v01 wifi_call_preference,
 Ims__WifiCallingPreference     *preference
)
{
  boolean result = FALSE;
  if (preference)
  {
    result = TRUE;
    switch (wifi_call_preference)
    {
      case IMS_SETTINGS_WFC_CALL_PREF_NONE_V01:
        *preference = IMS__WIFI_CALLING_PREFERENCE__WIFI_PREF_NONE;
        break;
      case IMS_SETTINGS_WFC_WLAN_PREFERRED_V01:
        *preference = IMS__WIFI_CALLING_PREFERENCE__WIFI_PREFERRED;
        break;
      case IMS_SETTINGS_WFC_WLAN_ONLY_V01:
        *preference = IMS__WIFI_CALLING_PREFERENCE__WIFI_ONLY;
        break;
      case IMS_SETTINGS_WFC_CELLULAR_PREFERRED_V01:
        *preference = IMS__WIFI_CALLING_PREFERENCE__CELLULAR_PREFERRED;
        break;
      case IMS_SETTINGS_WFC_CELLULAR_ONLY_V01:
        *preference = IMS__WIFI_CALLING_PREFERENCE__CELLULAR_ONLY;
        break;
      default:
        result = FALSE;
        break;
    }
  }
  return result;
}
