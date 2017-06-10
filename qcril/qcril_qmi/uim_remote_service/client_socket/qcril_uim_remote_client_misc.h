/*!
  @file
  qcril_uim_remote_client_misc.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#ifndef QCRIL_UIM_REMOTE_CLIENT_MISC_H
#define QCRIL_UIM_REMOTE_CLIENT_MISC_H


#include "ril.h"
#include "qcrili.h"
#include "uim_remote_client.pb.h"


RIL_Token qcril_uim_remote_client_convert_uim_token_to_ril_token(uint32_t ims_token);
uint32_t qcril_uim_remote_client_free_and_convert_ril_token_to_uim_token(RIL_Token ril_token);
com_qualcomm_uimremoteclient_MessageId qcril_uim_remote_client_map_event_to_request(int event);

#endif /* QCRIL_UIM_REMOTE_CLIENT_MISC_H */
