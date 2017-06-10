/*===========================================================================
  @file
  qmi_client_helper.c

  @brief
  QCCI client library wrapper for performing port binding management.

  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/24/14   hm      Initial module.

===========================================================================*/

#include "qmi_port_defs.h"
#include "qmi_platform.h"
#include "qmi_client_helper.h"
#include "wireless_data_service_v01.h"
#include "data_filter_service_v01.h"
#include "ds_util.h"

typedef struct
{
  ds_target_t target;
  qmi_client_qmux_instance_type qmux_inst;
  qmi_client_qmux_instance_type qmux_inst2; /* For dual-modem targets */
} qmi_client_xport_reg_s;

static qmi_client_xport_reg_s xport_reg_arr[] =
{
  { DS_TARGET_MSM,              QMI_CLIENT_QMUX_RMNET_INSTANCE_0,        -1                                   },
  { DS_TARGET_APQ,              -1,                                      -1                                   },
  { DS_TARGET_SVLTE1,           QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0,    QMI_CLIENT_QMUX_RMNET_INSTANCE_0     },
  { DS_TARGET_SVLTE2,           QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0,    QMI_CLIENT_QMUX_RMNET_INSTANCE_0     },
  { DS_TARGET_CSFB,             QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0,    -1                                   },
  { DS_TARGET_SGLTE,            QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0,   QMI_CLIENT_QMUX_RMNET_INSTANCE_0     },
  { DS_TARGET_SGLTE2,           QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0,   QMI_CLIENT_QMUX_RMNET_INSTANCE_0     },
  { DS_TARGET_DSDA,             QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0,   QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0 },
  { DS_TARGET_DSDA2,            QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0,    QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0 },
  { DS_TARGET_DSDA3,            QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0,   QMI_CLIENT_QMUX_RMNET_INSTANCE_0     },
  { DS_TARGET_MDM,              QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0,    -1                                   },
  { DS_TARGET_FUSION4_5_PCIE,   QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0,    -1                                   },
  { DS_TARGET_LE_MDM9X35,       QMI_CLIENT_QMUX_RMNET_INSTANCE_0,        -1                                   },
  { DS_TARGET_LE_MDM9X25,       QMI_CLIENT_QMUX_RMNET_INSTANCE_0,        -1                                   },
  { DS_TARGET_LE_MDM9X15,       QMI_CLIENT_QMUX_RMNET_INSTANCE_0,        -1                                   },
  { DS_TARGET_LE_LEGACY,        QMI_CLIENT_QMUX_RMNET_INSTANCE_0,        -1                                   },
};

#define NUM_XPORT_REG_ARR (sizeof(xport_reg_arr)/sizeof(xport_reg_arr[0]))


/*===========================================================================

                         EXTERNAL FUNCTION DEFINITIONS

===========================================================================*/

/*===========================================================================
  FUNCTION: qmi_client_helper_reg_xport
===========================================================================*/
/**
  @ingroup qmi_client_helper

  @brief
  Register correct QMUX transports with QCCI library based on target.

  @param[in]   None.
  @retval      None.
*/
static void qmi_client_helper_reg_xport()
{
  int i;
  ds_target_t target = ds_get_target();

  QMI_DEBUG_MSG_1("qmi_client_helper_reg_xport(): Target %s",
                  ds_get_target_str(target));
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);

  for (i = 0; i < (int) NUM_XPORT_REG_ARR; i++)
  {
    if (xport_reg_arr[i].target == target)
    {
      if (-1 != (int) xport_reg_arr[i].qmux_inst)
      {
        QMI_DEBUG_MSG_2("qmi_client_helper: Reg QMUX transport %d for target %s",
                        xport_reg_arr[i].qmux_inst, ds_get_target_str(target));
        qmi_cci_qmux_xport_register(xport_reg_arr[i].qmux_inst);
      }

      if (-1 != (int) xport_reg_arr[i].qmux_inst2)
      {
        QMI_DEBUG_MSG_2("qmi_client_helper: Reg QMUX transport %d for target %s",
                        xport_reg_arr[i].qmux_inst, ds_get_target_str(target));
        qmi_cci_qmux_xport_register(xport_reg_arr[i].qmux_inst2);
      }

      break;
    }
  }
}

/*===========================================================================

                         EXTERNAL FUNCTION DEFINITIONS

===========================================================================*/

/*===========================================================================
  FUNCTION: qmi_client_wds_init_instance
===========================================================================*/
/**
  @ingroup qmi_client_helper

  @brief
  Helper blocking function to lookup and initialize a connection with
  WDS service over a specific connection.

  @details
  Data services QMI clients need to communicate with WDS service over
  a specific connection. These connection string names are different over
  different transports. In newer architectures, binding to a mux-id is
  required. This helper routine performs all of these background activites
  so clients can easily use QMI services for multi-port scenarios.
  Other parameter definitions are present in qmi_client.h header file.

  @param[in]   dev_str            Connection ID string("rmnet_data1" etc)
  @param[in]   service_obj        Service object
  @param[in]   instance_id        Instance ID of the service.
                                  Use QMI_CLIENT_INSTANCE_ANY if there is
                                  no preference on the instance ID.
  @param[in]   ind_cb             Indication callback function
  @param[in]   ind_cb_data        Indication callback user-data
  @param[in]   os_params          OS-specific parameters. It can be a pointer
                                  to event object, or signal mask and TCB
  @param[in]   timeout            Time-out in milliseconds. 0 = no timeout
  @param[out]  user_handle        Handle used by the infrastructure to
                                  identify different clients.

  @retval  QMI_NO_ERR             Success
           QMI_INTERNAL_ERR       Invalid input parameters
           QMI_TIMEOUT_ERR        No service of the required instance_id
                                  was found in the time provided by timeout.

  @notes
  See qmi_client.h:qmi_client_init_instance() for other usage details of
  the API including blocking behavior, qmi_instance_id and timeout usage.
*/
qmi_client_error_type qmi_client_wds_init_instance
(
 const char                 *dev_str,
 qmi_idl_service_object_type service_obj,
 qmi_service_instance        instance_id,
 qmi_client_ind_cb           ind_cb,
 void                        *ind_cb_data,
 qmi_client_os_params        *os_params,
 uint32_t                    timeout,
 qmi_client_type             *user_handle
)
{
  int ep_type = -1;
  int epid = -1;
  int mux_id = -1;
  qmi_connection_id_type conn_id;
  qmi_client_error_type rc;
  wds_bind_mux_data_port_req_msg_v01 req;
  wds_bind_mux_data_port_resp_msg_v01 resp;

  QMI_DEBUG_MSG_0("qmi_client_wds_init_instance(): Entry()");

  /* Register correct QMUX transport with QCCI based on target */
  qmi_client_helper_reg_xport();

  /* Create client using QCCI libary - this is a blocking call */
  rc = qmi_client_init_instance(service_obj,
                                instance_id,
                                ind_cb,
                                ind_cb_data,
                                os_params,
                                timeout,
                                user_handle);
  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_client_init_instance() error %d", rc);
    return rc;
  }

  /* Get EPID, MUX-ID information */
  if (QMI_CONN_ID_INVALID ==
      (conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID_EX(dev_str,
                                                     &ep_type,
                                                     &epid,
                                                     &mux_id)))
  {
    QMI_ERR_MSG_1("Dev to conn_id translation failed: dev %s", dev_str);
    return QMI_INTERNAL_ERR;
  }

  /* Bind to a epid/mux-id if mux-id-binding is supported */
  if (0 < mux_id)
  {
    memset (&req, 0, sizeof(req));
    memset (&resp, 0, sizeof(resp));

    req.ep_id_valid = (epid != -1);
    req.ep_id.ep_type = (data_ep_type_enum_v01) ep_type;
    req.ep_id.iface_id = (uint32_t) epid;

    req.mux_id_valid = (mux_id != -1);
    req.mux_id = (uint8_t) mux_id;

#ifdef FEATURE_QMI_IWLAN
    req.reversed_valid = req.reversed =
      (0 == strncmp(dev_str,
                    QMI_PLATFORM_REV_RMNET_DATA_PREFIX,
                    strlen(QMI_PLATFORM_REV_RMNET_DATA_PREFIX)));
#endif

    /* Send bind_req */
    rc = qmi_client_send_msg_sync(*user_handle,
                                  QMI_WDS_BIND_MUX_DATA_PORT_REQ_V01,
                                  (void *)&req,
                                  sizeof(wds_bind_mux_data_port_req_msg_v01),
                                  (void*)&resp,
                                  sizeof(wds_bind_mux_data_port_resp_msg_v01),
                                  timeout);
  }

  return rc;
}

/*===========================================================================
  FUNCTION: qmi_client_dfs_init_instance
===========================================================================*/
/**
  @ingroup qmi_client_helper

  @brief
  Helper blocking function to lookup and initialize a connection with
  DFS service over a specific connection.

  @details
  Data services QMI clients need to communicate with DFS service over
  a specific connection. These connection string names are different over
  different transports. In newer architectures, binding to a mux-id is
  required. This helper routine performs all of these background activites
  so clients can easily use QMI services for multi-port scenarios.
  Other parameter definitions are present in qmi_client.h header file.

  @param[in]   dev_str            Connection ID string("rmnet_data1" etc)
  @param[in]   service_obj        Service object
  @param[in]   instance_id        Instance ID of the service.
                                  Use QMI_CLIENT_INSTANCE_ANY if there is
                                  no preference on the instance ID.
  @param[in]   ind_cb             Indication callback function
  @param[in]   ind_cb_data        Indication callback user-data
  @param[in]   os_params          OS-specific parameters. It can be a pointer
                                  to event object, or signal mask and TCB
  @param[in]   timeout            Time-out in milliseconds. 0 = no timeout
  @param[out]  user_handle        Handle used by the infrastructure to
                                  identify different clients.

  @retval  QMI_NO_ERR             Success
           QMI_INTERNAL_ERR       Invalid input parameters
           QMI_TIMEOUT_ERR        No service of the required instance_id
                                  was found in the time provided by timeout.

  @notes
  See qmi_client.h:qmi_client_init_instance() for other usage details of
  the API including blocking behavior, qmi_instance_id and timeout usage.
*/
qmi_client_error_type qmi_client_dfs_init_instance
(
 const char                 *dev_str,
 qmi_idl_service_object_type service_obj,
 qmi_service_instance        instance_id,
 qmi_client_ind_cb           ind_cb,
 void                        *ind_cb_data,
 qmi_client_os_params        *os_params,
 uint32_t                    timeout,
 qmi_client_type             *user_handle
)
{

  int ep_type = -1;
  int epid = -1;
  int mux_id = -1;
  qmi_connection_id_type conn_id;
  qmi_client_error_type rc;
  dfs_bind_client_req_msg_v01 req;
  dfs_bind_client_resp_msg_v01 resp;

  QMI_DEBUG_MSG_0("qmi_client_dfs_init_instance(): Entry()");

  /* Register correct QMUX transport with QCCI based on target */
  qmi_client_helper_reg_xport();

  /* Create client using QCCI libary - this is a blocking call */
  rc = qmi_client_init_instance(service_obj,
                                instance_id,
                                ind_cb,
                                ind_cb_data,
                                os_params,
                                timeout,
                                user_handle);
  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_1("qmi_client_init_instance() error %d", rc);
    return rc;
  }

  /* Get EPID, MUX-ID information */
  if (QMI_CONN_ID_INVALID ==
      (conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID_EX(dev_str,
                                                     &ep_type,
                                                     &epid,
                                                     &mux_id)))
  {
    QMI_ERR_MSG_1("Dev to conn_id translation failed: dev %s", dev_str);
    return QMI_INTERNAL_ERR;
  }

  /* Bind to a epid/mux-id if mux-id-binding is supported */
  if (0 < mux_id)
  {
    memset (&req, 0, sizeof(req));
    memset (&resp, 0, sizeof(resp));

    req.ep_id_valid = (epid != -1);
    req.ep_id.ep_type = (data_ep_type_enum_v01) ep_type;
    req.ep_id.iface_id = (uint32_t) epid;

    req.mux_id_valid = (mux_id != -1);
    req.mux_id = (uint8_t) mux_id;

    /* Send bind_req */
    rc = qmi_client_send_msg_sync(*user_handle,
                                  QMI_DFS_BIND_CLIENT_REQ_V01,
                                  (void *)&req,
                                  sizeof(req),
                                  (void*)&resp,
                                  sizeof(resp),
                                  timeout);
  }

  return rc;
}

/*===========================================================================
  FUNCTION: qmi_client_qos_init_instance
===========================================================================*/
/**
  @ingroup qmi_client_helper

  @brief
  Helper blocking function to lookup and initialize a connection with
  QOS service over a specific connection.

  @details
  Data services QMI clients need to communicate with QOS service over
  a specific connection. These connection string names are different over
  different transports. In newer architectures, binding to a mux-id is
  required. This helper routine performs all of these background activites
  so clients can easily use QMI services for multi-port scenarios.
  Other parameter definitions are present in qmi_client.h header file.

  @param[in]   dev_str            Connection ID string("rmnet_data1" etc)
  @param[in]   service_obj        Service object
  @param[in]   instance_id        Instance ID of the service.
                                  Use QMI_CLIENT_INSTANCE_ANY if there is
                                  no preference on the instance ID.
  @param[in]   ind_cb             Indication callback function
  @param[in]   ind_cb_data        Indication callback user-data
  @param[in]   os_params          OS-specific parameters. It can be a pointer
                                  to event object, or signal mask and TCB
  @param[in]   timeout            Time-out in milliseconds. 0 = no timeout
  @param[out]  user_handle        Handle used by the infrastructure to
                                  identify different clients.

  @retval  QMI_NO_ERR             Success
           QMI_INTERNAL_ERR       Invalid input parameters
           QMI_TIMEOUT_ERR        No service of the required instance_id
                                  was found in the time provided by timeout.

  @notes
  See qmi_client.h:qmi_client_init_instance() for other usage details of
  the API including blocking behavior, qmi_instance_id and timeout usage.
*/
qmi_client_error_type qmi_client_qos_init_instance
(
 const char                 *dev_str,
 qmi_idl_service_object_type service_obj,
 qmi_service_instance        instance_id,
 qmi_client_ind_cb           ind_cb,
 void                        *ind_cb_data,
 qmi_client_os_params        *os_params,
 uint32_t                    timeout,
 qmi_client_type             *user_handle
)
{
  /* Supress compiler warning for unused variables */
  (void) dev_str;
  (void) service_obj;
  (void) instance_id;
  (void) ind_cb;
  (void) ind_cb_data;
  (void) os_params;
  (void) timeout;
  (void) user_handle;

  QMI_ERR_MSG_0("QoS client helper is not supported");
  return QMI_INTERNAL_ERR;
}


