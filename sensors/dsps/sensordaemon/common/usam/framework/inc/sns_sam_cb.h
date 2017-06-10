#ifndef SNS_SAM_CB_H
#define SNS_SAM_CB_H

/*============================================================================
  @file sns_sam_cb.h

  @brief
  Generate the callback functions for an algorithm.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include <stdint.h>
#include "fixed_point.h"
#include "sns_sam_algo_api.h"

/*============================================================================
  Function Declarations
  ===========================================================================*/

/**
 * Generate and return the appropriate callback struct for the given algorithm.
 *
 * @param[i] algoInst Algorithm instance which will use the callback functions
 * @param[o] cbFunctions Struct to fill-in
 */
void
sns_sam_init_cb( sns_sam_algo_inst const *algoInst,
    sns_sam_algo_callback *cbFunctions );

/**
 * Process all algorithm timer events received.
 */
void
sns_sam_handle_algo_timer( void );

/**
 * Clear all algorithm timer events associated with the specified algorithm
 * instance.
 *
 * @param[i] algoInst Algorithm instance to search for
 */
void
sns_sam_clear_algo_timer( sns_sam_algo_inst const *algoInst );

#endif /* SNS_SAM_CB_H */