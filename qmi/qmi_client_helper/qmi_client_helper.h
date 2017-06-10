#ifndef QMI_CLIENT_HELPER_H
#define QMI_CLIENT_HELPER_H
/*===========================================================================
  @file
  qmi_client_helper.h

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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "qmi_client.h"
#include "qmi.h"
#include "qmi_client_instance_defs.h"

/*===========================================================================

                              FUNCTION DEFINITIONS

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif

#endif /* QMI_CLIENT_HELPER_H */

