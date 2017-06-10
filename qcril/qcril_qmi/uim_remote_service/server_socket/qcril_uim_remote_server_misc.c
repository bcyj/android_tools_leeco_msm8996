/******************************************************************************
  @file    qcril_uim_remote_server_misc.c
  @brief   qcril uim remote server - misc

  DESCRIPTION
    Utility functions for uim remote server socket.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qcril_uim_remote_server_misc.h"
#include "qcril_log.h"
#include "qcrili.h"

#define UIM_REMOTE_SERVER_TOKEN_SPACE (0xE8000000)

//===========================================================================
// qcril_uim_remote_server_convert_uim_token_to_ril_token
//===========================================================================
RIL_Token qcril_uim_remote_server_convert_uim_token_to_ril_token(uint32_t uim_token)
{
  RIL_Token ret = qcril_malloc(sizeof(uint32_t));
  if (NULL != ret)
  {
    uint32_t *tmp = (uint32_t*) ret;
    *tmp = uim_token ^ UIM_REMOTE_SERVER_TOKEN_SPACE;
  }
  return ret;
} // qcril_uim_remote_server_convert_uim_token_to_ril_token

//===========================================================================
// qcril_uim_remote_server_free_and_convert_ril_token_to_uim_token
//===========================================================================
uint32_t qcril_uim_remote_server_free_and_convert_ril_token_to_uim_token(RIL_Token ril_token)
{
  uint32_t ret = 0xFFFFFFFF;
  if (ril_token)
  {
      ret = (*((uint32_t *) ril_token)) ^ UIM_REMOTE_SERVER_TOKEN_SPACE;
      QCRIL_LOG_INFO("uim token: %d", ret);
      qcril_free((void*) ril_token);
  }
  else
  {
      QCRIL_LOG_INFO("ril_token is NULL");
  }

  return ret;
} // qcril_uim_remote_server_free_and_convert_ril_token_to_uim_token

//===========================================================================
// qcril_uim_remote_server_convert_ril_error_to_uim_error
//===========================================================================
Error qcril_uim_remote_server_convert_ril_error_to_uim_error(RIL_Errno err)
{
    Error e = Error_RIL_E_GENERIC_FAILURE;
    switch(err)
    {
    case RIL_E_SUCCESS:
        e = Error_RIL_E_SUCCESS;
        break;
    case RIL_E_RADIO_NOT_AVAILABLE:
        e = Error_RIL_E_RADIO_NOT_AVAILABLE;
        break;
    case RIL_E_GENERIC_FAILURE:
        e = Error_RIL_E_GENERIC_FAILURE;
        break;
    case RIL_E_REQUEST_NOT_SUPPORTED:
        e = Error_RIL_E_REQUEST_NOT_SUPPORTED;
        break;
    case RIL_E_CANCELLED:
        e = Error_RIL_E_CANCELLED;
        break;
    case RIL_E_ILLEGAL_SIM_OR_ME:
        e = Error_RIL_E_INVALID_PARAMETER;
        break;
    default:
        e = Error_RIL_E_GENERIC_FAILURE;
        break;
    }
    return e;
} //qcril_uim_remote_server_convert_ril_error_to_uim_error


