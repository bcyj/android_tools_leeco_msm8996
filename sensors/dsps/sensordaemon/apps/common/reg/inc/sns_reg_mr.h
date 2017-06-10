#ifndef _SNS_REG_MR_H_
#define _SNS_REG_MR_H_
/*============================================================================
  @file sns_reg_mr.h

  @brief
  Header file specific for message routing and handling of Sensors Registry.

  <br><br>

  DEPENDENCIES: None.

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/
#include "qmi_idl_lib_internal.h"
#include <qmi_csi.h>

/*===========================================================================

  FUNCTION:   sns_reg_mr_params_init

  ===========================================================================*/
/*!
  @brief Init any req params

  @param[i] os_params: QMI object containing QCSI initialization parameters.

  @return None
*/
/*=========================================================================*/
void sns_reg_mr_params_init(qmi_csi_os_params *os_params);


/*===========================================================================

  FUNCTION:   sns_reg_mr_params_deinit

  ===========================================================================*/
/*!
  @brief De-init/Clean any req params

  @param None

  @return None
*/
/*=========================================================================*/
void sns_reg_mr_params_deinit(void);


/*===========================================================================

  FUNCTION:   sns_reg_mr_handle_evts

  ===========================================================================*/
/*!
  @brief Waits for any mr event's and handles them.

  @param[i] service_handle: qmi service handle used in registering reg service.
  @param[i] os_params: pointer to qmi_csi_os_params used in registering reg service.

  @return None
*/
/*=========================================================================*/
void sns_reg_mr_handle_evts(qmi_csi_service_handle *service_handle,
                            qmi_csi_os_params *os_params);

#endif /* _SNS_REG_MR_H_ */
