/******************************************************************************
  @file    qmi_sap_client.c
  @brief   The QMI Restricted Access Proxy (SAP) common header file

  DESCRIPTION
  QMI Restricted Access Proxy routines.

  ---------------------------------------------------------------------------
  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#include "qmi_idl_lib.h"
#include "qmi_sap.h"
#include "qmi_sap_target_ext.h"
#include "qmi_client.h"
#include "qmi_common.h"
#include "service_access_proxy_v01.h"

typedef struct qmi_sap_client_objs_s
{
  qmi_idl_service_object_type service_obj;
  qmi_client_type clnt;
}qmi_sap_client_objs;

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
)
{
  qmi_client_type notifier;
  qmi_service_info info[2];
  uint32_t num_services,num_entries;
  sap_register_service_req_msg_v01 reg_req;
  sap_register_service_resp_msg_v01 reg_resp;
  qmi_sap_client_objs *client;
  int rc;
  qmi_idl_service_object_type qsap_service_object = sap_get_service_object_v01();

  client = QSAP_CALLOC(sizeof(qmi_sap_client_objs),1);
  if (!client || !os_params || !user_handle)
  {
    if (user_handle)
    {
      *user_handle = NULL;
    }
    return QMI_SAP_INTERNAL_ERR;
  }
  client->service_obj = service_obj;
  /* Connect to the QSAP Service on modem and send registration message */
  rc = qmi_client_notifier_init(qsap_service_object, os_params, &notifier);
  QMI_CCI_OS_SIGNAL_WAIT(os_params, 0);
  QMI_CCI_OS_SIGNAL_CLEAR(os_params);
  num_entries = 2;
  rc = qmi_client_get_service_list( qsap_service_object, info, &num_entries, &num_services);
  if (num_services != 0)
  {
    /* TODO Pass client pointer as ind_cb_data parameter eventually */
    rc =  qmi_client_init(info,qsap_service_object,NULL,NULL,os_params,&client->clnt);
  }else
  {
    QSAP_FREE(client);
    *user_handle = NULL;
    qmi_client_release(notifier);
    return QMI_SAP_INTERNAL_ERR;
  }
  qmi_client_release(notifier);
  if (rc != QMI_NO_ERR)
  {
    QSAP_FREE(client);
    *user_handle = NULL;
    return QMI_SAP_INTERNAL_ERR;
  }

  qmi_idl_get_service_id(service_obj,&reg_req.service_obj.service_id);
  qmi_idl_get_idl_version(service_obj,&reg_req.service_obj.major_vers);
  qmi_idl_get_max_service_len(service_obj,&reg_req.service_obj.max_msg_len);

  rc = qmi_client_send_msg_sync(client->clnt,QMI_SAP_REGISTER_SERVICE_REQ_V01,&reg_req,
                               sizeof(sap_register_service_req_msg_v01),&reg_resp,
                               sizeof(sap_register_service_resp_msg_v01),0);
  if (rc != QMI_NO_ERR || reg_resp.resp.result)
  {
    QSAP_FREE(client);
    *user_handle = NULL;
    return QMI_SAP_INTERNAL_ERR;
  }

  *user_handle = client;
  return QMI_SAP_NO_ERR;
}

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
)
{
  qmi_sap_client_objs *client;
  sap_deregister_service_req_msg_v01 reg_req;
  sap_deregister_service_resp_msg_v01 reg_resp;
  client = (qmi_sap_client_objs *)user_handle;
  if (client)
  {
      qmi_idl_get_service_id(client->service_obj,&reg_req.service_obj.service_id);
      qmi_idl_get_idl_version(client->service_obj,&reg_req.service_obj.major_vers);
      qmi_idl_get_max_service_len(client->service_obj,&reg_req.service_obj.max_msg_len);

      qmi_client_send_msg_sync(client->clnt,QMI_SAP_DEREGISTER_SERVICE_REQ_V01,&reg_req,
                               sizeof(sap_register_service_req_msg_v01),&reg_resp,
                               sizeof(sap_register_service_resp_msg_v01),0);
      QSAP_FREE(user_handle);
  }

  return QMI_SAP_NO_ERR;
}
