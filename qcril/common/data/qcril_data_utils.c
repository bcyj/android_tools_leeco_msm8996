/*!
  @file
  qcril_data_utils.c

  @brief
  Handles utility functions of RIL requests for DATA services.

*/

/*===========================================================================

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/17/11   sy     Initial version

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <netinet/in.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <cutils/properties.h>

#include "qcril_data.h"
#include "qcril_data_defs.h"
#include "qcril_data_utils.h"
#include "qcril_log.h"
#include <linux/if.h>

/* Retry timer values (in msec) */
#define RIL_RETRY_INVALID (-1)

#define QCRIL_DATA_UTIL_DEFAULT_PARTIAL_RETRY_TIMEOUT  (5)
#define QCRIL_DATA_UTIL_DEFAULT_MAX_PARTIAL_RETRY_TIMEOUT    (100)

#define QCRIL_DATA_PROPERTY_MAX_PARTIAL_RETRY_TIMEOUT  "persist.qcril.max_retry_timeout"

/* Property to disable/enable partial retry */
#define QCRIL_DATA_PROPERTY_DISABLE_PARTIAL_RETRY          "persist.qcril.disable_retry"
#define QCRIL_DATA_PROPERTY_DISABLE_PARTIAL_RETRY_DEFAULT  "false"
#define QCRIL_DATA_PROPERTY_DISABLE_PARTIAL_RETRY_SIZE     (6)

#define QCRIL_DATA_UTIL_GET_PARTIAL_RETRY_TIMEOUT(num_retry)    \
  (QCRIL_DATA_UTIL_DEFAULT_PARTIAL_RETRY_TIMEOUT << (num_retry))

typedef struct
{
  dsi_data_bearer_tech_t  dsi_bearer_tech;
  const char *tech_string;
}dsi_bearer_tech_strings_s;

/* Table for RIL error code translation from verbose QMI error code. */
/* Note: Only exception cases appear here. Certain DSI types are
 * passthough as RIL API values match QMI values. */
qcril_data_ce_map qcril_data_ce_map_tbl[] =
{
/*{RIL code,                           QMI code,                           DSI type}*/
  {PDP_FAIL_SIGNAL_LOST,                  QMI_WDS_VERBOSE_CE_NO_SRV,          DSI_CE_TYPE_CALL_MANAGER_DEFINED},
  {PDP_FAIL_SIGNAL_LOST,                  QMI_WDS_VERBOSE_CE_FADE,            DSI_CE_TYPE_CALL_MANAGER_DEFINED},
  {PDP_FAIL_PREF_RADIO_TECH_CHANGED,      QMI_WDS_VERBOSE_CE_UE_RAT_CHANGE,   DSI_CE_TYPE_CALL_MANAGER_DEFINED},
  {PDP_FAIL_PROTOCOL_ERRORS,              QMI_WDS_VERBOSE_CE_PREFIX_UNAVAILABLE,          DSI_CE_TYPE_IPV6},
  {PDP_FAIL_USER_AUTHENTICATION,          QMI_WDS_VERBOSE_CE_PPP_CHAP_FAILURE,          DSI_CE_TYPE_PPP},
  {PDP_FAIL_USER_AUTHENTICATION,          QMI_WDS_VERBOSE_CE_PPP_PAP_FAILURE,           DSI_CE_TYPE_PPP},
  {PDP_FAIL_ONLY_IPV4_ALLOWED,            QMI_WDS_VERBOSE_CE_ERR_PDN_IPV6_CALL_DISALLOWED, DSI_CE_TYPE_INTERNAL},
  {PDP_FAIL_ONLY_IPV6_ALLOWED,            QMI_WDS_VERBOSE_CE_ERR_PDN_IPV4_CALL_DISALLOWED, DSI_CE_TYPE_INTERNAL},
  {PDP_FAIL_MISSING_UKNOWN_APN,           QMI_WDS_VERBOSE_CE_APN_DISABLED,                 DSI_CE_TYPE_INTERNAL},
  {PDP_FAIL_ONLY_IPV4_ALLOWED,            QMI_WDS_VERBOSE_CE_IPV6_DISABLED,                DSI_CE_TYPE_IPV6},
  {PDP_FAIL_INSUFFICIENT_RESOURCES,       QMI_WDS_VERBOSE_CE_MAX_V4_CONNECTIONS,           DSI_CE_TYPE_INTERNAL},
  {PDP_FAIL_INSUFFICIENT_RESOURCES,       QMI_WDS_VERBOSE_CE_MAX_V6_CONNECTIONS,           DSI_CE_TYPE_INTERNAL},
  {PDP_FAIL_UNKNOWN_PDP_ADDRESS_TYPE,     QMI_WDS_VERBOSE_CE_IP_VERSION_MISMATCH,          DSI_CE_TYPE_INTERNAL},
  {PDP_FAIL_SERVICE_OPTION_NOT_SUPPORTED, QMI_WDS_VERBOSE_CE_DUN_CALL_DISALLOWED,          DSI_CE_TYPE_INTERNAL},
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
  {PDP_FAIL_TETHERED_CALL_ACTIVE,      QMI_WDS_VERBOSE_CE_APP_PREEMPTED,   DSI_CE_TYPE_INTERNAL}
#else
  {PDP_FAIL_TETHERED_CALL_ON,          QMI_WDS_VERBOSE_CE_APP_PREEMPTED,   DSI_CE_TYPE_INTERNAL}
#endif /* ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */
};
#ifdef FEATURE_DATA_EMBMS

qcril_data_embms_status_map qcril_data_embms_status_map_tbl[] =
{
  {QCRIL_DATA_EMBMS_ERROR_UNKNOWN              /*1*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_UNKNOWN},
  {QCRIL_DATA_EMBMS_ERROR_UNKNOWN              /*1*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_RADIO_CONFIG},
  {QCRIL_DATA_EMBMS_ERROR_UNKNOWN              /*1*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_CHANNEL_UNAVAILABLE},
  {QCRIL_DATA_EMBMS_ERROR_UNKNOWN              /*1*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_EMBMS_NOT_ENABLED},
  {QCRIL_DATA_EMBMS_ERROR_UNKNOWN              /*1*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_OUT_OF_COVERAGE},
  {QCRIL_DATA_EMBMS_ERROR_ALREADY_DONE         /*2*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS_DUP_ACTIVATE},
  {QCRIL_DATA_EMBMS_ERROR_NOT_ALLOWED          /*3*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_NOT_ALLOWED},
  {QCRIL_DATA_EMBMS_ERROR_MISSING_CONTROL_INFO /*4*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_MISSING_CTL_INFO},
  {QCRIL_DATA_EMBMS_ERROR_MISSING_TMGI         /*5*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_MISSING_TMGI},
  {QCRIL_DATA_EMBMS_ERROR_MCAST_OOC            /*6*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_MCAST_OOS},
  {QCRIL_DATA_EMBMS_ERROR_UCAST_OOS            /*7*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_UNICAST_OOS},
  {QCRIL_DATA_EMBMS_ERROR_FREQUENCY_CONFLICT   /*8*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_CAMP_ON_OTHER_FREQ},
  {QCRIL_DATA_EMBMS_ERROR_ALREADY_DONE         /*2*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_ACTIVATIOIN_IN_PROGRESS},
  {QCRIL_DATA_EMBMS_ERROR_UNKNOWN              /*1*/, QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_DEACTIVATION_IN_PROGRESS},
  {QCRIL_DATA_EMBMS_ERROR_UNKNOWN,                    QMI_WDS_EMBMS_TMGI_DEACTIVATE_FAIL_NOT_ACTIVATED_CTRL_POINT},
  {QCRIL_DATA_EMBMS_ERROR_ALREADY_DONE,               QMI_WDS_EMBMS_TMGI_DEACTIVATE_FAIL_NOT_ACTIVATED},
  {QCRIL_DATA_EMBMS_ERROR_UNKNOWN,                    QMI_WDS_EMBMS_TMGI_DEACTIVATE_FAIL_DEACTIVATION_IN_PROGRESS},
  {QCRIL_DATA_EMBMS_ERROR_UNKNOWN,                    QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_SAI_MISMATCH},
  {QCRIL_DATA_EMBMS_ERROR_MAX_TMGI_ALREADY_ACTIVE,    QMI_WDS_EMBMS_TMGI_ACTIVATION_FAIL_MAX_TMGI_ALREADY_ACTIVE},
  {QCRIL_DATA_EMBMS_SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_IDLE, QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS_IDLE_RADIO_TUNE},
  {QCRIL_DATA_EMBMS_SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_CONNECTED, QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS_CONN_RADIO_TUNE}
};
#endif


/* this mutex protects dsi callback tbl */
pthread_mutex_t dsi_cb_tbl_mutex;

extern void qcril_data_partial_retry_hdlr
(
  union sigval sval
);

static boolean qcril_data_is_partial_retry_disabled = FALSE;

static unsigned long qcril_data_max_partial_retry_timeout = QCRIL_DATA_UTIL_DEFAULT_MAX_PARTIAL_RETRY_TIMEOUT;

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/
/*===========================================================================

  FUNCTION:  qcril_data_util_is_ce_failure_permanent

===========================================================================*/
/*!
@brief
  Determine if the call end reason corresponds to a permanent failure (which
  implies that no retries are intended.)

@return
  TRUE  - if failure is permanent
  FALSE - Otherwise
*/
/*=========================================================================*/
static boolean
qcril_data_util_is_ce_failure_permanent
(
  int  ril_ce_reason
)
{
  boolean ret = FALSE;

  switch (ril_ce_reason)
  {
     case PDP_FAIL_SERVICE_OPTION_NOT_SUPPORTED:
     case PDP_FAIL_SERVICE_OPTION_NOT_SUBSCRIBED:
     case PDP_FAIL_NSAPI_IN_USE:
     case PDP_FAIL_ONLY_IPV4_ALLOWED:
     case PDP_FAIL_ONLY_IPV6_ALLOWED:
     case PDP_FAIL_PROTOCOL_ERRORS:
     case PDP_FAIL_MISSING_UKNOWN_APN:
     case PDP_FAIL_UNKNOWN_PDP_ADDRESS_TYPE:
       ret = TRUE;
       break;

     default:
       break;
  }

  return ret;
}


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_data_cleanup_call_info_tbl

===========================================================================*/
/*!
    @brief
    frees up info_tbl entry. Updates the call information in
    call_tbl before info_tbl entry is cleaned up.

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    None
*/
/*=========================================================================*/
void qcril_data_cleanup_call_state
(
  qcril_data_call_info_tbl_type *info_tbl_ptr
)
{
  int qmi_err_code;

  QCRIL_LOG_FUNC_ENTRY();

  if ( !VALIDATE_LOCAL_DATA_OBJ( info_tbl_ptr ) )
  {
    QCRIL_LOG_ERROR( "invalid info_tbl_ptr" );
    goto err_label;
  }

  /* Invalidate the corresponding dsi callback table entry */
  qcril_data_clear_dsi_cb_tbl_entry(&dsi_cb_tbl[info_tbl_ptr->index]);

  /* Cleanup the corresponding response timer */
  qcril_data_util_stop_timer(&info_tbl_ptr->timer_id);

  /* Cleanup the corresponding partial retry timer */
  qcril_data_util_stop_timer(&info_tbl_ptr->retry_timer_id);

  /* update the corresponding entry in the call tbl as well */
  qcril_data_util_update_call_state( info_tbl_ptr, CALL_INACTIVE, PDP_FAIL_NONE );

  QCRIL_LOG_DEBUG( "clean up local info tbl, index [%d], cid [%d]",
                   info_tbl_ptr->index, info_tbl_ptr->cid );

  if (QCRIL_DSI_HNDL_INVALID != info_tbl_ptr->dsi_hndl)
  {
    QCRIL_LOG_DEBUG("releasing dsi_hndl [0x%x]", info_tbl_ptr->dsi_hndl);

    dsi_rel_data_srvc_hndl(info_tbl_ptr->dsi_hndl);
  }

  if (QCRIL_DATA_HNDL_INVALID != info_tbl_ptr->qmi_wds_hndl)
  {
    QCRIL_LOG_DEBUG("releasing qmi_wds_hndl [0x%x]", info_tbl_ptr->qmi_wds_hndl);

    qmi_wds_srvc_release_client(info_tbl_ptr->qmi_wds_hndl,
                                &qmi_err_code);
  }

  /* call is down. Let go of the profiles associated with it. */
  if (info_tbl_ptr->qdp_umts_profile_id != QCRIL_INVALID_PROFILE_ID)
  {
    QCRIL_LOG_DEBUG("releasing QDP profile id [%d]",
                    info_tbl_ptr->qdp_umts_profile_id);
    qdp_profile_release(info_tbl_ptr->qdp_umts_profile_id);
  }
  if (info_tbl_ptr->qdp_cdma_profile_id != QCRIL_INVALID_PROFILE_ID)
  {
    QCRIL_LOG_DEBUG("releasing QDP profile id [%d]",
                    info_tbl_ptr->qdp_cdma_profile_id)
    qdp_profile_release(info_tbl_ptr->qdp_cdma_profile_id);
  }

  info_tbl_ptr->cid      = CALL_ID_INVALID;
  info_tbl_ptr->dsi_hndl = QCRIL_DSI_HNDL_INVALID;
  info_tbl_ptr->qmi_wds_hndl = QCRIL_DATA_HNDL_INVALID;
  info_tbl_ptr->pend_tok = NULL;
  info_tbl_ptr->pend_req = DS_RIL_REQ_INVALID;
  info_tbl_ptr->self     = NULL;
  info_tbl_ptr->info_flg = FALSE;
  info_tbl_ptr->is_partial_retry = FALSE;
  info_tbl_ptr->partial_retry_count = 0;
  info_tbl_ptr->qdp_umts_profile_id = QCRIL_INVALID_PROFILE_ID;
  info_tbl_ptr->qdp_cdma_profile_id = QCRIL_INVALID_PROFILE_ID;

  memset( &info_tbl_ptr->call_info, 0, sizeof info_tbl_ptr->call_info );

  if (NULL != info_tbl_ptr->call_params)
  {
    free(info_tbl_ptr->call_params);
    info_tbl_ptr->call_params = NULL;
  }

  QCRIL_LOG_DEBUG( "EXIT with succ" );
  return;

err_label:
  QCRIL_LOG_DEBUG( "exit with FAILURE" );
  return;
}/* qcril_data_cleanup_call_state() */

/*===========================================================================

  FUNCTION:  qcril_data_update_dsi_cb_tbl_entry

===========================================================================*/
/*!
    @brief
    Updates the requested dsi_cb_tbl[] entry with the given values

    @return
    None.

    @note
    This funcion should not call any DSI or QCRIL API functions
*/
/*=========================================================================*/
void qcril_data_update_dsi_cb_tbl_entry
(
  qcril_data_dsi_cb_tbl_type *dsi_cb_tbl_ptr,
  qcril_instance_id_e_type   instance_id,
  qcril_modem_id_e_type      modem_id,
  RIL_Token                  pend_tok,
  dsi_hndl_t                 dsi_hndl,
  qcril_data_call_info_tbl_type *info_tbl_ptr
)
{
  if (NULL == dsi_cb_tbl_ptr)
  {
    QCRIL_LOG_ERROR( "invalid dsi_cb_tbl_ptr" );
    return;
  }

  QCRIL_DATA_MUTEX_LOCK(&dsi_cb_tbl_mutex);

  QCRIL_LOG_DEBUG( "updating entry [%#x]", dsi_cb_tbl_ptr );

  /* Update the corresponding dsi callback table entry */
  dsi_cb_tbl_ptr->instance_id  = instance_id;
  dsi_cb_tbl_ptr->modem_id     = modem_id;
  dsi_cb_tbl_ptr->pend_tok     = pend_tok;
  QCRIL_LOG_INFO( "DEBUG:  %s step %d - dsi_cb_tbl_ptr->pend_tok[0x%x]",
                  __func__, 0, dsi_cb_tbl_ptr->pend_tok );
  dsi_cb_tbl_ptr->dsi_hndl     = dsi_hndl;
  dsi_cb_tbl_ptr->info_tbl_ptr = info_tbl_ptr;

  QCRIL_DATA_MUTEX_UNLOCK(&dsi_cb_tbl_mutex);
}

/*===========================================================================

  FUNCTION:  qcril_data_clear_dsi_cb_tbl_entry

===========================================================================*/
/*!
    @brief
    Clears the requested entry in dsi_cb_tbl[]

    @return
    None.

    @note
    This funcion should not call any DSI or QCRIL API functions
*/
/*=========================================================================*/
void qcril_data_clear_dsi_cb_tbl_entry
(
  qcril_data_dsi_cb_tbl_type *dsi_cb_tbl_ptr
)
{
  if (NULL == dsi_cb_tbl_ptr)
  {
    QCRIL_LOG_ERROR( "invalid dsi_cb_tbl_ptr" );
    return;
  }

  QCRIL_DATA_MUTEX_LOCK(&dsi_cb_tbl_mutex);

  QCRIL_LOG_DEBUG( "cleaning up entry [%#x]", dsi_cb_tbl_ptr );

  /* Invalidate the corresponding dsi callback table entry */
  dsi_cb_tbl_ptr->instance_id  = QCRIL_DATA_INVALID_INSTANCE_ID;
  dsi_cb_tbl_ptr->modem_id     = QCRIL_DATA_INVALID_MODEM_ID;
  dsi_cb_tbl_ptr->pend_tok     = NULL;
  dsi_cb_tbl_ptr->dsi_hndl     = QCRIL_DSI_HNDL_INVALID;
  dsi_cb_tbl_ptr->info_tbl_ptr = NULL;

  QCRIL_DATA_MUTEX_UNLOCK(&dsi_cb_tbl_mutex);
}


/*=========================================================================
  FUNCTION:  qcril_data_util_update_call_state

===========================================================================*/
/*!
    @brief
    Updates call state.

    @return
    NONE
*/
/*=========================================================================*/
void qcril_data_util_update_call_state
(
  qcril_data_call_info_tbl_type *info_tbl_ptr,
  int call_state,
  int status
)
{
  /* validate input parameter */
  if(!info_tbl_ptr )
  {
    QCRIL_LOG_ERROR("bad param(s) "
                    "info_tbl_ptr=0x%x", info_tbl_ptr);
    return;
  }

  QCRIL_DS_ASSERT( ( info_tbl_ptr->index < MAX_CONCURRENT_UMTS_DATA_CALLS ),
                   "validate info_tbl index value range" );

  /* lock call_tbl while updating */
  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);
  info_tbl_ptr->call_info.active = call_state;

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
  info_tbl_ptr->status = status;
#endif /* #if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */

  /* unlock call_tbl now */
  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
}/* qcril_data_util_update_call_state */

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
/*===========================================================================

  FUNCTION:  qcril_data_util_get_dual_ip_call_status

===========================================================================*/
/*!
    @brief
    Routine to obtain the call status of a Dual-IP call. This function must
    only be called for a Dual-IP call otherwise a PDP_FAIL_ERROR_UNSPECIFIED
    is returned.

    @pre Before calling, info_tbl_mutex must be locked by the calling thread

    @return
    Call status (PDP_FAIL_xxx).
*/
/*=========================================================================*/
boolean qcril_data_util_get_dual_ip_call_status
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr,
  int                            *err_code
)
{
  dsi_ce_reason_t ce_reason_v4 = {DSI_CE_TYPE_INVALID, QMI_WDS_VERBOSE_CE_INVALID};
  dsi_ce_reason_t ce_reason_v6 = {DSI_CE_TYPE_INVALID, QMI_WDS_VERBOSE_CE_INVALID};
  int             status_v4    = PDP_FAIL_ERROR_UNSPECIFIED;
  int             status_v6    = PDP_FAIL_ERROR_UNSPECIFIED;
  boolean         attempt_retry = FALSE;

  if (!info_tbl_ptr || !err_code)
  {
    QCRIL_LOG_ERROR("Invalid input param");
    goto bail;
  }

  *err_code = PDP_FAIL_ERROR_UNSPECIFIED;

  /* For a Dual-IP call, verify the status of both V4, V6 calls */
  if (DSI_IP_VERSION_4_6 != info_tbl_ptr->dsi_ip_version)
  {
    QCRIL_LOG_ERROR("not a dual-ip call");
    goto bail;
  }

  if (DSI_SUCCESS != dsi_get_call_end_reason(info_tbl_ptr->dsi_hndl,
                                             &ce_reason_v4,
                                             DSI_IP_FAMILY_V4))
  {
    goto bail;
  }

  if (DSI_SUCCESS != dsi_get_call_end_reason(info_tbl_ptr->dsi_hndl,
                                             &ce_reason_v6,
                                             DSI_IP_FAMILY_V6))
  {
    goto bail;
  }

  /* If either of the CE reasons hasn't been populated, don't retry.
     Initiate retry on the next DSI_EVT_NET_PARTIAL_CONN event */
  if (DSI_CE_TYPE_UNINIT == ce_reason_v4.reason_type ||
      DSI_CE_TYPE_UNINIT == ce_reason_v6.reason_type)
  {
    QCRIL_LOG_VERBOSE("Uninitialized CE type v4=%d, v6=%d, skipping retry",
                      ce_reason_v4.reason_type,
                      ce_reason_v6.reason_type);
    goto bail;
  }

  /* Get a more specific error code for V4 */
  if (DSI_CE_TYPE_INVALID != ce_reason_v4.reason_type)
  {
    QCRIL_LOG_DEBUG("dsi_get_call_end_reason for V4 returned [%d]",
                    ce_reason_v4.reason_code);
    if (SUCCESS != qcril_data_get_ril_ce_code(&ce_reason_v4, &status_v4))
    {
      QCRIL_LOG_ERROR("qcril_data_get_ril_ce_code failed for V4");
    }
  }

  /* Get a more specific error code for V6 */
  if (DSI_CE_TYPE_INVALID != ce_reason_v6.reason_type)
  {
    if (SUCCESS != qcril_data_get_ril_ce_code(&ce_reason_v6, &status_v6))
    {
      QCRIL_LOG_ERROR("qcril_data_get_ril_ce_code failed for V6");
    }
  }

  QCRIL_LOG_DEBUG( "RIL CE status_v4=%d, status_v6=%d",
                   status_v4,
                   status_v6 );

  /* Return a more specific status if available */
  if (PDP_FAIL_ERROR_UNSPECIFIED != status_v4)
  {
    *err_code = status_v4;
  }
  else
  {
    *err_code = status_v6;
  }

  attempt_retry = TRUE;

bail:
  if (NULL != err_code)
  {
    QCRIL_LOG_INFO( "set call end reason [%d]", *err_code );
  }
  return attempt_retry;
}/* qcril_data_util_get_dual_ip_call_status() */

#endif

/*=========================================================================
  FUNCTION:  qcril_data_get_ril_ce_code

===========================================================================*/
/*!
    @brief
    gets ril call end reason code given qmi call end reason code

    @return
    FAILURE - if bad params received
    SUCCESS - other wise (must return valid RIL cause code)
*/
/*=========================================================================*/
int qcril_data_get_ril_ce_code
(
  dsi_ce_reason_t * dsi_ce_reason,
  int * ril_ce_code
)
{
  unsigned int i=0;
  unsigned int max=sizeof(qcril_data_ce_map_tbl)/sizeof(qcril_data_ce_map);
  int ret = FAILURE;

  do
  {
    /* the only real error condition for this function */
    if (NULL == dsi_ce_reason ||
        NULL == ril_ce_code )
    {
      QCRIL_LOG_ERROR("**programming err** NULL param rcvd");
      break;
    }

    /* we must return some cause code. If we can't find any,
     * we return UNSPECIFIED, but this function must return success
     * from this point onwards */
    ret = SUCCESS;

    /* check to see if it matches with cause codes as defined in
     * qcril_data_ce_map_tbl */
    for(i=0; i<max; i++)
    {
      if (qcril_data_ce_map_tbl[i].qmi_data_ce_code == dsi_ce_reason->reason_code &&
          qcril_data_ce_map_tbl[i].reason_category == dsi_ce_reason->reason_type
        )
      {
        *ril_ce_code = qcril_data_ce_map_tbl[i].ril_data_ce_code;
        QCRIL_LOG_DEBUG("found matching cause code [%d]",
                        *ril_ce_code);
        break;
      }
    }
    /* break out if we found matching cause code */
    if (i < max)
    {
      break;
    }

    /* if no match found in qcril_data_ce_map_tbl, handle passthrough
     * and unsupported cases */
    switch( dsi_ce_reason->reason_type )
    {
      case DSI_CE_TYPE_3GPP_SPEC_DEFINED:
        *ril_ce_code = dsi_ce_reason->reason_code;
        QCRIL_LOG_DEBUG("returning 3gpp spec defined cause code [%d]",
                        *ril_ce_code);
        break;

      case DSI_CE_TYPE_INTERNAL:
        QCRIL_LOG_DEBUG("unsupported Internal call end reason rcvd [%d]",
                        dsi_ce_reason->reason_code);
        *ril_ce_code = PDP_FAIL_ERROR_UNSPECIFIED;
        break;

      case DSI_CE_TYPE_MOBILE_IP:
        QCRIL_LOG_DEBUG("unsupported MobileIP call end reason rcvd [%d]",
                        dsi_ce_reason->reason_code);
        *ril_ce_code = PDP_FAIL_ERROR_UNSPECIFIED;
        break;

      case DSI_CE_TYPE_CALL_MANAGER_DEFINED:
        QCRIL_LOG_DEBUG("unsupported CallManager call end reason rcvd [%d]",
                        dsi_ce_reason->reason_code);
        *ril_ce_code = PDP_FAIL_ERROR_UNSPECIFIED;
        break;

      case DSI_CE_TYPE_PPP:
        QCRIL_LOG_DEBUG("%s","unsupported PPP call end reason rcvd [%d]",
                        dsi_ce_reason->reason_code);
        *ril_ce_code = PDP_FAIL_ERROR_UNSPECIFIED;
        break;

      case DSI_CE_TYPE_EHRPD:
        QCRIL_LOG_DEBUG("%s","unsupported EHRPD call end reason rcvd [%d]",
                        dsi_ce_reason->reason_code);
        *ril_ce_code = PDP_FAIL_ERROR_UNSPECIFIED;
        break;

      default:
        /* if we reach here, return UNSPECIFIED cause code */
        *ril_ce_code = PDP_FAIL_ERROR_UNSPECIFIED;
        QCRIL_LOG_DEBUG("unspecified/invalid call end reason rcvd");
        break;
    }
  } while (0);

  return ret;
}
#ifdef FEATURE_DATA_EMBMS

/*=========================================================================
  FUNCTION:  qcril_data_utils_embms_copy_tmgi_list

===========================================================================*/
int qcril_data_utils_embms_copy_tmgi_list
(
  qmi_wds_embms_tmgi_type **tmgi_list_ptr_to,
  qmi_wds_embms_tmgi_type *tmgi_list_ptr_from,
  unsigned char            len
)
{
  int count = 0;
  int ret = FAILURE;

  QCRIL_LOG_DEBUG("%s", ">>>qcril_data_utils_embms_copy_tmgi_list: ENTRY");

  do
  {
    /* validate input */
    if(NULL == tmgi_list_ptr_from ||
       NULL == tmgi_list_ptr_to)
    {
      QCRIL_LOG_ERROR("%s", "rcvd invalid input data");
      break;
    }

    /* allocate memory */
    *tmgi_list_ptr_to = (qmi_wds_embms_tmgi_type *)
                       malloc(len * sizeof(qmi_wds_embms_tmgi_type));
    if(NULL != (*tmgi_list_ptr_to))
    {
      /* zero out memory */
      memset(*tmgi_list_ptr_to,
             0,
             len * sizeof(qmi_wds_embms_tmgi_type));
      for (count = 0; count < len; count++)
      {
        memcpy((*tmgi_list_ptr_to)[count].tmgi,
               tmgi_list_ptr_from[count].tmgi,
               QMI_WDS_EMBMS_TMGI_SIZE);
        (*tmgi_list_ptr_to)[count].session_id =
                         tmgi_list_ptr_from[count].session_id;

        (*tmgi_list_ptr_to)[count].session_id_valid =
                         tmgi_list_ptr_from[count].session_id_valid;

      }
    }
    else
    {
      QCRIL_LOG_ERROR("%s", "can not allocate memory!");
      break;
    }
    ret = SUCCESS;
  }while(0);

  if (SUCCESS == ret)
  {
    QCRIL_LOG_VERBOSE("%s", ">>>qcril_data_utils_embms_copy_tmgi_list: EXIT with suc");
  }
  else
  {
    QCRIL_LOG_VERBOSE("%s", ">>>qcril_data_utils_embms_copy_tmgi_list: EXIT with err");
  }
  return ret;
}
/*=========================================================================
  FUNCTION:  qcril_data_utils_embms_get_ril_status_code

===========================================================================*/
/*!
    @brief
    gets ril activate or deactivate status code given qmi status code

    @return
    FAILURE - if bad params received
    SUCCESS - other wise (must return valid RIL cause code)
*/
/*=========================================================================*/
int qcril_data_utils_embms_get_ril_status_code
(
  int   qmi_status,
  int * ril_status
)
{
  unsigned int i=0;
  unsigned int max=sizeof(qcril_data_embms_status_map_tbl)/sizeof(qcril_data_embms_status_map);
  int ret = FAILURE;

  do
  {
    /* the only real error condition for this function */
    if (NULL == ril_status )
    {
      QCRIL_LOG_ERROR("**programming err** NULL param rcvd");
      break;
    }

    /* we must return some cause code. If we can't find any,
     * we return UNSPECIFIED, but this function must return success
     * from this point onwards */
    ret = SUCCESS;

    /* check to see if it matches with cause codes as defined in
     * qcril_data_embms_status_map_tbl */
    for(i=0; i<max; i++)
    {
      if (qcril_data_embms_status_map_tbl[i].qmi_embms_status_code == qmi_status )
      {
        *ril_status = qcril_data_embms_status_map_tbl[i].ril_embms_status_code;
        QCRIL_LOG_DEBUG("found matching cause code [%d]",
                        *ril_status);
        break;
      }
    }
    /* break out if we found matching cause code */
    if (i < max)
    {
      break;
    }
    /* for others not mapped n qcril_data_embms_status_map_tbl, assign
       generic ERROR_UNKNOWN */
    *ril_status = QCRIL_DATA_EMBMS_ERROR_UNKNOWN;
  } while (0);

  return ret;
}

#endif /* FEATURE_DATA_EMBMS */
/*===========================================================================

  FUNCTION:  qcril_data_util_stop_timer

===========================================================================*/
/*!
    @brief
    Routine to stop and delete the given timer

    @pre Before calling, info_tbl_mutex must be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_util_stop_timer
(
  timer_t  *timer_id
)
{
  QCRIL_LOG_FUNC_ENTRY();

  if (!timer_id)
  {
    QCRIL_LOG_ERROR( "invalid input" );
    return;
  }
  else if (QCRIL_DATA_INVALID_TIMERID == *timer_id)
  {
    QCRIL_LOG_DEBUG( "invalid timer_id" );
    return;
  }

  QCRIL_LOG_VERBOSE( "deleting timer [%#x]", *timer_id );

  /* Delete the timer */
  timer_delete(*timer_id);
  *timer_id = QCRIL_DATA_INVALID_TIMERID;

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_data_util_disarm_timer

===========================================================================*/
/*!
    @brief
    Routine to disarm (stop) the given timer

    @pre Before calling, info_tbl_mutex must be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_util_disarm_timer
(
  timer_t  *timer_id
)
{
  struct itimerspec itimers;

  QCRIL_LOG_FUNC_ENTRY();

  if (!timer_id)
  {
    QCRIL_LOG_ERROR( "invalid input" );
    return;
  }
  else if (QCRIL_DATA_INVALID_TIMERID == *timer_id)
  {
    QCRIL_LOG_DEBUG( "invalid timer_id" );
    return;
  }

  QCRIL_LOG_VERBOSE( "disarming timer [%#x]", *timer_id );

  /* Disarm the timer */
  memset(&itimers, 0, sizeof(itimers));

  /* Start the timer */
  if (-1 == timer_settime(*timer_id, 0, &itimers, NULL))
  {
    QCRIL_LOG_ERROR( "failed to disarm timer for timer_id [%#x]... ",
                     *timer_id);
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_data_get_ifindex

===========================================================================*/
/*!
    @brief
    Retrive ifindex from device name.

    @return
    SUCCESS: ifindex is retrieved successfully
    FAILURE: ifindx is not retrieved
*/
/*=========================================================================*/
int qcril_data_get_ifindex(const char * dev, int * ifindex)
{
  struct ifreq ifr;
  int fd;

  QCRIL_LOG_FUNC_ENTRY();

  if(NULL == ifindex || NULL == dev)
  {
    QCRIL_LOG_ERROR("invalid input");
    return FAILURE;
  }

  *ifindex = -1;

  /* Open a temporary socket of datagram type to use for issuing the ioctl */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    QCRIL_LOG_ERROR("Open datagram socket failed");
    return FAILURE;
  }

  /* Set device name in the ioctl req struct */
  memset(&ifr, 0x0, sizeof(ifr));
  (void)strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));

  /* Issue ioctl on the device */
  if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    QCRIL_LOG_ERROR("Issue ioctl on the device failed, errno: %d", errno);
    close(fd);
    return FAILURE;
  }
  *ifindex = ifr.ifr_ifindex;

  /* Close temporary socket */
  close(fd);

  QCRIL_LOG_FUNC_RETURN();

  return SUCCESS;
}


/*===========================================================================

  FUNCTION:  qcril_data_get_ril_tech_string

===========================================================================*/
/*!
@brief
  Return the string corresponding to the given ril technology value

@return
  String representation of the given ril technology
*/
/*=========================================================================*/
const char *
qcril_data_util_get_ril_tech_string(qcril_data_ril_radio_tech_t  ril_tech)
{
  static const char *ril_tech_strings[] =
  {
    "RADIO_TECH_UNKNOWN",
    "RADIO_TECH_GPRS",
    "RADIO_TECH_EDGE",
    "RADIO_TECH_UMTS",
    "RADIO_TECH_IS95A",
    "RADIO_TECH_IS95B",
    "RADIO_TECH_1xRTT",
    "RADIO_TECH_EVDO_0",
    "RADIO_TECH_EVDO_A",
    "RADIO_TECH_HSDPA",
    "RADIO_TECH_HSUPA",
    "RADIO_TECH_HSPA",
    "RADIO_TECH_EVDO_B",
    "RADIO_TECH_EHRPD",
    "RADIO_TECH_LTE",
    "RADIO_TECH_HSPAP",
    "RADIO_TECH_GSM"
#if (RIL_QCOM_VERSION >= 3)
    ,"RADIO_TECH_TD_SCDMA"
#endif /* (RIL_QCOM_VERSION >= 3) */
  };

  const char *ril_string = ril_tech_strings[0];

  if (ril_tech < QCRIL_DATA_RIL_RADIO_TECH_UNKNOWN || ril_tech >= QCRIL_DATA_RIL_RADIO_TECH_LAST)
  {
    QCRIL_LOG_ERROR("invalid input");
    goto bail;
  }

  /* QCRIL Data RIL technology definitions are RIL_RadioTechnolog+2 */
  ril_string = ril_tech_strings[ril_tech-2];

bail:
  return ril_string;
}

/*===========================================================================

  FUNCTION:  qcril_data_get_dsi_bearer_tech_string

===========================================================================*/
/*!
@brief
  Return the string corresponding to the given dsi bearer technology value

@return
  String representation of the given dsi bearer technology
*/
/*=========================================================================*/
const char *
qcril_data_util_get_dsi_bearer_tech_string(dsi_data_bearer_tech_t  dsi_bearer_tech)
{
  static dsi_bearer_tech_strings_s dsi_bearer_tech_strings[] =
  {
    {DSI_DATA_BEARER_TECH_UNKNOWN,       "BEARER_TECH_UNKNOWN"},
    /* CDMA related data bearer technologies */
    {DSI_DATA_BEARER_TECH_CDMA_1X,       "BEARER_TECH_CDMA_1X"},
    {DSI_DATA_BEARER_TECH_EVDO_REV0,     "BEARER_TECH_EVDO_REV0"},
    {DSI_DATA_BEARER_TECH_EVDO_REVA,     "BEARER_TECH_EVDO_REVA"},
    {DSI_DATA_BEARER_TECH_EVDO_REVB,     "BEARER_TECH_EVDO_REVB"},
    {DSI_DATA_BEARER_TECH_EHRPD,         "BEARER_TECH_EHRPD"},
    {DSI_DATA_BEARER_TECH_FMC,           "BEARER_TECH_FMC"},
    {DSI_DATA_BEARER_TECH_HRPD,          "BEARER_TECH_HRPD"},
    {DSI_DATA_BEARER_TECH_3GPP2_WLAN,    "BEARER_TECH_3GPP2_WLAN"},
    /* UMTS related data bearer technologies */
    {DSI_DATA_BEARER_TECH_WCDMA,         "BEARER_TECH_WCDMA"},
    {DSI_DATA_BEARER_TECH_GPRS,          "BEARER_TECH_GPRS"},
    {DSI_DATA_BEARER_TECH_HSDPA,         "BEARER_TECH_HSDPA"},
    {DSI_DATA_BEARER_TECH_HSUPA,         "BEARER_TECH_HSUPA"},
    {DSI_DATA_BEARER_TECH_EDGE,          "BEARER_TECH_EDGE"},
    {DSI_DATA_BEARER_TECH_LTE,           "BEARER_TECH_LTE"},
    {DSI_DATA_BEARER_TECH_HSDPA_PLUS,    "BEARER_TECH_HSDPA_PLUS"},
    {DSI_DATA_BEARER_TECH_DC_HSDPA_PLUS, "BEARER_TECH_DC_HSDPA_PLUS"},
    {DSI_DATA_BEARER_TECH_64_QAM,        "BEARER_TECH_64_QAM"},
    {DSI_DATA_BEARER_TECH_TDSCDMA,       "BEARER_TECH_TDSCDMA"},
    {DSI_DATA_BEARER_TECH_GSM,           "BEARER_TECH_GSM"},
    {DSI_DATA_BEARER_TECH_3GPP_WLAN,     "BEARER_TECH_3GPP_WLAN"},
    {DSI_DATA_BEARER_TECH_MAX,           "BEARER_TECH_UNKNOWN"}
  };

  const char *dsi_bearer_string = dsi_bearer_tech_strings[0].tech_string;
  int i = 0;

  if (dsi_bearer_tech >= DSI_DATA_BEARER_TECH_MAX)
  {
    QCRIL_LOG_ERROR("invalid input");
    goto bail;
  }

  while (dsi_bearer_tech_strings[i].dsi_bearer_tech < DSI_DATA_BEARER_TECH_MAX)
  {
    if (dsi_bearer_tech == dsi_bearer_tech_strings[i].dsi_bearer_tech)
    {
      dsi_bearer_string = dsi_bearer_tech_strings[i].tech_string;
    }
    i++;
  }

bail:
  return dsi_bearer_string;
}

/*===========================================================================

  FUNCTION:  qcril_data_get_active_call_list

===========================================================================*/
/*!
    @brief
    Retrive a list of active calls.

    @return
    None
*/
/*=========================================================================*/
void qcril_data_get_active_call_list
(
  void                 **response,
  size_t                *response_len
)
{
  /* validate input parameter */
  if(!response || !response_len)
  {
    QCRIL_LOG_ERROR("bad param(s) "
                    "response=0x%x, response_len=0x%x",
                    response, response_len);
    return;
  }
  size_t active_call_tbl_size = 0;
  size_t call_tbl_entry_size = 0;
  int i = 0;
  int index = 0;

  int ret = SUCCESS;

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))

#if (RIL_VERSION >= 11)

  RIL_Data_Call_Response_v11 *active_call_table = NULL;

#elif (RIL_VERSION >= 10)

  RIL_Data_Call_Response_v9 *active_call_table = NULL;

#else

  RIL_Data_Call_Response_v6 *active_call_table = NULL;

#endif /* RIL_VERSION >= 11 */


#else
  RIL_Data_Call_Response    *active_call_table = NULL;
#endif /* RIL_QCOM_VERSION >= 1 || RIL_VERSION >= 6 */

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))

#if (RIL_VERSION >= 11)

  call_tbl_entry_size = sizeof(RIL_Data_Call_Response_v11);

#elif (RIL_VERSION >= 10)

  call_tbl_entry_size = sizeof(RIL_Data_Call_Response_v9);

#else

  call_tbl_entry_size = sizeof(RIL_Data_Call_Response_v6);

#endif /* (RIL_VERSION >= 11)  */


#else
  call_tbl_entry_size = sizeof(RIL_Data_Call_Response);
#endif /* ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  /* Loop over call table record number of active call */
  for (i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++)
  {
    if(CALL_ID_INVALID != info_tbl[i].cid &&
       NULL == info_tbl[i].pend_tok)
    {
      active_call_tbl_size ++;
    }
  }

  do
  {
    /* no need to check <0 case since active_call_tbl_size is size_t*/
    if (0 == active_call_tbl_size)
    {
      ret = FAILURE;
      /* no active data call, no need to process further */
      QCRIL_LOG_VERBOSE("No active call");
      break;
    }

    /* allocate memory for active data call */
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))

#if (RIL_VERSION >= 11)

    active_call_table = (RIL_Data_Call_Response_v11 *)qcril_malloc(
                         active_call_tbl_size * call_tbl_entry_size);

#elif (RIL_VERSION >= 10)

    active_call_table = (RIL_Data_Call_Response_v9 *)qcril_malloc(
                         active_call_tbl_size * call_tbl_entry_size);


#else

    active_call_table = (RIL_Data_Call_Response_v6 *)qcril_malloc(
                         active_call_tbl_size * call_tbl_entry_size);


#endif /* (RIL_VERSION >= 11)  */


#else
    active_call_table = (RIL_Data_Call_Response *)qcril_malloc(
                         active_call_tbl_size * call_tbl_entry_size);
#endif /* ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)) */

    if( NULL == active_call_table)
    {
      ret = FAILURE;
      QCRIL_LOG_ERROR(" Can not allocate memory");
      break;
    }

    /* loop over call table, copy active call entry */
    for(i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++)
    {
      if(CALL_ID_INVALID != info_tbl[i].cid &&
         NULL == info_tbl[i].pend_tok)
      {
        active_call_table[index].active = info_tbl[i].call_info.active;
        active_call_table[index].type = info_tbl[i].call_info.type;
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
        active_call_table[index].addresses = info_tbl[i].call_info.address.fmtstr;
        active_call_table[index].cid = info_tbl[i].cid;
        active_call_table[index].dnses = info_tbl[i].call_info.dns.fmtstr;
        active_call_table[index].gateways = info_tbl[i].call_info.gateway.fmtstr;
        active_call_table[index].ifname = info_tbl[i].call_info.dev_name;
        active_call_table[index].status = info_tbl[i].status;
        active_call_table[index].suggestedRetryTime = info_tbl[i].suggestedRetryTime;

#if (RIL_VERSION >= 10)

        active_call_table[index].pcscf = info_tbl[i].call_info.pcscf.fmtstr;

#endif

#if (RIL_VERSION >= 11)

        active_call_table[index].mtu = info_tbl[i].call_info.mtu;
#endif

#else
        active_call_table[index].apn = info_tbl[i].call_info.apn;
        active_call_table[index].addresses = info_tbl[i].call_info.address;
        active_call_table[index].radioTech = info_tbl[i].call_info.radioTech;
        active_call_table[index].inactiveReason = info_tbl[i].call_info.inactiveReason;
#endif /* (RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)*/

        index++;
      }
    }
  } while(0);

  QCRIL_DATA_MUTEX_UNLOCK( &info_tbl_mutex );

  *response = (void *)active_call_table;
  *response_len = active_call_tbl_size * call_tbl_entry_size;

  QCRIL_LOG_DEBUG("active call table size:[%d] "
                  "start from: [0x%x]\n", active_call_tbl_size, active_call_table);
  if(FAILURE == ret)
  {
    QCRIL_LOG_DEBUG( "EXIT with failure" );
  }
  else
  {
    QCRIL_LOG_DEBUG( "EXIT with succ" );
  }
  return;
}/* qcril_data_get_active_call_list */


/*===========================================================================

  FUNCTION:  qcril_data_util_is_partial_retry_allowed

===========================================================================*/
/*!
@brief
  Determines if a partial retry for a Dual-IP call is allowed or not based
  on the current bearer and the corresponding profile configuration on the
  modem

@return
  TRUE  - if partial retry is allowed
  FALSE - Otherwise
*/
/*=========================================================================*/
int
qcril_data_util_is_partial_retry_allowed
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr
)
{
  qdp_profile_pdn_type    pdn_type = QDP_PROFILE_PDN_TYPE_INVALID;
  int                     ret = TRUE;
  dsi_data_bearer_tech_t  bearer_tech = DSI_DATA_BEARER_TECH_UNKNOWN;


  if (TRUE == qcril_data_is_partial_retry_disabled)
  {
    return FALSE;
  }
  else if (!info_tbl_ptr)
  {
    QCRIL_LOG_ERROR("Invalid input");
    return FALSE;
  }
  else if (DSI_IP_VERSION_4_6 != info_tbl_ptr->dsi_ip_version)
  {
    QCRIL_LOG_ERROR("Not a Dual-IP call cid [%d]",
                    info_tbl_ptr->cid);
    return FALSE;
  }
  else if (DSI_NUM_IP_FAMILIES == info_tbl_ptr->last_addr_count)
  {
    QCRIL_LOG_ERROR("Both address families are already UP cid [%d]",
                    info_tbl_ptr->cid);
    return FALSE;
  }
  else if (TRUE == info_tbl_ptr->is_partial_retry)
  {
    QCRIL_LOG_ERROR("Partial retry already in progress cid [%d]",
                    info_tbl_ptr->cid);
    return FALSE;
  }

  bearer_tech = dsi_get_current_data_bearer_tech(info_tbl_ptr->dsi_hndl);

  if (QCRIL_DATA_IS_DSI_DATA_BEARER_TECH_3GPP(bearer_tech))
  {
    pdn_type = info_tbl_ptr->qdp_3gpp_profile_pdn_type;
  }
  else if (QCRIL_DATA_IS_DSI_DATA_BEARER_TECH_3GPP2(bearer_tech))
  {
    pdn_type = info_tbl_ptr->qdp_3gpp2_profile_pdn_type;
  }

  if (QDP_PROFILE_PDN_TYPE_IPV4 == pdn_type ||
      QDP_PROFILE_PDN_TYPE_IPV6 == pdn_type)
  {
    ret = FALSE;
  }

  QCRIL_LOG_DEBUG("bearer_tech: [%s], Profile PDN type: 3gpp [%d] 3gpp2 [%d], ret: [%s]",
                  qcril_data_util_get_dsi_bearer_tech_string(bearer_tech),
                  info_tbl_ptr->qdp_3gpp_profile_pdn_type,
                  info_tbl_ptr->qdp_3gpp2_profile_pdn_type,
                  ret?"TRUE":"FALSE");

  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_util_schedule_partial_retry_attempt

===========================================================================*/
/*!
    @brief
    Create and start a partial retry timer for Dual-IP calls

    @return
    None.

    @note
    info_tbl_mutex must be locked by the caller
*/
/*=========================================================================*/
void qcril_data_util_schedule_partial_retry_attempt
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr,
  boolean                        check_teth_state,
  boolean                        check_ce_reason
)
{
  struct sigevent sigev;
  struct itimerspec itimers;
  unsigned long timeout = 0;
  int  ril_ce_reason;


  QCRIL_LOG_FUNC_ENTRY();

  if (!VALIDATE_LOCAL_DATA_OBJ(info_tbl_ptr))
  {
    QCRIL_LOG_ERROR( "invalid info_tbl_ptr:[%p] ",
                     (unsigned int *)info_tbl_ptr);
    goto bail;
  }
  /* Ensure that the modem profile configuration allows a retry */
  else if (FALSE == qcril_data_util_is_partial_retry_allowed(info_tbl_ptr))
  {
    goto bail;
  }

  /* Check if a retry shouldn't be attempted due to special network errors */
  if (FALSE == qcril_data_util_get_dual_ip_call_status(info_tbl_ptr, &ril_ce_reason))
  {
    QCRIL_LOG_VERBOSE( "Skipping partial retry due to invalid CE reason");
    goto bail;
  }

  if (TRUE == check_ce_reason &&
      TRUE == qcril_data_util_is_ce_failure_permanent(ril_ce_reason))
  {
    QCRIL_LOG_VERBOSE( "Partial retry not allowed due to permanent fail reason=%d",
                       ril_ce_reason );
    goto bail;
  }
  /* Check if a retry shouldn't be attempted due to tethered state */
  else if (TRUE == check_teth_state &&
           PDP_FAIL_TETHERED_CALL_ACTIVE == ril_ce_reason)
  {
    QCRIL_LOG_VERBOSE( "Partial retry not allowed due to active tethered call reason=%d",
                       ril_ce_reason );
    goto bail;
  }
  else if (QCRIL_DATA_INVALID_TIMERID == info_tbl_ptr->retry_timer_id)
  {
    QCRIL_LOG_ERROR( "invalid retry_timer_id:[%#"PRIxPTR"], skipping retry",
                     (uintptr_t)info_tbl_ptr->retry_timer_id);
    goto bail;
  }

  /* If the setup timer_id is active, delete it */
  qcril_data_util_stop_timer(&info_tbl_ptr->timer_id);

  timeout = QCRIL_DATA_UTIL_GET_PARTIAL_RETRY_TIMEOUT(info_tbl_ptr->partial_retry_count);

  if (timeout > qcril_data_max_partial_retry_timeout)
  {
    timeout = qcril_data_max_partial_retry_timeout;
  }

  ++info_tbl_ptr->partial_retry_count;

  /* Set the timeout */
  itimers.it_value.tv_sec     = timeout;
  itimers.it_value.tv_nsec    = 0;
  itimers.it_interval.tv_sec  = 0;
  itimers.it_interval.tv_nsec = 0;

  /* Start the timer */
  if (-1 == timer_settime(info_tbl_ptr->retry_timer_id, 0, &itimers, NULL))
  {
    QCRIL_LOG_ERROR( "failed to start timer for timer_id [%#"PRIxPTR"], deleting... ",
                     (uintptr_t)info_tbl_ptr->retry_timer_id);

    qcril_data_util_stop_timer(&info_tbl_ptr->retry_timer_id);
  }
  else
  {
    QCRIL_LOG_VERBOSE( "call [%d] scheduling next partial retry [%lu] attempt after [%lu] seconds",
                       info_tbl_ptr->index,
                       info_tbl_ptr->partial_retry_count,
                       timeout );
  }

bail:
  QCRIL_LOG_FUNC_RETURN();
}


/*===========================================================================

  FUNCTION:  qcril_data_util_reinitiate_partial_retry

===========================================================================*/
/*!
    @brief
    Go through the info_tbl and check if we need to start partial retry
    attempts for any of the active calls

    @pre Before calling, info_tbl_mutex must be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_util_reinitiate_partial_retry
(
  boolean  check_teth_state,
  boolean  check_ce_reason
)
{
  int i;

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  for (i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; ++i)
  {
    if (!VALIDATE_LOCAL_DATA_OBJ(&info_tbl[i]) ||
        CALL_ID_INVALID == info_tbl[i].cid     ||
        CALL_INACTIVE == info_tbl[i].call_info.active)
    {
      continue;
    }

    /* If there's an active Dual-IP call but with fewer addresses*/
    if (DSI_IP_VERSION_4_6 == info_tbl[i].dsi_ip_version &&
        info_tbl[i].last_addr_count < DSI_NUM_IP_FAMILIES)
    {
      QCRIL_LOG_DEBUG( "scheduling retry for info_tbl index:[%d] ",
                       i);

      info_tbl[i].partial_retry_count = 0;
      qcril_data_util_schedule_partial_retry_attempt(&info_tbl[i],
                                                     check_teth_state,
                                                     check_ce_reason);
    }
  }

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
}


/*===========================================================================

  FUNCTION:  qcril_data_util_data_sys_status_changed

===========================================================================*/
/*!
    @brief
    Compares the current and new data system status information

    @return
    TRUE  - if the new_sys_status is different from cur_sys_status
    FALSE - otherwise
*/
/*=========================================================================*/
boolean qcril_data_util_data_sys_status_changed
(
  qmi_wds_data_sys_status_type  *cur_sys_status,
  qmi_wds_data_sys_status_type  *new_sys_status
)
{
  boolean ret = FALSE;
  int i;
  int network_len;
  qmi_wds_data_network_type  pref_nw;
  qmi_wds_data_sys_status_network_info_type  *cur_nw_info = NULL;
  qmi_wds_data_sys_status_network_info_type  *new_nw_info = NULL;

  /* Validate the input */
  if (!cur_sys_status                                                      ||
      !new_sys_status                                                      ||
      cur_sys_status->network_info_len != new_sys_status->network_info_len ||
      0 == new_sys_status->network_info_len                                ||
      !cur_sys_status->network_info                                        ||
      !new_sys_status->network_info)
  {
    QCRIL_LOG_ERROR("Invalid input");
    goto bail;
  }

  /* Verify that the new system status corresponds to a valid technology */
  network_len = new_sys_status->network_info_len;
  pref_nw = new_sys_status->pref_network;

  /* Find the new network_info entry corresponding to the preferred network */
  for (i = 0; i < network_len; ++i)
  {
    if (pref_nw == new_sys_status->network_info[i].network)
    {
      new_nw_info = &new_sys_status->network_info[i];
      break;
    }
  }

  if (!new_nw_info)
  {
    goto bail;
  }
  else if (QMI_WDS_DATA_NETWORK_TYPE_3GPP == pref_nw)
  {
    /* Check if the new 3GPP technology is valid */
    if (UMTS_DONT_CARE   == new_nw_info->rat_mask.umts_rat_mask ||
        UMTS_NULL_BEARER == new_nw_info->rat_mask.umts_rat_mask)
    {
      goto bail;
    }
  }
  else if (QMI_WDS_DATA_NETWORK_TYPE_3GPP2 == pref_nw)
  {
    /* Check if the new 3GPP2 technology is valid */
    if (CDMA_DONT_CARE   == new_nw_info->rat_mask.cdma_rat_mask ||
        CDMA_NULL_BEARER == new_nw_info->rat_mask.cdma_rat_mask)
    {
      goto bail;
    }
  }

  /* If the preferred network is different */
  if (cur_sys_status->pref_network != pref_nw)
  {
    ret = TRUE;
  }
  else
  {
    /* Find the current network_info entry corresponding to the preferred network */
    for (i = 0; i < network_len; ++i)
    {
      if (pref_nw == cur_sys_status->network_info[i].network)
      {
        cur_nw_info = &cur_sys_status->network_info[i];
        break;
      }
    }

    if (!cur_nw_info)
    {
      QCRIL_LOG_ERROR("Unable to find network_info entry for pref_network=%d",
                      pref_nw);
      goto bail;
    }

    /* Check if the technology has changed */
    if (QMI_WDS_DATA_NETWORK_TYPE_3GPP == pref_nw)
    {
      if (cur_nw_info->rat_mask.umts_rat_mask != new_nw_info->rat_mask.umts_rat_mask)
      {
        ret = TRUE;
      }
    }
    else if (QMI_WDS_DATA_NETWORK_TYPE_3GPP2 == pref_nw)
    {
      if ((cur_nw_info->rat_mask.cdma_rat_mask != new_nw_info->rat_mask.cdma_rat_mask) ||
          (cur_nw_info->db_so_mask.so_mask_value != new_nw_info->db_so_mask.so_mask_value))
      {
        ret = TRUE;
      }
    }
  }


bail:
  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_data_util_update_partial_retry_enabled_flag

===========================================================================*/
/*!
    @brief
    Updates the flag indicating whether partial retries are enabled based on
    the Android property

    @return
    None
*/
/*=========================================================================*/
void qcril_data_util_update_partial_retry_enabled_flag (void)
{
  static char  args[PROPERTY_VALUE_MAX];
  char  def[QCRIL_DATA_PROPERTY_DISABLE_PARTIAL_RETRY_SIZE+1];
  int ret;

  /* Default property value */
  memset(def, 0, sizeof(def));
  strlcpy(def, QCRIL_DATA_PROPERTY_DISABLE_PARTIAL_RETRY_DEFAULT, sizeof(def));

  /* Prepare destination buffer */
  memset(args, 0, sizeof(args));

  /* Read the partial retry property */
  ret = property_get(QCRIL_DATA_PROPERTY_DISABLE_PARTIAL_RETRY, args, def);

  if (ret > PROPERTY_VALUE_MAX)
  {
    QCRIL_LOG_ERROR("property_get for partial_retry returned %d size", ret);
    return;
  }

  QCRIL_LOG_DEBUG("%s property has %s value set on it",
                  QCRIL_DATA_PROPERTY_DISABLE_PARTIAL_RETRY,
                  args);

  /* Check if partial retries should be disabled */
  if (!strcasecmp(args, "true"))
  {
    QCRIL_LOG_DEBUG("disabling partial retries");
    qcril_data_is_partial_retry_disabled = TRUE;
  }
}

/*===========================================================================

  FUNCTION:  qcril_data_util_update_max_partial_retry_timeout

===========================================================================*/
/*!
    @brief
    Updates the max partial retry timeout for QCRIL data calls

    @return
    None
*/
/*=========================================================================*/
void qcril_data_util_update_max_partial_retry_timeout (void)
{
  static char  args[PROPERTY_VALUE_MAX];
  char *end_ptr;
  int ret;
  int len;
  unsigned long ret_val;

  /* Prepare destination buffer */
  memset(args, 0, sizeof(args));

  /* Read the partial retry property */
  ret = property_get(QCRIL_DATA_PROPERTY_MAX_PARTIAL_RETRY_TIMEOUT, args, "");

  if (ret > PROPERTY_VALUE_MAX)
  {
    QCRIL_LOG_ERROR("property_get for max partial_retry timeout returned %d size", ret);
    return;
  }

  len = strlen( args );
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
      QCRIL_LOG_ERROR( "Fail to convert partial retry timeout %s\n", args );
    }
    else
    {
      QCRIL_LOG_DEBUG("%s property has %s value set on it",
                  QCRIL_DATA_PROPERTY_MAX_PARTIAL_RETRY_TIMEOUT,
                  args);
      qcril_data_max_partial_retry_timeout = ret_val;
    }
  }
}

/*===========================================================================

  FUNCTION:  qcril_data_util_is_new_rat_1x_ehrpd

===========================================================================*/
/*!
    @brief
    Check if there is a new RAT change to 1x or eHRPD

    @return
    TRUE - If RAT changed to 1x or eHRPD
    FALSE - otherwise
*/
/*=========================================================================*/
boolean qcril_data_util_is_new_rat_1x_ehrpd
(
  qmi_wds_data_sys_status_type               *new_sys_status,
  qmi_wds_data_sys_status_network_info_type  **nw_info
)
{
  boolean ret = FALSE;
  int i;
  int network_len;
  qmi_wds_data_network_type  pref_nw;
  qmi_wds_data_sys_status_network_info_type  *new_nw_info = NULL;

  /* Validate the input */
  if (!new_sys_status                       ||
      0 == new_sys_status->network_info_len ||
      !new_sys_status->network_info         ||
      !nw_info)
  {
    QCRIL_LOG_ERROR("Invalid input");
    goto bail;
  }

    /* Verify that the new system status corresponds to a valid technology */
  network_len = new_sys_status->network_info_len;
  pref_nw = new_sys_status->pref_network;

  /* We are only interested in 3GPP2 RAT change */
  if (QMI_WDS_DATA_NETWORK_TYPE_3GPP2 != pref_nw)
  {
    goto bail;
  }

  /* Find the new network_info entry corresponding to the preferred network */
  for (i = 0; i < network_len; ++i)
  {
    if (pref_nw == new_sys_status->network_info[i].network)
    {
      new_nw_info = &new_sys_status->network_info[i];
      break;
    }
  }

  if (!new_nw_info)
  {
    QCRIL_LOG_ERROR("Unable to find network_info entry for pref_network=%d",
                    pref_nw);
    goto bail;
  }

  /* Check if the new 3GPP2 technology is valid */
  if (CDMA_DONT_CARE   == new_nw_info->rat_mask.cdma_rat_mask ||
      CDMA_NULL_BEARER == new_nw_info->rat_mask.cdma_rat_mask)
  {
    goto bail;
  }

  /* Check if the reported 3GPP2 technology is not FMC */
  if (CDMA_FMC != new_nw_info->rat_mask.cdma_rat_mask)
  {
    *nw_info = new_nw_info;
    ret = TRUE;
  }

bail:
  return ret;
}

/*===========================================================================*/
/*!
    @brief
    Register for extended IP config change indication

    @return
    None
*/
/*=========================================================================*/
void qcril_data_util_register_ext_ip_config_ind
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr
)
{
  qmi_wds_indication_reg_req_type  ext_ip_req;
  int rc = QMI_NO_ERR, qmi_err;

  QCRIL_LOG_FUNC_ENTRY();

  if (NULL == info_tbl_ptr ||
      QCRIL_DATA_HNDL_INVALID == info_tbl_ptr->qmi_wds_hndl)
  {
    QCRIL_LOG_ERROR( "invalid input parameter" );
    goto bail;
  }

  memset(&ext_ip_req, 0, sizeof(ext_ip_req));

  ext_ip_req.param_mask = QMI_WDS_EXT_IP_CONFIG_CHANGE_IND_REG_LIST_PARAM_MASK;
  ext_ip_req.ext_ip_pref = QMI_WDS_EXT_IP_CONFIG_CHANGE_REPORT;

  if (QMI_NO_ERR != (rc = qmi_wds_indication_register(info_tbl_ptr->qmi_wds_hndl,
                                                      ext_ip_req,
                                                      &qmi_err)))
  {
    QCRIL_LOG_ERROR( "failed to register for ext IP config ind cid=%d rc=%d, qmi_err=%d",
                     info_tbl_ptr->cid,
                     rc,
                     qmi_err );
  }
  else
  {
    QCRIL_LOG_DEBUG( "successfully registered for ext IP config ind cid=%d",
                     info_tbl_ptr->cid );
  }

bail:
  QCRIL_LOG_FUNC_RETURN();
}

