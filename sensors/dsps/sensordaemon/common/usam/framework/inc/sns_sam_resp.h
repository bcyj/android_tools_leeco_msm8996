#ifndef SNS_SAM_RESP_H
#define SNS_SAM_RESP_H

/*============================================================================
  @file sns_sam_resp.h

  @brief
  Handles processing of all incoming response messages to the SAM Framework.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include <stdint.h>
#include "qmi_idl_lib.h"
#include "fixed_point.h"
#include "sns_queue.h"
#include "sns_sam.h"
#include "sns_usmr.h"

/*============================================================================
  Function Declarations
  ===========================================================================*/

/**
 * Handle an incoming response message to the SAM framework.  The request
 * message associated with respMsg will be freed unless it is associated
 * with an active sensor request.
 *
 * @param[i] respMsg Message to be processed
 */
void
sns_sam_handle_resp( sns_sam_resp const *respMsg );

#endif /* SNS_SAM_RESP_H */
