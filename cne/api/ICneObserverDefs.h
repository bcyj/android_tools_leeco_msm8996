#ifndef _ICNE_OBSERVER_DEFS_H_
#define _ICNE_OBSERVER_DEFS_H_
/**
 * @file ICneObserverDefs.h
 *
 * @brief
 * Connectivity Engine Observer CO Definitions Header file
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Connectivity Engine Observer.
 *
 *
 *                   Copyright 2010-2012, 2013 Qualcomm Technologies, Inc.
 *                          All Rights Reserved.
 *                       Qualcomm Technologies Proprietary/GTDR
 */

#include <netinet/in.h>
#include <vector>
#include <time.h>

#ifndef IFHWADDRLEN
#define IFHWADDRLEN 6
#endif

#ifndef MAXSSIDLEN
#define MAXSSIDLEN 32
#endif

#ifndef MAXDNSADDRS
#define MAXDNSADDRS 2
#endif

/**
 * @addtogroup coi_ds
 * @{
 * Various Error codes returned by Connectivity Engine Observers APIs
 */
typedef enum
{
  CO_RET_SUCCESS             =  0, /**< -- Success */
  CO_RET_FAILED              = -1, /**< -- Generic error */
  CO_RET_SERVICE_UNAVAILABLE = -2, /**< -- CNO service unavailable */
  CO_RET_NOT_ALLOWED         = -3, /**< -- Operation not allowed */
  CO_RET_NET_INACTIVE        = -4, /**< -- Network is inactive */
  CO_RET_INVALID_ARGS        = -5, /**< -- Invalid argument */
  CO_RET_PERM_DENIED         = -6, /**< -- Permission denied */
  CO_RET_NO_NOTIFIER         = -7, /**< -- No notifier is registered */
}
CORetType;
/**
 * @} end group coi_ds
 */

/**
 * @addtogroup cbo_ds
 * @{
 */
/**
 * Possible Connectivity Engine Battery Observer Signal types
 */
typedef enum
{
  SIGNAL_BATTERY_ALL = 0,                      /**< -- All Signals */
  SIGNAL_BATTERY_MIN = SIGNAL_BATTERY_ALL,
  SIGNAL_BATTERY_STATE_CHANGE, /**< -- onBatteryStateChange signal */
  SIGNAL_BATTERY_MAX = SIGNAL_BATTERY_STATE_CHANGE
} CboSignalType;

/**
 * Possible Connectivity Engine Battery Observer charger types
 */
typedef enum
{
  CHARGER_TYPE_NONE = 0, /**< -- No charger */
  CHARGER_TYPE_WALL = 1, /**< -- Wall charger */
  CHARGER_TYPE_USB  = 2, /**< -- USB charger */
} CboChargerType;

/**
 * @brief
 * Connectivity Engine Battery Observer Battery Status Structure
 */
typedef struct
{
  bool isCharging;              /**< -- true if charging, false if not */
  CboChargerType charger;       /**< -- charger type */
  unsigned int remainingCharge; /**< -- percentage remaining charge */
} CboBatteryStatusType;
/**
 * @} end group cbo_ds
 */

/**
 * @addtogroup cno_ds
 * @{
 */

/**
 * Possible Connectivity Engine Network Observer Signal types
 */
typedef enum
{
  SIGNAL_NET_ALL = 0,                   /**< -- All Signals */
  SIGNAL_NET_MIN = SIGNAL_NET_ALL,
  SIGNAL_NET_DORMANCY_CHANGE,           /**< -- onDormancyStateChange signal */
  SIGNAL_NET_CONFIG_CHANGE,             /**< -- onNetConfigChange signal */
  SIGNAL_BITRATE_SERVICE_CHANGE,        /**< -- onBitrateServiceChange signal */
  SIGNAL_BITRATE_ESTIMATE_AVAILABLE,    /**< -- onBitrateEstimateAvailable signal */
  SIGNAL_LATENCY_RAW_VALUES_AVAILABLE,  /**< -- onLatencyRawValuesAvailable signal */
  SIGNAL_LATENCY_ESTIMATE_AVAILABLE,    /**< -- onLatencyEstimateAvailable signal */
  SIGNAL_LATENCY_SERVICE_CHANGE,        /**< -- onLatencyServiceChange signal */
  SIGNAL_NET_MAX = SIGNAL_LATENCY_SERVICE_CHANGE
} CnoSignalType;

/**
 * Possible Connectivity Engine Network Observer network types
 */
typedef enum
{
  NET_WWAN = 0, /**< -- RAT type WWAN */
  NET_WLAN,     /**< -- RAT type WLAN */
  NET_ANY,
  NET_NONE,
  NET_UNKNOWN
} CnoNetType;

/**
 * Possible Connectivity Engine Network Observer network subtypes
 */
typedef enum
{
  NET_SUBTYPE_UNKNOWN = 0, /**< -- Unknown */
  NET_SUBTYPE_GPRS,        /**< -- Sub type GPRS */
  NET_SUBTYPE_EDGE,        /**< -- Sub type EDGE */
  NET_SUBTYPE_UMTS,        /**< -- Sub type UMTS */
  NET_SUBTYPE_CDMA,        /**< -- Sub type CDMA IS-95 */
  NET_SUBTYPE_EVDO_0,      /**< -- Sub type EVDO Rev 0 */
  NET_SUBTYPE_EVDO_A,      /**< -- Sub type EVDO Rev A */
  NET_SUBTYPE_1xRTT,       /**< -- Sub type 1x RTT */
  NET_SUBTYPE_HSDPA,       /**< -- Sub type HSDPA */
  NET_SUBTYPE_HSUPA,       /**< -- Sub type HSUPA */
  NET_SUBTYPE_HSPA,        /**< -- Sub type HSPA */
  NET_SUBTYPE_IDEN,        /**< -- Sub type IDEN */
  NET_SUBTYPE_EVDO_B,      /**< -- Sub type EVDO Rev B */
  NET_SUBTYPE_LTE,         /**< -- Sub type LTE */
  NET_SUBTYPE_HSPAP,       /**< -- Sub type HSPAP */
  NET_SUBTYPE_WLAN_B = 20, /**< -- Sub type 802.11 B */
  NET_SUBTYPE_WLAN_G       /**< -- Sub type 802.11 G */
} CnoNetSubtype;

/**
 * Connectivity Engine Network Observer confidence values
 */
typedef enum
{
  CONF_LOW = 0, /**< -- Low confidence */
  CONF_MED,     /**< -- Medium confidence */
  CONF_HIGH,    /**< -- High confidence */
} CnoConfType;


/**
 * Connectivity Engine Network Observer network subtype structure
 */
typedef struct
{
  CnoNetSubtype fl; /**< fwd link subtype */
  CnoNetSubtype rl; /**< rev link subtype */
} CnoNetSubTypeStruct;

/**
 * Connectivity Engine Network Observer network type struct
 */
typedef struct
{
  CnoNetType type;          /**< -- network type */
  CnoNetSubTypeStruct subtype; /**< -- network subtype */
} CnoNetTypeStruct;

/**
 * Connectivity Engine Network Observer Network Dormancy Status request
 * response type
 */
typedef struct
{
  CnoNetTypeStruct net;       /**< -- network type & sub type */
  bool isDormant;          /**< -- true if dormant, othewise false */
  bool isDefault;          /**< -- true if network is default, false if not*/
} CnoDormancyStatusType;

/**
 * Connectivity Engine Network Observer Network Configuration request response
 * type structure
 */
typedef struct
{
  CnoNetTypeStruct net;       /**< -- network type & subtype */
  in_addr ip4;             /**< -- network ipv4 address */
  in6_addr ip6;            /**< -- network ipv6 address */
  unsigned int mtu;        /**< -- mtu size in bytes */
  bool isDefault;          /**< -- true if network is default, false if not*/
} CnoNetConfigType;

/**
 * Connectivity Engine Network Observer Any Service response type
 * type structure
 */
typedef struct {
  int status;             /**< -- current state of the service */
  int error;              /**< -- current error with service */
} CnoServiceResponseType;

/**
 * Connectivity Engine Network Observer Any Service Status Indication type
 */
typedef enum
{
  SERVICE_STARTED,  /**< -- service started */
  SERVICE_STOPPED,  /**< -- service stopped */
} CnoServiceStatusType;

/**
 * Connectivity Engine Network Observer Any Service Error Indication type
 */
typedef enum
{
  CNO_SVC_NO_ERROR,           /**< -- No error detected */
  CNO_SVC_NET_INACTIVE,       /**< -- No network available */
  CNO_SVC_NET_NOT_SUPPORTED,  /**< -- Network not supported */
  CNO_SVC_BUSY,               /**< -- Service is already running */
  CNO_SVC_DISABLED,           /**< -- Service is disabled */
  CNO_SVC_GENERIC_ERR,        /**< -- Error while trying to start a service */
} CnoServiceErrorType;

/**
 * Connectivity Engine Network Observer Historical Latency Type
 */
typedef struct
{
  int latency;          /**< -- latency to destination in milli seconds */
  timespec timestamp;   /**< -- time at which latency was caculated */
} CnoHistoricalLatencyType;

/**
 * Connectivity Engine Network Observer Latency Record Type.
 * Contains information of latency for a socket to the specified destination.
 */
typedef struct
{
  in_addr localIp4Addr;     /**< -- local ipv4 address of the socket */
  in_addr remoteIp4Addr;    /**< -- destination host ipv4 address of the socket */
  int port;                 /**< -- local port of the socket */
  int rtt;                  /**< -- round trip time for socket in milli seconds */
} CnoLatencyRecordType;

/**
 * Connectivity Engine Network Observer Latency Estimation request response type
 */
typedef struct
{
  CnoNetTypeStruct net;
    /**< -- network type & subtype */
  CnoHistoricalLatencyType histDstLatency;
    /**< -- historical latency estimate towards specified  destination */
  CnoHistoricalLatencyType histIfaceLatency;
    /**< -- historical latency estimate of the interface */
} CnoLatencyEstimateType;

/**
 * Connectivity Engine Network Observer Latency Measurement
 * request response type
 */
typedef std::vector<CnoLatencyRecordType> CnoLatencyMeasurementsType;
/**< -- latency records to a given destination */

/**
 * Connectivity Engine Network Observer Bitrate estimation request response type
 * Contains bitrate estimates for the network. Specifically longterm average
 * since start was called, max bitrate since start was called and instantaneous
 * bitrate estimate.
 */
typedef struct
{
  CnoNetTypeStruct net; /**< -- network type & subtype */
  int longTerm; /**< -- bitrate estimate since start was called */
  int max;  /**< -- Max bit rate since start was called */
  int instantaneous;    /**< -- instantaneous bit rate estimate */
} CnoBitrateEstimateType;

/**
* Supplemental wlan network configuration information.
*/
typedef struct
{
  char bssid[IFHWADDRLEN]; /**< -- mac address of access point */
  char ssid[MAXSSIDLEN]; /**< -- SSID of access point */
  in_addr ip4dns[MAXDNSADDRS]; /**< -- IPv4 addresses of DNS server */
  in6_addr ip6dns[MAXDNSADDRS]; /**< -- IPv6 addresses of DNS server */
} CnoWlanSuppNetConfigType;


/**
 * @} end group cno_ds
 */

/**
 * @addtogroup cfo_ds
 * @{
 */

/**
 * Possible Connectivity Engine Feature Observer Signal types
 */
typedef enum
{
  SIGNAL_FEATURE_ALL = 0,                   /**< -- All Signals */
  SIGNAL_FEATURE_MIN = SIGNAL_FEATURE_ALL,
  SIGNAL_IWLAN_USER_PREF_CHANGE,        /**< -- OnIwlanUserPrefChange signal */
  SIGNAL_FEATURE_STATUS_CHANGE,         /**< -- OnFeatureStatusChange signal */
  SIGNAL_FEATURE_MAX = SIGNAL_FEATURE_STATUS_CHANGE
} CfoSignalType;


/**
* Connectivity Engine Feature Observer feature status type
*/
typedef enum
{
  CFO_STATUS_INACTIVE = 0,
  CFO_STATUS_MIN = CFO_STATUS_INACTIVE,
  CFO_STATUS_ACTIVE,
  CFO_STATUS_MAX = CFO_STATUS_ACTIVE,
} CfoFeatureStatusType;

/**
* Connectivity Engine feature name type
*/
typedef enum
{
  CFO_FEATURE_WQE = 0, /* Wifi Quality Estimation */
  CFO_FEATURE_MIN = CFO_FEATURE_WQE,
  CFO_FEATURE_MAX = CFO_FEATURE_WQE
} CfoFeatureNameType;

/**
* Connectivity Engine Network Observer
* Inter-working WLAN (IWLAN) user preference type
*/
typedef enum
{
  CFO_IWLAN_PREF_UNSET = 0,
  CFO_IWLAN_PREF_MIN = CFO_IWLAN_PREF_UNSET,
  CFO_IWLAN_PREF_DISABLED,
  CFO_IWLAN_PREF_ENABLED,
  CFO_IWLAN_PREF_MAX = CFO_IWLAN_PREF_ENABLED
} CfoIwlanUserPrefType;

/**
* Connectivity Engine Network Observer
* IWLAN user preference acknowledgement type
*
* Indicates whether setting user preference
* was successful
*/
typedef enum
{
  CFO_IWLAN_ACK_FAIL = 0,
  CFO_IWLAN_ACK_MIN = CFO_IWLAN_ACK_FAIL,
  CFO_IWLAN_ACK_SUCCESS,
  CFO_IWLAN_ACK_MAX = CFO_IWLAN_ACK_SUCCESS
} CfoIwlanUserPrefAckType;

/**
 * @} end group cfo_ds
 */

#endif /* _ICNE_OBSERVER_DEFS_H_ */
