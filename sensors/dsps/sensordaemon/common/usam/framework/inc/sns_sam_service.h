#ifndef SNS_SAM_SERVICE_H
#define SNS_SAM_SERVICE_H

/*============================================================================
  @file sns_sam_service.h

  @brief
  The SAM Framework both acts as a QMI service and a QMI client.  This file
  contains the declarations for all service duties.

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
#include "sns_sam_algo_api.h"
#include "sns_usmr.h"

/*============================================================================
  Function Declarations
  ===========================================================================*/

/**
 * Initialize all static objects contained within sns_sam_service.c.
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to initialize Framework
 *         SAM_EFAILED Framework initialization failed
 */
sns_sam_err
sns_sam_service_init_fw( void );

/**
 * Register a SAM algorithm/service with SMR.  This should not be called until
 * all algorithm dependencies have been met (SMGR, Registry, SAM, etc.).
 *
 * @param[i] algo Algorithm to be registered with SMR
 *
 * @return SAM_ENONE
 *         SAM_ESMR Error registering service
 */
sns_sam_err sns_sam_service_reg( sns_sam_sensor_algo *algo );

/**
 * Unregister a SAM algorithm/service with SMR.  This should be called when a
 * sns_sam_client_error_cb is received from one of the algorithm's
 * dependencies.  The SAM Framework should subsequently call
 * sns_sam_client_check() to attempt to re-initialize the algorithm/service.
 *
 * @param[i] algo Algorithm to unregister
 *
 * @return SAM_ENONE
 *         SAM_ESMR Error unregistering service
 */
sns_sam_err sns_sam_service_unreg( sns_sam_sensor_algo const *algo );

/**
 * Send a response message.
 *
 * @param[i] reqMsg The request message associated with this response
 * @param[i] respMsg Message to send
 *
 * @return SAM_ENONE
 *         SAM_ESMR Error sending response message to client
 */
sns_sam_err sns_sam_service_send_resp( sns_sam_req_msg const *reqMsg,
    struct sns_sam_msg const *respMsg );

/**
 * Send an indication request to the specified client.
 *
 * @param[i] message Indication message to send
 * @param[i] clientReq Message destination
 *
 * @return SAM_ENONE
 *         SAM_ENOT_AVAILABLE Client is presently unable to process indications
 *         SAM_ESMR Error sending indication to client
 */
sns_sam_err sns_sam_service_send_ind( sns_sam_ind_msg const *indMsg,
    sam_client_req const *clientReq );

/**
 * Process all messages on the incoming request queue.  Will result in calls
 * to sns_sam_handle_req().
 */
void
sns_sam_process_req_q( void );

/**
 * Process all messages on the "resume" queue.  This is a list of all clients
 * that were previously blocked from receiving indications, and should now
 * start receiving them.
 */
void sns_sam_process_resume_q( void );

#endif /* SNS_SAM_SERVICE_H */