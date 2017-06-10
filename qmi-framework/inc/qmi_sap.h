#ifndef QMI_SAP_H
#define QMI_SAP_H
/******************************************************************************
  @file    qmi_sap.h
  @brief   The QMI Service Access Proxy (SAP) Header file.

  DESCRIPTION
  QMI service access routines.  Services wanting to export their
  functionality off chip via QMUX will use these routines.

  ---------------------------------------------------------------------------
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include "qmi_sap_target_ext.h"
#include "qmi_idl_lib.h"
#include "qmi_client.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *qmi_sap_client_handle;

typedef enum
{
  QMI_SAP_NO_ERR = 0,
  QMI_SAP_INTERNAL_ERR
}qmi_sap_error;

/*=============================================================================
  FUNCTION qmi_sap_register
=============================================================================*/
/*!
@brief
  This function allows a client to register a particular service with QSAP

@param[in]   service_obj        Pointer to the service object of the service being
                                registered with QSAP
@param[out]  user_handle        Pointer to a handle the client can use to deregister
                                from QSAP

@retval      QMI_SAP_NO_ERR     Success
*/
/*=========================================================================*/
qmi_sap_error qmi_sap_register
(
  qmi_idl_service_object_type service_obj,
  qmi_client_os_params        *os_params,
  qmi_sap_client_handle       *user_handle
);

/*=============================================================================
  FUNCTION qmi_sap_deregister
=============================================================================*/
/*!
@brief
  Deregisters a service from QSAP

@param[in]   user_handle        Pointer to the user_handle provided by the
                                qmi_sap_register function

@retval      QMI_SAP_NO_ERR     Success
*/
/*=========================================================================*/
qmi_sap_error qmi_sap_deregister
(
  qmi_sap_client_handle user_handle
);

#ifdef __cplusplus
}
#endif
#endif /* QMI_SAP_H */
