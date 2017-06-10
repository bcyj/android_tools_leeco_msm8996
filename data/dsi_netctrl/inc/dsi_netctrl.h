/**
  @file
  dsi_netctrl.h

  @brief
  This file provides an API to interact with underlying data control
  plane.

*/

/*===========================================================================

  Copyright (c) 2008-2014 Qualcomm Technologies, Inc. All Rights Reserved

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
04/19/10   js      modified original to suit multi modem requirements

===========================================================================*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef DSI_NETCTRL_H
#define DSI_NETCTRL_H

#include <sys/socket.h>

#include <netdb.h>

/*---------------------------------------------------------------------------
                           DECLARATIONS
---------------------------------------------------------------------------*/

typedef void * dsi_hndl_t;

#include "dsi_netctrl_qos.h"
#include "qmi_wds_srvc.h"

/* these are unique bitmasks. always make sure
 * the next technology bitmap is 1 left shift of
 * previous technolgoy bitmap */
#define DSI_RADIO_TECH_MIN 0x00000001
#define DSI_RADIO_TECH_UMTS DSI_RADIO_TECH_MIN
#define DSI_RADIO_TECH_CDMA 0x00000002
#define DSI_RADIO_TECH_1X 0x00000004
#define DSI_RADIO_TECH_DO 0x00000008
#define DSI_RADIO_TECH_LTE 0x00000010
#define DSI_RADIO_TECH_TDSCDMA 0x00000020
/* this is integer number telling how many techs we have */
#define DSI_RADIO_TECH_MAX 6
#define DSI_RADIO_TECH_UNKNOWN 0

/* PCSCF address limit */
#define DSI_PCSCF_ADDR_LIST_MAX 20
#define DSI_PCSCF_FQDN_LIST_MAX 15

#define DSI_MAX_FQDN_STR_SIZE   256

typedef struct sockaddr_storage sockaddr_storage_t;

typedef enum dsi_auth_pref_e
{
  DSI_AUTH_PREF_PAP_CHAP_NOT_ALLOWED = 0,
  DSI_AUTH_PREF_PAP_ONLY_ALLOWED,
  DSI_AUTH_PREF_CHAP_ONLY_ALLOWED,
  DSI_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED
} dsi_auth_pref_t;

typedef enum dsi_ce_reason_type_e
{
  DSI_CE_TYPE_UNINIT  = -2,
  DSI_CE_TYPE_INVALID = 0XFF,
  DSI_CE_TYPE_MOBILE_IP = 0x01,
  DSI_CE_TYPE_INTERNAL = 0x02,
  DSI_CE_TYPE_CALL_MANAGER_DEFINED = 0x03,
  DSI_CE_TYPE_3GPP_SPEC_DEFINED = 0x06,
  DSI_CE_TYPE_PPP = 0x07,
  DSI_CE_TYPE_EHRPD = 0x08,
  DSI_CE_TYPE_IPV6 = 0x09
} dsi_ce_reason_type_t;

/** @cond
*/

typedef struct dsi_ce_reason_s
{
  dsi_ce_reason_type_t reason_type; /* discriminator for reason codes */
  int reason_code; /* overloaded cause codes discriminated by reason_type */
} dsi_ce_reason_t;

/** @endcond */

typedef enum dsi_embms_tmgi_deactivated_reason_type_e
{
  DSI_EMBMS_TMGI_DEACTIVATED_TYPE_INVALID                    = -1,
  DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_RADIO_CONFIG      = 0x00010000,
  DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_CHANNEL_UNAVAIL   = 0x00010001,
  DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_EMBMS_NOT_ENABLED = 0x00010002,
  DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_OUT_OF_COVERAGE   = 0x00010003,
} dsi_embms_tmgi_deactivate_reason_type_t;

typedef qmi_wds_embms_tmgi_list_type     dsi_embms_tmgi_list_type;
typedef qmi_wds_embms_tmgi_type          dsi_embms_tmgi_type;
typedef qmi_wds_embms_actdeact_tmgi_info_type dsi_embms_actdeact_tmgi_info_type;
typedef qmi_wds_embms_oos_warning_type   dsi_embms_tmgi_oos_warn_type;

typedef enum dsi_ext_tech_pref_e
{
  DSI_EXT_TECH_INVALID = 0,
  DSI_EXT_TECH_CDMA,
  DSI_EXT_TECH_UMTS,
  DSI_EXT_TECH_EMBMS,
  DSI_EXT_TECH_MODEM_LINK_LOCAL /* used only by IMS not by qcril */
} dsi_ext_tech_pref_t;

typedef dsi_ext_tech_pref_t dsi_call_tech_type;

/** @addtogroup datatypes
@{ */

/** Return value from most of the DSI functions. */
#define DSI_SUCCESS   0   /**< Indicates that the operation was successful. */
#define DSI_ERROR    -1   /**< Indicates that the operation was not successful. */

/** @} */ /* end_addtogroup datatypes */


/** @addtogroup errorcodes
@{ */

#define DSI_EFAULT        101
#define DSI_EBADF         102
#define DSI_EINTERNAL     103
#define DSI_EINITED       104   /**< Indicates the library is already inited. */

/** @} */ /* end_addtogroup datatypes */

/* currently using #define constants
 * instead of typedef primarily so that
 * user can pass in an integer without
 * typcasting */
#define DSI_CALL_TYPE_TETHERED 0
#define DSI_CALL_TYPE_EMBEDDED 1

#define DSI_IP_VERSION_4   4
#define DSI_IP_VERSION_6   6
#define DSI_IP_VERSION_4_6 10

/** @addtogroup datatypes
@{ */


/** IMS portforwarding status defines **/
typedef enum dsi_port_forwarding_status_enum_e
{
  DSI_PORT_FORWARDING_INVALID = -1,
  DSI_PORT_FORWARDING_ENABLED,
  DSI_PORT_FORWARDING_DISABLED,
}dsi_port_forwarding_status_t;


/** Specifies which configuration parameter to update using
dsi_set_data_call_param(). */
typedef enum dsi_call_info_enum_e
{
  DSI_CALL_INFO_MIN = 0,
  DSI_CALL_INFO_UMTS_PROFILE_IDX, /**< UMTS profile ID. */
  DSI_CALL_INFO_APN_NAME,         /**< APN name. */
  DSI_CALL_INFO_USERNAME,         /**< APN user name (if any). */
  DSI_CALL_INFO_PASSWORD,         /**< APN password (if any). */
  DSI_CALL_INFO_AUTH_PREF,        /**< Authentication preference. */
  DSI_CALL_INFO_CDMA_PROFILE_IDX, /**< CDMA profile ID. */
  DSI_CALL_INFO_IP_ADDR,
  DSI_CALL_INFO_DEVICE_NAME,
  DSI_CALL_INFO_TECH_PREF,        /**< Technology preference. */
  DSI_CALL_INFO_CALL_TYPE,
  DSI_CALL_INFO_IP_VERSION,       /**< Preferred IP family for the call. */
  DSI_CALL_INFO_EXT_TECH_PREF,
  DSI_CALL_INFO_PARTIAL_RETRY,
  DSI_CALL_INFO_APP_TYPE,
  DSI_CALL_INFO_MAX
} dsi_call_param_identifier_t;

typedef enum dsi_call_param_app_type
{
  DSI_CALL_PARAM_APP_TYPE_INVALID = -1,
  DSI_CALL_PARAM_APP_TYPE_DEFAULT = 0,
  DSI_CALL_PARAM_APP_TYPE_INTERNET = DSI_CALL_PARAM_APP_TYPE_DEFAULT,
  DSI_CALL_PARAM_APP_TYPE_IMS
} dsi_param_app_type_t;

/** Specifies string parameter values for dsi_set_data_call_param(). */
typedef struct
{
  char * buf_val;  /**< Pointer to the buffer containing the parameter value
                        that is to be set. */
  int num_val;     /**< Size of the parameter buffer. */
} dsi_call_param_value_t;

/** @} */ /* end_addtogroup datatypes */

#define DSI_CALL_INFO_USERNAME_MAX_LEN (127 + 1)
#define DSI_CALL_INFO_PASSWORD_MAX_LEN (127 + 1)
/* rmnet_sdioxx, rmnet_xx */
#define DSI_CALL_INFO_DEVICE_NAME_MAX_LEN 12

/* max supported dsi handles */
#define DSI_MAX_DATA_CALLS     20

/** @addtogroup datatypes
@{ */

/**
   DSI event names.

   These event names are sent along with the callback that
   has been registered during dsi_get_data_service_hndl().
*/
typedef enum
{
  DSI_EVT_INVALID = 0x0,              /**< Invalid event. */
  DSI_EVT_NET_IS_CONN,                /**< Call is connected. */
  DSI_EVT_NET_NO_NET,                 /**< Call is disconnected. */
  DSI_EVT_PHYSLINK_DOWN_STATE,        /**< Physlink becomes dormant. */
  DSI_EVT_PHYSLINK_UP_STATE,          /**< Physlink becomes active. */
  DSI_EVT_NET_RECONFIGURED,           /**< Interface is reconfigured. */
  DSI_EVT_QOS_STATUS_IND,             /**< A status for the associated QoS has
                                           changed. */
  DSI_EVT_NET_NEWADDR,                /**< New address is generated. */
  DSI_EVT_NET_DELADDR,                /**< An address for the interface has been deleted. */
  DSI_EVT_NET_PARTIAL_CONN,           /**< Address is available for either IPv4
                                           or IPv6 only. @newpage */
  DSI_NET_TMGI_ACTIVATED,
  DSI_NET_TMGI_DEACTIVATED,
  DSI_NET_TMGI_ACTIVATED_DEACTIVATED,
  DSI_NET_TMGI_LIST_CHANGED,
  DSI_NET_SAI_LIST_CHANGED,
  DSI_NET_CONTENT_DESC_CONTROL,

  DSI_EVT_NET_HANDOFF,
  DSI_EVT_WDS_CONNECTED,
  DSI_EVT_NET_NEWMTU,                 /**< MTU update. */

  DSI_EVT_MAX
} dsi_net_evt_t;

/** @} */ /* end_addtogroup datatypes */

typedef enum dsi_embms_tmgi_session_id_valid_e
{
  DSI_EMBMS_TMGI_SESSION_ID_NOT_VALID = 0,
  DSI_EMBMS_TMGI_SESSION_ID_VALID     = 1
}dsi_embms_tmgi_session_id_state_t;

typedef enum dsi_embms_tmgi_activate_request_type_e
{
  DSI_EMBMS_TMGI_REQUEST_INVALID    = -1,
  DSI_EMBMS_TMGI_REQUEST_ACTIVATE   = 0,
  DSI_EMBMS_TMGI_REQUEST_DEACTIVATE = 1
}dsi_embms_tmgi_activate_request_type_t;

/** @cond
*/

typedef struct dsi_embms_tmgi_info_s
{
  dsi_embms_tmgi_list_type                 list_type;    /* activate or available tmgi list */
  qmi_wds_embms_actdeact_tmgi_info_type    embms_tmgi_actdeact_params;
  dsi_embms_tmgi_oos_warn_type             oos_warning;
}dsi_embms_tmgi_info_type;

typedef qmi_wds_embms_sai_list_ind_type dsi_embms_sai_info_type;

typedef qmi_wds_embms_content_desc_control_ind_type dsi_embms_content_desc_info_type;
typedef qmi_wds_embms_content_desc_update_info_type dsi_embms_content_desc_update_info_type;

typedef qmi_wds_handoff_info_ind_type dsi_handoff_info_type;


typedef enum
{
  DSI_WDS_RAT_EX_NULL_BEARER  = QMI_WDS_BEARER_TECH_RAT_EX_NULL_BEARER,

  /* 3GPP RAT Values */
  DSI_WDS_RAT_EX_3GPP_WCDMA   = QMI_WDS_BEARER_TECH_RAT_EX_3GPP_WCDMA,
  DSI_WDS_RAT_EX_3GPP_GERAN   = QMI_WDS_BEARER_TECH_RAT_EX_3GPP_GERAN,
  DSI_WDS_RAT_EX_3GPP_LTE     = QMI_WDS_BEARER_TECH_RAT_EX_3GPP_LTE,
  DSI_WDS_RAT_EX_3GPP_TDSCDMA = QMI_WDS_BEARER_TECH_RAT_EX_3GPP_TDSCDMA,
  DSI_WDS_RAT_EX_3GPP_WLAN    = QMI_WDS_BEARER_TECH_RAT_EX_3GPP_WLAN,
  DSI_WDS_RAT_EX_3GPP_MAX     = QMI_WDS_BEARER_TECH_RAT_EX_3GPP_MAX,

  /* 3GPP2 RAT Values */
  DSI_WDS_RAT_EX_3GPP2_1X     = QMI_WDS_BEARER_TECH_RAT_EX_3GPP2_1X,
  DSI_WDS_RAT_EX_3GPP2_HRPD   = QMI_WDS_BEARER_TECH_RAT_EX_3GPP2_HRPD,
  DSI_WDS_RAT_EX_3GPP2_EHRPD  = QMI_WDS_BEARER_TECH_RAT_EX_3GPP2_EHRPD,
  DSI_WDS_RAT_EX_3GPP2_WLAN   = QMI_WDS_BEARER_TECH_RAT_EX_3GPP2_WLAN,
  DSI_WDS_RAT_EX_3GPP2_MAX    = QMI_WDS_BEARER_TECH_RAT_EX_3GPP2_MAX,

  /* Force to 32 bits */
  DSI_WDS_RAT_EX_MAX          = QMI_WDS_BEARER_TECH_RAT_EX_MAX
}dsi_wds_rat_e;

typedef struct dsi_wds_handoff_info
{
  dsi_handoff_info_type    handoff_type;
  dsi_wds_rat_e            source_rat;
  dsi_wds_rat_e            target_rat;
  qmi_ip_family_pref_type  ip_type;
} dsi_wds_handoff_info_type;

typedef enum
{
  QMI_WDS_HANDOFF_IND = 0
} dsi_ind_reg_type;

typedef enum
{
  DSI_IND_REGISTER = 0,
  DSI_IND_UNREGISTER
} dsi_ind_registration_t;

typedef enum
{
  CHECK_ACTIVATE_STATUS = 0,
  CHECK_DEACTIVATE_STATUS = 1,
  CHECK_ACTIVATE_DEACTIVATE_STATUS = 3
}dsi_embms_tmgi_status_field_type;

/** @addtogroup datatypes
@{ */

/* Enum of different IP families */
typedef enum
{
  DSI_IP_FAMILY_V4,
  DSI_IP_FAMILY_V6,
  DSI_NUM_IP_FAMILIES
} dsi_ip_family_t;


/** Event payload sent with event callback. */
typedef struct evt_info_s
{
  /* Populated for events: DSI_EVT_QOS_STATUS_IND */

  struct qos_info_s
  {
    dsi_qos_id_type              flow_id;     /**< QoS flow ID .*/
    dsi_qos_flow_type            flow_type;   /**< Flow type. */
    dsi_qos_status_event_type    status_evt;  /**< Flow status. See
                                                   dsi_qos_status_event_type. */
    dsi_qmi_qos_reason_code_type reason_code; /**< Reason code if the flow is
                                                   disconnected. */
  } qos_info /** Event information associated with a QoS status indication.
                 See qos_info_s in the next subsection. */;

  /* Populated for events: DSI_NET_TMGI_ACTIVATED */
  dsi_embms_tmgi_info_type   embms_tmgi_info; /**< Event information associated
                                                   with eMBMS event information. */

  /* Populated for events: DSI_NET_SAI_LIST_CHANGED */
  dsi_embms_sai_info_type    embms_sai_info;

  /* Populated for events: DSI_NET_CONTENT_DESC_CONTROL */
  dsi_embms_content_desc_info_type embms_content_desc_info;

  dsi_wds_handoff_info_type  handoff_info;    /**< Event information associated
                                                   with WDS Handoff event
                                                   indication */
  dsi_ip_family_t ip_type;      /* This member is used only while generating
                                   DSI_EVT_WDS_CONNECTED event
                                 */
} dsi_evt_payload_t;

/** Callback function prototype for DSI NetCtrl events. */
typedef void (*dsi_net_ev_cb)( dsi_hndl_t         hndl,         /**< Handle for which this event is associated. */
                               void              *user_data,    /**< Application-provided user data. */
                               dsi_net_evt_t      evt,          /**< Event identifier. */
                               dsi_evt_payload_t *payload_ptr   /**< Associated event information. */
                               );


/** Structure used to represent the IP address. */
typedef struct dsi_addr_s
{
  /* indicates if a valid address is available or not */
  char valid_addr; /**< Stores whether the address is valid (for IPv6). */
  /* stores ip address */
  struct sockaddr_storage addr; /**< Address structure. */
} dsi_addr_t;

/** IP address information structure. */
typedef struct dsi_addr_info_s
{
  /* Network interface address */
  dsi_addr_t iface_addr_s; /**< Interface IP address. */
  unsigned int iface_mask; /**< Subnet mask.       */
  /* Gateway server address */
  dsi_addr_t gtwy_addr_s;  /**< Gateway IP address. */
  unsigned int gtwy_mask;  /**< Subnet mask. */
  /* Primary DNS server  address */
  dsi_addr_t dnsp_addr_s;  /**< Primary DNS address. */
  /* Secondary DNS server  address */
  dsi_addr_t dnss_addr_s;  /**< Secondary DNS address. */
} dsi_addr_info_t;

/** @} */ /* end_addtogroup datatypes */

#define DSI_GET_IP_FAMILY(ipf) ((AF_INET==(ipf))?"AF_INET": \
                                ((AF_INET6==(ipf))?"AF_INET6":"UNKNOWN"))

/** @addtogroup datatypes
@{ */

/** Datatypes used by IOCTL functions */
typedef struct
{
  int                         fqdn_length;
  unsigned char               fqdn_string[DSI_MAX_FQDN_STR_SIZE];
}dsi_wds_fqdn_strings_t;

/** PCSCF address information */
typedef struct dsi_pcscf_addr_info_s
{
  unsigned int addr_count;
  dsi_addr_t pcscf_address[DSI_PCSCF_ADDR_LIST_MAX];
} dsi_pcscf_addr_info_t;

typedef struct dsi_pcscf_fqdn_list_s
{
  unsigned int fqdn_count;
  dsi_wds_fqdn_strings_t pcscf_domain_list[DSI_PCSCF_FQDN_LIST_MAX];
} dsi_pcscf_fqdn_list_t;

typedef qmi_wds_delegated_ipv6_prefix_type dsi_delegated_ipv6_prefix_type;

/** Enum definition for the IOCTLs */
typedef enum
{
  DSI_IFACE_IOCTL_MIN = 0,
  DSI_IFACE_IOCTL_GET_PCSCF_SERV_ADDRESS = DSI_IFACE_IOCTL_MIN,
  DSI_IFACE_IOCTL_GET_PCSCF_DOMAIN_NAME_LIST,
  DSI_IFACE_IOCTL_REMOVE_DELEGATED_IPV6_PREFIX,
  DSI_IFACE_IOCTL_MAX,
  _DSI_IFACE_IOCTL_FORCE_32_BIT = 0x7FFFFFFF // Force 32 bits to be used for enum
}dsi_iface_ioctl_enum_t;

/** @} */ /* end_addtogroup datatypes */

/** @addtogroup datatypes
@{ */

/** Bearer technology types (returned with dsi_get_current_data_bearer_tech). */
typedef enum
{
  DSI_DATA_BEARER_TECH_UNKNOWN,       /**< Unknown bearer. */

  /* CDMA related data bearer technologies */
  DSI_DATA_BEARER_TECH_CDMA_1X,       /**< 1X technology. */
  DSI_DATA_BEARER_TECH_EVDO_REV0,     /**< CDMA Rev 0. */
  DSI_DATA_BEARER_TECH_EVDO_REVA,     /**< CDMA Rev A. */
  DSI_DATA_BEARER_TECH_EVDO_REVB,     /**< CDMA Rev B. */
  DSI_DATA_BEARER_TECH_EHRPD,         /**< EHRPD. */
  DSI_DATA_BEARER_TECH_FMC,           /**< Fixed mobile convergence. */
  DSI_DATA_BEARER_TECH_HRPD,          /**< HRPD */
  DSI_DATA_BEARER_TECH_3GPP2_WLAN,    /**< IWLAN */

  /* UMTS related data bearer technologies */
  DSI_DATA_BEARER_TECH_WCDMA,         /**< WCDMA. */
  DSI_DATA_BEARER_TECH_GPRS,          /**< GPRS. */
  DSI_DATA_BEARER_TECH_HSDPA,         /**< HSDPA. */
  DSI_DATA_BEARER_TECH_HSUPA,         /**< HSUPA. */
  DSI_DATA_BEARER_TECH_EDGE,          /**< EDGE. */
  DSI_DATA_BEARER_TECH_LTE,           /**< LTE. */
  DSI_DATA_BEARER_TECH_HSDPA_PLUS,    /**< HSDPA+. */
  DSI_DATA_BEARER_TECH_DC_HSDPA_PLUS, /**< DC HSDPA+. */
  DSI_DATA_BEARER_TECH_HSPA,          /**< HSPA */
  DSI_DATA_BEARER_TECH_64_QAM,        /**< 64 QAM. */
  DSI_DATA_BEARER_TECH_TDSCDMA,       /**< TD-SCDMA. */
  DSI_DATA_BEARER_TECH_GSM,           /**< GSM */
  DSI_DATA_BEARER_TECH_3GPP_WLAN,      /**< IWLAN */
  DSI_DATA_BEARER_TECH_MAX

} dsi_data_bearer_tech_t;


/** Packet statistics structure (returned with dsi_get_pkt_stats). */
typedef struct
{
  unsigned long pkts_tx;           /**< Number of packets transmitted. */
  unsigned long pkts_rx;           /**< Number of packets received. */
  long long     bytes_tx;          /**< Number of bytes transmitted. */
  long long     bytes_rx;          /**< Number of bytes received. */
  unsigned long pkts_dropped_tx;   /**< Number of transmit packets dropped. */
  unsigned long pkts_dropped_rx;   /**< Number of receive packets dropped. */
} dsi_data_pkt_stats;

/** Data channel rate structure (dsi_get_current_data_channel_rate) */
typedef struct
{
  unsigned long current_tx_rate;   /**< Current TX data rate for the channel*/
  unsigned long current_rx_rate;   /**< Current RX data rate for the channel*/
  unsigned long max_tx_rate;       /**< Max TX data rate for the channel*/
  unsigned long max_rx_rate;       /**< Max RX data rate for the channel*/
} dsi_data_channel_rate_t;


typedef enum
{
  DSI_DATA_DEFAULT_SUBS = 0x0000,
  DSI_DATA_PRIMARY_SUBS = 0x0001,
  DSI_DATA_SECONDARY_SUBS = 0x0002,
  DSI_DATA_TERITIARY_SUBS = 0x0003,
  DSI_DATA_DONT_CARE_SUBS = 0x00FF
}dsi_data_modem_subscription_id;

/** @} */ /* end_addtogroup datatypes */

/*===========================================================================
                    EXTERNAL FUNCTION PROTOTYPES
===========================================================================*/

/* these are the currently supported modes
   for dsi_netctrl */
#define DSI_MODE_GENERAL 0 /* generic default mode */
#define DSI_MODE_TEST 1 /* used for test mode */

/*===========================================================================
  FUNCTION:  dsi_init_ex
===========================================================================*/
/** @ingroup dsi_init_ex

    Initializes the DSI_NetCtrl library for the specified operating mode and
    executes a callback function with the callback data,
    given as an argument after completing init.

    This function must be invoked once per process, typically on process
    startup.

    @note1hang
    Only DSI_MODE_GENERAL is to be used by applications.

    param [in] init mode
    param [in] dsi_init_cb_func
    param [in] dsi_init_cb_data

    @return
    DSI_SUCCESS -- Initialization was successful.
    DSI_ERROR -- Initialization failed.

    @dependencies
    None.
*/
/*=========================================================================*/
extern int dsi_init_ex
(
  int mode,
  void (* dsi_init_cb_func)( void * ),
  void *dsi_init_cb_data
);


/*===========================================================================
  FUNCTION:  dsi_init
===========================================================================*/
/** @ingroup dsi_init

    Initializes the DSI_NetCtrl library for the specified operating mode.

    This function must be invoked once per process, typically on process
    startup.

    @note1hang
    Only DSI_MODE_GENERAL is to be used by applications.

    @param[in] mode Mode of operation in which to initialize the library.
                    The library can be initialized to operate in one of
                    two modes: \n
                    - DSI_MODE_GENERAL
                    - DSI_MODE_TEST
                    @tablebulletend

    @return
    DSI_SUCCESS -- Initialization was successful. \n
    DSI_ERROR -- Initialization failed.

    @dependencies
    None.
*/
/*=========================================================================*/
extern int dsi_init(int mode);

/*===========================================================================
  FUNCTION:  dsi_release
===========================================================================*/
/** @ingroup dsi_release

    Clean-up the DSI_NetCtrl library.

    @return
    DSI_SUCCESS -- Cleanup was successful. \n
    DSI_ERROR -- Cleanup failed.

    @dependencies
    None.
*/
/*=========================================================================*/
extern int dsi_release(int mode);

/*===========================================================================
  FUNCTION:  dsi_get_data_srvc_hndl
===========================================================================*/
/** @ingroup dsi_get_data_srvc_hndl

    Gets an opaque data service handle. All subsequent functions use this
    handle as an input parameter.

    @note1hang
    The DSI_NetCtrl library waits for initialization from the lower layers
    (QMI ports being opened, the RmNet interfaces being available, etc.) to
    support data services functionality. During initial bootup scenarios,
    these dependencies may not be available, which will cause
    dsi_get_data_srvc_hndl to return an error. In such cases, clients are
    asked to retry this function call repeatedly using a 500 ms timeout
    interval. Once a non-NULL handle is returned, clients can exit out of
    the delayed retry loop.

    @param[in] cb_fn Client callback function used to post event indications.
                     Refer to Section 3.2.3 for the function prototype.
    @param[in] user_data Pointer to the client context block (cookie). The
                         value may be NULL.

    @return
    dsi_hndl_t if successfull, NULL otherwise.

    @dependencies
    dsi_init() must be called.
*/
/*=========================================================================*/
extern dsi_hndl_t dsi_get_data_srvc_hndl
(
  dsi_net_ev_cb cb_fn,
  void * user_data
);

/*===========================================================================
  FUNCTION:  dsi_rel_data_srvc_hndl
===========================================================================*/
/** @ingroup dsi_rel_data_srvc_hndl

    Releases a data service handle. All resources associated with the
    library are released.

    @note1hang
    If the user starts an interface with this handle,
    the corresponding interface is stopped before the DSI
    hndl is released.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().

    @return
    None.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl().
*/
/*=========================================================================*/
extern void dsi_rel_data_srvc_hndl(dsi_hndl_t hndl);

/*===========================================================================
  FUNCTION: dsi_ind_registration
===========================================================================*/
/*!
  @brief
  This function can be used to register/unregister for WDS handoff
  indications

  @return
  DSI_SUCCESS
  DSI_FAILURE

  @note
  This function must be called only after SNI is done and we have
  an iface association. The WDS client binding will happen only at
  that point in time
*/
/*=========================================================================*/
int dsi_ind_registration
(
  dsi_hndl_t              hndl,
  dsi_ind_reg_type        ind_type,
  dsi_ind_registration_t  reg_unreg
);

/*===========================================================================
  FUNCTION:  dsi_start_data_call
===========================================================================*/
/** @ingroup dsi_start_data_call

    Starts a data call.

    An immediate call return value indicates whether the request was sent
    successfully. The client receives asynchronous notfications via a callback
    registered with dsi_get_data_srvc_hndl() indicating the data call bring-up
    status.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().

    @return
    DSI_SUCCESS -- The data call start request was sent successfully. \n
    DSI_ERROR -- The data call start request was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl().
*/
/*=========================================================================*/
extern int dsi_start_data_call(dsi_hndl_t hndl);

/*===========================================================================
  FUNCTION:  dsi_stop_data_call
===========================================================================*/
/** @ingroup dsi_stop_data_call

    Stops a data call.

    An immediate call return value indicates whether the request was sent
    successfully. The client receives asynchronous notification via a callback
    registered with dsi_get_data_srvc_hndl() indicating the data call tear-down
    status.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().

    @return
    DSI_SUCCESS -- The data call stop request was sent successfully. \n
    DSI_ERROR -- The data call stop request was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl(). \n
    The call must have been brought up using dsi_start_data_call().
*/
/*=========================================================================*/
extern int dsi_stop_data_call(dsi_hndl_t hndl);

/*===========================================================================
  FUNCTION:  dsi_set_data_call_param
===========================================================================*/
/** @ingroup dsi_set_data_call_param

    Sets the data call parameter before trying to start a data call.

    Clients may call this function multiple times with various types of
    parameters to be set.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] identifier Identifies the type of the third input parameter.
    @param[in] info Parameter value that is to be set.

    @return
    DSI_SUCCESS -- The data call parameter was set successfully. \n
    DSI_ERROR -- The data call parameter was not set successfully.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl().
*/
/*=========================================================================*/
extern int dsi_set_data_call_param
(
  dsi_hndl_t hndl,
  dsi_call_param_identifier_t identifier,
  dsi_call_param_value_t *info
);

/*===========================================================================
  FUNCTION:  dsi_get_device_name
===========================================================================*/
/** @ingroup dsi_get_device_name

    Queries the data interface name for the data call associated with the
    specified data service handle.

    @note1hang
    len must be at least DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1 long.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[out] buf Buffer to hold the data interface name string.
    @param[in] len Length of the buffer allocated by client.

    @return
    DSI_SUCCESS -- The data interface name was returned successfully. \n
    DSI_ERROR -- The data interface name was not returned successfully.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl().
*/
/*=========================================================================*/
extern int dsi_get_device_name(dsi_hndl_t hndl, char * buf, int len);

/*===========================================================================
  FUNCTION:  dsi_get_call_end_reason
===========================================================================*/
/** @ingroup dsi_get_call_end_reason

    Queries for the reason for a call being ended.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[out] ce_reason Structure to hold the fields of the call ending
                           reason.
    @param[in] ipf IP family for which the call end reason was requested.

    @return
    DSI_SUCCESS -- The call end reason was queried successfully. \n
    DSI_ERROR -- The call end reason query was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl().
*/
/*=========================================================================*/
extern int dsi_get_call_end_reason
(
  dsi_hndl_t hndl,
  dsi_ce_reason_t * ce_reason,
  dsi_ip_family_t ipf
);

/*===========================================================================
  FUNCTION:  dsi_get_call_tech
===========================================================================*/
/** @ingroup dsi_get_call_tech

    Gets the techcology on which the call was brought up. This function can
    be called any time after the client receives the DSI_EVT_NET_IS_CONN event
    and before the client releases the dsi handle.

    On successful return, the call_tech parameter is set to a valid call
    technology.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[out] call_tech Pointer to the buffer containing the call
                           technology.

    @return
    DSI_SUCCESS -- The call bring-up technology was queried successfully. \n
    DSI_ERROR -- The call bring-up technology query was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl().
*/
/*=========================================================================*/
extern int dsi_get_call_tech
(
  dsi_hndl_t hndl,
  dsi_call_tech_type *call_tech
);

/*===========================================================================
  FUNCTION:  dsi_get_ip_addr_count
===========================================================================*/
/** @ingroup dsi_get_ip_addr_count

    Gets the number of IP addresses (IPv4 and global IPv6) associated with
    the DSI interface.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().

    @return
    The number of IP addresses associated with the DSI.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl()
*/
/*=========================================================================*/
extern unsigned int dsi_get_ip_addr_count
(
  dsi_hndl_t hndl
);

/*===========================================================================
  FUNCTION:  dsi_get_ip_addr
===========================================================================*/
/** @ingroup dsi_get_ip_addr

    Gets the IP address information structure (network order).

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[out] info_ptr Pointer to the buffer containing the network order
                          information.
    @param[in] len Length of the network order information.

    @return
    DSI_SUCCESS -- The network order query was successful. \n
    DSI_ERROR -- The network order query was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl(). \n
    The length parameter can be obtained by calling dsi_get_ip_addr_count(). \n
    It is assumed that the client has allocated memory for enough structures
    specified by the len field.
*/
/*=========================================================================*/
extern int dsi_get_ip_addr
(
  dsi_hndl_t hndl,
  dsi_addr_info_t * info_ptr,
  int len
);


/*===========================================================================
  FUNCTION:  dsi_iface_ioctl
===========================================================================*/
/*!
    @brief
    This function is in interface to get IOCTL values.

    @param[in] dsi_hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] one of the dsi_iface_ioctl_enum_t values.
    @param[out] argval_ptr Pointer to the buffer containing information.
                      for the requested ioctl.
    @param[out] dsi_err Valid only if the return values is DSI_ERROR.
                Will take one of the following values DSI_ERROR_HNDL_INVALID
                                                      DSI_ERROR_IOCTL_INVALID
                                                      DSI_ERROR_INTERNAL
    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
extern int dsi_iface_ioctl
(
  dsi_hndl_t dsi_hndl,
  dsi_iface_ioctl_enum_t ioctl_name,
  void *argval_ptr,
  int  *err_code
);

/*===========================================================================
  FUNCTION:  dsi_enable_port_forwarding
===========================================================================*/
/*!
    @brief
    This function can be used to enable port forwarding by installing
    the iptable rules appropriately. If no IWLAN calls are active then
    the client preference is saved and rules are installed on bring up
    of the first IWLAN call.

    @param[in] dsi_hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] ip_family - AF_INET/AF_INET6

    @return
    DSI_ERROR
    DSI_SUCCESS

    @dependencies
    There has to be at least one iwlan call up for the specified family
    for the API to take effect.
*/
/*=========================================================================*/
extern int dsi_enable_port_forwarding
(
   dsi_hndl_t dsi_hndl,
   int        ip_family
);

/*===========================================================================
  FUNCTION:  dsi_disable_port_forwarding
===========================================================================*/
/*!
    @brief
    This function can be used to disable port forwarding by uninstalling
    the iptable rules appropriately. If no IWLAN calls are active then
    the client preference is saved and rules will not be installed on
    bring up of the first IWLAN call.

    @param[in] dsi_hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] ip_family - AF_INET/AF_INET6

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
extern int dsi_disable_port_forwarding
(
   dsi_hndl_t dsi_hndl,
   int        ip_family
);

/*===========================================================================
  FUNCTION:  dsi_query_port_forwarding
===========================================================================*/
/*!
    @brief
    This function can be used to query the current port forwarding preference
    set by the client.

    @param[in] dsi_hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] ip_family - AF_INET/AF_INET6
    @param[out] forwarding_status return the current port forwarding status.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
extern int dsi_query_port_forwarding_status
(
   dsi_hndl_t                    dsi_hndl,
   int                           ip_family,
   dsi_port_forwarding_status_t* forwarding_status
);


/*===========================================================================
  FUNCTION:  dsi_embms_enable
===========================================================================*/
/*!
    @brief
    This function can be used to start an EMBMS data call.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
extern int dsi_embms_enable(dsi_hndl_t hndl);

/*===========================================================================
  FUNCTION:  dsi_embms_tmgi_activate
===========================================================================*/
/*!
    @brief
    This function can be used to activate TMGI on EMBMS data call.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
extern int dsi_embms_tmgi_activate
(
   dsi_hndl_t                          hndl,
   char                               *tmgi,
   unsigned int                       *earfcnlist,
   unsigned char                       earfcnlist_len,
   unsigned long                       preemption_priority,
   unsigned int                       *sailist,
   unsigned char                       sailist_len,
   int                                 dbg_trace_id
);

/*===========================================================================
  FUNCTION:  dsi_embms_tmgi_deactivate
===========================================================================*/
/*!
    @brief
    This function can be used to deactivate TMGI on EMBMS data call.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
extern int dsi_embms_tmgi_deactivate
(
   dsi_hndl_t                          hndl,
   char                               *tmgi,
   int                                 dbg_trace_id
);
/*===========================================================================
  FUNCTION:  dsi_embms_tmgi_activate_deactivate
===========================================================================*/
/*!
    @brief
    This function can be used to both activate TMGI and deactivate TMGI on
    EMBMS data call.

    TMGI to be activated must be different than TMGI to be deactivated

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
extern int dsi_embms_tmgi_activate_deactivate
(
  dsi_hndl_t                          hndl,
  char                               *activate_tmgi,
  unsigned int                       *earfcnlist,
  unsigned char                       earfcnlist_len,
  unsigned long                       preemption_priority,
  char                               *deactivate_tmgi,
  unsigned int                       *sailist,
  unsigned char                       sailist_len,
  int                                 dbg_trace_id
);

/*===========================================================================
  FUNCTION:  dsi_get_embms_tmgi_deactivate_reason
===========================================================================*/
/*!
    @brief
    used to get tmgi deactivated reason.

    @return
    DSI_ERROR
    DSI_SUCCESS

*/
/*=========================================================================*/
extern int dsi_embms_get_tmgi_deactivate_reason
(
  dsi_hndl_t                                hndl,
  dsi_embms_tmgi_status_field_type          status_field,
  int                                      *dsi_tmgi_deactivate_reason_ptr
);

/*===========================================================================
  FUNCTION:  dsi_embms_tmgi_list_query
===========================================================================*/
/*!
    @brief
    used to query TMGI list(active or available).

    @return
    DSI_ERROR
    DSI_SUCCESS

*/
/*=========================================================================*/
extern int dsi_embms_tmgi_list_query
(
  dsi_hndl_t                hndl,
  dsi_embms_tmgi_list_type  embms_tmgi_list_query,
  dsi_embms_tmgi_info_type *embms_tmgi_ptr,
  int                       dbg_trace_id
);

/*===========================================================================
  FUNCTION:  dsi_embms_content_desc_update
===========================================================================*/
/*!
    @brief
    used to update embms content desc.

    @return
    DSI_ERROR
    DSI_SUCCESS

*/
/*=========================================================================*/
int dsi_embms_content_desc_update
(
  dsi_hndl_t                hndl,
  char                     *content_desc_update_tmgi,
  unsigned char             content_desc_valid,
  unsigned int              content_desc_len,
  embms_content_desc_type  *content_desc,
  int                       dbg_trace_id
);

/*===========================================================================
  FUNTION:   dsi_get_current_data_channel_rate
===========================================================================*/
/*!
    @brief
    Returns the current channel rate of the call

    @param
    hndl - dsi interface handle

    @return
    DSI_ERROR
    DSI_SUCCESS

*/
/*=========================================================================*/
int dsi_get_current_data_channel_rate
(
  dsi_hndl_t               hndl,
  dsi_data_channel_rate_t *data_rate
);

/*===========================================================================
  FUNCTION:  dsi_get_current_data_bearer_tech
===========================================================================*/
/** @ingroup dsi_get_current_data_bearer_tech

    Returns the current data bearer technology on which a call was brought up.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().

    @return
    The data bearer technology.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl().
*/
/*=========================================================================*/
dsi_data_bearer_tech_t
dsi_get_current_data_bearer_tech
(
  dsi_hndl_t hndl
);

/*===========================================================================
  FUNCTION:  dsi_reset_pkt_stats
===========================================================================*/
/** @ingroup dsi_reset_pkt_stats

    Resets the packet data transfer statistics.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().

    @return
    DSI_SUCCESS -- The packet data transfer statistics were reset successfully. \n
    DSI_ERROR -- The packet data transfer statistics reset was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl().
*/
/*=========================================================================*/
extern int
dsi_reset_pkt_stats
(
  dsi_hndl_t  hndl
);


/*===========================================================================
  FUNCTION:  dsi_get_pkt_stats
===========================================================================*/
/** @ingroup dsi_get_pkt_stats

    Queries the packet data transfer statistics from the current packet
    data session.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] dsi_data_stats Memory to hold the queried statistics details.

    @return
    DSI_SUCCESS -- The packet data transfer statistics were queried successfully. \n
    DSI_ERROR -- The packet data transfer statistics query was unsuccessful.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl()
*/
/*=========================================================================*/
extern int
dsi_get_pkt_stats
(
  dsi_hndl_t          hndl,
  dsi_data_pkt_stats *dsi_data_stats
);

/* These error codes should never overlap with EAI_*** codes */
#define DSI_ERROR_BAD_HANDLE -100
#define DSI_NO_DNS_RESOLVER  -101
#define DSI_DNS_ERROR        -102

/*===========================================================================
  FUNCTION:  dsi_get_qmi_port_name
===========================================================================*/
/** @ingroup dsi_get_qmi_port_name

    Queries the QMI port name for the data call associated with the
    specified data service handle.

    @note1hang
    len must be at least DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1 long.

    @param[in] hndl Handle received from dsi_get_data_srvc_hndl().
    @param[out] buf Buffer to hold the QMI port name string.
    @param[in] len Length of the buffer allocated by client.

    @return
    DSI_SUCCESS -- The data interface name was returned successfully. \n
    DSI_ERROR -- The data interface name was not returned successfully.

    @dependencies
    dsi_init() must be called. \n
    The handle must be a valid handle obtained by dsi_get_data_srvc_hndl().
*/
/*=========================================================================*/
extern int dsi_get_qmi_port_name(dsi_hndl_t hndl, char * buf, int len);

/*===========================================================================
  FUNCTION:  dsi_process_screen_state_change
===========================================================================*/
/** @ingroup dsi_process_screen_state_change

    Performs any optimization processing when the screen state is turned off.

    @param[in] screen_state Screen state. 0 - OFF, 1 - ON

    @return
    DSI_SUCCESS -- The API returned success.
    DSI_ERROR -- The API returned failure.

    @dependencies
    dsi_init() must be called.
*/
/*=========================================================================*/
extern int dsi_process_screen_state_change(int screen_state);

/*===========================================================================
  FUNCTION:  dsi_get_link_mtu
===========================================================================*/
/*!
    @brief
    Used to get the MTU of the corresponding link

    @return
    DSI_ERROR
    DSI_SUCCESS

*/
/*=========================================================================*/
extern int dsi_get_link_mtu
(
  dsi_hndl_t    hndl,
  unsigned int  *mtu
);

#endif /* DSI_NETCTRL_H */
#ifdef __cplusplus
}
#endif

