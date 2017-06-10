#ifndef CNE_DEFS_H
#define CNE_DEFS_H

/**----------------------------------------------------------------------------
  @file CNE_Defs.h

  This file holds various definations that get used across, different CNE
  modules.
-----------------------------------------------------------------------------*/


/*============================================================================
               Copyright (c) 2009-2014 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Proprietary and Confidential.
============================================================================*/


/*============================================================================
  EDIT HISTORY FOR MODULE

  $Header: //depot/asic/sandbox/projects/cne/common/core/inc/CneDefs.h#7 $
  $DateTime: 2009/11/20 17:36:15 $
  $Author: chinht $
  $Change: 1092637 $

  when        who   what, where, why
  ----------  ---   -------------------------------------------------------
  2009-07-15  ysk   First revision.
  2011-07-27  jnb   Added more definitions
  2011-10-28  tej   Updated max application macro and CneAppInfoMsgDataType
  2012-03-09  mtony Added include file, stdint.h, which is directly needed
============================================================================*/
#include <string>
#include <set>
/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/**
 * Possible return codes
 *
 * New values should be added to CneUtils::init()
 */
typedef enum
{
  /* ADD other new error codes here */
  CNE_RET_SERVICE_NOT_AVAIL = -13,
  CNE_RET_ASYNC_RESPONSE = -12,
  CNE_RET_ERR_READING_FILE_STAT = -11,
  CNE_RET_PARSER_NO_MATCH = -10,
  CNE_RET_PARSER_VALIDATION_FAIL = -9,
  CNE_RET_PARSER_TRAVERSE_FAIL = -8,
  CNE_RET_PARSER_PARSE_FAIL = -7,
  CNE_RET_ERR_OPENING_FILE = -6,
  CNE_RET_INVALID_DATA = -5,
  CNE_RET_OUT_OF_MEM = -4,
  CNE_RET_ALREADY_EXISTS = -3,
  CNE_RET_NOT_ALLOWED_NOW = -2,
  CNE_RET_ERROR = -1,

  CNE_RET_OK = 1,
  CNE_RET_PARSER_MATCH = 2
} CneRetType;

#ifndef MAX
   #define  MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif /* MAX */

#ifndef MIN
   #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif /* MIN */


#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

/*----------------------------------------------------------------------------
 * Include C Files
 * -------------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdint.h>

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#ifndef TRUE
  /** Boolean true value. */
  #define TRUE   1
#endif /* TRUE */

#ifndef FALSE
  /** Boolean false value. */
  #define FALSE  0
#endif /* FALSE */

#ifndef NULL
/** NULL */
  #define NULL  0
#endif /* NULL */

#define CNE_IPA_IFACE_NAME_MAX 20 // Mirror with IPA_RESOURCE_NAME_MAX in msm_ipa.h

#define CNE_MAX_SSID_LEN 32
// Max BSSID size is 64 bits (EUI-64). We receive this value as a string in human readable format:
// (00:00:00:00:00:00:00:00). There are 23 chars + null termination char + 1 reserved = 25.
#define CNE_MAX_BSSID_LEN 25
#define CNE_MAX_SCANLIST_SIZE 40
#define CNE_MAX_APPLIST_SIZE 500
#define CNE_MAX_IPADDR_LEN 46
#define CNE_MAX_IFACE_NAME_LEN 16
#define CNE_MAX_TIMESTAMP_LEN 32
#define CNE_MAX_CAPABILITIES_LEN 256
#define CNE_MAX_URI_LEN 128
#define CNE_MAX_BQE_FILESIZE_LEN 10
#define CNE_MAX_VENDOR_DATA_LEN 512
#define CNE_SERVICE_DISABLED 0
#define CNE_SERVICE_ENABLED 1
#define CNE_MAX_BROWSER_APP_LIST 40
#define CNE_MAX_DNS_ADDRS 2
#define CNE_MAX_MCC_MNC_LEN 7 //6 for mccmnc number + 1 for null termination

#define CNE_APP_NAME_MAX_LEN 256
#define CNE_HASHES_MAX_LEN 256 //TODO

#define CNE_FEATURE_IWLAN_PROP "persist.sys.cnd.iwlan"
#define CNE_FEATURE_WQE_PROP "persist.sys.cnd.wqe"
#define CNE_FEATURE_NSRM_PROP "persist.sys.cnd.nsrm"
#define CNE_FEATURE_WQE_CQE_TIMER_PROP "persist.cne.cqetimer"

#define BSSID_PLACEHOLDER "00:00:00:00:00:00"

#define CND_RET_CODE_OK 0
#define CND_RET_CODE_UNKNOWN_ERROR -1
#define CND_RET_CODE_INVALID_DATA -2

#define STRUCT_PACKED __attribute__ ((packed))

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

typedef uint32_t u32;

/**
  This is a type for representing the Requests, Notifications that could be
  sent to CND.

  New values should also be added to CneUtils::init()
 */
typedef enum
{
  CNE_REQUEST_INIT_CMD=1,
    /**< Command to initialise the internal CNE modules.*/
  CNE_REQUEST_REG_ROLE_CMD=2,
    /**< Command for registering a role that the clients wants to play. */
  CNE_REQUEST_GET_COMPATIBLE_NWS_CMD=3,
    /**< Command to request a list of Nws for a registered role. */
  CNE_REQUEST_CONFIRM_NW_CMD=4,
    /**< Command to confirm whether the given networks list is ok. */
  CNE_REQUEST_DEREG_ROLE_CMD=5,
    /**< Command to deregister the role, once the client is done with it. */
  CNE_REQUEST_REG_NOTIFICATIONS_CMD=6,
    /**< Command to register a notifications call back, if the client does
         does not want to get notified via the default mechanism for that
         platform
     */
  CNE_REQUEST_UPDATE_BATTERY_INFO_CMD=7,
  CNE_REQUEST_UPDATE_WLAN_INFO_CMD=8,
  CNE_REQUEST_UPDATE_WWAN_INFO_CMD=9,
  CNE_NOTIFY_RAT_CONNECT_STATUS_CMD=10,
  CNE_NOTIFY_DEFAULT_NW_PREF_CMD=11,
    /**< Command to notify CNE about the uppper layer default nw preference. */
  CNE_REQUEST_UPDATE_WLAN_SCAN_RESULTS_CMD=12,

  CNE_NOTIFY_SENSOR_EVENT_CMD=13,
  CNE_REQUEST_CONFIG_IPROUTE2_CMD=14,


  CNE_NOTIFY_TIMER_EXPIRED_CMD=15,
  CNE_REQUEST_START_FMC_CMD=16,
  CNE_REQUEST_STOP_FMC_CMD=17,
  CNE_REQUEST_UPDATE_WWAN_DORMANCY_INFO_CMD=18,
  CNE_REQUEST_UPDATE_DEFAULT_NETWORK_INFO_CMD=19,
  CNE_NOTIFY_SOCKET_CLOSED_CMD=20,
  CNE_NOTIFY_ICD_RESULT=21,
  CNE_NOTIFY_NSRM_STATE_CMD=22,
   /** Either in response to CNE_REQUEST_GET_APP_INFO_LIST or as event when appinfo list updated */
  CNE_NOTIFY_APP_INFO_LIST_CMD=23,
  CNE_NOTIFY_WLAN_CONNECTIVITY_UP_CMD=24,
  CNE_NOTIFY_JRTT_RESULT=25,
  CNE_NOTIFY_BQE_POST_RESULT=26,
  CNE_NOTIFY_ICD_HTTP_RESULT=27,
  CNE_NOTIFY_ANDSF_DATA_READY=28,
  CNE_NOTIFY_BROWSERS_INFO_LIST_CMD=29,
  /** Request for feature status from java */
  CNE_REQ_GET_FEATURE_STATUS = 30,
  /** Request for feature preference update from java */
  CNE_REQ_SET_FEATURE_PREF = 31,
  CNE_NOTIFY_NSRM_CONFIG_READY=32,
  /** Result of the parent app request for nested apps for ATP */
  CNE_NOTIFY_ATP_GET_PARENT_APP_RESULT = 33,
  CNE_NOTIFY_SCREEN_STATE_CMD=34,
  /* Tethering Upstream information to provide to IPA driver */
  CNE_NOTIFY_TETHERING_UPSTREAM_INFO_CMD=35,
  CNE_NOTIFY_FWMARK_INFO = 36,

/**
    Add other commands here, note these should match with the ones in the
    java layer.

    CNE_REQUEST_VENDOR_CMD should always be last cmd in this enum
   */
  CNE_REQUEST_VENDOR_CMD

} cne_cmd_enum_type;

typedef cne_cmd_enum_type CneEvent;

/**
  This is a type for representing the expected events/responses, requests that
  the CND can send to the clients and the upperlayer connectionManager.

  New values should also be added to CneUtils::init()
 */
typedef enum
{

  CNE_RESPONSE_REG_ROLE_MSG = 1,
    /**< Response for the register role command. */
  CNE_RESPONSE_GET_COMPATIBLE_NWS_MSG = 2,
    /**< Response for the get compatible nws command. */
  CNE_RESPONSE_CONFIRM_NW_MSG = 3,
    /**< Response for the confirm Nw command. */
  CNE_RESPONSE_DEREG_ROLE_MSG = 4,
    /**< Response for the deregister role command. */
  CNE_REQUEST_BRING_RAT_DOWN_MSG = 5,
  CNE_REQUEST_BRING_RAT_UP_MSG = 6,
  CNE_NOTIFY_MORE_PREFERED_RAT_AVAIL_MSG = 7,
    /**< Notifications sent to the registered clients about a more prefered NW
         availability.
     */
  CNE_NOTIFY_RAT_LOST_MSG = 8,
    /**< Notification sent to clients when the RAT they are using is lost.*/
  CNE_REQUEST_START_SCAN_WLAN_MSG = 9,
  CNE_NOTIFY_INFLIGHT_STATUS_MSG = 10,
  CNE_NOTIFY_FMC_STATUS_MSG = 11,
  CNE_NOTIFY_HOST_ROUTING_IP_MSG = 12,
  /**
    CNE_NOTIFY_VENDOR_MSG should always be last msg in this enum
   */
  CNE_NOTIFY_VENDOR_MSG = 13,
  /** SWIM message including disallowed WiFi Access Point BSSIDs */
  CNE_NOTIFY_DISALLOWED_AP_MSG = 14,
  /** SWIM message to the java layer to start active probing for
   *  BQE analysis */
  CNE_REQUEST_START_ACTIVE_PROBE = 15,
  /** SWIM message to the java layer to add default route */
  CNE_REQUEST_SET_DEFAULT_ROUTE_MSG = 16,
  /** SWIM message to the java layer to start ICD */
  CNE_REQUEST_START_ICD = 17,
  CNE_REQUEST_GET_APP_INFO_LIST = 18,
  CNE_NOTIFY_DNS_PRIORITY_CMD = 19,
  CNE_REQUEST_STOP_ACTIVE_PROBE = 20,
  CNE_NOTIFY_ACCESS_DENIED = 21,
  CNE_REQUEST_POST_BQE_RESULTS = 22,
  CNE_NOTIFY_NSRM_BLOCKED_UID = 23,
  CNE_REQUEST_GET_BROWSERS_INFO_LIST = 24,
  /** Indication to clients that write to socket is allowed */
  CNE_NOTIFY_ACCESS_ALLOWED = 25,
  CNE_NOTIFY_FEATURE_STATUS = 26,
  CNE_RESP_SET_FEATURE_PREF = 27,
  /** Indication to clients that nsrm config update is done. */
  CNE_NOTIFY_POLICY_UPDATE_DONE = 28,
  /** request parent app info for a nested app for ATP   */
  CNE_REQUEST_ATP_GET_PARENT_APP = 29
} cne_msg_enum_type;

/*
 * Correspond to network State defined in NetworkInfo.java
 * New values should also be added to CneUtils::init()
 */
typedef enum // correspond to network State defined in NetworkInfo.java
{
  CNE_NETWORK_STATE_CONNECTING = 0,
  CNE_NETWORK_STATE_CONNECTED,
  CNE_NETWORK_SUSPENDED,
  CNE_NETWORK_DISCONNECTING,
  CNE_NETWORK_DISCONNECTED,
  CNE_NETWORK_UNKNOWN
} cne_network_state_enum_type;

typedef enum
{
  CNE_IPROUTE2_ADD_ROUTING = 0,
  CNE_IPROUTE2_DELETE_ROUTING,
  CNE_IPROUTE2_DELETE_DEFAULT_IN_MAIN,
  CNE_IPROUTE2_REPLACE_DEFAULT_ENTRY_IN_MAIN,
  CNE_IPROUTE2_ADD_HOST_IN_MAIN,
  CNE_IPROUTE2_REPLACE_HOST_DEFAULT_ENTRY_IN_MAIN,
  CNE_IPROUTE2_DELETE_HOST_ROUTING,
  CNE_IPROUTE2_DELETE_HOST_DEFAULT_IN_MAIN,
  CNE_IPROUTE2_DELETE_HOST_IN_MAIN
} cne_iproute2_cmd_enum_type;

typedef enum
{
  CNE_FMC_STATUS_ENABLED = 0,
  CNE_FMC_STATUS_CLOSED,
  CNE_FMC_STATUS_INITIALIZED,
  CNE_FMC_STATUS_SHUTTING_DOWN,
  CNE_FMC_STATUS_NOT_YET_STARTED,
  CNE_FMC_STATUS_FAILURE,
  CNE_FMC_STATUS_NOT_AVAIL,
  CNE_FMC_STATUS_DS_NOT_AVAIL,
  CNE_FMC_STATUS_RETRIED,
  CNE_FMC_STATUS_REGISTRATION_SUCCESS,
  CNE_FMC_STATUS_MAX
} cne_fmc_status_enum_type;

typedef enum
{
  CNE_DAP_REASON_FIREWALLED = 0,
  CNE_DAP_REASON_CONGESTED,
  CNE_DAP_REASON_NA //Not Applicable
} cne_dap_reason_enum_type;


//Nsrm feature related enums
typedef enum
{
  CNE_NSRM_WWAN_DATA_CONN_STATE_EVT = 0,
  CNE_NSRM_SCREEN_STATE_EVT,
  CNE_NSRM_GPS_STATE_EVT,
  CNE_NSRM_USB_STATE_EVT,
  CNE_NSRM_HEADSET_STATE_EVT,
  CNE_NSRM_BLUETOOTH_STATE_EVT,
  CNE_NSRM_MUSIC_ACTIVE_STATE_EVT,
  CNE_NSRM_MICROPHONE_MUTE_STATE_EVT,
  CNE_NSRM_HDMI_STATE_EVT,
  CNE_NSRM_WLAN_STATE_EVT,
  CNE_NSRM_SPEAKER_STATE_EVT,
  CNE_NSRM_EMERGENCY_ALERT_STATE_EVT,
  CNE_NSRM_EVT_MAX
} cne_background_event_enum_type;

/** Role Id Type. */
typedef int32_t cne_role_id_type;
/** Registration Id Type. */
typedef int32_t cne_reg_id_type;
/** BandWidth type */
typedef uint32_t cne_link_bw_type;

/**
  A call back funtion type that the clients would register with CNE if they
  do not want to be notified via the default mechanisim used for that platform.
 */
typedef void (*cne_event_notif_cb_type)
(
  cne_msg_enum_type event,
  void *event_data_ptr,
  void *cb_data_ptr
);

/**
* A call back funtion type that cnd register with CNE to be
* called by CNE when it wants to send unsolicited message.
  */
typedef void (*cne_messageCbType)
(
  int targetFd,
  int msgType,
  int dataLen,
  void *data
);

/**
  This is a type representing the list of possible RATs

  New values should also be added to CneUtils::init()
 */
typedef enum
{
  CNE_RAT_MIN = 0, //For tagging only
  CNE_RAT_WWAN = CNE_RAT_MIN,
  CNE_RAT_WLAN,
  /* any new rats should be added here */
  CNE_RAT_ANY,
  /**< Any of the above RATs */
  CNE_RAT_NONE,
  /**< None of the abvoe RATs */
  CNE_RAT_MAX, //For tagging only
  /** @internal */
  CNE_RAT_INVALID = CNE_RAT_MAX,
  /**< INVALID RAT */

}cne_rat_type;

/**
 * represents battery status, values should match
 * BatteryManager.java
 */
typedef enum
{
  CNE_BATTERY_STATUS_UNKNOWN = 1,
  CNE_BATTERY_STATUS_CHARGING,
  CNE_BATTERY_STATUS_DISCHARGING,
  CNE_BATTERY_STATUS_NOT_CHARGING,
  CNE_BATTERY_STATUS_FULL
} cne_battery_status;

/**
 * represets battery level
 */
typedef enum
{
  CNE_BATTERY_LEVEL_MIN = 0,
  CNE_BATTERY_LEVEL_MAX = 100
} cne_battery_level;

/**
 * represets charger type, values should match
 * BatteryManager.java
 */
typedef enum
{
  CNE_BATTERY_PLUGGED_NONE,
  CNE_BATTERY_PLUGGED_AC,
  CNE_BATTERY_PLUGGED_USB
} cne_battery_charger_type;

typedef enum
{
    CNE_FEATURE_OFF = 1,
    CNE_FEATURE_ON = 2,
    CNE_FEATURE_NSRM_OFF = 1,
    CNE_FEATURE_NSRM_CONNECT_DNS = 2,
    CNE_FEATURE_NSRM_CONNECT_DNS_WRITE = 3,
    CNE_FEATURE_STATUS_UNKNOWN = 65535
} cne_feature_status;

/** represents different policy types.
 *  Should match with values defined in CNE.java
 */
typedef enum
{
  CNE_POLICY_ANDSF = 1,
  CNE_POLICY_NSRM = 2,
  CNE_POLICY_UNKOWN = 3,
  CNE_POLICY_MAX = CNE_POLICY_UNKOWN
}cne_policy_type;

/**
 This is a type representing the list of possible subRATs.
 Always in sync with TelephonyManager.java

 New value should also be added to CneUtils::init()
 */
typedef enum
{

  CNE_NET_SUBTYPE_UNKNOWN = 0,
  /* Sub type GPRS */
  CNE_NET_SUBTYPE_GPRS,
  /* Sub type EDGE */
  CNE_NET_SUBTYPE_EDGE,
  /* Sub type UMTS */
  CNE_NET_SUBTYPE_UMTS,
  /* Sub type CDMA IS-95 */
  CNE_NET_SUBTYPE_CDMA,
  /* Sub type EVDO Rev 0 */
  CNE_NET_SUBTYPE_EVDO_0,
  /* Sub type EVDO Rev A */
  CNE_NET_SUBTYPE_EVDO_A,
  /* Sub type 1x RTT */
  CNE_NET_SUBTYPE_1xRTT,
  /* Sub type HSDPA */
  CNE_NET_SUBTYPE_HSDPA,
  /* Sub type HSUPA */
  CNE_NET_SUBTYPE_HSUPA,
  /* Sub type HSPA */
  CNE_NET_SUBTYPE_HSPA,
  /* Sub type IDEN */
  CNE_NET_SUBTYPE_IDEN,
  /* Sub type EVDO Rev B */
  CNE_NET_SUBTYPE_EVDO_B,
  /* Sub type LTE */
  CNE_NET_SUBTYPE_LTE,
  /* Sub type EHRPD */
  CNE_NET_SUBTYPE_EHRPD,
  /* Sub type HSPA + */
  CNE_NET_SUBTYPE_HSPAP,

  //CNE Defines for WLAN subtypes not in TelephonyManager.java
  /* Sub type 802.11 B */
  CNE_NET_SUBTYPE_WLAN_B = 20,
  /* Sub type 802.11 G */
  CNE_NET_SUBTYPE_WLAN_G
} cne_rat_subtype;

typedef enum // correspond to WIFI_AP_STATE defined in WifiManager.java
{
  CNE_SOFTAP_STATE_DISABLING=0,
  CNE_SOFTAP_STATE_DISABLED,
  CNE_SOFTAP_STATE_ENABLING,
  CNE_SOFTAP_STATE_ENABLED,
  CNE_SOFTAP_STATE_FAILED,
  CNE_SOFTAP_STATE_UNKNOWN=65535,
} cne_softApStatus_type;

typedef enum
{
  CNE_FEATURE_WQE = 1,
  CNE_FEATURE_IWLAN,
  CNE_FEATURE_NSRM,
  CNE_FEATURE_UNKNOWN=65535,
} cne_feature_type;

typedef enum {
  CNE_PKG_ACTION_MIN = 0,
  CNE_PKG_ACTION_ADD = CNE_PKG_ACTION_MIN,
  CNE_PKG_ACTION_REMOVE,
  CNE_PKG_ACTION_MAX = CNE_PKG_ACTION_REMOVE
} CnePkgActionType;

typedef enum {
  CNE_FAM_MIN = 0,
  CNE_FAM_NONE = CNE_FAM_MIN,
  CNE_FAM_V4,
  CNE_FAM_V6,
  CNE_FAM_V4_V6,
  CNE_FAM_MAX = CNE_FAM_V4_V6
} cne_fam_type;


/* cmd handlers will pass the cmd data as raw bytes.
 * the bytes specified below are for a 32 bit machine
 */
/** @note
   BooleanNote: the daemon will receive the boolean as a 4 byte integer
   cne may treat it as a 1 byte internally
 */
/**
  Command data structure to be passed for the CNE_REQUEST_REG_ROLE_CMD
 */
typedef struct
{
  cne_role_id_type role_id;
  /**< role Id 4 bytes */
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
  cne_link_bw_type fwd_link_bw;
  /**< forward link band width 4 bytes */
  cne_link_bw_type rev_link_bw;
  /**< reverse link band width 4 bytes */
} cne_reg_role_cmd_args_type;

/**
  Command data structure to be passed for the CNE_REQUEST_DEREG_ROLE_CMD
 */
typedef struct
{
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
} cne_dereg_role_cmd_args_type;

/**
  Command data structure to be passed for the CNE_REQUEST_GET_COMPATIBLE_NWS_CMD
 */
typedef struct
{
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
} cne_get_compatible_nws_cmd_args_type;

/**
  Command data structure to be passed for the CNE_REQUEST_REG_NOTIFICATIONS_CMD
 */
typedef struct
{
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
  cne_event_notif_cb_type cb_fn_ptr;
  /**< notification call back function pointer 4 bytes */
  void* cb_data_ptr;
  /**< call back data pointer 4 bytes */
} cne_reg_notifs_cmd_args_type;

/**
  Command data structure to be passed for the CNE_REQUEST_CONFIRM_NW_CMD
 */
typedef struct
{
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
  cne_rat_type rat;
  /**< rat to confirmed 4 bytes */
  uint8_t is_rat_ok;
  /**< was the rat given ok? TRUE if satisfied else FALSE 1 byte.
   */
  uint8_t is_notif_if_better_rat_avail;
  /**< TRUE if notifications be sent on better rat availability 1 byte */
  cne_rat_type new_rat;
  /**< if not satisfied with the given rat what is the new rat that you would
       like. 4 bytes
   */
} cne_confirm_nw_cmd_args_type;



/**
  Response info structure returned for the response CNE_RESPONSE_REG_ROLE_MSG
 */
typedef struct
{
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
  uint8_t is_success;
  /**< TRUE if the request was successful. 1 byte */
} cne_reg_role_rsp_evt_data_type;


/**
 * Response info structure returned for the response
 * CNE_RESPONSE_GET_COMPATIBLE_NWS_MSG
 */
typedef struct
{
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
  uint8_t is_success;
  /**< TRUE if the request was successful. 1 byte */
  /* if success send the rat info */
  cne_rat_type best_rat;
  /**< this is the best rat for this registration/role. 4 bytes */
  /* to do other ratInfo */
  cne_rat_type rat_pref_order[CNE_RAT_MAX];
  /**< Other Compatible RATs. CNE_RAT_MAX*4 bytes */
  char ip_addr[CNE_MAX_IPADDR_LEN];
  /**< IP Address of the best RAT in doted decimal format.*/
  uint32_t fl_bw_est;
  /**< forward link bandwidth estimate of the preffered RAT in kbps */
  uint32_t rl_bw_est;
  /**< reverse link bandwidth estimate of the preffered RAT in kbps */
} cne_get_compatible_nws_evt_rsp_data_type;


/**
  Response info structure returned for the response CNE_RESPONSE_CONFIRM_NW_MSG
 */
typedef struct
{
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
  uint8_t is_success;
  /**< TRUE if the request was successful. 1 byte */
} cne_confirm_nw_evt_rsp_data_type;


/**
  Response info structure returned for the response CNE_RESPONSE_DEREG_ROLE_MSG
 */
typedef struct
{
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
  uint8_t is_success;
  /**< TRUE if the request was successful. 1 byte */
} cne_dereg_role_evt_rsp_data_type;


/**
  Response info structure returned for the event CNE_NOTIFY_RAT_LOST_MSG
 */
typedef struct
{
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
  cne_rat_type rat;
  /**< Rat type which has lost the connectivity. 4 bytes */
} cne_rat_lost_evt_data_type;


/**
  Response info structure returned for the event
  CNE_NOTIFY_MORE_PREFERED_RAT_AVAIL_MSG
 */
typedef struct
{
  cne_reg_id_type reg_id;
  /**< regestration  Id 4 bytes */
  cne_rat_type rat;
  /**< Rat type which is better for this registration/role 4 bytes */
  char ip_addr[CNE_MAX_IPADDR_LEN];
  /**< IP Address of the preffered RAT in doted decimal format.*/
  uint32_t fl_bw_est;
  /**< forward link bandwidth estimate of the preffered RAT in kbps */
  uint32_t rl_bw_est;
  /**< reverse link bandwidth estimate of the preffered RAT in kbps */

} cne_pref_rat_avail_evt_data_type;

/**
  Response info structure returned for the event
  CNE_NOTIFY_INFLIGHT_STATUS_MSG
 */
typedef struct
{
  uint8_t is_flying;
  /**< true if in flight else false */
} cne_inflight_status_change_evt_data_type;


/**
  Info structure returned for the event
  CNE_NOTIFY_FMC_STATUS_MSG
 */
typedef struct
{
  uint8_t status;
} cne_fmc_status_evt_data_type;

/**
 Request info structure sent by CNE for the request
 CNE_REQUEST_SET_DEFAULT_ROUTE_MSG
 */
typedef struct
{
  cne_rat_type rat;
} cne_set_default_route_req_data_type;

typedef struct {
  int32_t size;
  cne_rat_type priorityList[CNE_RAT_MAX];
} CneDnsPriorityType;

typedef union {
    cne_rat_type rat;
    struct {
        cne_rat_type rat;
        char ssid[CNE_MAX_SSID_LEN];
    } wlan;
    struct {
        cne_rat_type rat;
    } wwan;
} CneRatInfoType;

typedef struct  _WlanInfo{
    int32_t type;
    int32_t status;
    cne_softApStatus_type softApStatus;
    int32_t rssi;
    char ssid[CNE_MAX_SSID_LEN];
    char bssid[CNE_MAX_BSSID_LEN];
    char ipAddr[CNE_MAX_IPADDR_LEN];
    char iface[CNE_MAX_IFACE_NAME_LEN];
    char ipAddrV6[CNE_MAX_IPADDR_LEN];
    char ifaceV6[CNE_MAX_IFACE_NAME_LEN];
    char timeStamp[CNE_MAX_TIMESTAMP_LEN];
    char dnsInfo[CNE_MAX_DNS_ADDRS][CNE_MAX_IPADDR_LEN];
    _WlanInfo(){
      for (int i = 0; i < CNE_MAX_DNS_ADDRS; i++)
      {
        memset(dnsInfo[i], 0, CNE_MAX_IPADDR_LEN);
      }
    }
} CneWlanInfoType;

typedef struct  {
    int32_t type;
    int32_t status;
    int32_t rssi;
    int32_t roaming;
    char ipAddr[CNE_MAX_IPADDR_LEN];
    char iface[CNE_MAX_IFACE_NAME_LEN];
    char ipAddrV6[CNE_MAX_IPADDR_LEN];
    char ifaceV6[CNE_MAX_IFACE_NAME_LEN];
    char timeStamp[CNE_MAX_TIMESTAMP_LEN];
    char mccMnc[CNE_MAX_MCC_MNC_LEN];
} CneWwanInfoType;

typedef struct {
    int32_t level;
    int32_t frequency;
    char ssid[CNE_MAX_SSID_LEN];
    char bssid[CNE_MAX_BSSID_LEN];
    char capabilities[CNE_MAX_CAPABILITIES_LEN];
}CneWlanScanListInfoType;

typedef struct  {
    char packageName[CNE_APP_NAME_MAX_LEN];
}CneBrowserAppListInfoType;

typedef struct  {
    int numItems;
    CneWlanScanListInfoType scanList[CNE_MAX_SCANLIST_SIZE];
} CneWlanScanResultsType;

typedef struct {
    int numItems;
    CneBrowserAppListInfoType appList[CNE_MAX_BROWSER_APP_LIST];
} CneBrowserAppType;

typedef struct {
    cne_rat_type rat;
    cne_network_state_enum_type ratStatus;
    char ipAddr[CNE_MAX_IPADDR_LEN];
    char ipAddrV6[CNE_MAX_IPADDR_LEN];
} CneRatStatusType;

typedef struct {
  uint32_t fd;
  uint32_t length;
  uint8_t data[CNE_MAX_VENDOR_DATA_LEN];
} CneVendorType;

typedef struct  {
    int32_t cmd;
    char ifName[CNE_MAX_IFACE_NAME_LEN];
    char ipAddr[CNE_MAX_IPADDR_LEN];
    char gatewayAddr[CNE_MAX_IPADDR_LEN];
} CneIpRoute2CmdType;

typedef struct {
    uint8_t disallowed; /// bool
    uint8_t reason; /// Congested/Firewalled
    char bssid[CNE_MAX_BSSID_LEN];
} CneDisallowedAPType;

typedef struct {
    int32_t uid;
    uint8_t isBlocked;
} CneNsrmBlockedUidType;

/* data structure used to for the parent app request and result for ATP */
typedef struct {
  int cookie;
  char childAppName[CNE_APP_NAME_MAX_LEN+1];
  uid_t parentUid;
} CneAtpParentAppInfoMsg_t;

typedef struct {
  char bssid[CNE_MAX_BSSID_LEN];
  char uri[CNE_MAX_URI_LEN];
  char httpuri[CNE_MAX_URI_LEN];
  char fileSize[CNE_MAX_BQE_FILESIZE_LEN];
} CneBQEActiveProbeMsgType;

typedef struct {
    char uri[CNE_MAX_URI_LEN];
    char httpuri[CNE_MAX_URI_LEN];
    char bssid[CNE_MAX_BSSID_LEN];
    uint32_t timeout;
    uint32_t tid;
} CneIcdStartMsgType;

typedef struct {
    char bssid[CNE_MAX_BSSID_LEN];
    uint8_t result;
    uint8_t flags;
    uint32_t tid;
    uint32_t icdQuota;
    uint8_t icdProb;
    uint32_t bqeQuota;
    uint8_t bqeProb;
    uint32_t mbw;
    uint32_t tputDl;
    uint32_t tputSdev;
} CneIcdResultCmdType;

typedef struct {
    char bssid[CNE_MAX_BSSID_LEN];
    uint8_t result;
    uint32_t tid;
    int family;
} CneIcdHttpResultCmdType;

typedef struct {
	int32_t type;
	int32_t state;
} CneStateType;

typedef struct _CnePkgDataType{
  char pkg_name[CNE_APP_NAME_MAX_LEN];
  uint32_t uid;
  char hashes[CNE_HASHES_MAX_LEN];

  bool operator<(const _CnePkgDataType& rhs) const
  {
    return uid < rhs.uid;
  }

  bool operator==(const _CnePkgDataType& rhs) const
  {
    return uid == rhs.uid;
  }
} CnePkgDataType;

typedef struct {
    CnePkgActionType action;
    uint16_t numOfApp;
    std::multiset< CnePkgDataType > pkg_data;
} CneAppInfoMsgDataType;

typedef struct {
    uint32_t result;
    uint32_t jrttMillis;
    uint32_t getTsSeconds;
    uint32_t getTsMillis;
} CneJrttResultCmdType;

typedef struct {
  char bssid[CNE_MAX_BSSID_LEN];
  char uri[CNE_MAX_URI_LEN];
  uint32_t tputKiloBitsPerSec;
  uint32_t timeStampSec;
} CneBQEPostParamsMsgType;

typedef struct {
  cne_feature_type featureId;
  cne_feature_status featureStatus;
} CneFeatureInfoType;

typedef struct {
  cne_feature_type featureId;
  cne_feature_status featureStatus;
  int32_t result;
} CneFeatureRespType;

/**
  Response info structure returned for the event
   CNE_NOTIFY_POLICY_UPDATE_DONE.
 */
typedef struct
{
  cne_policy_type policy;
  /**< policy type andsf or nsrm */
  int32_t result;
  /**< 0 for sucess -1 for failure */
} CnePolicyUpdateRespType;

typedef struct
{
  cne_fam_type family;
} CneWlanFamType;

typedef cne_rat_type CneRatType;
typedef cne_rat_subtype CneRatSubType;

typedef struct {
    struct {
        char upstream_ifname[CNE_IPA_IFACE_NAME_MAX];
        char tethered_ifname[CNE_IPA_IFACE_NAME_MAX];
        int ip_type;
    }ipa_msg;
    bool ioctl_type; // 0 for IPA_IOCTL_NOTIFY_WAN_UPSTREAM_ROUTE_DEL
                     // 1 for IPA_IOCTL_NOTIFY_WAN_UPSTREAM_ROUTE_ADD
}CneTetheringUpstreamInfo;


/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

#ifdef __cplusplus
  }
#endif /* __cplusplus */

#endif /* CNE_DEFS_H */
