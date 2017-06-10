#ifndef SNS_SAM_REG_H
#define SNS_SAM_REG_H

/*============================================================================
  @file sns_sam_reg.h

  @brief
  Implements all handling, processing, and generating messages to/from the
  Sensors Registry Service.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include <stdint.h>
#include "sns_sam_algo_api.h"

/*============================================================================
  Function Declarations
  ===========================================================================*/

/**
 * Request the specified group data to be read from or written to the Sensors
 * Registry.
 *
 * @param[io] regData Registry data to read/write
 *
 * @return
 *  SAM_ENONE
 *  SAM_EFAILED Unable to send registry request
 */
sns_sam_err sns_sam_reg_req( sns_sam_reg_data *regData );

/**
 * Process a registry response message.  Note that the SAM Framework only ever
 * will send a registry group read or group write request, so those are the
 * only responses we need to handle.
 *
 * @param[i] respMsg Response message to process
 *
 * @note Message buffer will be freed by caller
 *
 * @return
 *  SAM_ENONE
 *  SAM_EFAILED Error in response message
 */
sns_sam_err sns_sam_reg_handle_resp( sns_sam_resp const *respMsg );

#endif /* SNS_SAM_REG_H */