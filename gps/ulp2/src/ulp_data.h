/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                  LIBULP Data Variables File

GENERAL DESCRIPTION
  This file contains the data structure and variables used for ulp module to
  make decision, eg: the unique criterias, phone context info and other engine
  start/stop state.

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
=============================================================================*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/

#ifndef ULP_DATA_H
#define ULP_DATA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <ctype.h>
#include <stdbool.h>
#include <ulp_quipc.h>
#include <LocUlpProxy.h>

/*=============================================================================================
                                  DATA module
 =============================================================================================*/
#define MAX_NUM_UNIQUE_CRITERIA       16 // Maximum number of unique criteria

#define GNSS_TH_MAX   5
#define GNSS_TH_MIN  -5
#define QUIPC_TH_MAX  5
#define QUIPC_TH_MIN -5
#define GNP_TH_MAX    1
#define GNP_TH_MIN   -1
#define ZPP_TH_MAX    1
#define ZPP_TH_MIN   -1

// Constant defined used by ULP modules
#define ULP_HIGH_ACCURACY_MIN_TRACKING_INTERVAL_MSEC      1000         // 1 second
#define ULP_LOW_ACCURACY_MIN_TRACKING_INTERVAL_MSEC       (5 * 1000)   // 5 seconds
#define ULP_SINGLE_SHOT_MIN_TRACKING_INTERVAL_MSEC        (590 * 60 * 60 * 1000) // 590 hours

// QUIPC provider related variables
#define ULP_QUIPC_DEFAULT_TRACKING_INTERVAL_MSEC          1000         // 1 second rate to run QUIPC
#define ULP_QUIPC_NO_FINAL_POS_REPORT_THRESHOLD_MSEC      5000         // 5000 milli-seconds
#define ULP_QUIPC_RESTART_TIME_THRESHOLD_MSEC             (60*1000)    // 60 seconds (limit QUIPC restart to be less frequent than 30 seconds per restart)
#define ULP_QUIPC_INITIALIZING_THRESHOLD_MSEC             (60*1000)    // 60 seconds
#define ULP_QUIPC_LCI_TRANSITION_THRESHOLD_MSEC           (60*1000)    // 60 seconds

// GNSS provider related variables
#define ULP_GPS_PRN_FIRST                             1           // First GPS PRN is 1
#define ULP_GPS_PRN_LAST                              32          // Last GPS PRN is 32
#define ULP_GLONASS_PRN_FIRST                         65          // First Glonass PRN is 65
#define ULP_GLONASS_PRN_LAST                          88          // Last Glonass PRN is 88
#define ULP_STRONG_SV_CNT_TH                          3           // 3 SVs
#define ULP_STRONG_SV_SNR_TH                          30          // 30 snr
#define ULP_GNSS_HIGH_SPEED_TH                        20          // 20 meters/second
#define ULP_GNSS_STRONG_HIGH_STATE_ACCURACY_TH        10          // GNSS is of strong high
#define ULP_GNSS_HIGH_STATE_ACCURACY_TH               15          // GNSS is of high state if its position accuracy is less than 15 meters
#define ULP_GNSS_MEDIUM_STATE_ACCURACY_TH             30          // GNSS is of medium state if its position accuracy is between 15 and 30 meters
                                                                  // and accuracy to be less than 10 meters
#define ULP_GNSS_RECONFIGURE_MODE_THRESHOLD_MSEC      3000        // 3 seconds
#define ULP_GNSS_NO_FINAL_POS_REPORT_THRESHOLD_MSEC   2000        // 2000 milli-seconds
#define ULP_GNSS_RESTART_TIME_THRESHOLD_MSEC          (60*1000)   // 60 seconds
#define ULP_GNSS_INITIALIZING_THRESHOLD_MSEC          (260*1000)  // 260 seconds, GNSS defualt timeout is 255 seconds
                                                                  // we want this timeout value to be a bit longer than 255 seconds

// ULP coarse position related
#define ULP_QUIPC_COARSE_POSITION_UNCERTAINTY_THRESHOLD 1000      // 1000 meters
#define ULP_POS_HOR_UNCERTAINTY_GROW_RATE_FIRST_MIN   10          // 10 meters/second for first minute
#define ULP_POS_HOR_UNCERTAINTY_GROW_RATE_AFTER       30          // 30 meters/second afterwards

// ULP monitor thread related
#define ULP_MONITOR_THREAD_POLL_INTERVAL              3           // 3 seconds

// ZPP Provider related variables
#define ULP_ZPP_MIN_TRIGGER_INTERVAL                 10000        // 10 seconds

// Other misc. constants
#define LCI_ID_STRING_LENGTH                         (32+4)       // formatted LCI ID string length

// ULP engine provider name
#define GPS_PROVIDER_NAME                             "GPS"
#define PIP_PROVIDER_NAME                             "PIP"

// In this implementation, UlpLocationCriteria will be saved in an array. Each array element
// will contain the unique ulpLocationCriteria plus one bool field to indicate whether this
// array element is valid or not. Once the ulpLocationCriteria is being removed, the bool filed
// "isUsed" will be set to false.
typedef struct
{
   bool                isUsed;
   // This criteria has been evaulated in the past or no
   bool                isNew;
   int                 refCnt;
   UlpLocationCriteria locationCriteria;
} UlpCriteria;

// GNP state
typedef enum
{
   GNP_STATE_IDLE      = 0,
   GNP_STATE_ACTIVE    = 1,
   GNP_STATE_MAX_VALUE = 0x10000000 // force enum to be 32 bit
} gnp_state_e_type;

// GNP state
typedef enum
{
   ZPP_STATE_IDLE      = 0,
   ZPP_STATE_ACTIVE    = 1,
   ZPP_STATE_MAX_VALUE = 0x10000000 // force enum to be 32 bit
} zpp_state_e_type;

// GNSS state
typedef enum
{
   GNSS_STATE_IDLE            = 0,
   GNSS_STATE_INITIALIZING    = 1,
   GNSS_STATE_HIGH            = 2,
   GNSS_STATE_MEDIUM          = 3,
   GNSS_STATE_LOW             = 4,
   GNSS_STATE_FAILED          = 5,
   GNSS_STATE_MAX_VALUE       = 0x10000000 // force enum to be 32 bit
} gnss_state_e_type;

// QUIPC state
typedef enum
{
   QUIPC_STATE_IDLE            = 0,
   QUIPC_STATE_INITIALIZING    = 1,
   QUIPC_STATE_HIGH            = 2,
   QUIPC_STATE_LOW             = 3,
   QUIPC_STATE_FAILED          = 4,
   QUIPC_STATE_OUT_OF_COVERAGE = 5,
   QUIPC_STATE_LCI_TRANSITION  = 6,
   QUIPC_STATE_MAX_VALUE = 0x10000000 // force enum to be 32 bit
} quipc_state_e_type;

// Data structure that used to save GNSS provided related info, eg:
// GNSS state and etc
typedef struct
{
   bool               enabled;         // GNSS is enabled or not, via UI setting
   gnss_state_e_type  state;           // GNSS current state
   gnss_state_e_type  last_state;      // Previous GNSS state, if the current state is idle
   bool               selected_to_run; // This flag will be set to true when GPS provider
                                       // can be invoked to satisfy the requests. This flag
                                       // is and is only determined by fix requests.
                                       // Whehter this provider will be run or not
   int                onoff_cnt;       // When this count reaches certain threshold (GNSS_TH_MAX),
                                       // GNSS will be turned ON. When this count reaches certain
                                       // threshold (GNSS_TH_MIN), GNSS will be turned OFF.
                                       // When this count is in between, GNSS will remain in its
                                       // current ON/OFF state.
   GpsPositionRecurrence recurrence_type;
   uint32_t              fix_interval;
   GpsPositionMode       position_mode;
   bool                  new_request_active; // New request has been processed or not (only needed
                                             // for provider that can support both single shot and tracking)
   bool                  first_fix_pending;  // ULP is waiting for first fix report from GNSS engine since
                                             // ULP starts the GNSS engine
   bool                  tbf_update_pending;
   bool                  high_speed;         // Set to true if speed is more than 20m/s
   uint32_t              strong_sv_cnt;      // GPS and Glonass with SNR >= 30
   uint64_t              last_started_time_ms; // Record down the timestamp when QUIPC is last started
   uint64_t              last_stopped_time_ms; // Record down the timestamp when QUIPC is last stopped
   UlpLocation           last_position_received;
   uint64_t              last_position_received_time_ms;
} gnss_provider_info_s_type;

// Data structure that used to save QUIPC provided related info, eg:
// QUIPC state and etc
typedef struct
{
   bool               enabled;         // QUIPC is enabled or no, via UI setting and gps.conf
   quipc_state_e_type state;           // QUIPC current state
   quipc_state_e_type last_state;      // QUIPC last state
   bool               selected_to_run; // This flag will be set to true when QUIPC provider
                                       // can be invoked to satisfy the requests. This flag
                                       // is and is only determined by fix requests.
   int                onoff_cnt;       // When this count reaches certain threshold (QUIPC_TH_MAX),
                                       // QUIPC will be turned ON. When this count reaches certain
                                       // threshold (QUIPC_TH_MIN), QUIPC will be turned OFF.
                                       // When this count is in between, QUIPC will remain in its
                                       // current ON/OFF state.

   bool new_request_active;            // New request has been processed or not
   uint64_t last_started_time_ms;      // Record down the timestamp when QUIPC is last started
   uint64_t last_stopped_time_ms;      // Record down the timestamp when QUIPC is last stopped
   UlpLocation last_position_received;
   uint64_t last_position_received_time_ms;
   uint64_t last_lci_transition_time_ms;
                                       // Record last time LCI transition event received
} quipc_provider_info_s_type;

// Data structure that used to save GNP provided related info, eg:
// state and etc
typedef struct
{
   bool                   enabled;         // GNP is enabled or no: via UI setting
   gnp_state_e_type       state;           // GNP state
   gnp_state_e_type       last_state;      // Last saved state
   bool                   selected_to_run; // This flag will be set to true when GNP provider
                                           // can be invoked to satisfy the requests. This flag
                                           // is and is only determined by fix requests.
   int                    onoff_cnt;       // When this count reaches certain threshold (GNP_TH_MAX),
                                           // GNP will be turned ON. When this count reaches certain
                                           // threshold (GNP_TH_MIN), GNP will be turned OFF.
                                           // When this count is in between, GNP will remain in its
                                           // current ON/OFF state.
   UlpRecurrenceCriteria  recurrence_type;
   uint32_t               fix_interval;

   bool                   new_request_active; // New request has been processed or not
   bool                   first_fix_pending;  // ULP is waiting for first fix report from GNP engine since
                                              // ULP starts the GNP engine
   bool                   coarse_pos_req_pending; // GNP module is being requested for a coarse position
   uint64_t               last_started_time_ms; // Record down the timestamp when GNP is last started
   uint64_t               last_stopped_time_ms; // Record down the timestamp when GNP is last stopped
   UlpLocation            last_position_received;
   uint64_t               last_position_received_time_ms;
} gnp_provider_info_s_type;

// Data structure that used to save ZPP provided related info, eg:
// state and etc
typedef struct
{
   bool                   enabled;         // ZPP is enabled or no: via UI setting
   zpp_state_e_type       state;           // ZPP state
   bool                   selected_to_run; // This flag will be set to true when ZPP provider
                                           // can be invoked to satisfy the requests. This flag
                                           // is and is only determined by fix requests.
   int                    onoff_cnt;       // When this count reaches certain threshold (ZPP_TH_MAX),
                                           // ZPP will be turned ON. When this count reaches certain
                                           // threshold (ZPP_TH_MIN), ZPP will be turned OFF(when tracking supported)
                                           // When this count is in between, ZPP will remain in its
                                           // current ON/OFF state.
   UlpRecurrenceCriteria  recurrence_type;
   uint32_t               fix_interval;

   bool                   first_fix_pending;  // ULP is waiting for first fix report from ZPP engine since
                                              // ULP started the ZPP engine
   UlpLocation            last_position_received;
   uint64_t               last_position_received_time_ms;
   uint32_t               zpp_trigger_threshold;
   pthread_t               thread;            /* ZPP periodic lookup thread */
   bool                    periodic_session_active; /*ZPP periodic session active*/
   pthread_cond_t          tCond;
   pthread_mutex_t         tLock;
} zpp_provider_info_s_type;

typedef enum
{
   ULP_POSITION_TYPE_UNKNOWN = 0,
   ULP_POSITION_TYPE_GNSS    = 1,
   ULP_POSITION_TYPE_QUIPC   = 2,
   ULP_POSITION_TYPE_GNP     = 3,
   ULP_POSITION_TYPE_ZPP     = 4,
   ULP_POSITION_TYPE_MAX_VALUE  = 0x10000000
} ulp_position_e_type;

typedef enum
{
   ULP_DEBUG_INFO_NONE           = 0,
   ULP_DEBUG_INFO_ENGINE_MODE    = 1,
   ULP_DEBUG_INFO_MEASUREMENT    = 2,
   ULP_DEBUG_INFO_ALL            = 3,
   ULP_DEBUG_INFO_MAX_VALUE      = 0x10000000
} ulp_debug_info_e_type;

// This data structure holds variables needed in order
// to dynamically select between GNSS and QUIPC position.
typedef struct
{
   bool                first_fix_pending;         // Whether first fix for a tracking session is pending or no
   ulp_position_e_type last_report_position_type; // Whether last reported position is GNSS or QUIPC
   uint64_t            last_position_report_time; // Time when last position is reported from libulp to GPS HAL layer
} ulp_position_selection_info_s_type;

typedef struct
{
   LocUlpProxy*         loc_proxy;
   pthread_mutex_t      monitor_mutex;
   pthread_cond_t       monitor_cond;
   bool                 run_monitor_thread;

   // ULP criteria
   UlpCriteria             locationCriteriaArray[MAX_NUM_UNIQUE_CRITERIA];
   bool                    gps_provider_request_active;
   bool                    high_accuracy_request_active;
   bool                    low_accuracy_request_active;
   bool                    zpp_request_active;
   UlpRecurrenceCriteria   recurrence_type;
   uint32_t                fix_interval;
   uint32_t                gnss_interval_cache;
   UlpRecurrenceCriteria   gnss_recurrence_type_cache;

   // Data related to phone context info
   // Context_type in phoneSetting will be used to indicate what phone settings are valid
   bool                    phoneSettingRequested;
   UlpPhoneContextSettings phoneSetting;

   // ULP has received start request or not
   bool                    ulp_started;

   // ULP should run selection provider logic or no
   bool                    run_provider_selection_logic;

   //System event that was received
   UlpSystemEvent          system_event;
   uint64_t                system_event_arrival_time;

   // Provider info
   gnss_provider_info_s_type  gnss_provider_info;
   quipc_provider_info_s_type quipc_provider_info;
   gnp_provider_info_s_type   gnp_provider_info;
   zpp_provider_info_s_type   zpp_provider_info;

   // Position selection logic
   ulp_position_selection_info_s_type position_selection_info;

   // QUIPC related info
   // This is to be replaced with the configuration item in quipc_conf
   int                        quipc_enabled;

   // GTP WiFi enabled or disabled
   bool                       gtp_wifi_enabled;

   // GTP AP Cell enabled or disabled
   bool                       gtp_ap_cell_enabled;

   // Debug info state
   ulp_debug_info_e_type      ulp_debug_info_type;

   //ULP Call Back Fn pointers
   ulp_location_callback          ulp_loc_cb;
   ulp_create_thread              ulp_create_thread_cb;
   ulp_network_location_request   ulp_network_callback;
   ulp_request_phone_context      ulp_phone_context_req_cb;
} ulp_data_s_type;

extern ulp_data_s_type ulp_data;
extern const ulpQuipcInterface* ulp_quipc_inf;

#ifdef __cplusplus
}
#endif

#endif /* ULP_DATA_H */
