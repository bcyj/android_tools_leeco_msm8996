#ifndef CNE_SRM_DEFS_H
#define CNE_SRM_DEFS_H

/**----------------------------------------------------------------------------
  @file CneSrmDefs.h

  This file holds the definations of different types constants specific to the
  SRM module that are used internally by the SPM and SRM modules.

-----------------------------------------------------------------------------*/

/*=============================================================================
               Copyright (c) 2009-2014 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Proprietary and Confidential.
============================================================================*/



/*============================================================================
  EDIT HISTORY FOR MODULE

  $Header: //depot/asic/sandbox/projects/cne/common/core/inc/CneSrmDefs.h#14 $
  $DateTime: 2009/11/24 13:07:50 $
  $Author: mkarasek $
  $Change: 1094989 $

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2009-07-27  ysk  Initial revision.

============================================================================*/


/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "CneDefs.h"
#include <hash_map>
#include <set>

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
//#define CNE_SRM_ResourceTableSize 20
#define CNE_SRM_HISTORY_SIZE 10
#define CNE_SRM_RESOURCE_SIZE 8
#define CNE_SRM_MAX_BSSID_LEN CNE_MAX_BSSID_LEN
#define CNE_SRM_MAX_CAP_LEN 32
#define CNE_SRM_MAX_TIMESTAMP_LEN 32
#define CNE_SRM_ITEM_STATUS_UNKNOWN 65535 /* =0xffff, value to indicate item
                                           * status unknown */
// Battery Status
#define CNE_SRM_BATTERY_STATUS_UNKNOWN  1
#define CNE_SRM_BATTERY_STATUS_CHARGING  2
#define CNE_SRM_BATTERY_STATUS_DISCHARGING  3
#define CNE_SRM_BATTERY_STATUS_NOT_CHARGING  4
#define CNE_SRM_BATTERY_STATUS_FULL  5

// Battery Level
#define CNE_SRM_BATTERY_LEVEL_CLOSE_WARNING  20
#define CNE_SRM_BATTERY_LEVEL_WARNING  15

typedef struct {
     char ssid[CNE_MAX_SSID_LEN];
} CneSsidType;

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
typedef uint32_t CneSrmBatteryLevelType;

/* rat acquisition status call back function */
typedef void (*CneSrmRatAcqStatusCbType)
(
  uint8_t isOpSuccess,
  /* The Info in acqInfo excepting the rat, will be valid only if
   * bring up was successful
   */
  CneRatInfoType& acqInfo,
  void *cbData
);

typedef enum
{
  CNE_SRM_NO_RES = 0,
  CNE_SRM_BATTERY_RES,
  CNE_SRM_WLAN_INTERFACE_RES,
  CNE_SRM_WWAN_INTERFACE_RES,
  CNE_SRM_LAST_CONNECTED_WLAN_RES,
  CNE_SRM_LAST_CONNECTED_WWAN_RES
} CneSrmResourceType;

typedef enum
{
  CNE_SRM_RAT_STATUS_UP,
  CNE_SRM_RAT_STATUS_DOWN
} CneSrmRatStatusType;

typedef enum
{
  CNE_SRM_ICD_RESULT_SUCCESS,
  CNE_SRM_ICD_RESULT_PASS_NOT_STORE,
  CNE_SRM_ICD_RESULT_FAILURE,
  CNE_SRM_ICD_RESULT_TIMEOUT
} CneSrmIcdResultType;

typedef enum
{
	CNE_SRM_JRTT_RESULT_CONTINUE_ESTIMATION,
	CNE_SRM_JRTT_RESULT_STOP_ESTIMATION_REPORT_FAILURE,
	CNE_SRM_JRTT_RESULT_STOP_ESTIMATION_REPORT_PASS
} CneSrmJrttResultType;

enum
{
  CNE_SRM_FLAG_ICD_QUOTA_PRESENT = 0x01,
  CNE_SRM_FLAG_ICD_PROB_PRESENT = 0x02,
  CNE_SRM_FLAG_BQE_QUOTA_PRESENT = 0x04,
  CNE_SRM_FLAG_BQE_PROB_PRESENT = 0x08,
  CNE_SRM_FLAG_MBW_PRESENT = 0x10,
  CNE_SRM_FLAG_TPUT_DL_PRESENT = 0x20,
  CNE_SRM_FLAG_TPUT_SDEV_PRESENT = 0x40
};

typedef struct _Battery {
  int status; /* Charging, Discharging, Full, NotCharging */
  int plugIn;
  int level;
} CneBatteryResourceType;

typedef struct _FwmarkInfo {
    int ratType;
    int netId;
} CneFwmarkInfo;

typedef struct _Wlan {
  int type;
  int status;
  cne_softApStatus_type softApStatus;
  int rssi;
  char ssid[CNE_MAX_SSID_LEN];
  char bssid[CNE_MAX_BSSID_LEN];
  char ipAddr[CNE_MAX_IPADDR_LEN];
  char iface[CNE_MAX_IFACE_NAME_LEN];
  char ipAddrV6[CNE_MAX_IPADDR_LEN];
  char ifaceV6[CNE_MAX_IFACE_NAME_LEN];
  char timeStamp[CNE_SRM_MAX_TIMESTAMP_LEN];
  char dnsInfo[CNE_MAX_DNS_ADDRS][CNE_MAX_IPADDR_LEN];

  _Wlan(){
    for (int i = 0; i < CNE_MAX_DNS_ADDRS; i++)
    {
      memset(dnsInfo[i], 0, CNE_MAX_IPADDR_LEN);
    }
   }

} CneWlanResourceType;


typedef struct _Wwan {
  int type;
  int status;
  int rssi;
  int roaming;
  char ipAddr[CNE_MAX_IPADDR_LEN];
  char iface[CNE_MAX_IFACE_NAME_LEN];
  char ipAddrV6[CNE_MAX_IPADDR_LEN];
  char ifaceV6[CNE_MAX_IFACE_NAME_LEN];
  char timeStamp[CNE_SRM_MAX_TIMESTAMP_LEN];
  char mccMnc[CNE_MAX_MCC_MNC_LEN];
} CneWwanResourceType;

typedef struct _WwanDormancy {
  int status;
} CneWwanDormancyType;

typedef struct _DefaultNetwork {
  int network;
} CneDefaultNetworkType;

typedef struct  {
        int level;
        int frequency;
        char ssid[CNE_MAX_SSID_LEN];
        char bssid[CNE_SRM_MAX_BSSID_LEN];
        char capabilities[CNE_SRM_MAX_CAP_LEN];
} CneWlanScanListResourceType;

typedef struct
{
	uint8_t prob;
	uint32_t quota;
} CneLoadControl_t;

typedef struct
{
	uint32_t dl;
	uint32_t sdev;
} CneTputStats_t;

enum SrmEvent {
  SRM_CNE_CONNECTION_INIT_EVENT,
  SRM_RAT_STATUS_CHANGE_EVENT,
  SRM_BATTERY_STATUS_CHANGE_EVENT, /* CneSrmBatteryEventData *batt_data =
                                    * (CneSrmBatteryEventData*)event_data;
                                    */
  SRM_WLAN_STATUS_CHANGE_EVENT,
  SRM_WWAN_STATUS_CHANGE_EVENT,
  SRM_SQI_CHANGE_EVENT,
  SRM_DORMANCY_STATUS_CHANGE_EVENT,
  SRM_DEFAULT_NETWORK_CHANGE_EVENT,
  SRM_AP_STATUS_CHANGE_EVENT,
  SRM_ICD_RESULT_EVENT,
  SRM_BACKGROUND_STATE_CHANGE_EVENT,
  SRM_SCREEN_STATE_CHANGE_EVENT,
  SRM_WLAN_CONNECTIVITY_UP_EVENT,
  SRM_JRTT_RESULT_EVENT,
  SRM_ICD_HTTP_RESULT_EVENT,
  SRM_ANDSF_DATA_READY,
  SRM_NOTIFY_USER_WQE_DISABLE_EVENT,
  SRM_FEATURE_STATUS_CHANGE_EVENT,
  SRM_IWLAN_USER_PREF_CHANGE_EVENT,
  SRM_NSRM_CONFIG_READY,
  SRM_NOTIFY_USER_NSRM_FEATURE_CHANGE_EVENT,
  SRM_APP_LIST_CHANGE_EVENT,
  SRM_NOTIFY_ATP_GET_PARENT_APP_RESULT,
  SRM_NOTIFY_MCCMNC_CHANGE_EVENT,
  SRM_FAMILY_AVAILABLE_EVENT,
  SRM_NOTIFY_FWMARK_INFO_EVENT,
};

struct CneSrmBackgroundEventData
{
  //Enum to represent various types of background events
  cne_background_event_enum_type bgEvt;
  // describes a particular background event's on or off status.
  bool onoffStatus;
};

struct CneSrmRatEventData
{
  CneRatType rat;
  cne_network_state_enum_type ratStatus;
};

struct CneSrmBatteryEventData {
  CneSrmBatteryLevelType oldLevel;
  CneSrmBatteryLevelType newLevel;
  int status;
  int plugType;
};

enum CneSrmFamType{
  SRM_FAM_MIN = 0,
  SRM_FAM_NONE = SRM_FAM_MIN,
  SRM_FAM_V4,
  SRM_FAM_V6,
  SRM_FAM_V4V6,
  SRM_FAM_MAX = SRM_FAM_V4V6
};

struct CneSrmNetworkStatusEventData {
  cne_softApStatus_type softApStatus;
  cne_network_state_enum_type status;
  char *ssid;
  char *bssid;
  char *iface;
  char *ifaceV6;
  char *mccMnc;
  CneSrmFamType fam; // TODO move to another struct?
};

struct CneSrmSqiChangeEventData
{
  /* Rat Type */
  CneRatType rat;
  /* Sub type for the Rat defined above, see CneSubRatType def */
  CneRatSubType subrat;
  /* RSSI value for the rat sub type */
  int sqi;
  /* IP Address */
  char* ipAddr;
  /* Interface Name */
  char* iface;
  /* IPV6 Address */
  char* ipAddrV6;
  /* V6 Interface Name */
  char* ifaceV6;
  /* Connection Status of the network */
  cne_network_state_enum_type status;
  /* BSSID */
  char* bssid;
};

struct CneSrmDormancyChangeEventData
{
  /* Dormancy */
  cne_rat_type interface;
  int isDormant;
};

struct CneSrmDefaultNetworkChangeEventData
{
  cne_rat_type network;
};

struct CneSrmIcdResultEventData
{
  CneSrmIcdResultType result;
  char *bssid;
  uint8_t flags;
  uint32_t tid;
  CneLoadControl_t icdLoad;
  CneLoadControl_t bqeLoad;
	uint32_t mbw;
	CneTputStats_t tput;
};

struct CneSrmIcdHttpResultEventData
{
  CneSrmIcdResultType result;
  char *bssid;
  uint32_t tid;
  int family; // 1 - v4. 2 - v6
};

struct CneSrmJrttResultEventData
{
  CneSrmJrttResultType result;
  uint32_t  jrttMillis;
  uint32_t  getTsSeconds;
  uint32_t  getTsMillis;
};


// Available Cne feature names and the possible
// status a feature can have
// NOTE: Must match with type in ICneObserverDefs.h
enum CneSrmFeatureNameType
{
   CNE_SRM_FEATURE_WQE = 0,
   CNE_SRM_FEATURE_MIN = CNE_SRM_FEATURE_WQE,
   CNE_SRM_FEATURE_NSRM,
   CNE_SRM_FEATURE_MAX = CNE_SRM_FEATURE_NSRM
};

// NOTE: Must match with type in ICneObserverDefs.h
enum CneSrmFeatureStatusType
{
   CNE_SRM_FEATURE_STATUS_INACTIVE = 0,
   CNE_SRM_FEATURE_STATUS_MIN = CNE_SRM_FEATURE_STATUS_INACTIVE,
   CNE_SRM_FEATURE_STATUS_ACTIVE,
   CNE_SRM_FEATURE_STATUS_MAX = CNE_SRM_FEATURE_STATUS_ACTIVE
};

// Available ui preferences for ePDG/IWLAN feature
// NOTE: Must match with type in ICneObserverDefs.h
enum CneSrmIwlanUserPrefValueType
{
   CNE_SRM_IWLAN_PREF_UNSET = 0,
   CNE_SRM_IWLAN_PREF_MIN = CNE_SRM_IWLAN_PREF_UNSET,
   CNE_SRM_IWLAN_PREF_DISABLED,
   CNE_SRM_IWLAN_PREF_ENABLED,
   CNE_SRM_IWLAN_PREF_MAX = CNE_SRM_IWLAN_PREF_ENABLED
};


// Srm clients receive the following payload when feature status
// updated by end user via user option
struct CneSrmFeatureStatusChangeEventData
{
   CneSrmFeatureNameType name;
   CneSrmFeatureStatusType status;
};

// Srm clients receive the following payload when iwlan pref
// updated by end user via user option
struct CneSrmIwlanUserPrefChangeEventData
{
   CneSrmIwlanUserPrefValueType pref;
};

//
// hash_map to store uid and appname information about applications.
// N.B. cannot use std, since hash_multimap is not part of std
//
using namespace std;
/* struct holding the appinfo like UID and the 32 bit app signature hashes */
typedef struct
{
  set<uint32_t> hashes;
  uint32_t  uid;
} AppInfoType;


typedef struct {
  char* pkg_name;
  set<uint32_t> hashes;
} CneSrmAppInfoDataType;

// key = uid, data = CneSrmAppInfoDataType
typedef hash_multimap<uint32_t, CneSrmAppInfoDataType> CneAppInfoMultiMapType;

#endif /* CNE_SRM_DEFS_H */
