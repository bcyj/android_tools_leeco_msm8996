/*!
  @file
  qcril_uim_remote_server_misc.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#ifndef QCRIL_UIM_REMOTE_SERVER_MISC_H
#define QCRIL_UIM_REMOTE_SERVER_MISC_H


#include "ril.h"
#include "qcrili.h"
#include "sap-api.pb.h"


RIL_Token qcril_uim_remote_server_convert_uim_token_to_ril_token(uint32_t uim_token);
uint32_t qcril_uim_remote_server_free_and_convert_ril_token_to_uim_token(RIL_Token ril_token);
Error qcril_uim_remote_server_convert_ril_error_to_uim_error(RIL_Errno err);

#endif /* QCRIL_UIM_REMOTE_SERVER_MISC_H */
