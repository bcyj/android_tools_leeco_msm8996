/*==========================================================================
@file sns_time_priv.h

@brief
Provides functions to acquire and set settings in the sensors registry.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==========================================================================*/
#ifndef _SENSOR_TIME_PRIV_
#define _SENSOR_TIME_PRIV_

/*============================================================================
  INCLUDE FILES
  ==========================================================================*/

#include "sns_osa.h"
#include "sns_smr_util.h"
#include <stdbool.h>
#include <inttypes.h>

/**
 * OS flag group for the Sensors Time task
 */
OS_FLAG_GRP *sns_time_sig_grp;

/*============================================================================
  Function Declarations
  ============================================================================*/

/*!
  @brief Initialize message router data structures and connections.

  @return
  SNS_SUCCESS or error code.
*/
sns_err_code_e sns_time_mr_init();

/*!
  @brief Deinitialize message router data structures and connections.

  @return
  SNS_SUCCESS or error code.
*/
sns_err_code_e sns_time_mr_deinit();

/*!
  @brief Main thread loop for the Sensors Time task.

  @param[i] p_arg: Argument passed in during thread creation. Unused.

  @return
  None.
*/
void sns_time_mr_thread( void* );

/*!
  @brief Add a client to the client list.

  @param[i] client_hndl Opaque and unique client handle

  @return SNS_SUCCESS  upon success.
*/
sns_err_code_e sns_time_client_add( intptr_t client_hndl );

/*!
  @brief Removes a client from the client list.

  @param[i] client_hndl Opaque message routing client handle

  @return 0 upon success.
*/
int sns_time_client_delete( intptr_t client_hndl );

/*!
  @brief Generate the current apps-adsp timestamp offsets.

  @param[o] timestamp_apps AP timestamp
  @param[o] timestamp_dsps SSC timestamp corresponding to AP TS
  @param[o] dsps_rollover_cnt Number of SSC TS rollovers detected
  @param[o] timestamp_apps_boottime AP boottime timestamp

  @return
  0 upon success, otherwise error.

  @sideeffects
  Calculates and responds with an appropriate timestamp
*/
int sns_time_generate( uint64_t *timestamp_apps,
                       uint32_t *timestamp_dsps, uint32_t *dsps_rollover_cnt,
                       uint64_t *timestamp_apps_boottime );

/*!
  @brief Send a timestamp report indication to the specified client

  @param[i] client_hndl Opaque client handle
*/
void sns_time_mr_send_ind( intptr_t client_hndl );

#endif /* _SENSOR_TIME_PRIV_ */
