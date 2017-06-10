/*!
  @file
  qcril_data_utils.h

  @brief
  Handles utility functions of RIL requests for DATA services.

*/

/*===========================================================================

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/17/11   sy     Initial version

===========================================================================*/

#ifndef QCRIL_DATA_UTILS_H
#define QCRIL_DATA_UTILS_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "qcril_data_defs.h"
#include "qcril_data.h"

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
#define CALL_ID_INVALID           (-1)
#define QCRIL_DATA_HNDL_INVALID   (-1)
#define QCRIL_DSI_HNDL_INVALID    (NULL)
#define QCRIL_INVALID_PROFILE_ID  (-1)

#define QCRIL_DATA_INVALID_TIMERID      (0)

#define QCRIL_DATA_INVALID_INSTANCE_ID  (QCRIL_MAX_INSTANCE_ID)
#define QCRIL_DATA_INVALID_MODEM_ID     (0xFFFFFFFF)

/* Error codes */
#define SUCCESS  (0)
#define FAILURE  (-1)

#ifdef FEATURE_DATA_EMBMS

#define QCRIL_DATA_EMBMS_ERROR_NONE                    0
#define QCRIL_DATA_EMBMS_ERROR_UNKNOWN                 1
#define QCRIL_DATA_EMBMS_ERROR_ALREADY_DONE            2
#define QCRIL_DATA_EMBMS_ERROR_NOT_ALLOWED             3
#define QCRIL_DATA_EMBMS_ERROR_MISSING_CONTROL_INFO    4
#define QCRIL_DATA_EMBMS_ERROR_MISSING_TMGI            5
#define QCRIL_DATA_EMBMS_ERROR_MCAST_OOC               6
#define QCRIL_DATA_EMBMS_ERROR_UCAST_OOS               7
#define QCRIL_DATA_EMBMS_ERROR_FREQUENCY_CONFLICT      8
#define QCRIL_DATA_EMBMS_ERROR_MAX_TMGI_ALREADY_ACTIVE 9
#define QCRIL_DATA_EMBMS_SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_IDLE 100
#define QCRIL_DATA_EMBMS_SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_CONNECTED 101
#endif /* FEATUER_DATA_EMBMS */


#define QCRIL_DATA_IS_DATA_SYS_STATUS_RAT_MASK_3GPP2_1X_DO(nw,rat,so) ((nw == QMI_WDS_DATA_NETWORK_TYPE_3GPP2) &&                                            \
                                                                       ((rat == CDMA_1X) ||                                                                  \
                                                                        ((rat == CDMA_EVDO_REVA) && (so.so_mask_evdo_reva != CDMA_EVDO_REVA_EMPA_EHRPD)) ||  \
                                                                        ((rat == CDMA_EVDO_REVB) && ((so.so_mask_evdo_revb != CDMA_EVDO_REVB_EMPA_EHRPD)     \
                                                                                                    && (so.so_mask_evdo_revb != CDMA_EVDO_REVB_MMPA_EHRPD)))))

#define QCRIL_DATA_IS_DATA_SYS_STATUS_RAT_MASK_3GPP2_EHRPD(nw,rat,so) ((nw == QMI_WDS_DATA_NETWORK_TYPE_3GPP2) &&                                             \
                                                                       ((rat == CDMA_EHRPD) ||                                                                \
                                                                        ((rat == CDMA_EVDO_REVA) && (so.so_mask_evdo_reva == CDMA_EVDO_REVA_EMPA_EHRPD)) ||   \
                                                                        ((rat == CDMA_EVDO_REVB) && ((so.so_mask_evdo_revb == CDMA_EVDO_REVB_EMPA_EHRPD)      \
                                                                                                     || (so.so_mask_evdo_revb == CDMA_EVDO_REVB_MMPA_EHRPD)))))

/*===========================================================================

                         GLOBAL VARIABLES

===========================================================================*/

extern qcril_data_call_info_tbl_type info_tbl[ MAX_CONCURRENT_UMTS_DATA_CALLS ];

/* Table containing the state for handling dsi callbacks */
extern qcril_data_dsi_cb_tbl_type dsi_cb_tbl[ MAX_CONCURRENT_UMTS_DATA_CALLS ];

typedef struct qcril_data_ce_map_s
{
  int ril_data_ce_code;
  int qmi_data_ce_code;
  dsi_ce_reason_type_t reason_category;
} qcril_data_ce_map;

extern qcril_data_ce_map qcril_data_ce_map_tbl[];

#ifdef FEATURE_DATA_EMBMS
typedef struct qcril_data_embms_status_s
{
  int ril_embms_status_code;
  int qmi_embms_status_code;
}qcril_data_embms_status_map;

extern qcril_data_embms_status_map qcril_data_embms_status_map_tbl[];
#endif

/* this mutex protects dsi callback tbl */
extern pthread_mutex_t dsi_cb_tbl_mutex;

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

void qcril_data_cleanup_call_state
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
);

void qcril_data_update_dsi_cb_tbl_entry
(
  qcril_data_dsi_cb_tbl_type *dsi_cb_tbl_ptr,
  qcril_instance_id_e_type   instance_id,
  qcril_modem_id_e_type      modem_id,
  RIL_Token                  pend_tok,
  dsi_hndl_t                 dsi_hndl,
  qcril_data_call_info_tbl_type *info_tbl_ptr
);

void qcril_data_clear_dsi_cb_tbl_entry
(
  qcril_data_dsi_cb_tbl_type *dsi_cb_tbl_ptr
);

void qcril_data_util_update_call_state
(
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  int call_state,
  int status
);

void qcril_data_util_update_partial_retry_enabled_flag
(
   void
);

void qcril_data_util_update_max_partial_retry_timeout
(
  void
);

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
boolean qcril_data_util_get_dual_ip_call_status
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr,
  int                            *err_code
);
#endif

int qcril_data_get_ril_ce_code
(
  dsi_ce_reason_t * dsi_ce_reasion,
  int * ril_ce_code
);

#ifdef FEATURE_DATA_EMBMS
int qcril_data_utils_embms_copy_tmgi_list
(
  qmi_wds_embms_tmgi_type **tmgi_list_ptr_to,
  qmi_wds_embms_tmgi_type *tmgi_list_ptr_from,
  unsigned char            len
);
int qcril_data_utils_embms_get_ril_status_code
(
  int   qmi_status,
  int * ril_status
);

#endif /* FEATURE_DATA_EMBMS */

void qcril_data_util_stop_timer
(
  timer_t *timer_id
);

void qcril_data_util_disarm_timer
(
  timer_t  *timer_id
);

int qcril_data_get_ifindex
(
  const char * dev, int * ifindex
);

const char *
qcril_data_util_get_ril_tech_string
(
  qcril_data_ril_radio_tech_t  ril_tech
);

const char *
qcril_data_util_get_dsi_bearer_tech_string
(
  dsi_data_bearer_tech_t  dsi_bearer_tech
);

void qcril_data_get_active_call_list
(
  void                 **response,
  size_t                *response_len
);

int
qcril_data_util_is_partial_retry_allowed
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr
);

void qcril_data_util_schedule_partial_retry_attempt
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr,
  boolean                        check_teth_state,
  boolean                        check_ce_reason
);

void qcril_data_util_reinitiate_partial_retry
(
  boolean  check_teth_state,
  boolean  check_ce_reason
);

boolean qcril_data_util_data_sys_status_changed
(
  qmi_wds_data_sys_status_type  *curr_sys_status,
  qmi_wds_data_sys_status_type  *new_sys_status
);

boolean qcril_data_util_is_new_rat_1x_ehrpd
(
  qmi_wds_data_sys_status_type               *new_sys_status,
  qmi_wds_data_sys_status_network_info_type  **nw_info
);

void qcril_data_util_register_ext_ip_config_ind
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr
);

#endif /* QCRIL_DATA_UTILS_H */

