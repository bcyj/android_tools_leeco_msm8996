/*============================================================================
  @file sns_reg_mr_la.c

  @brief
  Linux Android specific Sensors Registry MR definitions.


  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/
#include "sns_reg.h"
#include "sns_reg_mr.h"

/*===========================================================================

  FUNCTION:   sns_reg_mr_params_init
  Sets init params if any.
 =========================================================================*/
void sns_reg_mr_params_init(qmi_csi_os_params *os_params)
{
    UNREFERENCED_PARAMETER(os_params);
    /* Nothing to init */
    return;
}

/*===========================================================================

  FUNCTION:   sns_reg_mr_params_deinit
  De-init/clean params if any.
 =========================================================================*/
void sns_reg_mr_params_deinit(void)
{
    /* Nothing to de-init */
    return;
}

/*===========================================================================
  FUNCTION:   sns_reg_mr_handle_evts
  Waits for qmi events and handles them.
 =========================================================================*/
void sns_reg_mr_handle_evts(qmi_csi_service_handle *service_handle, qmi_csi_os_params *os_params)
{
  qmi_csi_os_params os_params_in;
  fd_set fds;

  while(1)
  {
    fds = os_params->fds;
    select( (os_params->max_fd)+1, &fds, NULL, NULL, NULL );
    os_params_in.fds = fds;

    qmi_csi_handle_event(*service_handle, &os_params_in);
  }
}
