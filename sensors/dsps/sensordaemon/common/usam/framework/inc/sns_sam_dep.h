#ifndef SNS_SAM_DEP_H
#define SNS_SAM_DEP_H

/*============================================================================
  @file sns_sam_dep.h

  @brief
  Functions and operations related to handling and maintaining dependent sensor
  requests.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

/*============================================================================
  Include Files
  ===========================================================================*/
#include <stdint.h>
#include "sns_common.h"
#include "qmi_idl_lib.h"
#include "sns_queue.h"
#include "sns_sam_algo_api.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ===========================================================================*/

/*============================================================================
  Type Declarations
  ===========================================================================*/

/*============================================================================
  Function Declarations
  ===========================================================================*/

/**
 * Calculate the aggregate batch period for all clients sharing this algorithm
 * instance, and use the algorithm API to inform the instance.
 *
 * @param[i] algoInst Active algorithm instance
 *
 * @return Batch period (in seconds); 0 if no batching clients
 */
void sns_sam_inform_batch( sns_sam_algo_inst const *algoInst );

/**
 * Find an acceptable sensor request for this algorithm dependency.  Create
 * new request if none are available.  This function will also rectify all
 * requests to this particular sensor request.
 *
 * Only one request per dependent sensor is allowed, so this
 * request may replace an existing one (will send a new request, and cancel
 * the existing one).
 *
 * @param[i] algoInst Active algorithm instance
 * @param[i] sensorUID Dependent sensor to be enabled
 *
 * @return SAM_ENONE
 *         SAM_EFAILED Error sending enable request message
 *         SAM_ENOMEM Not enough memory to allocate or enable new sensor req
 *         SAM_EMAX Too many sensor requests for this algorithm instance
 *         SAM_ETYPE Unknown/unavailable sensor UID
 */
sns_sam_err sns_sam_handle_sensor_change( sns_sam_algo_inst *algoInst,
  sns_sam_sensor_uid const *sensorUID );

/**
 * Checks if there are any remaining clients of the specified
 * sensor request, and cancels the request if necessary.  This may also involve
 * changing the sensor requests for other algorithm instances that were sharing
 * the dependent stream.
 *
 * @note This function will not dissociate a sensor request from any
 *       algorithm instance.
 *
 * @param[i] sensorRequest Sensor request to be removed
 *
 * @return SAM_ENONE
 *         SAM_EFAILED Error sending disable request message
 */
sns_sam_err
sns_sam_remove_sensor_req( sns_sam_sensor_req *sensorRequest );

/**
 * Start all dependencies for a specified algorithm
 *
 * @param[i] algo The associated algorithm
 * @param[o] algoInst The created algorithm instance object
 *
 * @return SAM_ENONE
 *         SAM_EFAILED Unable to initialize the algorithm dependencies
 */
sns_sam_err sns_sam_start_dependencies( sns_sam_sensor_algo const *algo,
  sns_sam_algo_inst *algoInst );

/**
 * Update the batch period for a dependent sensor (only applicable for SAM
 * sensors).  If the sensor request is shared amongst other algorithm
 * instances, we will simply note the request, but not change the stream.
 *
 * @param[i] algoInst Active algorithm instance
 * @param[i] sensorUID Dependent sensor to be disabled
 * @param[i] batchPeriod Batch period requested (in seconds, Q16)
 *
 * @return SAM_ENONE
 *         SAM_ETYPE There is no sensor of sensorUID presently associated
 *         SAM_EFAILED Error sending batch request
 */
sns_sam_err sns_sam_set_batch_period( sns_sam_algo_inst *algoInst,
  sns_sam_sensor_uid const *sensorUID, uint32_t batchPeriod );

#endif /* SNS_SAM_DEP_H */
