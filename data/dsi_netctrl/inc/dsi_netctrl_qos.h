/**
  @file
  dsi_netctrl_qos.h

  @brief
  This file provides an API to manage Quality of Service elements.

*/

/*===========================================================================

  Copyright (c) 2011, 2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/23/13   leo     (Tech Pubs) Edited/added Doxygen comments and markup.
04/14/11   ar      Initial version

===========================================================================*/

#ifndef DSI_NETCTRL_QOS_H
#define DSI_NETCTRL_QOS_H

#include <linux/socket.h> /* sockaddr_storage */
#include "qmi_qos_srvc.h"

/*===========================================================================
                           DECLARATIONS
===========================================================================*/

typedef qmi_qos_req_opcode_type         dsi_qos_req_opcode_type;
typedef unsigned long                   dsi_qos_id_type;
typedef qmi_qos_err_rsp_type            dsi_qos_err_rsp_type;
typedef qmi_qos_granted_info_rsp_type   dsi_qos_granted_info_type;
typedef qmi_qos_granted_flow_data_type  dsi_qos_granted_flow_type;
typedef qmi_qos_status_info             dsi_qos_status_type;
typedef qmi_qos_reason_code             dsi_qmi_qos_reason_code_type;

typedef qmi_qos_spec_type               dsi_qos_spec_type;
typedef qmi_qos_granted_filter_data_type dsi_qos_granted_filter_data_type;
typedef qmi_qos_flow_req_type           dsi_qos_flow_req_type;
typedef qmi_qos_filter_req_type         dsi_qos_filter_req_type;
typedef qmi_qos_umts_traffic_class_type dsi_qos_umts_traffic_class_type;
typedef qmi_qos_tos_filter_type         dsi_qos_tos_filter_type;
typedef qmi_qos_ipv4_addr_filter_type   dsi_qos_ipv4_addr_filter_type;
typedef qmi_qos_ipv6_addr_filter_type   dsi_qos_ipv6_addr_filter_type;

/** @addtogroup datatypes
@{ */

/** Event status types associated with QoS operations. These are
 *  included along with DSI_EVT_QOS_STATUS_IND. */
typedef enum
{
  DSI_QOS_ACTIVATED_EV          = 0x01,  /**< QoS flow is activated. */
  DSI_QOS_SUSPENDED_EV          = 0x02,  /**< QoS flow is suspended. */
  DSI_QOS_GONE_EV               = 0x03,  /**< QoS flow is released. */
  DSI_QOS_MODIFY_ACCEPTED_EV    = 0x04,  /**< QoS flow modify operation is successful. */
  DSI_QOS_MODIFY_REJECTED_EV    = 0x05,  /**< QoS flow modify operation is rejected. */
  DSI_QOS_INFO_CODE_UPDATED_EV  = 0x06,  /**< New information code is available
                                              regarding the QoS status. */
  DSI_QOS_FLOW_ENABLED_EV       = 0x10,  /**< QoS data path flow is enabled. */
  DSI_QOS_FLOW_DISABLED_EV      = 0x11   /**< QoS data path flow is disabled. */
} dsi_qos_status_event_type;

/** @} */ /* end_addtogroup datatypes */

typedef enum
{
  DSI_QOS_FLOW_TYPE_INVALID  = -1,
  DSI_QOS_FLOW_TYPE_UE_INIT  =  0,
  DSI_QOS_FLOW_TYPE_NW_INIT  =  1
} dsi_qos_flow_type;


#define DSI_QOS_REQUEST    (QMI_QOS_REQUEST)
#define DSI_QOS_CONFIGURE  (QMI_QOS_CONFIGURE)

#define DSI_QOS_MAX_FLOW_FILTERS  (QMI_QOS_MAX_FLOW_FILTER)
#define DSI_QOS_MAX_PROFILES      (QMI_QOS_MAX_PROFILES)


/*===========================================================================
                    EXTERNAL FUNCTION PROTOTYPES
===========================================================================*/

/*===========================================================================
  FUNCTION:  dsi_request_qos
===========================================================================*/
/** @ingroup dsi_request_qos

    Requests a new QoS flow and filter on a pre-existing network connection.

    Multiple QoS specifications may be specified per invocation; however, the
    number of specifications supported is network service dependent. Each QoS
    specification contains one or more flow/filter specifications.

    For network service supporting multiple flow/filter specifications (e.g.,
    3GPP2), only one flow/filter specification in a set is granted via
    negotiation with the network.

    The return code indicates a successful start of the transaction by the
    modem, while the final outcome is determined via asynchronous event
    indications per Section (?*? fill in later). Subsequent QoS status event indications
    are automatically generated for each flow created on the modem

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] num_qos_specs Number of QoS specifications passed in the
                             qos_spec_list parameter (limited to a maximum
                             of 10). This also dictates the size of
                             qos_id_list and qos_spec_err_list parameters,
                             for which the client must allocate storage.
    @param[in] qos_spec_list Array of QoS specifications. One or more
                             instances of this type may be present.
    @param[in] req_opcode Request operation code. Indicates a request or
                          configure operation.
    @param[out] qos_id_list List of opaque handles for each QoS flow created.
    @param[out] qos_spec_err_list Array of QoS specification errors returned
                                  from the modem.

    @return
    DSI_SUCCESS -- The request was sent successfully. \n
    DSI_ERROR -- The request was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl(). \n
    There must be an active data call for the handle.
*/
/*=========================================================================*/
extern int dsi_request_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_specs,
  dsi_qos_spec_type      *qos_spec_list,
  dsi_qos_req_opcode_type req_opcode,
  dsi_qos_id_type        *qos_id_list,
  dsi_qos_err_rsp_type   *qos_spec_err_list
);


/*===========================================================================
  FUNCTION:  dsi_release_qos
===========================================================================*/
/** @ingroup dsi_release_qos

    Deletes existing QoS flow and filters from a network connection.

    The return code indicates a successful start of the transaction by the
    modem, while the final outcome is determined via an asynchronous event
    indication per Section ?*?.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] num_qos_ids Number of QoS flow handles passed in the
                           qos_id_list parameter.
    @param[in] qos_id_list Array of QoS flow handles. One or more
                           instances of this type may be present.

    @return
    DSI_SUCCESS -- The release operation was successful. \n
    DSI_ERROR -- The release operation was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl(). \n
    QoS flows must be allocated using dsi_request_qos().
*/
/*=========================================================================*/
extern int dsi_release_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_ids,
  dsi_qos_id_type        *qos_id_list
);


/*===========================================================================
  FUNCTION:  dsi_modify_qos
===========================================================================*/
/** @ingroup dsi_modify_qos

    Changes existing QoS flows on a network connection.

    The return code indicates a successful start of the transaction by the
    modem, while the final outcome is determined via an asynchronous event
    indication per Section ?*?.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] num_qos_specs Number of QoS specifications passed in the
                             qos_spec_list parameter. This also dictates the
                             size of the qos_id_list and qos_spec_err_list
                             parameters, for which the client must allocate
                             storage.
    @param[in] qos_spec_list Array of QoS specifications. One or more
                             instances of this type may be present.
    @param[out] qos_spec_err_list Array of QoS specification errors returned
                                  from the modem.

    @return
    DSI_SUCCESS -- The modify operation was successful. \n
    DSI_ERROR -- The modify operation was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl(). \n
    QoS flows must be allocated using dsi_request_qos().
*/
/*=========================================================================*/
extern int dsi_modify_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_specs,
  dsi_qos_spec_type      *qos_spec_list,
  dsi_qos_err_rsp_type   *qos_spec_err_list
);


/*===========================================================================
  FUNCTION:  dsi_suspend_qos
===========================================================================*/
/** @ingroup dsi_suspend_qos

    Disables prioritized packet handling for existing QoS flows
    on a network connection. Further packet transmission on those QoS
    flows is treated as best-effort traffic.

    The return code indicates a successful start of the transaction by the
    modem, while the final outcome is determined via an asynchronous event
    indication per Section ?*?.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] num_qos_ids Number of QoS flow handles passed in the
                           qos_id_list parameter.
    @param[in] qos_id_list Array of QoS flow handles. One or more
                           instances of this type may be present.

    @return
    DSI_SUCCESS -- The suspend operation was successful. \n
    DSI_ERROR -- The suspend operation was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl(). \n
    QoS flows must be allocated using dsi_request_qos().
*/
/*=========================================================================*/
extern int dsi_suspend_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_ids,
  dsi_qos_id_type        *qos_id_list
);


/*===========================================================================
  FUNCTION:  dsi_resume_qos
===========================================================================*/
/** @ingroup dsi_resume_qos

    Enables prioritized packet handling for existing QoS flows on a network
    connection.

    The return code indicates a successful start of the transaction by the
    modem, while the final outcome is determined via an asynchronous event
    indication per Section ?*?.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] num_qos_ids Number of QoS flow handles passed in the
                           qos_id_list parameter.
    @param[in] qos_id_list Array of QoS flow handles. One or more
                           instances of this type may be present.

    @return
    DSI_SUCCESS -- The resume operation was successful. \n
    DSI_ERROR -- The resume operation was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl(). \n
    QoS flows must be allocated using dsi_request_qos(). \n
    Only a suspended QoS flow can be resumed.
*/
/*=========================================================================*/
extern int dsi_resume_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_ids,
  dsi_qos_id_type        *qos_id_list
);


/*===========================================================================
  FUNCTION:  dsi_get_granted_qos
===========================================================================*/
/** @ingroup dsi_get_granted_qos

    Queries the QoS information for an existing flow on a network
    connection.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] qos_id QoS flow handle.
    @param[in] ip_family specify ip family - AF_INET or AF_INET6
    @param[out] qos_info QoS information returned from the modem.

    @return
    DSI_SUCCESS -- The query was successful. \n
    DSI_ERROR -- The query operation was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl(). \n
    QoS flows must be allocated using dsi_request_qos().
*/
/*=========================================================================*/
extern int dsi_get_granted_qos
(
  dsi_hndl_t                 hndl,
  dsi_qos_id_type            qos_id,
  int                        ip_family,
  dsi_qos_granted_info_type *qos_info
);

/*===========================================================================
  FUNCTION:  dsi_get_qos_status
===========================================================================*/
/** @ingroup dsi_get_qos_status

    Queries the QoS activated/suspended/gone state for an
    existing flow on a network connection.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] qos_id QoS flow handle.
    @param[out] qos_status QoS flow status returned from the modem.

    @return
    DSI_SUCCESS -- The query was successful. \n
    DSI_ERROR -- The query operation was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl(). \n
    QoS flows must be allocated using dsi_request_qos().
*/
/*=========================================================================*/
extern int dsi_get_qos_status
(
  dsi_hndl_t           hndl,
  dsi_qos_id_type      qos_id,
  dsi_qos_status_type *qos_status
);


/*===========================================================================
  FUNCTION:  dsi_process_qos_ind
===========================================================================*/
/*!
    @brief
    callback function registered for qos indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
extern void dsi_process_qos_ind
(
  int qos_hndl,
  qmi_service_id_type sid,
  void * user_data,
  qmi_qos_indication_id_type ind_id,
  qmi_qos_indication_data_type * ind_data
);

#endif /* DSI_NETCTRL_QOS_H */
