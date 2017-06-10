/******************************************************************************
  @file    qcril_uim_remote_client_misc.c
  @brief   qcril uim remote client - misc

  DESCRIPTION
    Utility functions for uim remote client socket.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qcril_uim_remote_client_misc.h"
#include "qcril_log.h"
#include "qcrili.h"

#define UIM_REMOTE_CLIENT_TOKEN_SPACE (0xE0000000)

//===========================================================================
// qcril_uim_remote_client_convert_uim_token_to_ril_token
//===========================================================================
RIL_Token qcril_uim_remote_client_convert_uim_token_to_ril_token(uint32_t uim_token)
{
  RIL_Token ret = qcril_malloc(sizeof(uint32_t));
  if (NULL != ret)
  {
    uint32_t *tmp = (uint32_t*) ret;
    *tmp = uim_token ^ UIM_REMOTE_CLIENT_TOKEN_SPACE;
  }
  return ret;
} // qcril_uim_remote_client_convert_uim_token_to_ril_token

//===========================================================================
// qcril_uim_remote_client_free_and_convert_ril_token_to_uim_token
//===========================================================================
uint32_t qcril_uim_remote_client_free_and_convert_ril_token_to_uim_token(RIL_Token ril_token)
{
  uint32_t ret = 0xFFFFFFFF;
  if (ril_token)
  {
      ret = (*((uint32_t *) ril_token)) ^ UIM_REMOTE_CLIENT_TOKEN_SPACE;
      QCRIL_LOG_INFO("uim token: %d", ret);
      qcril_free((void*) ril_token);
  }
  else
  {
      QCRIL_LOG_INFO("ril_token is NULL");
  }

  return ret;
} // qcril_uim_remote_client_free_and_convert_ril_token_to_uim_token

//===========================================================================
// qcril_uim_remote_client_map_event_to_request
//===========================================================================
com_qualcomm_uimremoteclient_MessageId qcril_uim_remote_client_map_event_to_request(int event)
{
    com_qualcomm_uimremoteclient_MessageId ret;

    switch(event)
    {
      case QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_EVENT:
        ret = com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_EVENT;
        break;
      case QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_APDU:
        ret = com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_APDU;
        break;
      default:
        QCRIL_LOG_DEBUG("didn't find direct mapping for event %d", event);
        if ( event > QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_BASE && event < QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_MAX )
        {
          ret = event - QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_BASE;
        }
        else
        {
          ret = com_qualcomm_uimremoteclient_MessageId_UNKNOWN_REQ;
        }
        break;
    }

    QCRIL_LOG_INFO("event %d mapped to uim_msg %d", event, ret);
    return ret;
} //qcril_uim_remote_client_map_event_to_request
