#ifndef QMI_CLIENT_INSTANCE_DEFS_H
#define QMI_CLIENT_INSTANCE_DEFS_H
/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include <stdint.h>

#include "qmi_client.h"
#include "qmi_port_defs.h"

#ifdef __cplusplus
extern "C" {
#endif
/* QMI port identifiers
*/
typedef enum
{
  QMI_CLIENT_QMUX_BASE_INSTANCE = 0x80,
  QMI_CLIENT_QMUX_RMNET_INSTANCE_0 = QMI_CLIENT_QMUX_BASE_INSTANCE + QMI_CONN_ID_RMNET_0,
  QMI_CLIENT_QMUX_RMNET_INSTANCE_1,
  QMI_CLIENT_QMUX_RMNET_INSTANCE_2,
  QMI_CLIENT_QMUX_RMNET_INSTANCE_3,
  QMI_CLIENT_QMUX_RMNET_INSTANCE_4,
  QMI_CLIENT_QMUX_RMNET_INSTANCE_5,
  QMI_CLIENT_QMUX_RMNET_INSTANCE_6,
  QMI_CLIENT_QMUX_RMNET_INSTANCE_7,
  QMI_CLIENT_QMUX_RMNET_INSTANCE_8,
  QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_0 = QMI_CLIENT_QMUX_BASE_INSTANCE + QMI_CONN_ID_RMNET_SDIO_0,
  QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_1,
  QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_2,
  QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_3,
  QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_4,
  QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_5,
  QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_6,
  QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_7,
  QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0 = QMI_CLIENT_QMUX_BASE_INSTANCE + QMI_CONN_ID_RMNET_USB_0,
  QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_1,
  QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_2,
  QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_3,
  QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_4,
  QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_5,
  QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_6,
  QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_7,
  QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0 = QMI_CLIENT_QMUX_BASE_INSTANCE + QMI_CONN_ID_RMNET_SMUX_0,
  QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0 = QMI_CLIENT_QMUX_BASE_INSTANCE + QMI_CONN_ID_RMNET_MHI_0,
  QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_1,
  QMI_CLIENT_QMUX_PROXY_INSTANCE = QMI_CLIENT_QMUX_BASE_INSTANCE + QMI_CONN_ID_PROXY,
  QMI_CLIENT_QMUX_MAX_INSTANCE_IDS = QMI_CLIENT_QMUX_BASE_INSTANCE + QMI_MAX_CONN_IDS,
} qmi_client_qmux_instance_type;

/*===========================================================================
  FUNCTION  qmi_cci_qmux_xport_register
===========================================================================*/
/*!
@brief
  Register a QMUX xport instance with QCCI

@param[in]  instance      Required QMUX instance to be registered
                          Must be invoked before any QCCI APIs

@retval  QMI_NO_ERR       Success
         QMI_INTERNAL_ERR Invalid input parameters
*/
/*=========================================================================*/
qmi_client_error_type qmi_cci_qmux_xport_register
(
 qmi_client_qmux_instance_type instance
);

/*===========================================================================
  FUNCTION  qmi_cci_qmux_xport_unregister
===========================================================================*/
/*!
@brief
  Unregister a QMUX xport instance with QCCI

@param[in]  instance      Required QMUX instance to be unregistered

@retval  QMI_NO_ERR       Success
         QMI_INTERNAL_ERR Invalid input parameters
*/
/*=========================================================================*/
qmi_client_error_type qmi_cci_qmux_xport_unregister
(
 qmi_client_qmux_instance_type instance
);

#ifdef __cplusplus
}
#endif
#endif
