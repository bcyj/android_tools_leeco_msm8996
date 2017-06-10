#ifndef SNS_SAM_REQ_H
#define SNS_SAM_REQ_H

/*============================================================================
  @file sns_sam_req.h

  @brief
  Handles processing of all incoming request messages to the SAM Framework.

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
 * Removes a client request, and does all additional necessary cleanup.
 * Will remove orphaned algorithm instances.  See q_action_func_t template.
 *
 * @param[i] clientReq Client request to be removed
 */
void sns_sam_remove_client_req( sam_client_req *clientReq, void *unused );

/**
 * Removes all client request for a specific client connection.
 *
 * @param[i] serviceHndl Associated SMR service handle
 */
void sns_sam_remove_all_client_req( smr_qmi_client_handle serviceHndl );

/**
 * Handle a request message sent to a SAM algorithm.
 *
 * @param[i] reqMsg Request message to process
 */
void sns_sam_handle_req( sns_sam_req_msg const *reqMsg );

/**
 * Allocate a new output object.
 *
 * @param[i] algoInst Algorithm instance used to determine memory source
 *
 * @return Newly allocated object, or NULL if out of memory
 */
sns_sam_algo_output *sns_sam_alloc_output( sns_sam_algo_inst *algoInst );

/**
 * Allocate a new input object.
 *
 * @param[i] algoInst Algorithm instance used to determine memory source
 *
 * @return Newly allocated object, or NULL if out of memory
 */
sns_sam_algo_input *sns_sam_alloc_input( sns_sam_algo_inst *algoInst );

/**
 * Free an input/output object.
 *
 * @param[i] algoInst Algorithm instance used to determine memory source
 * @param[i] ioData Input/Output object to free
 */
void sns_sam_free_io( sns_sam_algo_inst const *algoInst,
   intptr_t ioData );

/**
 * Mark a SAM client as "not busy", which controls whether indications
 * will be sent.
 *
 * @param[i] serviceHndl SMR handle representing the client connection
 */
void sns_sam_mark_client_avail( smr_service_hndl serviceHndl );

/**
 * Register the duty cycle timer for an algorithm instance. Delta timer will
 * be determined based on current dc state and the DC on percentages of all
 * known client requests.
 *
 * @param[i] algoInst Algorithm Instance whose DC timer to register
 */
void
sns_sam_register_dc_timer( sns_sam_algo_inst *algoInst );

/**
 * Register the client timer for the given client request.  Only applicable
 * if the sensor is configured in a periodic reporting mode, or if batching
 * is enabled.
 *
 * @param[i] clientReq Client Request to update
 */
void
sns_sam_register_client_timer( sam_client_req *clientReq );

/**
 * Generate a new output data from an algorithm instance.
 *
 * @param[i] algoInst The associated algorithm instance
 * @param[o] outputData Generated output
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate algo inst object
 *         SAM_EFAILED The algorithm failed to generate an output data
 */
sns_sam_err
sns_sam_generate_output( sns_sam_algo_inst *algoInst, sns_sam_algo_output **outputDataIn );

#endif /* SNS_SAM_REQ_H */