/*!
  @file
  qcril_data_defs.h

  @brief
  Common definitions shared by module files.

*/

/*===========================================================================

  Copyright (c) 2011 - 2012  Qualcomm Technologies, Inc. All Rights Reserved

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
04/14/11   ar      Initial version

===========================================================================*/

#ifndef QCRIL_DATA_DEFS_H
#define QCRIL_DATA_DEFS_H

#include "qdp.h"
#include "ds_sl_list.h"
#include "qmi_wds_srvc.h"
#include "dsi_netctrl.h"
#include "dsi_netctrl_qos.h"


/*===========================================================================

                           DEFINITIONS AND TYPES

===========================================================================*/
#define MAX_CONCURRENT_UMTS_DATA_CALLS (DSI_MAX_DATA_CALLS) /* bounded as [0, 255] */

#define DS_RIL_REQ_INVALID             (0)

#ifdef QCRIL_DATA_OFFTARGET
  #define SA_FAMILY(addr)         (addr).sa_family
  #define SA_DATA(addr)           (addr).sa_data
  #define SASTORAGE_FAMILY(addr)  (addr).ss_family
  #define SASTORAGE_DATA(addr)    (addr).__ss_padding
  #define PTHREAD_MUTEX_RECURSIVE  PTHREAD_MUTEX_RECURSIVE_NP
#else
  #define SA_FAMILY(addr)         (addr).sa_family
  #define SA_DATA(addr)           (addr).sa_data
  #define SASTORAGE_FAMILY(addr)  (addr).ss_family
  #define SASTORAGE_DATA(addr)    (addr).__data
#endif

#define QCRIL_DATA_IP_FAMILY_V4   "IP"
#define QCRIL_DATA_IP_FAMILY_V6   "IPV6"
#define QCRIL_DATA_IP_FAMILY_V4V6 "IPV4V6"
#define QCRIL_DATA_IPV4           (4)
#define QCRIL_DATA_IPV6           (6)
#define QCRIL_DATA_EMBMS_TMGI_LEN (6)

/* RIL API call states */
#define CALL_INACTIVE                (0)
#define CALL_ACTIVE_PHYSLINK_DOWN    (1)
#define CALL_ACTIVE_PHYSLINK_UP      (2)


#define DS_CALL_INFO_APN_MAX_LEN  (QMI_WDS_MAX_APN_STR_SIZE)
#define DS_CALL_INFO_ADDR_BUF_SIZE     (16)
/* Format: xxx.xxx.xxx.xxx/yy */
#define DS_CALL_INFO_ADDR_IPV4_MAX_LEN (18)
/* Format: xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx/yyy */
#define DS_CALL_INFO_ADDR_IPV6_MAX_LEN (43)

#define DS_CALL_INFO_ADDR_IPV4V6_MAX_LEN (DS_CALL_INFO_ADDR_IPV4_MAX_LEN +  \
                                          DS_CALL_INFO_ADDR_IPV6_MAX_LEN + 1)
                                          /* IPv6 & IPv6 with space delimiter */
#define DS_CALL_INFO_IP_ADDR_MAX_LEN   (DS_CALL_INFO_ADDR_IPV6_MAX_LEN)

#define QCRIL_DATA_TECHNOLOGY_MAX_LEN  4
#define QCRIL_DATA_AUTH_PREF_MAX_LEN   2
#define QCRIL_PROFILE_ID_STR_MAX       4
#define QCRIL_IP_FAMILY_STR_MAX        sizeof(QCRIL_DATA_IP_FAMILY_V4V6)

#define QCRIL_DATA_IS_DSI_DATA_BEARER_TECH_CDMA_1X_EVDO(t) ((t == DSI_DATA_BEARER_TECH_CDMA_1X)   || \
                                                            (t == DSI_DATA_BEARER_TECH_EVDO_REV0) || \
                                                            (t == DSI_DATA_BEARER_TECH_EVDO_REVA) || \
                                                            (t == DSI_DATA_BEARER_TECH_EVDO_REVB) || \
                                                            (t == DSI_DATA_BEARER_TECH_HRPD))

#define QCRIL_DATA_IS_DSI_DATA_BEARER_TECH_3GPP(t)  ((t == DSI_DATA_BEARER_TECH_WCDMA)         || \
                                                     (t == DSI_DATA_BEARER_TECH_GPRS)          || \
                                                     (t == DSI_DATA_BEARER_TECH_HSDPA)         || \
                                                     (t == DSI_DATA_BEARER_TECH_HSUPA)         || \
                                                     (t == DSI_DATA_BEARER_TECH_EDGE)          || \
                                                     (t == DSI_DATA_BEARER_TECH_LTE)           || \
                                                     (t == DSI_DATA_BEARER_TECH_HSDPA_PLUS)    || \
                                                     (t == DSI_DATA_BEARER_TECH_DC_HSDPA_PLUS) || \
                                                     (t == DSI_DATA_BEARER_TECH_HSPA)          || \
                                                     (t == DSI_DATA_BEARER_TECH_64_QAM)        || \
                                                     (t == DSI_DATA_BEARER_TECH_TDSCDMA)       || \
                                                     (t == DSI_DATA_BEARER_TECH_GSM)           || \
                                                     (t == DSI_DATA_BEARER_TECH_3GPP_WLAN))

#define QCRIL_DATA_IS_DSI_DATA_BEARER_TECH_3GPP2(t)  (QCRIL_DATA_IS_DSI_DATA_BEARER_TECH_CDMA_1X_EVDO(t) || \
                                                      (t == DSI_DATA_BEARER_TECH_EHRPD)                  || \
                                                      (t == DSI_DATA_BEARER_TECH_3GPP2_WLAN))

#define QCRIL_DATA_IS_DSI_DATA_BEARER_TECH_3GPP_EHRPD(t)  (QCRIL_DATA_IS_DSI_DATA_BEARER_TECH_3GPP(t) || \
                                                          (t == DSI_DATA_BEARER_TECH_EHRPD))

typedef struct
{
  union
  {
    uint8   u6_addr8[16];
    uint16  u6_addr16[8];
    uint32  u6_addr32[4];
    uint64  u6_addr64[2];
  } in6_u;
} qcril_data_addr6_t;

typedef struct
{
  char                fmtstr[ DS_CALL_INFO_ADDR_IPV4V6_MAX_LEN + 1 ];
} qcril_data_addr_string_t;

typedef struct
{
  char                fmtstr[ DS_CALL_INFO_ADDR_IPV4V6_MAX_LEN +   \
                              DS_CALL_INFO_ADDR_IPV4V6_MAX_LEN + 1 ];
} qcril_data_dns_addr_string_t;


#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >=6))
typedef struct
{
  char            cid[4];

#define DS_CALL_INFO_TYPE_MAX_LEN 10
  char            type[ DS_CALL_INFO_TYPE_MAX_LEN + 1 ];       /* X.25, IP, IPV6, etc. */

  char            apn[ DS_CALL_INFO_APN_MAX_LEN + 1 ];

  qcril_data_addr_string_t     address;           /* IPv4 & IPv6 */

  qcril_data_addr_string_t     gateway;           /* IPv4 & IPv6 */

  qcril_data_dns_addr_string_t dns;     /* Primary & Secondary, IPv4 & IPv6 */

#define DS_CALL_INFO_DEV_NAME_MAX_LEN 12 /* Device name format: rmnetXX or rmnet_sdioXX */
  char            dev_name[ DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1];

  /* when call is connected, fill this with a valid bearer technology */
  RIL_RadioTechnology   radioTech;

  /* stores RIL call end reasons */
  int inactiveReason;

  /* stores state of data call */
  int active;/* CALL_INACTIVE/CALL_ACTIVE_PHYSLINK_DOWN/CALL_ACTIVE_PHYSLINK_UP */

#ifdef FEATURE_DATA_EMBMS
  /* store embms tmgi deactivation reason */
  int embmsTmgiDeactivateReason;
#endif

#if (RIL_VERSION >= 10)

  qcril_data_addr_string_t     pcscf;

#endif

#if (RIL_VERSION >= 11)
  int mtu;
#endif
} qcril_data_call_info_t;
#else
typedef struct
{
  char            cid[4];

#define DS_CALL_INFO_TYPE_MAX_LEN 10
  char            type[ DS_CALL_INFO_TYPE_MAX_LEN + 1 ];       /* X.25, IP, IPV6, etc. */

  char            apn[ DS_CALL_INFO_APN_MAX_LEN + 1 ];

#define DS_CALL_INFO_IP_ADDR_MAX_LEN 40
  char            address[ DS_CALL_INFO_IP_ADDR_MAX_LEN + 1 ];

#define DS_CALL_INFO_DEV_NAME_MAX_LEN 12 /* Device name format: rmnetXX or rmnet_sdioXX */
  char            dev_name[ DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1];

  /* when call is connected, fill this
   with a valid bearer technology */
  RIL_RadioTechnology   radioTech;

  /* stores RIL call end reasons */
  int inactiveReason;

  /* stores state of data call */
  int active; /* CALL_INACTIVE/CALL_ACTIVE_PHYSLINK_DOWN/CALL_ACTIVE_PHYSLINK_UP */

#ifdef FEATURE_DATA_EMBMS
  /* store embms reasons */
  int embmsTmgiDeactivateReason;
#endif
} qcril_data_call_info_t;
#endif /* (RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6) */


typedef struct qcril_data_qos_state_s
{
  list_link_type      link;              /* Must be first */
  dsi_qos_id_type     flow_id;
}qcril_data_qos_state_type;

typedef struct
{
  list_link_type           link;              /* Must be first */
  RIL_Token                pend_tok;
  int                      pend_req;
  char                     session_id_valid;
  int                      session_id;
  char                     tmgi[QCRIL_DATA_EMBMS_TMGI_LEN];
  char                     deact_tmgi[QCRIL_DATA_EMBMS_TMGI_LEN];

} qcril_data_embms_tmgi_requests_type;

typedef struct
{
  char  ril_tech[QCRIL_DATA_TECHNOLOGY_MAX_LEN];

  char  ril_profile[QCRIL_IP_FAMILY_STR_MAX];

  char  ril_apn[DS_CALL_INFO_APN_MAX_LEN];

  char  ril_user[QMI_WDS_MAX_USERNAME_PASS_STR_SIZE];

  char  ril_pass[QMI_WDS_MAX_USERNAME_PASS_STR_SIZE];

  char  ril_auth_pref[QCRIL_DATA_AUTH_PREF_MAX_LEN];

  char  ril_ipfamily[QCRIL_IP_FAMILY_STR_MAX];

} qcril_data_call_params_type;

typedef struct
{
  qcril_instance_id_e_type instance_id;

  qcril_modem_id_e_type    modem_id;

  /* Index to this and call table */
  unsigned int             index;

  int                      cid;

  RIL_Token                pend_tok;

  int                      pend_req;

  dsi_hndl_t               dsi_hndl;
  int                      qmi_wds_hndl;
  /* flag to indicate whether dev_name and call_index is valid */
  unsigned char            info_flg;

  qcril_data_call_info_t   call_info;

  void                     *self;

  /* stores QDP profile id */
  int                      qdp_umts_profile_id;
  int                      qdp_cdma_profile_id;

  /* stores QDP profile PDN types */
  qdp_profile_pdn_type     qdp_3gpp_profile_pdn_type;
  qdp_profile_pdn_type     qdp_3gpp2_profile_pdn_type;

  /* Setup data call response timer associated with this entry */
  timer_t                  timer_id;

  /* Partial retry timer associated with this entry */
  timer_t                  retry_timer_id;

  /* Count of the partial retry attempts made so far */
  unsigned long            partial_retry_count;

  /* Stores QOS flow list */
  list_type                qos_flow_list;

  /* Stores IP version of the call */
  int                      dsi_ip_version;

  /* Flag indicating if a partial retry is being attempted for a
     Dual-IP call */
  int                      is_partial_retry;

  /* Count of valid IP addresses on the interface when the information
     was last queried from DSI Netctrl */
  int                      last_addr_count;

  /* Call paramters provided by Telephony layer */
  qcril_data_call_params_type  *call_params;

  int               status;

  int               suggestedRetryTime; /* If status != 0, this fields indicates the suggested retry
                                           back-off timer value RIL wants to override the one
                                           pre-configured in FW.
                                           The unit is miliseconds.
                                           The value < 0 means no value is suggested.
                                           The value 0 means retry should be done ASAP.
                                           The value of MAX_INT(0x7fffffff) means no retry. */
  /*List of pending embms transactions*/
  list_type                embms_txns_list;

  /* Dormancy status */
  qmi_wds_dorm_status_type dorm_status;

} qcril_data_call_info_tbl_type;

/* Information necessary for handling dsi callbacks */
typedef struct
{
  qcril_instance_id_e_type instance_id;

  qcril_modem_id_e_type    modem_id;

  RIL_Token                pend_tok;

  dsi_hndl_t               dsi_hndl;

  qcril_data_call_info_tbl_type *info_tbl_ptr;

} qcril_data_dsi_cb_tbl_type;

typedef struct
{
  dsi_net_evt_t      evt;
  void              *data;
  int                data_len;
  void              *self;
  dsi_evt_payload_t  payload;
} qcril_data_event_data_t;

typedef enum
{
  QCRIL_DATA_STACK_SWITCH_CMD,
  QCRIL_DATA_MAX_CMD
}qcril_data_cmd_id;

typedef struct
{
  qcril_data_cmd_id  cmd_id;
  qcril_modem_stack_id_e_type old_stack_id;
  qcril_modem_stack_id_e_type new_stack_id;
  void              *self;
} qcril_data_cmd_data_t;

#define DS_RIL_MAX_QOS_SPECS_PER_REQ        (10)    /* QMI limit */
#define DS_RIL_MAX_QOS_FLOWS_PER_SPEC       (2)     /* Tx, Rx direction */

typedef union qos_resp_u
{
  /* Response to setup QOS request (single flowID assumed) */
  struct
  {
    char *return_code;
    char *qos_flow_id[DS_RIL_MAX_QOS_SPECS_PER_REQ];
  } setup;

  /* Response to get granted QOS request */
  struct
  {
    char *return_code;
    char *status;
    char *qos_spec1;
    char *qos_spec2;
  } get_status;

  /* Response to other QOS operations */
  struct
  {
    char *return_code;
  } result;

  /* Unsolicited response */
  struct
  {
    char *qos_flow_id;
    char *status_ind;
  } unsol;

  /* Generic struct for memory mangement */
  struct
  {
    char *string1;
    char *string2;
    char *string3;
    char *string4;
  } dummy;
} qcril_qos_resp_t;

#define QCRIL_DATA_NUM_OMH_PROFILES_EXPECTED  (6)

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
typedef struct qcril_data_call_response_s
{

#if (RIL_VERSION >= 11)

  /* Response to setup data call request */
  RIL_Data_Call_Response_v11  setup_rsp;

  /* Response to get call list request */
  RIL_Data_Call_Response_v11 *list;

#elif (RIL_VERSION >= 10)

  /* Response to setup data call request */
  RIL_Data_Call_Response_v9  setup_rsp;

  /* Response to get call list request */
  RIL_Data_Call_Response_v9 *list;

#else

  /* Response to setup data call request */
  RIL_Data_Call_Response_v6  setup_rsp;

  /* Response to get call list request */
  RIL_Data_Call_Response_v6 *list;

#endif /* (RIL_VERSION >= 11)  */

  /* Response to Request for last call fail cause */
  int   cause_code;

  /*Response to get_data_call_profile request*/
  struct
  {
    int                      num_profiles;
    RIL_DataCallProfileInfo
      profile_info[QCRIL_DATA_NUM_OMH_PROFILES_EXPECTED];
  }omh_profile_info;

  /* Response for QOS operations */
  qcril_qos_resp_t qos_resp;

  /* size */
  size_t size;

} qcril_data_call_response_type;
#else
typedef struct qcril_data_call_response_s
{
  /* Response to setup data call request */
  struct
  {
    char *cid;
    char *dev_name;
    char *ip_addr;
  }setup_rsp;

  /* Response to Request for last call fail cause */
  int   cause_code;

  /* Response to get call list request */
  RIL_Data_Call_Response *list;

  /*Response to get_data_call_profile request*/
  struct
  {
    int                      num_profiles;
    RIL_DataCallProfileInfo
      profile_info[QCRIL_DATA_NUM_OMH_PROFILES_EXPECTED];
  }omh_profile_info;

  /* Response for QOS operations */
  qcril_qos_resp_t qos_resp;

  /* size */
  size_t size;

} qcril_data_call_response_type;
#endif /* (RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6) */


#define VALIDATE_LOCAL_DATA_OBJ( ptr ) \
    ( ( (ptr) != NULL ) &&  ( (ptr)->self == (void *)(ptr) ) )

extern qcril_data_call_info_tbl_type info_tbl[ MAX_CONCURRENT_UMTS_DATA_CALLS ];

extern pthread_mutex_t info_tbl_mutex;

/* Function entry/exit logging macros */
#define QCRIL_LOG_ENTRY  QCRIL_LOG_VERBOSE( "%s: ENTRY", __func__ );
#define QCRIL_LOG_EXIT                                     \
  if( DSI_SUCCESS == ret ) {                               \
    QCRIL_LOG_VERBOSE( "%s: EXIT with suc", __func__ );    \
  } else {                                                 \
    QCRIL_LOG_VERBOSE( "%s: EXIT with err", __func__ );    \
  }

/* Dynamic memory macros */
#define QCRIL_DATA_ALLOC_STORAGE( ptr, size, label )                     \
  {                                                                      \
    (ptr) = malloc( (size) );                                            \
    if( NULL == (ptr) )                                                  \
    {                                                                    \
      QCRIL_LOG_ERROR("failed to allocate storage [%d] bytes",(size));   \
      goto label;                                                        \
    }                                                                    \
  }
#define QCRIL_DATA_RELEASE_STORAGE( ptr ) \
    if( NULL != ptr )                     \
    {                                     \
      free( ptr );                        \
      ptr = NULL;                         \
    }                                     \


/* Data system determination mode in use */
typedef enum
{
  QCRIL_DATA_LEGACY_DSD_MODE_UNKNOWN = -1,
  QCRIL_DATA_LEGACY_DSD_MODE_NAS,
  QCRIL_DATA_LEGACY_DSD_MODE_DATA_SYS_STATUS,
  QCRIL_DATA_LEGACY_DSD_MODE_PREF_DATA_TECH
} qcril_data_legacy_dsd_mode_t;

/*=========================================================================
  FUNCTION:  qcril_data_util_is_pending

===========================================================================*/
/*!
    @brief

    @pre caller must have locked info_tbl_mutex prior to calling this function.

    @return
    NONE
*/
/*=========================================================================*/
extern int qcril_data_util_is_req_pending
(
  const qcril_data_call_info_tbl_type *,
  unsigned int *
);


/*=========================================================================
  FUNCTION:  qcril_data_response_success

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList. Send RIL_E_SUCCESS response.

    @return
    None
*/
/*=========================================================================*/
extern void qcril_data_response_success
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request,
  void *response,
  size_t response_len
);


/*=========================================================================
  FUNCTION:  qcril_data_response_generic_failure

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList. Send E_GENERIC_FAILURE response.

    @return
    None
*/
/*=========================================================================*/
extern void qcril_data_response_generic_failure
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request
);


#if (RIL_QCOM_VERSION >= 2)
/*=========================================================================
  FUNCTION:  qcril_data_unsol_qos_state_changed

===========================================================================*/
/*!
    @brief
    send RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED to QCRIL

    @return
    None
*/
/*=========================================================================*/
extern void qcril_data_unsol_qos_state_changed
(
  qcril_instance_id_e_type instance_id,
  void                    *response,
  size_t                   response_len
);

/*===========================================================================

  FUNCTION: qcril_data_qos_status_ind_handler

===========================================================================*/
/*!
    @brief
    Process QOS status indication messages received from lower-layer.
    The appropriate response buffer and length are returned, along with
    status and matching command request.  If no request match, response is
    assumed to be unsolicited.

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
int qcril_data_qos_status_ind_handler
(
  const qcril_data_event_data_t  *evt_info_ptr,
  void                          **response_pptr,
  size_t                         *response_len_ptr
);

/*===========================================================================

  FUNCTION:  qcril_data_qos_init

===========================================================================*/
/*!
    @brief
    Initialize the DATA QOS subsystem of the RIL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_qos_init( void );
#endif /* RIL_QCOM_VERSION >= 2 */


/*===========================================================================

  FUNCTION:  qcril_data_client_init

===========================================================================*/
/*!
    @brief
    Initialize the client module

    @return
    None
*/
/*=========================================================================*/
void qcril_data_client_init ( void );

#endif /* QCRIL_DATA_DEFS_H */
