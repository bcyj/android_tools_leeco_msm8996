#ifndef SNS_SAM_CLIENT_H
#define SNS_SAM_CLIENT_H

/*============================================================================
  @file sns_sam_client.h

  @brief
  The SAM Framework both acts as a QMI service and a QMI client.  This file
  contains the declarations for all client duties.

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
  External Objects
  ===========================================================================*/

/* Temporary objects used during SAM initialization. */
extern sns_sam_sensor_req sensorReqREG;
extern sns_sam_sensor_req sensorReqSMGR;
extern sns_sam_sensor_req sensorReqSMGRI;

/*============================================================================
  Function Declarations
  ===========================================================================*/

/**
 * Register as a SMR client for the specified sensor request. Note that the
 * serviceObj and clientHndl fields must be set prior calling this function.
 *
 * @param[i] sensorReq Sensor to initialize
 *
 * @return SAM_ENONE
 *         SAM_ENOT_AVAILABLE Client has already been initialized
 *         SAM_ESMR SMR error received
 */
sns_sam_err sns_sam_client_init( sns_sam_sensor_req *sensorReq );

/**
 * Release an SMR client connection.  Dynamically allocated memory associated
 * with the sensor request object will be freed asynchronously.
 *
 * @param[i] sensorReq Sensor to release
 */
void sns_sam_client_release( sns_sam_sensor_req *sensorReq );

/**
 * Check if the specified QMI service-Instance ID combo is available.
 * Registers a callback function to alert the SAM Framework.
 *
 * @param[i] serviceObj QMI service to look for
 * @param[i] instanceID Instance ID to look for; or SMR_CLIENT_INSTANCE_ANY
 * @param[i] timeout How long to wait for the service; 0 indefinitely
 *
 * @return SAM_ENONE
 *         SAM_ESMR SMR error received
 */
sns_sam_err sns_sam_client_check( qmi_idl_service_object_type serviceObj,
    qmi_service_instance instanceID, uint32_t timeout );

/**
 * Send a request message to some other sensor service.
 *
 * @note The callee should not free the request message at this time, but
 *       instead wait until after the response has been received/processed.
 *
 * @param[i] sensorReq Sensor request associated with this message
 * @param[i] reqMsg Message to be sent
 *
 * @return SAM_ENONE
 *         SAM_ETYPE Unknown sensor UID or message ID
 *         SAM_ESMR SMR error received
 */
sns_sam_err sns_sam_client_send( sns_sam_sensor_req const *sensorReq,
    struct sns_sam_msg const *reqMsg );

/**
 * Handle all received error callbacks from the SMR.
 */
void sns_sam_process_error_q( void );

/**
 * Process all messages on the incoming response queue.  Will result in calls
 * to sns_sam_handle_resp().
 */
void sns_sam_process_resp_q( void );

/**
 * Process all messages on the incoming indication queue.  Will result in calls
 * to sns_sam_handle_ind().
 */
void sns_sam_process_ind_q( void );

/**
 * Initialize all static objects contained within sns_sam_client.c.
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to initialize Framework
 *         SAM_EFAILED Framework initialization failed
 */
sns_sam_err sns_sam_client_init_fw( void );

#endif /* SNS_SAM_CLIENT_H */
